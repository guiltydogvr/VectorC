#ifndef lexer_h
#define lexer_h

#include "token.h"

// Return a string name for the given token type.
const char* getTokenName(TokenType token);

// Initialize lexer state with the provided source string.
void initLexer(const char* source);

// Release any resources owned by the lexer.
void destroyLexer(void);

// Retrieve the next token from the source stream.
Token scanToken(void);

// Scan the entire source and return an array of tokens.
const Token* scanTokens(void);
#endif

