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
