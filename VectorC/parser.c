//
//  parser.c
//  VectorC
//
//  Created by Claire Rogers on 04/01/2025.
//

#include "parser.h"
#include "token.h"
#include "stb_ds.h"

// Return a pointer to the current token in the stream.
// parser - parser state tracking the token array and index.
// Returns: pointer to current Token.
static const Token* currentToken(Parser* parser) {
	return &parser->tokens[parser->current];
}

// Move to the next token if not already at EOF.
// parser - parser state to advance.
static void advance(Parser* parser) {
	if (currentToken(parser)->type != TOKEN_EOF) {
		parser->current++;
	}
}

// Consume a token of the given type if present.
// parser - parser state.
// type   - token type to match.
// Returns: 1 if the token was consumed, 0 otherwise.
static int match(Parser* parser, TokenType type) {
	if (currentToken(parser)->type == type) {
		advance(parser);
		return 1;
	}
	return 0;
}
/*
// Get operator precedence for a token type. Higher numbers bind more tightly.
// type - token to query.
// Returns: integer precedence value, or 0 if not an operator.
static int getPrecedence(TokenType type) {
	switch (type) {
		case TOKEN_STAR:
		case TOKEN_SLASH:
		case TOKEN_MOD:
			return 10;
		case TOKEN_PLUS:
		case TOKEN_MINUS:
			return 5;
		default:
			return 0;
	}
}
*/
static int getPrecedence(TokenType type) {
	switch (type) {
		case TOKEN_STAR:    // *
		case TOKEN_SLASH:   // /
		case TOKEN_MOD:     // %
			return 20;

		case TOKEN_PLUS:    // +
		case TOKEN_MINUS:   // -
			return 15;

		case TOKEN_SHIFT_LEFT:   // <<
		case TOKEN_SHIFT_RIGHT:  // >>
			return 14;

		case TOKEN_AND:     // &
			return 13;

		case TOKEN_XOR:     // ^
			return 12;

		case TOKEN_OR:      // |
			return 11;

//		case TOKEN_LOGICAL_AND: // &&
//			return 10;

//		case TOKEN_LOGICAL_OR:  // ||
//			return 10;

		default:
			return 0; // Not a binary operator
	}
}

// Determine if a token represents a binary operator.
// Returns: non-zero if token has defined precedence.
static int isBinaryOperator(TokenType type) {
	return getPrecedence(type) > 0;
}

// Parse an expression using precedence climbing.
// parser - parser state.
// minPrec - minimum precedence level to parse.
// Returns: AST node representing the expression.
ExpressionNode* parseExpression(Parser* parser, int minPrec) {
	ExpressionNode* left = NULL;

	// Parse factor or unary
	if (match(parser, TOKEN_LEFT_PAREN)) {
		left = parseExpression(parser, 0);
		if (!match(parser, TOKEN_RIGHT_PAREN)) {
			printf("Error: Expected ')'\n");
			exit(EXIT_FAILURE);
		}
	}
	else if (match(parser, TOKEN_TILDE)) {
		left = createUnaryNode(UNARY_COMPLEMENT, parseExpression(parser, 100));
	}
	else if (match(parser, TOKEN_MINUS)) {
		left = createUnaryNode(UNARY_NEGATE, parseExpression(parser, 100));
	}
	else if (match(parser, TOKEN_NUMBER)) {
		const Token* previousToken = &parser->tokens[parser->current - 1];
		left = createIntConstant(previousToken->value.intValue);
	}
	else {
		printf("Error: ...");
		exit(EXIT_FAILURE);
	}

	// Parse binary ops using precedence climbing
	while (isBinaryOperator(currentToken(parser)->type) &&
		   getPrecedence(currentToken(parser)->type) >= minPrec) {

		BinaryOperator op;
		switch (currentToken(parser)->type) {
			case TOKEN_AND:
				op = BINOP_BITWISE_AND;
				break;
			case TOKEN_PLUS:
				op = BINOP_ADD;
				break;
			case TOKEN_MINUS:
				op = BINOP_SUBTRACT;
				break;
			case TOKEN_MOD:
				op = BINOP_MODULO;
				break;
			case TOKEN_OR:
				op = BINOP_BITWISE_OR;
				break;
			case TOKEN_SHIFT_LEFT:
				op = BINOP_SHIFT_LEFT;
				break;
			case TOKEN_SHIFT_RIGHT:
				op = BINOP_SHIFT_RIGHT;
				break;
			case TOKEN_STAR:
				op = BINOP_MULTIPLY;
				break;
			case TOKEN_SLASH:
				op = BINOP_DIVIDE;
				break;
			case TOKEN_XOR:
				op = BINOP_BITWISE_XOR;
				break;
			default:
				printf("Unknown binary operator\n");
				exit(EXIT_FAILURE);
		}
		int prec = getPrecedence(currentToken(parser)->type);
		advance(parser);
		ExpressionNode* right = parseExpression(parser, prec + 1);
		left = createBinaryNode(op, left, right);
	}
	return left;
}

