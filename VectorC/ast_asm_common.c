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

extern inline int alignTo(int value, int alignment);

// Map an Architecture enum to a human readable string.
//
// arch - Architecture value.
// returns - String name for the architecture.
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

// Write assembly code for all functions in a Program to the specified file.
//
// program        - Input assembly program.
// outputFilename - Target file path.
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

// Dump the contents of a Program to stdout, selecting the
// appropriate printer for each function's architecture.
//
// program - Program to print.
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
