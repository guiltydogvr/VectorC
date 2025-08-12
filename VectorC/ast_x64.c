//
//  ast_x64.c
//  VectorC
//
//  Created by Claire Rogers on 05/01/2025.
//

#include "ast_x64.h"
#include "stb_ds.h"
#include "tacky.h"
#include <stdio.h>
#include <stdbool.h>

// Convert an operand to a string for x64 (helper function)
void getX64Operand(const Operand* op, char* buffer, size_t bufferSize) {
	switch (op->type) {
		case OPERAND_IMM:
			snprintf(buffer, bufferSize, "$%d", op->immValue);
			break;
		case OPERAND_VARNAME:
			snprintf(buffer, bufferSize, "%s", op->varName);
			break;
		case OPERAND_STACK_SLOT:
			snprintf(buffer, bufferSize, "%d(%%rbp)", op->stackOffset);
			break;
		case OPERAND_REGISTER:
			snprintf(buffer, bufferSize, "%s", op->regName);
			break;
	}
}

// --------------------------------------------------
// Local helper to track tmp -> stack offsets
// --------------------------------------------------

static TmpMapping* s_tmpMappings = NULL;

static int s_nextOffset = -4; // global or passed in

int getOrAssignStackOffsetX64(const char* tmpName) {
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
	s_nextOffset -= 4; // Move down the stack
	return assigned;
}

//---------------------------------------------------------
// X64 CODEGEN
//---------------------------------------------------------
void generateX64Function(FILE* outputFile, const Function* func)
{
	// Decide function label
	// Typically: `_main` on macOS or `main` on Linux.
	// This example just demonstrates the logic:
	const char* funcName = func->name;

#ifdef __APPLE__
	if (strcmp(func->name, "main") == 0) {
		funcName = "_main";
	}
#endif

	fprintf(outputFile, ".global %s\n", funcName);
	fprintf(outputFile, "%s:\n", funcName);

	int bytesToAllocate = alignTo(-s_nextOffset, 16);
	
	// X86-64 prologue
	fprintf(outputFile, "    pushq %%rbp\n");
	fprintf(outputFile, "    movq %%rsp, %%rbp\n");
	fprintf(outputFile, "    subq $%d, %%rsp\n", bytesToAllocate); // example stack allocation

	// Emit instructions
	const X64Instruction* instructions = (const X64Instruction*)func->instructions;
	for (size_t j = 0; j < func->instructionCount; j++) {
		const X64Instruction* instr = &instructions[j];

		char srcBuffer[32], dstBuffer[32];
		getX64Operand(&instr->src, srcBuffer, sizeof(srcBuffer));
		getX64Operand(&instr->dst, dstBuffer, sizeof(dstBuffer));

		switch (instr->type) {
			case X64_ADD:
				fprintf(outputFile, "    addl %s, %s\n", srcBuffer, dstBuffer);
				break;
			case X64_AND:
				fprintf(outputFile, "    andl %s, %s\n", srcBuffer, dstBuffer);
				break;
			case X64_CDQ:
				fprintf(outputFile, "    cdq\n");
				break;
			case X64_IDIV:
				fprintf(outputFile, "    idivl %s\n", srcBuffer);
				break;
			case X64_IMUL:
				fprintf(outputFile, "    imull %s, %s\n", srcBuffer, dstBuffer);
				break;
			case X64_MOV:
				// Example: move immediate into a register or memory
				fprintf(outputFile, "    movl %s, %s\n", srcBuffer, dstBuffer);
				break;
			case X64_NEG:
				fprintf(outputFile, "    negl %s\n", srcBuffer);
				break;
			case X64_NOT:
				fprintf(outputFile, "    notl %s\n", srcBuffer);
				break;
			case X64_OR:
				fprintf(outputFile, "    orl %s, %s\n", srcBuffer, dstBuffer);
				break;
			case X64_RET:
				// X86-64 epilogue
				fprintf(outputFile, "    movq %%rbp, %%rsp\n");
				fprintf(outputFile, "    popq %%rbp\n");
				fprintf(outputFile, "    ret\n");
				break;
			case X64_SAR_CL: {
				fprintf(outputFile, "    sarl %%cl, %s\n", dstBuffer);
				break;
			}
			case X64_SAR_IMM: {
				fprintf(outputFile, "    sarl %s, %s\n", srcBuffer, dstBuffer);
				break;
			}
			case X64_SHL_CL: {
				fprintf(outputFile, "    shll %%cl, %s\n", dstBuffer);
				break;
			}
			case X64_SHL_IMM: {
				fprintf(outputFile, "    shll %s, %s\n", srcBuffer, dstBuffer);
				break;
			}
			case X64_SUB:
				fprintf(outputFile, "    subl %s, %s\n", srcBuffer, dstBuffer);
				break;
			case X64_XOR:
				fprintf(outputFile, "    xorl %s, %s\n", srcBuffer, dstBuffer);
				break;
		}
	}

	fprintf(outputFile, "\n"); // Blank line between functions
}

