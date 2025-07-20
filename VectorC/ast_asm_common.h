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
#include <string.h>

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

// Operand types (shared across architectures)
typedef enum {
	OPERAND_IMM,
	OPERAND_VARNAME,
	OPERAND_STACK_SLOT,
	OPERAND_REGISTER
} OperandType;

typedef struct {
	OperandType type;
	union {
		int immValue;        // Immediate value
		int stackOffset;
		union {
			const char* varName; // Temporary variable name (from Tacky).
			const char* regName; // Register name
		};
	};
} Operand;

typedef struct {
	const char* tmpName;
	int stackOffset;  // e.g., 4, 8, etc.
} TmpMapping;

const char* getArchitectureName(Architecture arch);
void generateCode(const Program* program, const char* outputFilename);
void printAsmProgram(const Program* program);

inline int alignTo(int value, int alignment) {
	return (value + alignment - 1) & ~(alignment - 1);
}

#endif /* ast_asm_common_h */
