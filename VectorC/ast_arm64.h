//
//  ast_arm64.h
//  VectorC
//
//  Created by Claire Rogers on 05/01/2025.
//

#ifndef ast_arm64_h
#define ast_arm64_h

#include "ast_asm_common.h"

// ARM64 instruction types
typedef enum {
	ARM64_MOV,
	ARM64_RET,
	// Add more ARM64-specific instructions
} ARM64InstructionType;

// ARM64 instruction structure
typedef struct ARM64Instruction {
	ARM64InstructionType type;
	Operand src; // Source operand
	Operand dst; // Destination operand
} ARM64Instruction;

// Function declarations for ARM64 code generation
const char* getARM64Operand(const Operand* op, char* buffer, size_t bufferSize);
void generateARM64Code(const ARM64Instruction* instr);
void printARM64Function(const Function* function);

#endif /* ast_arm64_h */
