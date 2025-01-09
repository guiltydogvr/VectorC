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

typedef struct {
	const Token* tokens;
	size_t current; // Current token index
} Parser;

ExpressionNode* parseExpression(Parser* parser);
StatementNode* parseStatement(Parser* parser);
FunctionNode* parseFunction(Parser* parser);
ProgramNode* parseProgram(Parser* parser);
ProgramNode* parseProgramTokens(const Token* tokens);

#endif /* parser_h */
