//
//  ast_arm64.h
//  VectorC
//
//  Created by Claire Rogers on 05/01/2025.
//

#ifndef ast_arm64_h
#define ast_arm64_h

#include "ast_asm_common.h"
#include "tacky.h"
#include "stb_ds.h"

// ARM64 instruction types
typedef enum {
	// Arithmetic
	ARM64_ADD,
	ARM64_SUB,
	ARM64_MUL,
	ARM64_SDIV,

	// Bitwise logical
	ARM64_AND,
	ARM64_ORR,
	ARM64_EOR,
	ARM64_MVN,   // bitwise NOT

	// Shifts (immediate)
	ARM64_LSL,   // lsl wd, wn, #imm
	ARM64_LSR,
	ARM64_ASR,   // asr wd, wn, #imm

	// Shifts (variable)
	ARM64_LSLV,  // lslv wd, wn, wm
	ARM64_LSRV,
	ARM64_ASRV,  // asrv wd, wn, wm

	// Moves and neg
	ARM64_MOV,
	ARM64_NEG,

	// Load/store
	ARM64_LDR,
	ARM64_STR,

	// Control flow
	ARM64_RET
} ARM64InstructionType;
// ARM64 instruction structure
typedef struct ARM64Instruction {
	ARM64InstructionType type;
	Operand src; // Source operand
	Operand src1;
	Operand dst; // Destination operand
} ARM64Instruction;

// Function declarations for ARM64 code generation
const char* getARM64Operand(const Operand* op, char* buffer, size_t bufferSize);
void generateARM64Function(FILE* outputFile, const Function* func);
void translateTackyToARM64(const TackyProgram* tackyProgram, Program* asmProgram);
void replacePseudoRegistersARM64(Program* asmProgram);
void fixupIllegalInstructionsARM64(Program* asmProgram, Program* finalAsmProgram);
void printARM64Function(const Function* function);

#endif /* ast_arm64_h */
