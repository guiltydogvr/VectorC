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

#if 0
void translateTackyToX64(const TackyProgram* tackyProgram, Program* asmProgram) {
	if (!tackyProgram) return;

	asmProgram->functions = NULL;

	for (size_t i = 0; i < arrlenu(tackyProgram->functions); ++i) {
		const TackyFunction* tackyFunc = &tackyProgram->functions[i];

		Function func = {0};
		func.name = tackyFunc->name;
		func.arch = ARCH_X64;

		X64Instruction* instrs = NULL;

		for (size_t j = 0; j < arrlenu(tackyFunc->instructions); ++j) {
			const TackyInstruction* instr = &tackyFunc->instructions[j];

			switch (instr->type) {
				case TACKY_INSTR_UNARY: {
					Operand reg = createRegisterOperand("eax");

					if (instr->unary.src.type == TACKY_VAL_CONSTANT) {
						Operand imm = createImmOperand(instr->unary.src.constantValue);
						X64Instruction mov = { X64_MOV, imm, reg };
						arrput(instrs, mov);
					} else if (instr->unary.src.type == TACKY_VAL_VAR) {
						// Nothing to do — already in eax.
					} else {
						fprintf(stderr, "Unsupported: source type in Unary not implemented\n");
						exit(EXIT_FAILURE);
					}

					if (instr->unary.op == TACKY_NEGATE) {
						X64Instruction neg = { X64_NEG, reg, {0} };
						arrput(instrs, neg);
					} else if (instr->unary.op == TACKY_COMPLEMENT) {
						X64Instruction not_ = { X64_NOT, reg, {0} };
						arrput(instrs, not_);
					}
				} break;

				case TACKY_INSTR_RETURN: {
					if (instr->ret.value.type == TACKY_VAL_CONSTANT) {
						Operand reg = createRegisterOperand("eax");
						Operand imm = createImmOperand(instr->ret.value.constantValue);
						X64Instruction movRet = { X64_MOV, imm, reg };
						arrput(instrs, movRet);
					}
					// Always emit ret
					X64Instruction ret = { X64_RET, {0}, {0} };
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

void translateTackyToX64(const TackyProgram* tackyProgram, Program* asmProgram) {
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
					// Ensure dst has a stack slot
					TmpMapping tmpMapping = (TmpMapping){instr->unary.dst.varName, nextOffset};
					arrput(tmpMappings, tmpMapping);
					int dstOffset = nextOffset;
					nextOffset += 4;

					// Load src into %eax
					if (instr->unary.src.type == TACKY_VAL_CONSTANT) {
						X64Instruction inst = {
							.type = X64_MOV,
							.src = (Operand){ .type = OPERAND_IMM, .immValue = instr->unary.src.constantValue },
							.dst = (Operand){ .type = OPERAND_REGISTER, .regName = "%eax" }
						};
						arrput(x64Instructions, inst);
					} else {
						int srcOffset = getStackOffsetForTmp(instr->unary.src.varName);
						X64Instruction inst = {
							.type = X64_MOV,
							.src = (Operand){ .type = OPERAND_STACK_SLOT, .stackOffset = srcOffset },
							.dst = (Operand){ .type = OPERAND_REGISTER, .regName = "%eax" }
						};
						arrput(x64Instructions, inst);
					}

					// Apply operation on %eax
					if (instr->unary.op == TACKY_NEGATE) {
						X64Instruction inst = {
							.type = X64_NEG,
							.src = (Operand){ .type = OPERAND_REGISTER, .regName = "%eax" }
						};
						arrput(x64Instructions, inst);
					} else if (instr->unary.op == TACKY_COMPLEMENT) {
						X64Instruction inst = {
							.type = X64_NOT,
							.src = (Operand){ .type = OPERAND_REGISTER, .regName = "%eax" }
						};
						arrput(x64Instructions, inst);
					}

					// Store %eax back to stack
					X64Instruction inst = {
						.type = X64_MOV,
						.src = (Operand){ .type = OPERAND_REGISTER, .regName = "%eax" },
						.dst = (Operand){ .type = OPERAND_STACK_SLOT, .stackOffset = dstOffset }
					};
					arrput(x64Instructions, inst);
					break;
				}

				case TACKY_INSTR_RETURN: {
					if (instr->ret.value.type == TACKY_VAL_CONSTANT) {
						X64Instruction inst = {
							.type = X64_MOV,
							.src = (Operand){ .type = OPERAND_IMM, .immValue = instr->ret.value.constantValue },
							.dst = (Operand){ .type = OPERAND_REGISTER, .regName = "%eax" }
						};
						arrput(x64Instructions, inst);
					} else {
						int srcOffset = getStackOffsetForTmp(instr->ret.value.varName);
						X64Instruction inst = {
							.type = X64_MOV,
							.src = (Operand){ .type = OPERAND_STACK_SLOT, .stackOffset = srcOffset },
							.dst = (Operand){ .type = OPERAND_REGISTER, .regName = "%eax" }
						};
						arrput(x64Instructions, inst);
					}

					X64Instruction inst = { .type = X64_RET };
					arrput(x64Instructions, inst);
					break;
				}
			}
		}

		asmFunc.instructions = x64Instructions;
		asmFunc.instructionCount = arrlenu(x64Instructions);
		arrput(asmProgram->functions, asmFunc);
		asmProgram->functionCount = arrlenu(asmProgram->functions);
		arrfree(tmpMappings);
	}
}
#endif
