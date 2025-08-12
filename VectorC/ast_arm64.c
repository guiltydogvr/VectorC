//
//  ast_arm64.c
//  VectorC
//
//  Created by Claire Rogers on 05/01/2025.
//

#include "ast_arm64.h"
#include "stb_ds.h"
#include "tacky.h"
#include <assert.h>
#include <stdio.h>
#include <stdbool.h>

// Convert an operand to a string for ARM64 (helper function)
const char* getARM64Operand(const Operand* op, char* buffer, size_t bufferSize) {
	switch (op->type) {
		case OPERAND_IMM:
			snprintf(buffer, bufferSize, "#%d", op->immValue);
			break;
		case OPERAND_VARNAME:
			snprintf(buffer, bufferSize, "%s", op->varName);
			break;
		case OPERAND_STACK_SLOT:
			snprintf(buffer, bufferSize, "[fp, %d]", op->stackOffset);
			break;
		case OPERAND_REGISTER:
			snprintf(buffer, bufferSize, "%s", op->regName);
			break;
	}
	return buffer;
}

// --------------------------------------------------
// Local helper to track tmp -> stack offsets
// --------------------------------------------------

static TmpMapping* s_tmpMappings = NULL;

static int s_nextOffset = -16; // global or passed in

int getOrAssignStackOffsetARM64(const char* tmpName) {
	for (int i = 0; i < arrlenu(s_tmpMappings); i++) {
		if (strcmp(s_tmpMappings[i].tmpName, tmpName) == 0) {
			return s_tmpMappings[i].stackOffset;
		}
	}
	// New tmp — assign a new slot
	arrput(s_tmpMappings, ((TmpMapping){
		.tmpName = strdup(tmpName),
		.stackOffset = s_nextOffset
	}));
	int assigned = s_nextOffset;
	s_nextOffset -= 16; // Move down the stack
	return assigned;
}


const char* getARM64InstructionName(ARM64InstructionType type)
{
	static const char* s_instructionNames[] = {
		[ARM64_ADD] = "add",
		[ARM64_AND] = "and",
		[ARM64_EOR] = "eor",
		[ARM64_MUL] = "mul",
		[ARM64_SDIV] = "sdiv",
		[ARM64_LDR] = "ldr",
		[ARM64_MOV] = "mov",
		[ARM64_MVN] = "mvn",
		[ARM64_NEG] = "neg",
		[ARM64_ORR] = "orr",
		[ARM64_RET] = "ret",
		[ARM64_STR] = "str",
		[ARM64_SUB] = "sub",
		// Shifts (immediate)
		[ARM64_LSL] = "lsl",   // lsl wd, wn, #imm
		[ARM64_ASR] = "asr",   // asr wd, wn, #imm
		// Shifts (variable)
		[ARM64_LSLV] = "lslv",  // lslv wd, wn, wm
		[ARM64_ASRV] = "asrv",  // asrv wd, wn, wm

	};

	static_assert(sizeof(s_instructionNames) / sizeof(const char*) == (int32_t)ARM64_RET+1, "Invalid Instruction");
	return s_instructionNames[type];
}

