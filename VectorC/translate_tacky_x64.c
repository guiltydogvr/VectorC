//
//  translate_tacky_x64.c
//  VectorC
//
//  Created by Claire Rogers on 09/07/2025.
//

#include <stdio.h>
#include <stdlib.h>
#include "translate_tacky_x64.h"
#include "ast_x64.h"
#include "stb_ds.h"

// --------------------------------------------------
// Local helper to track tmp -> stack offsets
// --------------------------------------------------

typedef struct {
	const char* tmpName;
	int stackOffset;  // e.g., 4, 8, etc.
} TmpMapping;

static TmpMapping* tmpMappings = NULL;

static int getStackOffsetForTmp(const char* tmpName) {
	for (int i = 0; i < arrlenu(tmpMappings); i++) {
		if (strcmp(tmpMappings[i].tmpName, tmpName) == 0) {
			return tmpMappings[i].stackOffset;
		}
	}
	// Should never happen in well formed code
	fprintf(stderr, "Unknown tmp variable: %s\n", tmpName);
	exit(EXIT_FAILURE);
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
#define SLOT(offset) ((Operand){ .type = OPERAND_STACK_SLOT, .stackOffset = offset })
#define IMM(val) ((Operand){ .type = OPERAND_IMM, .immValue = val })
	for (size_t i = 0; i < arrlenu(tackyProgram->functions); i++) {
		const TackyFunction* tackyFunc = &tackyProgram->functions[i];
		Function asmFunc = {0};
		asmFunc.name = strdup(tackyFunc->name);
		asmFunc.arch = ARCH_X64;

		X64Instruction* x64Instructions = (X64Instruction*)asmFunc.instructions;

		tmpMappings = NULL;

		int nextOffset = 4;  // start at -4(%rbp)
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
					if (instr->binary.op == TACKY_ADD ||
						instr->binary.op == TACKY_SUBTRACT ||
						instr->binary.op == TACKY_MULTIPLY) {

						Operand src0;
						if (instr->binary.lhs.type == TACKY_VAL_CONSTANT) {
							src0 = IMM(instr->binary.lhs.constantValue);
						} else {
							src0 = VAR(instr->binary.lhs.varName);
						}
						
						emitX64(&x64Instructions, ((X64Instruction) {
							.type = X64_MOV,
							.src = src0,
							//						.dst = REG("%eax"),
							.dst = VAR(instr->binary.dst.varName),
						}));
					}
					
					Operand src1;
					if (instr->binary.rhs.type == TACKY_VAL_CONSTANT) {
						src1 = IMM(instr->binary.rhs.constantValue);
					} else {
						src1 = VAR(instr->binary.rhs.varName);
					}

					switch (instr->binary.op) {
						case TACKY_ADD:
							emitX64(&x64Instructions, (X64Instruction){
								.type = X64_ADD,
								.src = src1,
								.dst = VAR(instr->binary.dst.varName),
							});
							break;

						case TACKY_SUBTRACT:
							emitX64(&x64Instructions, (X64Instruction){
								.type = X64_SUB,
								.src = src1,
								.dst = VAR(instr->binary.dst.varName),
							});
							break;

						case TACKY_MULTIPLY:
							emitX64(&x64Instructions, (X64Instruction){
								.type = X64_IMUL,
								.src = src1,
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
							emitX64(&x64Instructions, ((X64Instruction) {
								.type = X64_MOV,
								.src = src0,
								.dst = REG("%eax"),
							}));
							
							// Sign-extend %eax into %edx
							emitX64(&x64Instructions, (X64Instruction){
								.type = X64_CDQ
							});
							
							Operand src1;
							if (instr->binary.rhs.type == TACKY_VAL_CONSTANT) {
								src1 = IMM(instr->binary.rhs.constantValue);
							} else {
								src1 = VAR(instr->binary.rhs.varName);
							}
							// Perform signed division: edx:eax / rhs
							emitX64(&x64Instructions, (X64Instruction){
								.type = X64_IDIV,
								.src = src1,
							});
							
							// Store result
							emitX64(&x64Instructions, (X64Instruction){
								.type = X64_MOV,
								.src = (instr->binary.op == TACKY_DIVIDE) ? REG("%eax") : REG("%edx"),
								.dst = VAR(instr->binary.dst.varName),
							});
							break;
						}
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
		arrfree(tmpMappings);
#undef IMM
#undef SLOT
#undef REG
#undef VAR
	}
}
