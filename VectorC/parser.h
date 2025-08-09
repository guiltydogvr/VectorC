//
//  parser.h
//  VectorC
//
//  Created by Claire Rogers on 04/01/2025.
//

#ifndef parser_h
#define parser_h

#include <stdio.h>
#include "token.h"
#include "ast_c.h"

// Parser state carrying the token array and current index.
typedef struct {
	const Token* tokens;
	size_t current; // Current token index
} Parser;

// Parse an expression starting at the current token with a minimum precedence.
ExpressionNode* parseExpression(Parser* parser, int minPrec);

// Parse a single factor (number, grouped expression, or unary op).
ExpressionNode* parseFactor(Parser* parser);

// Parse a standalone statement.
StatementNode* parseStatement(Parser* parser);

// Parse a full function definition.
FunctionNode* parseFunction(Parser* parser);

// Parse an entire program containing multiple functions.
ProgramNode* parseProgram(Parser* parser);

// Helper to parse a program directly from a token array.
ProgramNode* parseProgramTokens(const Token* tokens);

#endif /* parser_h */
