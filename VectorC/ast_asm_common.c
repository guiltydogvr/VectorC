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
