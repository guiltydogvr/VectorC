//
//  tacky.h
//  VectorC
//
//  Created by Claire Rogers on 09/07/2025.
//


#ifndef TACKY_H
#define TACKY_H

#include <stdint.h>

#include "ast_c.h"
// --------------------------------------------------
// Enums
// --------------------------------------------------

typedef enum {
    TACKY_COMPLEMENT,
    TACKY_NEGATE
} TackyUnaryOperator;

typedef enum {
	TACKY_ADD,
	TACKY_SUBTRACT,
	TACKY_MULTIPLY,
	TACKY_DIVIDE,
	TACKY_MODULO
} TackyBinaryOperator;

typedef enum {
    TACKY_VAL_CONSTANT,
    TACKY_VAL_VAR
} TackyValueType;

// --------------------------------------------------
// Tacky Value (operand)
// --------------------------------------------------

typedef struct {
    TackyValueType type;
    union {
        int constantValue;    // for TACKY_VAL_CONSTANT
        const char* varName;  // for TACKY_VAL_VAR
    };
} TackyValue;

// --------------------------------------------------
// Tacky Instructions
// --------------------------------------------------

typedef enum {
    TACKY_INSTR_RETURN,
    TACKY_INSTR_UNARY,
	TACKY_INSTR_BINARY,
} TackyInstructionType;

typedef struct {
    TackyInstructionType type;

    union {
        struct {
            TackyValue value;
        } ret;

        struct {
            TackyUnaryOperator op;
            TackyValue src;
            TackyValue dst;
        } unary;
		struct {
			TackyBinaryOperator op;
			TackyValue dst;
			TackyValue lhs;
			TackyValue rhs;
		} binary;
    };
} TackyInstruction;

// --------------------------------------------------
// Tacky Function
// --------------------------------------------------

typedef struct {
    const char* name;
    TackyInstruction* instructions;  // stb_ds dynamic array
} TackyFunction;

// --------------------------------------------------
// Tacky Program
// --------------------------------------------------

typedef struct {
    TackyFunction* functions;  // stb_ds dynamic array
} TackyProgram;

TackyProgram* generateTackyFromAst(const ProgramNode* ast);
void printTackyProgram(const TackyProgram* program);

#endif // TACKY_H
