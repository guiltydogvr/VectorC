//
//  translate_tacky_arm64.h
//  VectorC
//
//  Created by Claire Rogers on 10/07/2025.
//


#ifndef TRANSLATE_TACKY_ARM64_H
#define TRANSLATE_TACKY_ARM64_H

#include "tacky.h"
#include "ast_asm_common.h"

void translateTackyToARM64(const TackyProgram* tackyProgram, Program* asmProgram);

#endif