// Parse the smallest units of expressions (numbers, grouped or unary ops).
// Returns: AST node representing the factor.
ExpressionNode* parseFactor(Parser* parser) {
	if (match(parser, TOKEN_LEFT_PAREN)) {  // Handle grouped expressions
		ExpressionNode* expr = parseExpression(parser, 0);  // Parse inside parentheses
		if (!match(parser, TOKEN_RIGHT_PAREN)) {  // Ensure closing `)`
			printf("Error: Expected closing ')'\n");
			exit(EXIT_FAILURE);
		}
		return expr;
	}
	else if (match(parser, TOKEN_TILDE)) {  // Bitwise NOT (~)
		ExpressionNode* operand = parseFactor(parser);
		return createUnaryNode(UNARY_COMPLEMENT, operand);
	}
	else if (match(parser, TOKEN_MINUS)) {  // Negation (-)
		ExpressionNode* operand = parseFactor(parser);
		return createUnaryNode(UNARY_NEGATE, operand);
	}
	else if (match(parser, TOKEN_NUMBER)) {  // Constant numbers
		const Token* token = &parser->tokens[parser->current - 1];
		return createIntConstant(token->value.intValue);
	}

	printf("Error: Expected a number or unary operator, got '%s'\n", currentToken(parser)->start);
	exit(EXIT_FAILURE);
}

// Parse a single statement such as a return statement.
// Returns: AST node for the parsed statement.
StatementNode* parseStatement(Parser* parser) {
	if (match(parser, TOKEN_RETURN)) {
		ExpressionNode* expr = parseExpression(parser, 0);
		if (!match(parser, TOKEN_SEMICOLON)) {
			printf("Error: Expected ';' after return expression.\n");
			exit(EXIT_FAILURE);
		}
		return createReturnStatementNode(expr);
	}
	printf("Error: Unexpected token '%s'\n", currentToken(parser)->start);
	exit(EXIT_FAILURE);
}

// Parse a block of statements delimited by braces.
// Returns: pointer to first statement node or NULL if block is empty.
StatementNode* parseStatementList(Parser* parser) {
	if (match(parser, TOKEN_RIGHT_BRACE)) {
		return NULL; // Empty statement list, but now we properly consume the '}'
	}

	StatementNode* stmt = parseStatement(parser);

	return stmt;
}

// Parse a function definition including its body.
// Returns: newly allocated FunctionNode.
FunctionNode* parseFunction(Parser* parser) {
	if (!match(parser, TOKEN_INT) && !match(parser, TOKEN_VOID)) {
		printf("Error: Expected return type ('int' or 'void').\n");
		exit(EXIT_FAILURE);
	}

	if (!match(parser, TOKEN_IDENTIFIER)) {
		printf("Error: Expected function name.\n");
		exit(EXIT_FAILURE);
	}
	const Token* nameToken = &parser->tokens[parser->current - 1];

	if (!match(parser, TOKEN_LEFT_PAREN)) {
		printf("Error: Expected '(' after function name.\n");
		exit(EXIT_FAILURE);
	}

	if (!match(parser, TOKEN_VOID)) {
		printf("Error: Only 'void' parameters supported for now.\n");
		exit(EXIT_FAILURE);
	}

	if (!match(parser, TOKEN_RIGHT_PAREN)) {
		printf("Error: Expected ')' after parameter list.\n");
		exit(EXIT_FAILURE);
	}

	if (!match(parser, TOKEN_LEFT_BRACE)) {
		printf("Error: Expected '{' to start function body.\n");
		exit(EXIT_FAILURE);
	}

	StatementNode* body = parseStatementList(parser);

	// ✅ Ensure function body ends correctly
	if (!match(parser, TOKEN_RIGHT_BRACE)) {
		printf("Error: Expected '}' at end of function body.\n");
		exit(EXIT_FAILURE);
	}

	char functionName[nameToken->length + 1];
	strncpy(functionName, nameToken->start, nameToken->length);
	functionName[nameToken->length] = '\0';

	return createFunctionNode(functionName, body);
}

// Parse an entire program consisting of multiple function definitions.
// parser - parser state.
// Returns: ProgramNode representing all parsed functions.
ProgramNode* parseProgram(Parser* parser) {
	ProgramNode* program = NULL;
	FunctionNode* lastFunction = NULL;

	while (currentToken(parser)->type != TOKEN_EOF) {
		// Ensure we are parsing a valid function definition
		if (currentToken(parser)->type == TOKEN_INT || currentToken(parser)->type == TOKEN_VOID) {
			FunctionNode* function = parseFunction(parser);

			if (!program) {
				program = createProgramNode(function);
			} else {
				// Append the function to the existing program
				lastFunction->next = function; // Requires `next` in `FunctionNode`
			}
			lastFunction = function;
		} else {
			// Handle EOF properly
			if (currentToken(parser)->type == TOKEN_EOF) {
				return program;  // Stop parsing safely at EOF
			}

			// Unexpected token error
			printf("Error: Unexpected token '%.*s' (type: %d) at top level.\n", (int)currentToken(parser)->length, currentToken(parser)->start, currentToken(parser)->type);
			exit(EXIT_FAILURE);
		}
	}
	return program;
}

// Convenience entry point: parse a program from an array of tokens.
// tokens - array produced by the lexer.
// Returns: ProgramNode for the entire input.
ProgramNode* parseProgramTokens(const Token* tokens) {
	Parser parser = {tokens, 0};
	return parseProgram(&parser);
}
