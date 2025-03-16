//
//  ast_asm_common.c
//  VectorC
//
//  Created by Claire Rogers on 05/01/2025.
//

#include "ast_asm_common.h"
#include "ast_x64.h"
#include "ast_arm64.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

Function createFunction(const char* name, void* instructions, size_t instructionCount, Architecture arch) {
	Function func;
	func.name = strdup(name);
	func.instructions = instructions;
	func.instructionCount = instructionCount;
	func.arch = arch;
	return func;
}

Program createProgram(Function* functions, size_t functionCount) {
	Program prog;
	prog.functions = functions;
	prog.functionCount = functionCount;
	return prog;
}

// Create an immediate operand
Operand createImmOperand(int value) {
	Operand op;
	op.type = OPERAND_IMM;
	op.immValue = value;
	return op;
}

// Create a register operand
Operand createRegisterOperand(const char* regName) {
	Operand op;
	op.type = OPERAND_REGISTER;
	op.regName = regName;
	return op;
}

const char* getArchitectureName(Architecture arch)
{
	static const char* s_architectureNames[] = {
		[ARCH_X64] = "x86_64",
		[ARCH_ARM64] = "arm64",
		[ARCH_RISCV] = "riscv",
//		[ARCH_UNKNOWN] = "unknown",
	};

	static_assert(sizeof(s_architectureNames) / sizeof(const char*) == (int32_t)ARCH_UNKNOWN, "Invalid Architecture");
	return s_architectureNames[arch];
}

#if 0
void generateCode(const Program* program, const char* outputFilename) {
	if (!program || program->functionCount == 0) {
		fprintf(stderr, "Error: No functions to generate code for.\n");
		return;
	}

	FILE* outputFile = fopen(outputFilename, "w");
	if (!outputFile) {
		perror("Error opening output file");
		exit(EXIT_FAILURE);
	}

	for (size_t i = 0; i < program->functionCount; i++) {
		const Function* func = &program->functions[i];

		// Use _main for macOS, main for Linux
		if (strcmp(func->name, "main") == 0) {
			fprintf(outputFile, ".global _main\n");
			fprintf(outputFile, "_main:\n");
		} else {
			fprintf(outputFile, ".global %s\n", func->name);
			fprintf(outputFile, "%s:\n", func->name);
		}

		// ✅ **Function Prologue (Ensures Stack Frame Setup)**
		fprintf(outputFile, "    pushq %%rbp\n");   // Save caller's base pointer
		fprintf(outputFile, "    movq %%rsp, %%rbp\n"); // Create a new stack frame
		fprintf(outputFile, "    subq $16, %%rsp\n");  // Ensure stack alignment (16-byte)

		switch (func->arch) {
			case ARCH_X64: {
				const X64Instruction* instructions = (const X64Instruction*)func->instructions;
				for (size_t j = 0; j < func->instructionCount; j++) {
					const X64Instruction* instr = &instructions[j];
					char srcBuffer[32], dstBuffer[32];
					getX64Operand(&instr->src, srcBuffer, sizeof(srcBuffer));
					getX64Operand(&instr->dst, dstBuffer, sizeof(dstBuffer));

					switch (instr->type) {
						case X64_MOV:
							fprintf(outputFile, "    movl $%s, %s\n", srcBuffer, dstBuffer);
							break;
						case X64_RET:
							// ✅ **Function Epilogue (Restores Stack)**
							fprintf(outputFile, "    movq %%rbp, %%rsp\n");  // Restore stack pointer
							fprintf(outputFile, "    popq %%rbp\n");  // Restore old base pointer
							fprintf(outputFile, "    ret\n");  // Return to caller
							break;
					}
				}
				break;
			}
			case ARCH_ARM64: {
				const ARM64Instruction* instructions = (const ARM64Instruction*)func->instructions;
				for (size_t j = 0; j < func->instructionCount; j++) {
					const ARM64Instruction* instr = &instructions[j];
					char srcBuffer[32], dstBuffer[32];
					getARM64Operand(&instr->src, srcBuffer, sizeof(srcBuffer));
					getARM64Operand(&instr->dst, dstBuffer, sizeof(dstBuffer));

					switch (instr->type) {
						case ARM64_MOV:
							fprintf(outputFile, "    mov %s, %s\n", dstBuffer, srcBuffer);
							break;
						case ARM64_RET:
							// ✅ ARM64 uses `ret` without prologue/epilogue, but stack alignment might be needed
							fprintf(outputFile, "    ret\n");
							break;
					}
				}
				break;
			}
			default:
				fprintf(stderr, "Error: Unsupported architecture.\n");
				fclose(outputFile);
				return;
		}

		fprintf(outputFile, "\n");
	}

	fclose(outputFile);
}
#endif

void generateCode(const Program* program, const char* outputFilename)
{
	if (!program || program->functionCount == 0) {
		fprintf(stderr, "Error: No functions to generate code for.\n");
		return;
	}

	FILE* outputFile = fopen(outputFilename, "w");
	if (!outputFile) {
		perror("Error opening output file");
		exit(EXIT_FAILURE);
	}

	// Iterate over each function in the Program
	for (size_t i = 0; i < program->functionCount; i++) {
		const Function* func = &program->functions[i];

		switch (func->arch) {
			case ARCH_X64:
				generateX64Function(outputFile, func);
				break;

			case ARCH_ARM64:
				generateARM64Function(outputFile, func);
				break;

			default:
				fprintf(stderr, "Error: Unsupported architecture.\n");
				fclose(outputFile);
				return;
		}
	}

	fclose(outputFile);
}
// Print a program, dispatching based on architecture
void printAsmProgram(const Program* program)
{
	for (size_t i = 0; i < program->functionCount; i++) {
		const Function* func = &program->functions[i];
		printf("Function %s:\n", func->name);

		switch (func->arch) {
			case ARCH_X64:
				printX64Function(func);
				break;
			case ARCH_ARM64:
				printARM64Function(func);
				break;
			default:
				printf("Unknown architecture\n");
				break;
		}
	}
}
