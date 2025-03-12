//
//  ast_asm_common.h
//  VectorC
//
//  Created by Claire Rogers on 05/01/2025.
//

#ifndef ast_asm_common_h
#define ast_asm_common_h

#include <stdio.h>
#include <stdint.h>

typedef enum {
	ARCH_X64,
	ARCH_ARM64,
	// Future architectures
	ARCH_RISCV,
	ARCH_UNKNOWN
} Architecture;

typedef struct Function {
	const char* name;    		// Function name
	void* instructions;  		// Dynamic array of instructions (x64 or ARM64)
	size_t instructionCount;	// Number of instructions
	Architecture arch;
} Function;

typedef struct Program {
	Function* functions;  		// Array of functions
	size_t functionCount; 		// Number of functions
} Program;

// Shared constructor functions
Function createFunction(const char* name, void* instructions, size_t instructionCount, Architecture arch);
Program createProgram(Function* functions, size_t functionCount);

// Operand types (shared across architectures)
typedef enum {
	OPERAND_IMM,
	OPERAND_REGISTER,
} OperandType;

// Operand structure
typedef struct Operand {
	OperandType type;
	union {
		int immValue;        // Immediate value
		const char* regName; // Register name
	};
} Operand;

// Helper functions for operands
Operand createImmOperand(int value);
Operand createRegisterOperand(const char* regName);

void generateCode(const Program* program, const char* outputFilename);
void printAsmProgram(const Program* program);

#endif /* ast_asm_common_h */
