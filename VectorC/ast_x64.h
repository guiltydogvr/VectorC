//
//  ast_x64.h
//  VectorC
//
//  Created by Claire Rogers on 05/01/2025.
//

#ifndef ast_x64_h
#define ast_x64_h

#include "ast_asm_common.h"

// x64 instruction types
typedef enum {
	X64_ADD,
	X64_CDQ,
	X64_IMUL,
	X64_IDIV,
	X64_MOV,
	X64_NEG,
	X64_NOT,
	X64_RET,
	X64_SUB,
	// Add more x64-specific instructions
} X64InstructionType;

// x64 instruction structure
typedef struct X64Instruction {
	X64InstructionType type;
	Operand src; // Source operand
	Operand dst; // Destination operand
} X64Instruction;

// Function declarations for x64 code generation
void getX64Operand(const Operand* op, char* buffer, size_t bufferSize);
int getOrAssignStackOffset(const char* tmpName);
void generateX64Function(FILE* outputFile, const Function* func);
//void emitX64(X64InstructionType op, Operand src, Operand dst);
void printX64Function(const Function* function);

#endif /* ast_x64_h */