//---------------------------------------------------------
// ARM64 CODEGEN
//---------------------------------------------------------
void generateARM64Function(FILE* outputFile, const Function* func)
{
	// Decide function label
	if (strcmp(func->name, "main") == 0) {
		fprintf(outputFile, ".global _main\n");
		fprintf(outputFile, "_main:\n");
	} else {
		fprintf(outputFile, ".global %s\n", func->name);
		fprintf(outputFile, "%s:\n", func->name);
	}

	int bytesToAllocate = alignTo(-s_nextOffset, 16);

	// ARM64 prologue
	// Typically: Save x29 (frame pointer) and x30 (link register)
	fprintf(outputFile, "    stp x29, x30, [sp, -16]!\n");
	fprintf(outputFile, "    mov x29, sp\n");
	// Reserve local stack space if needed:
	fprintf(outputFile, "    sub sp, sp, #%d\n", bytesToAllocate);

	// Emit instructions
	const ARM64Instruction* instructions = (const ARM64Instruction*)func->instructions;
	for (size_t j = 0; j < func->instructionCount; j++) {
		const ARM64Instruction* instr = &instructions[j];

		char srcBuffer[32], src1Buffer[32], dstBuffer[32];
		getARM64Operand(&instr->src, srcBuffer, sizeof(srcBuffer));
		getARM64Operand(&instr->src1, src1Buffer, sizeof(src1Buffer));
		getARM64Operand(&instr->dst, dstBuffer, sizeof(dstBuffer));
		const char* instructionName = getARM64InstructionName(instr->type);
		switch (instr->type) {
			case ARM64_ADD:
			case ARM64_AND:
			case ARM64_EOR:
			case ARM64_SDIV:
			case ARM64_MUL:
			case ARM64_ORR:
			case ARM64_SUB:
			case ARM64_LSL:
			case ARM64_LSR:
			case ARM64_ASR:
			case ARM64_LSLV:
			case ARM64_LSRV:
			case ARM64_ASRV:
				fprintf(outputFile, "    %s %s, %s, %s\n", instructionName, dstBuffer, srcBuffer, src1Buffer);
				break;
			case ARM64_LDR:
			case ARM64_NEG:
			case ARM64_MVN:
				fprintf(outputFile, "    %s %s, %s\n", instructionName, dstBuffer, srcBuffer);
				break;
			case ARM64_MOV:
				// Example: mov x0, #100 => "mov x0, #100"
				// In your code, you might parse it into srcBuffer= #100, dstBuffer= x0
				if (instr->src.type == OPERAND_REGISTER && instr->dst.type == OPERAND_STACK_SLOT) {
					fprintf(outputFile, "    str %s, %s\n", srcBuffer, dstBuffer);
				} else if (instr->src.type == OPERAND_STACK_SLOT && instr->dst.type == OPERAND_REGISTER) {
					fprintf(outputFile, "    ldr %s, %s\n", dstBuffer, srcBuffer);
				} else if (instr->src.type == OPERAND_IMM) {
					fprintf(outputFile, "    mov %s, #%d\n", dstBuffer, instr->src.immValue);
				} else {
					fprintf(outputFile, "    mov %s, %s\n", dstBuffer, srcBuffer);
				}
				break;
			case ARM64_RET:
				// ARM64 epilogue
				fprintf(outputFile, "    add sp, sp, #%d\n", bytesToAllocate);
				fprintf(outputFile, "    ldp x29, x30, [sp], #16\n");
				fprintf(outputFile, "    ret\n");
				break;
			case ARM64_STR:
				fprintf(outputFile, "    str %s, %s\n", srcBuffer, dstBuffer);
				break;
		}
	}

	fprintf(outputFile, "\n"); // Blank line between functions
}

static void emitARM64(ARM64Instruction** instructions, ARM64Instruction arm64Instruction) {
	arrput(*instructions, arm64Instruction);
}

static ARM64InstructionType selectShift(bool is_right, bool is_var, bool is_signed)
{
	if (!is_right)      return is_var ? ARM64_LSLV : ARM64_LSL;
	if (is_signed)      return is_var ? ARM64_ASRV : ARM64_ASR;
	/* unsigned >> */
	return is_var ? ARM64_LSRV : ARM64_LSR;
}

// --------------------------------------------------
// Main translation function
// --------------------------------------------------

