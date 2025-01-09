//
//  parser.c
//  VectorC
//
//  Created by Claire Rogers on 04/01/2025.
//

#include "parser.h"
#include "token.h"
#include "stb_ds.h"

// Get the current token
static const Token* currentToken(Parser* parser) {
	return &parser->tokens[parser->current];
}

// Advance to the next token
static void advance(Parser* parser) {
	if (currentToken(parser)->type != TOKEN_EOF) {
		parser->current++;
	}
}

// Check if the current token matches a type
static int match(Parser* parser, TokenType type) {
	if (currentToken(parser)->type == type) {
		advance(parser);
		return 1;
	}
	return 0;
}

ExpressionNode* parseExpression(Parser* parser) {
	if (match(parser, TOKEN_NUMBER)) {
		const Token* token = &parser->tokens[parser->current - 1];
		return createConstantNode(token->value.intValue);
	}
	printf("Error: Expected a number, got '%s'\n", currentToken(parser)->start);
	exit(EXIT_FAILURE);
}

StatementNode* parseStatement(Parser* parser) {
	if (match(parser, TOKEN_RETURN)) {
		ExpressionNode* expr = parseExpression(parser);
		if (!match(parser, TOKEN_SEMICOLON)) {
			printf("Error: Expected ';' after return expression.\n");
			exit(EXIT_FAILURE);
		}
		return createReturnStatementNode(expr);
	}
	printf("Error: Unexpected token '%s'\n", currentToken(parser)->start);
	exit(EXIT_FAILURE);
}

StatementNode* parseStatementList(Parser* parser) {
	if (match(parser, TOKEN_RIGHT_BRACE)) {
		return NULL; // Empty statement list
	}

	StatementNode* stmt = parseStatement(parser);
/*
	StatementNode* nextStmt = parseStatementList(parser);
	if (nextStmt) {
		// Append next statement to the current one
		stmt->next = nextStmt; // Add `next` pointer to StatementNode struct
	}
*/
	return stmt;
}

FunctionNode* parseFunction(Parser* parser) {
	// Parse return type
	if (!match(parser, TOKEN_INT) && !match(parser, TOKEN_VOID)) {
		printf("Error: Expected return type ('int' or 'void').\n");
		exit(EXIT_FAILURE);
	}
//	const Token* typeToken = &parser->tokens[parser->current - 1];

	// Parse function name
	if (!match(parser, TOKEN_IDENTIFIER)) {
		printf("Error: Expected function name.\n");
		exit(EXIT_FAILURE);
	}
	const Token* nameToken = &parser->tokens[parser->current - 1];

	// Parse parameters
	if (!match(parser, TOKEN_LEFT_PAREN)) {
		printf("Error: Expected '(' after function name.\n");
		exit(EXIT_FAILURE);
	}
	if (!match(parser, TOKEN_VOID)) { // For simplicity, assume 'void' only
		printf("Error: Only 'void' parameters supported for now.\n");
		exit(EXIT_FAILURE);
	}
	if (!match(parser, TOKEN_RIGHT_PAREN)) {
		printf("Error: Expected ')' after parameter list.\n");
		exit(EXIT_FAILURE);
	}

	// Parse body
	if (!match(parser, TOKEN_LEFT_BRACE)) {
		printf("Error: Expected '{' to start function body.\n");
		exit(EXIT_FAILURE);
	}
	StatementNode* body = parseStatementList(parser);

	char functionName[nameToken->length+1];
	strncpy(functionName, nameToken->start, nameToken->length);
	functionName[nameToken->length] = '\0';
	return createFunctionNode(functionName, body);
}
ProgramNode* parseProgram(Parser* parser) {
	FunctionNode* function = parseFunction(parser);
	return createProgramNode(function);
}

ProgramNode* parseProgramTokens(const Token* tokens) {
	Parser parser = {tokens, 0};
	return parseProgram(&parser);
}
