//
//  translate_tacky_x64.h
//  VectorC
//
//  Created by Claire Rogers on 09/07/2025.
//


#ifndef TRANSLATE_TACKY_X64_H
#define TRANSLATE_TACKY_X64_H

#include "tacky.h"
#include "ast_asm_common.h"

void translateTackyToX64(const TackyProgram* tackyProgram, Program* asmProgram);

#endif