void translateTackyToARM64(const TackyProgram* tackyProgram, Program* asmProgram) {
#define VAR(var) ((Operand){ .type = OPERAND_VARNAME, .varName = var })
#define REG(reg) ((Operand){ .type = OPERAND_REGISTER, .regName = reg })
#define IMM(val) ((Operand){ .type = OPERAND_IMM, .immValue = val })
	for (size_t i = 0; i < arrlenu(tackyProgram->functions); i++) {
		const TackyFunction* tackyFunc = &tackyProgram->functions[i];
		Function asmFunc = {0};
		asmFunc.name = strdup(tackyFunc->name);
		asmFunc.arch = ARCH_ARM64;
		
		ARM64Instruction* arm64Instructions = (ARM64Instruction*)asmFunc.instructions;
		
		for (size_t j = 0; j < arrlenu(tackyFunc->instructions); j++) {
			const TackyInstruction* instr = &tackyFunc->instructions[j];
			
			switch (instr->type) {
				case TACKY_INSTR_UNARY: {
					// Load src into %eax
					Operand srcOperand;
					if (instr->unary.src.type == TACKY_VAL_CONSTANT) {
						srcOperand = IMM(instr->unary.src.constantValue);
					} else {
						srcOperand = VAR(instr->unary.src.varName);
					}
					
					emitARM64(&arm64Instructions, ((ARM64Instruction) {
						.type = ARM64_MOV,
						.src = srcOperand,
						.dst = VAR(instr->unary.dst.varName),
					}));
					
					// Apply operation on %eax
					ARM64InstructionType opcodeType;
					switch (instr->unary.op) {
						case TACKY_NEGATE:
							opcodeType = ARM64_NEG;
							break;
						case TACKY_COMPLEMENT:
							opcodeType = ARM64_MVN;
							break;
					}
					
					emitARM64(&arm64Instructions, (ARM64Instruction) {
						.type = opcodeType,
						.src = VAR(instr->unary.dst.varName),
					});
					break;
				}
				case TACKY_INSTR_BINARY: {
					Operand src0;
					if (instr->binary.lhs.type == TACKY_VAL_CONSTANT) {
						src0 = IMM(instr->binary.lhs.constantValue);
					} else {
						src0 = VAR(instr->binary.lhs.varName);
					}
					Operand src1;
					if (instr->binary.rhs.type == TACKY_VAL_CONSTANT) {
						src1 = IMM(instr->binary.rhs.constantValue);
					} else {
						src1 = VAR(instr->binary.rhs.varName);
					}
					
					switch (instr->binary.op) {
						case TACKY_ADD:
							emitARM64(&arm64Instructions, (ARM64Instruction){
								.type = ARM64_ADD,
								.src = src0,
								.src1 = src1,
								.dst = VAR(instr->binary.dst.varName),
							});
							break;
							
						case TACKY_SUBTRACT:
							emitARM64(&arm64Instructions, (ARM64Instruction){
								.type = ARM64_SUB,
								.src = src0,
								.src1 = src1,
								.dst = VAR(instr->binary.dst.varName),
							});
							break;
							
						case TACKY_MULTIPLY:
							emitARM64(&arm64Instructions, (ARM64Instruction){
								.type = ARM64_MUL,
								.src = src0,
								.src1 = src1,
								.dst = VAR(instr->binary.dst.varName),
							});
							break;
							
						case TACKY_DIVIDE:
						case TACKY_MODULO:
						{
							// LHS must be in %eax
							Operand src0;
							if (instr->binary.lhs.type == TACKY_VAL_CONSTANT) {
								src0 = IMM(instr->binary.lhs.constantValue);
							} else {
								src0 = VAR(instr->binary.lhs.varName);
							}
							Operand src1;
							if (instr->binary.rhs.type == TACKY_VAL_CONSTANT) {
								src1 = IMM(instr->binary.rhs.constantValue);
							} else {
								src1 = VAR(instr->binary.rhs.varName);
							}
							// Perform signed division: edx:eax / rhs
							emitARM64(&arm64Instructions, (ARM64Instruction){
								.type = ARM64_SDIV,
								.src = src0,
								.src1 = src1,
								.dst = VAR(instr->binary.dst.varName),
							});
							if (instr->binary.op == TACKY_MODULO) {
								emitARM64(&arm64Instructions, (ARM64Instruction){
									.type = ARM64_MUL,
									.src = VAR(instr->binary.dst.varName),
									.src1 = src1,
									.dst = VAR(instr->binary.dst.varName),
								});
								emitARM64(&arm64Instructions, (ARM64Instruction){
									.type = ARM64_SUB,
									.src = src0,
									.src1 = VAR(instr->binary.dst.varName),
									.dst = VAR(instr->binary.dst.varName),
								});
							}
							break;
						}
						case TACKY_BITWISE_AND:
							emitARM64(&arm64Instructions, (ARM64Instruction){
								.type = ARM64_AND,
								.src = src0,
								.src1 = src1,
								.dst = VAR(instr->binary.dst.varName),
							});
							break;
						case TACKY_BITWISE_OR:
							emitARM64(&arm64Instructions, (ARM64Instruction){
								.type = ARM64_ORR,
								.src = src0,
								.src1 = src1,
								.dst = VAR(instr->binary.dst.varName),
							});
							break;
						case TACKY_BITWISE_XOR:
							emitARM64(&arm64Instructions, (ARM64Instruction){
								.type = ARM64_EOR,
								.src = src0,
								.src1 = src1,
								.dst = VAR(instr->binary.dst.varName),
							});
							break;
						case TACKY_SHIFT_LEFT: {
							const bool is_var = (src1.type != OPERAND_IMM);
							ARM64InstructionType op = selectShift(/*is_right=*/false, is_var, /*is_signed=*/true /*or from type*/);
							emitARM64(&arm64Instructions, (ARM64Instruction){
								.type = op,
								.src  = src0,
								.src1 = src1,   // #imm or reg; both are fine for the chosen op
								.dst  = VAR(instr->binary.dst.varName),
							});
							break;
						}
						case TACKY_SHIFT_RIGHT: {
							const bool is_var = (src1.type != OPERAND_IMM);
							const bool is_signed = true; // TODO: derive from the TACKY/semantic type (int => true, unsigned => false)
							ARM64InstructionType op = selectShift(/*is_right=*/true, is_var, is_signed);
							emitARM64(&arm64Instructions, (ARM64Instruction){
								.type = op,
								.src  = src0,
								.src1 = src1,
								.dst  = VAR(instr->binary.dst.varName),
							});
							break;
						}					}
					break;
				}
				case TACKY_INSTR_RETURN: {
					Operand srcOperand;
					if (instr->ret.value.type == TACKY_VAL_CONSTANT) {
						srcOperand = IMM(instr->ret.value.constantValue);
					} else {
						srcOperand = VAR(instr->ret.value.varName);
					}
					emitARM64(&arm64Instructions, (ARM64Instruction) {
						.type = ARM64_MOV,
						.src = srcOperand,
						.dst = REG("w0")
					});
					
					emitARM64(&arm64Instructions, (ARM64Instruction) {
						.type = ARM64_RET,
					});
					break;
				}
			}
		}
		
		asmFunc.instructions = arm64Instructions;
		asmFunc.instructionCount = arrlenu(arm64Instructions);
		arrput(asmProgram->functions, asmFunc);
		asmProgram->functionCount = arrlenu(asmProgram->functions);
#undef IMM
#undef REG
#undef VAR
	}
}

