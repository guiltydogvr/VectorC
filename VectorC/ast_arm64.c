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
		case OPERAND_VARNAME:
			snprintf(buffer, bufferSize, "%s", op->varName);
			break;
		case OPERAND_STACK_SLOT:
			snprintf(buffer, bufferSize, "[fp, -%d]", op->stackOffset);
			break;
		case OPERAND_REGISTER:
			snprintf(buffer, bufferSize, "%s", op->regName);
			break;
	}
	return buffer;
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

	// ARM64 prologue
	// Typically: Save x29 (frame pointer) and x30 (link register)
	fprintf(outputFile, "    stp x29, x30, [sp, -16]!\n");
	fprintf(outputFile, "    mov x29, sp\n");
	// Reserve local stack space if needed:
	// fprintf(outputFile, "    sub sp, sp, #16\n");

	// Emit instructions
	const ARM64Instruction* instructions = (const ARM64Instruction*)func->instructions;
	for (size_t j = 0; j < func->instructionCount; j++) {
		const ARM64Instruction* instr = &instructions[j];

		char srcBuffer[32], dstBuffer[32];
		getARM64Operand(&instr->src, srcBuffer, sizeof(srcBuffer));
		getARM64Operand(&instr->dst, dstBuffer, sizeof(dstBuffer));

		switch (instr->type) {
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
			case ARM64_NEG:
				fprintf(outputFile, "    neg %s, %s\n", dstBuffer, srcBuffer);
				break;
			case ARM64_NOT:
				fprintf(outputFile, "    mvn %s, %s\n", dstBuffer, srcBuffer);
				break;
			case ARM64_RET:
				// ARM64 epilogue
				fprintf(outputFile, "    ldp x29, x30, [sp], #16\n");
				fprintf(outputFile, "    ret\n");
				break;
		}
	}

	fprintf(outputFile, "\n"); // Blank line between functions
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
			case ARM64_NOT:
				printf("  mvn %s, %s\n", getARM64Operand(&instr->dst, dstBuffer, sizeof(dstBuffer)), getARM64Operand(&instr->src, srcBuffer, sizeof(srcBuffer)));
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
