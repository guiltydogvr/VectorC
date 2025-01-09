#ifndef lexer_h
#define lexer_h

#include "token.h"

const char* getTokenName(TokenType token);

void initLexer(const char* source);
void destroyLexer(void);
Token scanToken(void);
const Token* scanTokens(void);
#endif