void replacePseudoRegistersARM64(Program* asmProgram) {
#define SLOT(offset) ((Operand){ .type = OPERAND_STACK_SLOT, .stackOffset = offset })
	for (size_t iFunc = 0; iFunc < asmProgram->functionCount; iFunc++) {
		const Function* func = &asmProgram->functions[iFunc];

		const ARM64Instruction* instructions = (const ARM64Instruction*)func->instructions;

		for (size_t i = 0; i < func->instructionCount; i++) {
			ARM64Instruction* instr = (ARM64Instruction*)&instructions[i];
			
			if (instr->src.type == OPERAND_VARNAME) {
				int offset = getOrAssignStackOffsetARM64(instr->src.varName);
				instr->src = SLOT(offset);
			}

			if (instr->src1.type == OPERAND_VARNAME) {
				int offset = getOrAssignStackOffsetARM64(instr->src1.varName);
				instr->src1 = SLOT(offset);
			}

			if (instr->dst.type == OPERAND_VARNAME) {
				int offset = getOrAssignStackOffsetARM64(instr->dst.varName);
				instr->dst = SLOT(offset);
			}
		}
	}
#undef SLOT
}

void fixupIllegalInstructionsARM64(Program* asmProgram, Program* finalAsmProgram) {
#define REG(reg) ((Operand){ .type = OPERAND_REGISTER, .regName = reg })
//	const Operand scratch = REG("w10");

	for (size_t iFunc = 0; iFunc < asmProgram->functionCount; iFunc++) {
		const Function* srcFunc = &asmProgram->functions[iFunc];

		Function outFunc = {
			.name = strdup(srcFunc->name),
			.arch = srcFunc->arch
		};

		ARM64Instruction* fixedInstructions = NULL;
		const ARM64Instruction* instrs = (const ARM64Instruction*)srcFunc->instructions;

		for (size_t i = 0; i < srcFunc->instructionCount; i++) {
			const ARM64Instruction* instr = &instrs[i];
			bool srcIsMemOrImm = instr->src.type == OPERAND_STACK_SLOT || instr->src.type == OPERAND_IMM;
			bool dstIsMem = instr->dst.type == OPERAND_STACK_SLOT;
			bool src2IsMemOrImm = instr->src1.type == OPERAND_STACK_SLOT || instr->src1.type == OPERAND_IMM;
			switch (instr->type) {
				case ARM64_ADD:
				case ARM64_SUB:
				case ARM64_MUL:
				case ARM64_SDIV:
				case ARM64_AND:
				case ARM64_ORR:
				case ARM64_EOR:
					if (srcIsMemOrImm || src2IsMemOrImm || dstIsMem) {
						// Load any memory operands to scratch registers
						Operand reg1 = instr->src;
						Operand reg2 = instr->src1;
//						Operand dst = instr->dst;

						if (srcIsMemOrImm) {
							arrput(fixedInstructions, ((ARM64Instruction){
								.type = instr->src.type == OPERAND_STACK_SLOT ? ARM64_LDR : ARM64_MOV,
								.src = instr->src,
								.dst = REG("w11")
							}));
							reg1 = REG("w11");
						}
						if (src2IsMemOrImm) {
							arrput(fixedInstructions, ((ARM64Instruction){
								.type = instr->src1.type == OPERAND_STACK_SLOT ? ARM64_LDR : ARM64_MOV,
								.src = instr->src1,
								.dst = REG("w12")
							}));
							reg2 = REG("w12");
						}

						// Perform operation into scratch
						arrput(fixedInstructions, ((ARM64Instruction){
							.type = instr->type,
							.src = reg1,
							.src1 = reg2,
							.dst = REG("w10")
						}));

						// Store result if dst is memory
						if (dstIsMem) {
							arrput(fixedInstructions, ((ARM64Instruction){
								.type = ARM64_STR,
								.src = REG("w10"),
								.dst = instr->dst
							}));
						} else {
							arrput(fixedInstructions, ((ARM64Instruction){
								.type = ARM64_MOV,
								.src = REG("w10"),
								.dst = instr->dst
							}));
						}
					} else {
						arrput(fixedInstructions, *instr);
					}
					break;

				case ARM64_MOV:
				case ARM64_STR:
				case ARM64_LDR:
					arrput(fixedInstructions, *instr);
					break;
				case ARM64_LSL:
				case ARM64_LSR:
				case ARM64_ASR:
				case ARM64_LSLV:
				case ARM64_LSRV:
				case ARM64_ASRV: {
					bool lhsBad  = (instr->src.type  == OPERAND_STACK_SLOT) || (instr->src.type  == OPERAND_IMM);
					bool rhsBad  = (instr->src1.type == OPERAND_STACK_SLOT) ||
								   (instr->src1.type == OPERAND_IMM && (instr->type==ARM64_LSLV || instr->type==ARM64_LSRV || instr->type==ARM64_ASRV));
					bool dstIsMem = (instr->dst.type == OPERAND_STACK_SLOT);

					Operand lhs = instr->src;
					Operand rhs = instr->src1;
					if (lhsBad) {
						arrput(fixedInstructions, ((ARM64Instruction){ .type = (instr->src.type==OPERAND_STACK_SLOT)?ARM64_LDR:ARM64_MOV, .src = instr->src, .dst = REG("w11") }));
						lhs = REG("w11");
					}
					if (rhsBad) {
						// Only needed for variable shifts (rhs must be a reg)
						arrput(fixedInstructions, ((ARM64Instruction){ .type = (instr->src1.type==OPERAND_STACK_SLOT)?ARM64_LDR:ARM64_MOV, .src = instr->src1, .dst = REG("w12") }));
						rhs = REG("w12");
					}

					// Emit shift into w10
					arrput(fixedInstructions, ((ARM64Instruction){ .type = instr->type, .src = lhs, .src1 = rhs, .dst = REG("w10") }));

					if (dstIsMem) {
						arrput(fixedInstructions, ((ARM64Instruction){ .type = ARM64_STR, .src = REG("w10"), .dst = instr->dst }));
					} else {
						arrput(fixedInstructions, ((ARM64Instruction){ .type = ARM64_MOV, .src = REG("w10"), .dst = instr->dst }));
					}
				} break;
				default:
					// Just pass through anything else
					arrput(fixedInstructions, *instr);
					break;
			}
		}

		outFunc.instructions = fixedInstructions;
		outFunc.instructionCount = arrlenu(fixedInstructions);
		arrput(finalAsmProgram->functions, outFunc);
		finalAsmProgram->functionCount = arrlenu(finalAsmProgram->functions);
	}

#undef REG
}

