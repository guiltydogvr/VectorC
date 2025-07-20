//
//  translate_tacky_x64.c
//  VectorC
//
//  Created by Claire Rogers on 09/07/2025.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "translate_tacky_x64.h"
#include "ast_x64.h"
#include "stb_ds.h"

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
				int offset = getOrAssignStackOffset(instr->src.varName);
				instr->src = SLOT(offset);
			}

			if (instr->dst.type == OPERAND_VARNAME) {
				int offset = getOrAssignStackOffset(instr->dst.varName);
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
