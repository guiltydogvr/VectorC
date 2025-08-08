//
//  ast_x64.h
//  VectorC
//
//  Created by Claire Rogers on 05/01/2025.
//

#ifndef ast_x64_h
#define ast_x64_h

#include "ast_asm_common.h"
#include "tacky.h"
#include "stb_ds.h"

// x64 instruction types
typedef enum {
	X64_ADD,
	X64_AND,
	X64_CDQ,
	X64_IMUL,
	X64_IDIV,
	X64_MOV,
	X64_NEG,
	X64_NOT,
	X64_OR,
	X64_RET,
	X64_SUB,
	X64_XOR,
	// Shifts
	X64_SHL_IMM, // shl $imm, r/m32
	X64_SHL_CL,  // shl %cl, r/m32
	X64_SAR_IMM, // sar $imm, r/m32 (signed >>)
	X64_SAR_CL,  // sar %cl, r/m32 (signed >>)
	// Later if you add unsigned >>:
//	X64_SHR_IMM, // shr $imm, r/m32
//	X64_SHR_CL   // shr %cl, r/m32
} X64InstructionType;

// x64 instruction structure
typedef struct X64Instruction {
	X64InstructionType type;
	Operand src; // Source operand
	Operand dst; // Destination operand
} X64Instruction;

// Function declarations for x64 code generation
void getX64Operand(const Operand* op, char* buffer, size_t bufferSize);
int getOrAssignStackOffsetX64(const char* tmpName);
void generateX64Function(FILE* outputFile, const Function* func);
void translateTackyToX64(const TackyProgram* tackyProgram, Program* asmProgram);
void replacePseudoRegistersX64(Program* asmProgram);
void fixupIllegalInstructionsX64(Program* asmProgram, Program* finalAsmProgram);
void printX64Function(const Function* function);

#endif /* ast_x64_h */