void printARM64Function(const Function* function)
{
		const ARM64Instruction* instructions = (const ARM64Instruction*)function->instructions;
		char srcBuffer[32];
		char src1Buffer[32];
		char dstBuffer[32];
		
		for (size_t i = 0; i < function->instructionCount; i++) {
			const ARM64Instruction* instr = &instructions[i];
			const char* instructionName = getARM64InstructionName(instr->type);
			switch (instr->type) {
				case ARM64_ADD:
				case ARM64_AND:
				case ARM64_EOR:
				case ARM64_SDIV:
				case ARM64_MUL:
				case ARM64_ORR:
				case ARM64_SUB:
				case ARM64_LSL:
				case ARM64_LSR:
				case ARM64_ASR:
				case ARM64_LSLV:
				case ARM64_LSRV:
				case ARM64_ASRV:
					getARM64Operand(&instr->src, srcBuffer, sizeof(srcBuffer));
					getARM64Operand(&instr->src1, src1Buffer, sizeof(src1Buffer));
					getARM64Operand(&instr->dst, dstBuffer, sizeof(dstBuffer));
					printf("  %s %s, %s, %s\n", instructionName, dstBuffer, srcBuffer, src1Buffer);
					break;
				case ARM64_LDR:
					printf("  ldr %s, %s\n", getARM64Operand(&instr->dst, dstBuffer, sizeof(dstBuffer)), getARM64Operand(&instr->src, srcBuffer, sizeof(srcBuffer)));
					break;
				case ARM64_MOV:
#if _DEBUG
					assert(!(instr->src.type == OPERAND_STACK_SLOT && instr->dst.type == OPERAND_STACK_SLOT) && "mem->mem mov should be legalized earlier");
#endif
					if (instr->src.type == OPERAND_REGISTER && instr->dst.type == OPERAND_STACK_SLOT) {
						printf("  str %s, %s\n", getARM64Operand(&instr->src, srcBuffer, sizeof(srcBuffer)), getARM64Operand(&instr->dst, dstBuffer, sizeof(dstBuffer)));
					} else if (instr->src.type == OPERAND_STACK_SLOT && instr->dst.type == OPERAND_REGISTER) {
						printf("  ldr %s, %s\n", getARM64Operand(&instr->dst, dstBuffer, sizeof(dstBuffer)), getARM64Operand(&instr->src, srcBuffer, sizeof(srcBuffer)));
					} else if (instr->src.type == OPERAND_IMM) {
						printf("  mov %s, #%d\n", getARM64Operand(&instr->dst, dstBuffer, sizeof(dstBuffer)), instr->src.immValue);
					} else {
						printf("  mov %s, %s\n", getARM64Operand(&instr->dst, dstBuffer, sizeof(dstBuffer)), getARM64Operand(&instr->src, srcBuffer, sizeof(srcBuffer)));
					}
					break;
				case ARM64_NEG:
					printf("  neg %s, %s\n", getARM64Operand(&instr->dst, dstBuffer, sizeof(dstBuffer)), getARM64Operand(&instr->src, srcBuffer, sizeof(srcBuffer)));
					break;
				case ARM64_MVN:
					printf("  mvn %s, %s\n", getARM64Operand(&instr->dst, dstBuffer, sizeof(dstBuffer)), getARM64Operand(&instr->src, srcBuffer, sizeof(srcBuffer)));
					break;
				case ARM64_RET:
					printf("  ret\n");
					break;
				case ARM64_STR:
					printf("  str %s, %s\n", getARM64Operand(&instr->src, srcBuffer, sizeof(srcBuffer)), getARM64Operand(&instr->dst, dstBuffer, sizeof(dstBuffer)));
					break;
				default:
					printf("  Unknown instruction\n");
					break;
			}
		}
	}
