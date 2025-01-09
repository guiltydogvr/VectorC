//
//  translate.h
//  VectorC
//
//  Created by Claire Rogers on 05/01/2025.
//

#ifndef translate_h
#define translate_h

#include "ast_c.h"
#include "ast_x64.h"
#include "ast_arm64.h"
#include "ast_asm_common.h"

void translateFunctionToX64(const FunctionNode* cFunction, Function* asmFunction);
void translateFunctionToARM64(const FunctionNode* cFunction, Function* asmFunction);
void translateReturnToX64(const StatementNode* stmt, X64Instruction** instructions, size_t* count);
void translateReturnToARM64(const StatementNode* stmt, ARM64Instruction** instructions, size_t* count);
void translateProgramToX64(const ProgramNode* cProgram, Program* asmProgram);
void translateProgramToARM64(const ProgramNode* cProgram, Program* asmProgram);

#endif /* translate_h */
