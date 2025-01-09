//
//  ast_x64.c
//  VectorC
//
//  Created by Claire Rogers on 05/01/2025.
//

#include "ast_x64.h"
#include <stdio.h>

// Convert an operand to a string for x64 (helper function)
void getX64Operand(const Operand* op, char* buffer, size_t bufferSize) {
	switch (op->type) {
		case OPERAND_IMM:
			snprintf(buffer, bufferSize, "%d", op->immValue);
			break;
		case OPERAND_REGISTER:
			snprintf(buffer, bufferSize, "%s", op->regName);
			break;
	}
}

// Generate x64 code for an instruction
void generateX64Code(const X64Instruction* instr) {
	char srcBuffer[32];
	char dstBuffer[32];
	
	switch (instr->type) {
		case X64_MOV:
			getX64Operand(&instr->src, srcBuffer, sizeof(srcBuffer));
			getX64Operand(&instr->dst, dstBuffer, sizeof(dstBuffer));
			printf("mov %s, %s\n", dstBuffer, srcBuffer);
			break;
		case X64_RET:
			printf("ret\n");
			break;
	}
}

void printX64Function(const Function* function) {
	const X64Instruction* instructions = (const X64Instruction*)function->instructions;
	char srcBuffer[32];
	char dstBuffer[32];

	for (size_t i = 0; i < function->instructionCount; i++) {
		const X64Instruction* instr = &instructions[i];
		switch (instr->type) {
			case X64_MOV:
				getX64Operand(&instr->src, srcBuffer, sizeof(srcBuffer));
				getX64Operand(&instr->dst, dstBuffer, sizeof(dstBuffer));
				printf("  mov %s, %s\n", dstBuffer, srcBuffer);
				break;
			case X64_RET:
				printf("  ret\n");
				break;
			default:
				printf("  Unknown instruction\n");
				break;
		}
	}
}