static void emitX64(X64Instruction** instructions, X64Instruction x64Instruction) {
	arrput(*instructions, x64Instruction);
}

// --------------------------------------------------
// Main translation function
// --------------------------------------------------

void translateTackyToX64(const TackyProgram* tackyProgram, Program* asmProgram) {
#define VAR(var) ((Operand){ .type = OPERAND_VARNAME, .varName = var })
#define REG(reg) ((Operand){ .type = OPERAND_REGISTER, .regName = reg })
#define IMM(val) ((Operand){ .type = OPERAND_IMM, .immValue = val })
	for (size_t i = 0; i < arrlenu(tackyProgram->functions); i++) {
		const TackyFunction* tackyFunc = &tackyProgram->functions[i];
		Function asmFunc = {0};
		asmFunc.name = strdup(tackyFunc->name);
		asmFunc.arch = ARCH_X64;

		X64Instruction* x64Instructions = (X64Instruction*)asmFunc.instructions;

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

					emitX64(&x64Instructions, ((X64Instruction) {
						.type = X64_MOV,
						.src = srcOperand,
						.dst = VAR(instr->unary.dst.varName),
					}));
					
					// Apply operation on %eax
					X64InstructionType opcodeType;
					switch (instr->unary.op) {
						case TACKY_NEGATE:
							opcodeType = X64_NEG;
							break;
						case TACKY_COMPLEMENT:
							opcodeType = X64_NOT;
							break;
					}

					emitX64(&x64Instructions, ((X64Instruction) {
						.type = opcodeType,
						.src = VAR(instr->unary.dst.varName),
					}));
					break;
				}
				case TACKY_INSTR_BINARY: {
					const TackyBinaryOperator op = instr->binary.op;

					// --- Special-case: DIV/MOD need EAX/EDX + CDQ + IDIV ---
					if (op == TACKY_DIVIDE || op == TACKY_MODULO) {
						// LHS -> %eax (dividend low 32)
						Operand lhs = (instr->binary.lhs.type == TACKY_VAL_CONSTANT)
							? IMM(instr->binary.lhs.constantValue)
							: VAR(instr->binary.lhs.varName);

						emitX64(&x64Instructions, (X64Instruction){
							.type = X64_MOV, .src = lhs, .dst = REG("%eax")
						});

						// Sign-extend EAX into EDX (so EDX:EAX is the dividend)
						emitX64(&x64Instructions, (X64Instruction){ .type = X64_CDQ });

						// Divisor can be imm or var; your Pass 3 already fixes imm->reg for IDIV
						Operand rhs = (instr->binary.rhs.type == TACKY_VAL_CONSTANT)
							? IMM(instr->binary.rhs.constantValue)
							: VAR(instr->binary.rhs.varName);

						emitX64(&x64Instructions, (X64Instruction){
							.type = X64_IDIV, .src = rhs
						});

						// Store result: quotient -> EAX for DIV, remainder -> EDX for MOD
						emitX64(&x64Instructions, (X64Instruction){
							.type = X64_MOV,
							.src  = (op == TACKY_DIVIDE) ? REG("%eax") : REG("%edx"),
							.dst  = VAR(instr->binary.dst.varName),
						});
						break; // done with DIV/MOD
					}

					// --- Generic path: dst = lhs; then apply op with rhs (covers & | ^ << >> and + - *) ---
					const char* dst = instr->binary.dst.varName;

					Operand lhs = (instr->binary.lhs.type == TACKY_VAL_CONSTANT)
						? IMM(instr->binary.lhs.constantValue)
						: VAR(instr->binary.lhs.varName);

					// 1) dst = lhs
					emitX64(&x64Instructions, (X64Instruction){
						.type = X64_MOV, .src = lhs, .dst = VAR(dst)
					});

					// 2) Apply the operation
					if (op == TACKY_SHIFT_LEFT || op == TACKY_SHIFT_RIGHT) {
						const bool rhs_is_imm = (instr->binary.rhs.type == TACKY_VAL_CONSTANT);
						if (rhs_is_imm) {
							emitX64(&x64Instructions, (X64Instruction){
								.type = (op == TACKY_SHIFT_LEFT) ? X64_SHL_IMM : X64_SAR_IMM, // signed int => SAR
								.src  = IMM(instr->binary.rhs.constantValue),
								.dst  = VAR(dst),
							});
						} else {
							emitX64(&x64Instructions, (X64Instruction){
								.type = X64_MOV,
								.src  = VAR(instr->binary.rhs.varName),
								.dst  = REG("%ecx"), // CL
							});
							emitX64(&x64Instructions, (X64Instruction){
								.type = (op == TACKY_SHIFT_LEFT) ? X64_SHL_CL : X64_SAR_CL,
								.dst  = VAR(dst), // CL is implicit
							});
						}
					} else {
						X64InstructionType xop =
							(op == TACKY_ADD)           ? X64_ADD :
							(op == TACKY_SUBTRACT)      ? X64_SUB :
							(op == TACKY_MULTIPLY)      ? X64_IMUL :
							(op == TACKY_BITWISE_AND)   ? X64_AND :
							(op == TACKY_BITWISE_OR)    ? X64_OR  :
							/* TACKY_BITWISE_XOR */       X64_XOR;

						Operand rhs = (instr->binary.rhs.type == TACKY_VAL_CONSTANT)
							? IMM(instr->binary.rhs.constantValue)
							: VAR(instr->binary.rhs.varName);

						emitX64(&x64Instructions, (X64Instruction){
							.type = xop, .src = rhs, .dst = VAR(dst)
						});
					}
					break;
				}
				case TACKY_INSTR_RETURN: {
					Operand srcOperand;
					if (instr->ret.value.type == TACKY_VAL_CONSTANT) {
						srcOperand = IMM(instr->ret.value.constantValue);
					} else {
						srcOperand = VAR(instr->ret.value.varName);
					}
					emitX64(&x64Instructions, (X64Instruction) {
						.type = X64_MOV,
						.src = srcOperand,
						.dst = REG("%eax")
					});

					emitX64(&x64Instructions, (X64Instruction) {
						.type = X64_RET,
					});
					break;
				}
			}
		}

		asmFunc.instructions = x64Instructions;
		asmFunc.instructionCount = arrlenu(x64Instructions);
		arrput(asmProgram->functions, asmFunc);
		asmProgram->functionCount = arrlenu(asmProgram->functions);
