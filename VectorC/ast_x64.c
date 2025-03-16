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

//---------------------------------------------------------
// X64 CODEGEN
//---------------------------------------------------------
void generateX64Function(FILE* outputFile, const Function* func)
{
	// Decide function label
	// Typically: `_main` on macOS or `main` on Linux.
	// This example just demonstrates the logic:
	if (strcmp(func->name, "main") == 0) {
		fprintf(outputFile, ".global _main\n");
		fprintf(outputFile, "_main:\n");
	} else {
		fprintf(outputFile, ".global %s\n", func->name);
		fprintf(outputFile, "%s:\n", func->name);
	}

	// X86-64 prologue
	fprintf(outputFile, "    pushq %%rbp\n");
	fprintf(outputFile, "    movq %%rsp, %%rbp\n");
	fprintf(outputFile, "    subq $16, %%rsp\n"); // example stack allocation

	// Emit instructions
	const X64Instruction* instructions = (const X64Instruction*)func->instructions;
	for (size_t j = 0; j < func->instructionCount; j++) {
		const X64Instruction* instr = &instructions[j];

		char srcBuffer[32], dstBuffer[32];
		getX64Operand(&instr->src, srcBuffer, sizeof(srcBuffer));
		getX64Operand(&instr->dst, dstBuffer, sizeof(dstBuffer));

		switch (instr->type) {
			case X64_MOV:
				// Example: move immediate into a register or memory
				fprintf(outputFile, "    movl $%s, %s\n", srcBuffer, dstBuffer);
				break;

			case X64_RET:
				// X86-64 epilogue
				fprintf(outputFile, "    movq %%rbp, %%rsp\n");
				fprintf(outputFile, "    popq %%rbp\n");
				fprintf(outputFile, "    ret\n");
				break;
		}
	}

	fprintf(outputFile, "\n"); // Blank line between functions
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
