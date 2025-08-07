//
//  token.h
//  VectorC
//
//  Created by Claire Rogers on 02/01/2025.
//

#ifndef token_h
#define token_h

#include <stdint.h>

typedef enum {
	// SIngle character tokens.
	TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN, TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE, TOKEN_COMMA, TOKEN_DOT, TOKEN_MINUS, TOKEN_PLUS, TOKEN_SEMICOLON, TOKEN_SLASH, TOKEN_STAR, TOKEN_TILDE, TOKEN_MOD,

	// One or two character tokens.
	TOKEN_BANG, TOKEN_BANG_EQUAL, TOKEN_EQUAL, TOKEN_EQUAL_EQUAL, TOKEN_GREATER, TOKEN_GREATER_EQUAL, TOKEN_LESS, TOKEN_LESS_EQUAL, TOKEN_SHIFT_LEFT, TOKEN_SHIFT_RIGHT, TOKEN_DEC, TOKEN_AND, TOKEN_OR, TOKEN_XOR,

	// Literals.
	TOKEN_IDENTIFIER, TOKEN_STRING, TOKEN_NUMBER,

	// Keyworkds.
	TOKEN_ELSE, TOKEN_FALSE, TOKEN_FOR, TOKEN_IF, TOKEN_INT, TOKEN_RETURN, TOKEN_VOID, TOKEN_WHILE,

	TOKEN_ERROR, TOKEN_EOF,
	
	TOKEN_COUNT
} TokenType;

typedef struct {
	TokenType type;
	const char* start;
	int32_t length;
	int32_t line;
	union {               // Union for different value types
		int intValue;
		double doubleValue;
		// Add more types here as needed
	} value;
} Token;

#endif /* token_h */