#undef IMM
#undef REG
#undef VAR
	}
}

void replacePseudoRegistersX64(Program* asmProgram) {
#define SLOT(offset) ((Operand){ .type = OPERAND_STACK_SLOT, .stackOffset = offset })
	for (size_t iFunc = 0; iFunc < asmProgram->functionCount; iFunc++) {
		const Function* func = &asmProgram->functions[iFunc];

		const X64Instruction* instructions = (const X64Instruction*)func->instructions;

		for (size_t i = 0; i < func->instructionCount; i++) {
			X64Instruction* instr = (X64Instruction*)&instructions[i];
			
			if (instr->src.type == OPERAND_VARNAME) {
				int offset = getOrAssignStackOffsetX64(instr->src.varName);
				instr->src = SLOT(offset);
			}

			if (instr->dst.type == OPERAND_VARNAME) {
				int offset = getOrAssignStackOffsetX64(instr->dst.varName);
				instr->dst = SLOT(offset);
			}
		}
	}
#undef SLOT
}

void fixupIllegalInstructionsX64(Program* asmProgram, Program* finalAsmProgram) {
#define REG(reg) ((Operand){ .type = OPERAND_REGISTER, .regName = reg })
	const Operand scratch = REG("%r10d");

	for (size_t iFunc = 0; iFunc < asmProgram->functionCount; iFunc++) {
		const Function* srcFunc = &asmProgram->functions[iFunc];

		Function outFunc = {
			.name = strdup(srcFunc->name),
			.arch = srcFunc->arch
		};

		X64Instruction* fixedInstructions = NULL;
		const X64Instruction* instrs = (const X64Instruction*)srcFunc->instructions;

		for (size_t i = 0; i < srcFunc->instructionCount; i++) {
			const X64Instruction* instr = &instrs[i];

			bool srcIsMem = instr->src.type == OPERAND_STACK_SLOT;
			bool dstIsMem = instr->dst.type == OPERAND_STACK_SLOT;

			switch (instr->type) {
				case X64_MOV:
					if (srcIsMem && dstIsMem) {
						// mov [mem], [mem] → use scratch reg
						arrput(fixedInstructions, ((X64Instruction){
							.type = X64_MOV,
							.src = instr->src,
							.dst = scratch
						}));
						arrput(fixedInstructions, ((X64Instruction){
							.type = X64_MOV,
							.src = scratch,
							.dst = instr->dst
						}));
					} else {
						arrput(fixedInstructions, *instr);
					}
					break;

				case X64_ADD:
				case X64_SUB:
				case X64_IMUL:
				case X64_AND:
				case X64_OR:
				case X64_XOR:
					if (srcIsMem && dstIsMem) {
						// <op> [mem], [mem] → fix via scratch
						arrput(fixedInstructions, ((X64Instruction){
							.type = X64_MOV,
							.src = instr->dst,
							.dst = scratch
						}));
						arrput(fixedInstructions, ((X64Instruction){
							.type = instr->type,
							.src = instr->src,
							.dst = scratch
						}));
						arrput(fixedInstructions, ((X64Instruction){
							.type = X64_MOV,
							.src = scratch,
							.dst = instr->dst
						}));
					}
					else if (instr->type == X64_IMUL && instr->src.type == OPERAND_IMM && instr->dst.type == OPERAND_STACK_SLOT) {
						// imull $imm, [mem] — illegal
						arrput(fixedInstructions, ((X64Instruction){
							.type = X64_MOV,
							.src = instr->dst,
							.dst = scratch
						}));
						arrput(fixedInstructions, ((X64Instruction){
							.type = X64_IMUL,
							.src = instr->src,      // $imm
							.dst = scratch
						}));
						arrput(fixedInstructions, ((X64Instruction){
							.type = X64_MOV,
							.src = scratch,
							.dst = instr->dst
						}));
					} else {
						arrput(fixedInstructions, *instr);  // fallback
					}
					break;
				case X64_IDIV:
					if (instr->src.type == OPERAND_IMM) {
						arrput(fixedInstructions, ((X64Instruction){
							.type = X64_MOV,
							.src = instr->src,
							.dst = scratch
						}));
						arrput(fixedInstructions, ((X64Instruction){
							.type = X64_IDIV,
							.src = scratch
						}));
					} else {
						arrput(fixedInstructions, *instr);
					}
					break;
				default:
					// All other instructions can be copied directly
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

void printX64Function(const Function* function) {
	const X64Instruction* instructions = (const X64Instruction*)function->instructions;
	char srcBuffer[32];
	char dstBuffer[32];

	for (size_t i = 0; i < function->instructionCount; i++) {
		const X64Instruction* instr = &instructions[i];
		switch (instr->type) {
			case X64_ADD:
				getX64Operand(&instr->src, srcBuffer, sizeof(srcBuffer));
				getX64Operand(&instr->dst, dstBuffer, sizeof(dstBuffer));
				printf("  addl %s, %s\n", srcBuffer, dstBuffer);
				break;
			case X64_AND:
				getX64Operand(&instr->src, srcBuffer, sizeof(srcBuffer));
				getX64Operand(&instr->dst, dstBuffer, sizeof(dstBuffer));
				printf("  andl %s, %s\n", srcBuffer, dstBuffer);
				break;
			case X64_CDQ:
				printf("  cdq\n");
				break;
			case X64_IDIV:
				getX64Operand(&instr->src, srcBuffer, sizeof(srcBuffer));
				getX64Operand(&instr->dst, dstBuffer, sizeof(dstBuffer));
				printf("  idivl %s\n", srcBuffer);
				break;
			case X64_IMUL:
				getX64Operand(&instr->src, srcBuffer, sizeof(srcBuffer));
				getX64Operand(&instr->dst, dstBuffer, sizeof(dstBuffer));
				printf("  imul %s, %s\n", srcBuffer, dstBuffer);
				break;
			case X64_MOV:
				getX64Operand(&instr->src, srcBuffer, sizeof(srcBuffer));
				getX64Operand(&instr->dst, dstBuffer, sizeof(dstBuffer));
				printf("  movl %s, %s\n", srcBuffer, dstBuffer);
				break;
			case X64_NEG:
				getX64Operand(&instr->src, srcBuffer, sizeof(srcBuffer));
				printf("  negl %s\n", srcBuffer);
				break;
			case X64_NOT:
				getX64Operand(&instr->src, srcBuffer, sizeof(srcBuffer));
				printf("  notl %s\n", srcBuffer);
				break;
			case X64_OR:
				getX64Operand(&instr->src, srcBuffer, sizeof(srcBuffer));
				getX64Operand(&instr->dst, dstBuffer, sizeof(dstBuffer));
				printf("  orl %s, %s\n", srcBuffer, dstBuffer);
				break;
			case X64_RET:
				printf("  ret\n");
				break;
			case X64_SAR_CL: {
				getX64Operand(&instr->dst, dstBuffer, sizeof dstBuffer);
				printf("  sarl %%cl, %s\n", dstBuffer);
				break;
			}
			case X64_SAR_IMM: {
				getX64Operand(&instr->dst, dstBuffer, sizeof dstBuffer);
				printf("  sarl $%d, %s\n", instr->src.immValue, dstBuffer);
				break;
			}
			case X64_SHL_CL: {
				getX64Operand(&instr->dst, dstBuffer, sizeof dstBuffer);
				printf("  shll %%cl, %s\n", dstBuffer);
				break;
			}
			case X64_SHL_IMM: {
				getX64Operand(&instr->dst, dstBuffer, sizeof dstBuffer);
				printf("  shll $%d, %s\n", instr->src.immValue, dstBuffer);
				break;
			}
			case X64_SUB:
				getX64Operand(&instr->src, srcBuffer, sizeof(srcBuffer));
				getX64Operand(&instr->dst, dstBuffer, sizeof(dstBuffer));
				printf("  subl %s, %s\n", srcBuffer, dstBuffer);
				break;
			case X64_XOR:
				getX64Operand(&instr->src, srcBuffer, sizeof(srcBuffer));
				getX64Operand(&instr->dst, dstBuffer, sizeof(dstBuffer));
				printf("  xorl %s, %s\n", srcBuffer, dstBuffer);
				break;
			default:
				printf("  Unknown instruction\n");
				break;
		}
	}
}
