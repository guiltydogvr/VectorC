//
//  translate_tacky_arm64.c
//  VectorC
//
//  Created by Claire Rogers on 10/07/2025.
//

#include <stdio.h>
#include <stdlib.h>
#include "translate_tacky_arm64.h"
#include "ast_arm64.h"
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

#if 0
void translateTackyToARM64(const TackyProgram* tackyProgram, Program* asmProgram) {
	if (!tackyProgram) return;

	asmProgram->functions = NULL;

	for (size_t i = 0; i < arrlenu(tackyProgram->functions); ++i) {
		const TackyFunction* tackyFunc = &tackyProgram->functions[i];

		Function func = {0};
		func.name = tackyFunc->name;
		func.arch = ARCH_ARM64;

		ARM64Instruction* instrs = NULL;

		for (size_t j = 0; j < arrlenu(tackyFunc->instructions); ++j) {
			const TackyInstruction* instr = &tackyFunc->instructions[j];

			switch (instr->type) {
				case TACKY_INSTR_UNARY: {
					Operand reg = createRegisterOperand("x0");

					if (instr->unary.src.type == TACKY_VAL_CONSTANT) {
						Operand imm = createImmOperand(instr->unary.src.constantValue);
						ARM64Instruction mov = { ARM64_MOV, imm, reg };
						arrput(instrs, mov);
					} else if (instr->unary.src.type == TACKY_VAL_VAR) {
						// Nothing to do — already in eax.
					} else {
						fprintf(stderr, "Unsupported: source type in Unary not implemented\n");
						exit(EXIT_FAILURE);
					}

					if (instr->unary.op == TACKY_NEGATE) {
						ARM64Instruction neg = { ARM64_NEG, reg, reg };
						arrput(instrs, neg);
					} else if (instr->unary.op == TACKY_COMPLEMENT) {
						ARM64Instruction not_ = { ARM64_NOT, reg, reg };
						arrput(instrs, not_);
					}
				} break;

				case TACKY_INSTR_RETURN: {
					if (instr->ret.value.type == TACKY_VAL_CONSTANT) {
						Operand reg = createRegisterOperand("x0");
						Operand imm = createImmOperand(instr->ret.value.constantValue);
						ARM64Instruction movRet = { ARM64_MOV, imm, reg };
						arrput(instrs, movRet);
					}
					// Always emit ret
					ARM64Instruction ret = { ARM64_RET, {0}, {0} };
					arrput(instrs, ret);
				} break;
			}
		}

		func.instructions = instrs;
		func.instructionCount = arrlenu(instrs);
		
		arrput(asmProgram->functions, func);
		asmProgram->functionCount = arrlenu(asmProgram->functions);
	}
}
#else

// --------------------------------------------------
// Main translation function
// --------------------------------------------------

void translateTackyToARM64(const TackyProgram* tackyProgram, Program* asmProgram) {
	for (size_t i = 0; i < arrlenu(tackyProgram->functions); i++) {
		const TackyFunction* tackyFunc = &tackyProgram->functions[i];
		Function asmFunc = {0};
		asmFunc.name = strdup(tackyFunc->name);
		asmFunc.arch = ARCH_ARM64;

		ARM64Instruction* arm64Instructions = (ARM64Instruction*)asmFunc.instructions;

		tmpMappings = NULL;

		int nextOffset = 4;  // start at -4(fp)
		for (size_t j = 0; j < arrlenu(tackyFunc->instructions); j++) {
			const TackyInstruction* instr = &tackyFunc->instructions[j];

			switch (instr->type) {
				case TACKY_INSTR_UNARY: {
					// Ensure dst has a stack slot
					TmpMapping tmpMapping = (TmpMapping){instr->unary.dst.varName, nextOffset};
					arrput(tmpMappings, tmpMapping);
					int dstOffset = nextOffset;
					nextOffset += 4; // advance stack offset

					// Load src into w0
					if (instr->unary.src.type == TACKY_VAL_CONSTANT) {
						ARM64Instruction inst = (ARM64Instruction) {
							.type = ARM64_MOV,
							.src = (Operand){ .type = OPERAND_IMM, .immValue = instr->unary.src.constantValue },
							.dst = (Operand){ .type = OPERAND_REGISTER, .regName = "w0" }
						};
//						ARM64Instruction* arm64Instructions = (ARM64Instruction*)asmFunc.instructions;
						arrput(arm64Instructions, inst);
					} else {
						int srcOffset = getStackOffsetForTmp(instr->unary.src.varName);
						ARM64Instruction inst = (ARM64Instruction) {
							.type = ARM64_MOV, // we'll emit as ldr
							.src = (Operand){ .type = OPERAND_STACK_SLOT, .stackOffset = srcOffset },
							.dst = (Operand){ .type = OPERAND_REGISTER, .regName = "w0" }
						};
//						ARM64Instruction* arm64Instructions = (ARM64Instruction*)asmFunc.instructions;
						arrput(arm64Instructions, inst);
					}

					// Apply operation in w0
					if (instr->unary.op == TACKY_NEGATE) {
						ARM64Instruction inst = (ARM64Instruction) {
							.type = ARM64_NEG,
							.src = (Operand){ .type = OPERAND_REGISTER, .regName = "w0" },
							.dst = (Operand){ .type = OPERAND_REGISTER, .regName = "w0" }
						};
						arrput(arm64Instructions, inst);
					} else if (instr->unary.op == TACKY_COMPLEMENT) {
						ARM64Instruction inst = (ARM64Instruction){
							.type = ARM64_NOT,
							.src = (Operand){ .type = OPERAND_REGISTER, .regName = "w0" },
							.dst = (Operand){ .type = OPERAND_REGISTER, .regName = "w0" }
						};
						arrput(arm64Instructions, inst);
					}

					// Store back to stack
					ARM64Instruction inst = (ARM64Instruction){
						.type = ARM64_MOV, // emit as str
						.src = (Operand){ .type = OPERAND_REGISTER, .regName = "w0" },
						.dst = (Operand){ .type = OPERAND_STACK_SLOT, .stackOffset = dstOffset }
					};
					arrput(arm64Instructions, inst);
					break;
				}

				case TACKY_INSTR_RETURN: {
					// Load return value into w0 (return register)
					if (instr->ret.value.type == TACKY_VAL_CONSTANT) {
						ARM64Instruction inst = (ARM64Instruction) {
							.type = ARM64_MOV,
							.src = (Operand){ .type = OPERAND_IMM, .immValue = instr->ret.value.constantValue },
							.dst = (Operand){ .type = OPERAND_REGISTER, .regName = "w0" }
						};
						arrput(arm64Instructions, inst);
					} else {
						int srcOffset = getStackOffsetForTmp(instr->ret.value.varName);
						ARM64Instruction inst = (ARM64Instruction) {
							.type = ARM64_MOV, // emit as ldr
							.src = (Operand){ .type = OPERAND_STACK_SLOT, .stackOffset = srcOffset },
							.dst = (Operand){ .type = OPERAND_REGISTER, .regName = "w0" }
						};
						arrput(arm64Instructions, inst);
					}
					ARM64Instruction inst = (ARM64Instruction) {
						.type = ARM64_RET
					};
					arrput(arm64Instructions, inst);
					break;
				}
			}
		}

		asmFunc.instructions = arm64Instructions;
		asmFunc.instructionCount = arrlenu(arm64Instructions);
		arrput(asmProgram->functions, asmFunc);
		asmProgram->functionCount = arrlenu(asmProgram->functions);
		arrfree(tmpMappings);
	}
}
#endif
