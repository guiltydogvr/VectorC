//
//  ast_arm64.c
//  VectorC
//
//  Created by Claire Rogers on 05/01/2025.
//

#include "ast_arm64.h"
#include <stdio.h>

// Convert an operand to a string for ARM64 (helper function)
const char* getARM64Operand(const Operand* op, char* buffer, size_t bufferSize) {
	switch (op->type) {
		case OPERAND_IMM:
			snprintf(buffer, bufferSize, "#%d", op->immValue);
			break;
		case OPERAND_REGISTER:
			snprintf(buffer, bufferSize, "%s", op->regName);
			break;
	}
	return buffer;
}

// Generate ARM64 code for an instruction
void generateARM64Code(const ARM64Instruction* instr) {
	char srcBuffer[32];
	char dstBuffer[32];

	switch (instr->type) {
		case ARM64_MOV:
			printf("mov %s, %s\n", getARM64Operand(&instr->dst, dstBuffer, sizeof(dstBuffer)), getARM64Operand(&instr->src, srcBuffer, sizeof(srcBuffer)));
			break;
		case ARM64_RET:
			printf("ret\n");
			break;
	}
}

void printARM64Function(const Function* function)
{
	const ARM64Instruction* instructions = (const ARM64Instruction*)function->instructions;
	char srcBuffer[32];
	char dstBuffer[32];

	for (size_t i = 0; i < function->instructionCount; i++) {
		const ARM64Instruction* instr = &instructions[i];
		switch (instr->type) {
			case ARM64_MOV:
				printf("  mov %s, %s\n", getARM64Operand(&instr->dst, dstBuffer, sizeof(dstBuffer)), getARM64Operand(&instr->src, srcBuffer, sizeof(srcBuffer)));
				break;
			case ARM64_RET:
				printf("  ret\n");
				break;
			default:
				printf("  Unknown instruction\n");
				break;
		}
	}
}
