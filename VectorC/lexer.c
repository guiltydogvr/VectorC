#include <stdio.h>
#include <string.h>

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "lexer.h"
#include "trie.h"
#include "stb_ds.h"

typedef struct {
	const char* start;
	const char* current;
	int32_t line;
} Lexer;

Lexer lexer;

static TrieNode* s_keywordsTrie = NULL;

void initLexer(const char* source) {
	lexer.start = source;
	lexer.current = source;
	lexer.line = 1;
	
	TrieNode* root = createTrieNode();

	typedef struct KeyToken {
		TokenType type;
		const char* keyword;
	} KeyToken;
	
	static KeyToken s_keywordTokenList[] = {
//		{ TOKEN_LEFT_PAREN, "(" },
//		{ TOKEN_RIGHT_PAREN, ")" },
//		{ TOKEN_LEFT_BRACE, "{" },
//		{ TOKEN_RIGHT_BRACE, "}" },
//		{ TOKEN_COMMA, "," },
//		{ TOKEN_DOT, "." },
//		{ TOKEN_MINUS, "-" },
//		{ TOKEN_PLUS, "+" },
//		{ TOKEN_SEMICOLON, ";" },
//		{ TOKEN_SLASH, "/" },
//		{ TOKEN_STAR, "*" },
//		{ TOKEN_BANG, "!" },
//		{ TOKEN_BANG_EQUAL, "!=" },
//		{ TOKEN_EQUAL, "=" },
//		{ TOKEN_EQUAL_EQUAL, "==" },
//		{ TOKEN_GREATER, ">" },
//		{ TOKEN_GREATER_EQUAL, ">=" },
//		{ TOKEN_LESS, "<" },
//		{ TOKEN_LESS_EQUAL, "<=" },
//		{ TOKEN_IDENTIFIER, "Identifier" },
//		{ TOKEN_STRING, "String" },
//		{ TOKEN_NUMBER, "Number" },
//		{ TOKEN_AND, "and" },
//		{ TOKEN_CLASS, "class" },
//		{ TOKEN_ELSE, "else" },
//		{ TOKEN_FALSE, "false" },
//		{ TOKEN_FUN, "fun" },
//		{ TOKEN_FOR, "for" },
//		{ TOKEN_IF, "if" },
		{ TOKEN_INT, "int" },
//		{ TOKEN_OR, "or" },
//		{ TOKEN_PRINT, "print" },
		{ TOKEN_RETURN, "return" },
//		{ TOKEN_SUPER, "super" },
//		{ TOKEN_THIS, "this" },
//		{ TOKEN_TRUE, "true" },
		{ TOKEN_VOID, "void" },
//		{ TOKEN_WHILE, "while" },
	};

	const size_t kNumTokens = sizeof(s_keywordTokenList) / sizeof(KeyToken);
	static_assert(kNumTokens < (int32_t)TOKEN_COUNT, "Invalid Token");

	for (int32_t token = 0; token < kNumTokens; ++token) {
		trieInsert(root, s_keywordTokenList[token].keyword, s_keywordTokenList[token].type);
	}
	s_keywordsTrie = root;
	
	printf("Begin Trie Test\n");
	for (int32_t i = 0; i < kNumTokens; i++) {
		const char* keyword = s_keywordTokenList[i].keyword; // testKeywords[i];
		TokenType token = trieSearch(root, keyword);
		printf("Keyword: %-10s => Token: %s\n", keyword, getTokenName(token));
	}
	printf("End Trie Test\n");
}

void destroyLexer(void) {
	freeTrie(s_keywordsTrie);
}

static bool isAlpha(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static bool isDigit(char c) {
	return c >= '0' && c <= '9';
}

static bool isAtEnd(void) {
	return *lexer.current == '\0';
}

static char advance(void) {
	lexer.current++;
	return lexer.current[-1];
}

static char peek(void) {
	return *lexer.current;
}

static char peekNext(void) {
	if (isAtEnd()) {
		return '\0';
	}
	return lexer.current[1];
}

static bool match(char expected) {
	if (isAtEnd()) {
		return false;
	}
	
	if (*lexer.current != expected) {
		return false;
	}
	lexer.current++;
	return true;
}

static Token makeToken(TokenType type) {
	Token token;
	token.type = type;
	token.start = lexer.start;
	token.length = (int32_t)(lexer.current - lexer.start);
	token.line = lexer.line;
	return token;
}

static Token makeNumberToken(TokenType type, int32_t value) {
	Token token;
	token.type = type;
	token.start = lexer.start;
	token.length = (int32_t)(lexer.current - lexer.start);
	token.line = lexer.line;
	token.value.intValue = value;
	return token;
}

Token makeIntToken(TokenType type, const char* lexeme, int value) {
	Token token;
	token.type = type;
	token.start = lexer.start;
	token.length = (int32_t)(lexer.current - lexer.start);
	token.line = lexer.line;
	token.value.intValue = value;
	return token;
}

Token makeDoubleToken(TokenType type, const char* lexeme, double value) {
	Token token;
	token.type = type;
	token.start = lexer.start;
	token.length = (int32_t)(lexer.current - lexer.start);
	token.line = lexer.line;
	token.value.doubleValue = value;
	return token;
}

// Generic macro for creating tokens
#define makeConstantToken(type, lexeme, value) \
	_Generic((value), \
		int: makeIntToken, \
		double: makeDoubleToken \
	)(type, lexeme, value)

static Token errorToken(const char* message) {
	Token token;
	token.type = TOKEN_ERROR;
	token.start = message;
	token.length = (int32_t)strlen(message);
	token.line = lexer.line;
	return token;
}

static void skipWhitespace(void) {
	for (;;) {
		char c = peek();
		switch (c) {
			case ' ':
			case '\r':
			case '\t':
				advance();
				break;
			case '\n':
				lexer.line++;
				advance();
				break;
			case '/':
				if (peekNext() == '/') {
					// A comment goes until the end of the line.
					while(peek() != '\n' && !isAtEnd()) {
						advance();
					}
				} else {
					return;
				}
				break;
			default:
				return;
		}
	}
}

static TokenType identifierType(void) {
	size_t len = lexer.current - lexer.start;
	char keyword[len+1];
	strncpy(keyword, lexer.start, len);
	keyword[len] = '\0';
	
	return trieSearch(s_keywordsTrie, keyword);
}

static Token identifier(void) {
	while (isAlpha(peek()) || isDigit(peek())) {
		advance();
	}
	return makeToken(identifierType());
}

static Token number(void) {
	const char* start = lexer.start;

	while (isDigit(peek())) advance();

	// 🚩 New check here
	if (isAlpha(peek())) {
		return errorToken("Invalid identifier: cannot start with a digit.");
	}

	int len = (int)(lexer.current - start);
	char numStr[len + 1];
	strncpy(numStr, start, len);
	numStr[len] = '\0';

	int value = atoi(numStr);
	return makeNumberToken(TOKEN_NUMBER, value);
}

static Token string(void) {
	while (peek() != '"' && !isAtEnd()) {
		if (peek() == '\n') {
			lexer.line++;
		}
		advance();
	}
	
	if (isAtEnd()) {
		return errorToken("Unterminated string.");
	}
	
	// The closing quote.
	advance();
	return makeToken(TOKEN_STRING);
}

Token scanToken(void) {
	skipWhitespace();
	lexer.start = lexer.current;

	if (isAtEnd()) {
		return makeToken(TOKEN_EOF);
	}
 
	char c = advance();
	if (isAlpha(c)) {
		return identifier();
	}
	if (isDigit(c)) {
		return number();
	}
	switch(c) {
		case '(':
			return makeToken(TOKEN_LEFT_PAREN);
		case ')':
			return makeToken(TOKEN_RIGHT_PAREN);
		case '{':
			return makeToken(TOKEN_LEFT_BRACE);
		case '}':
			return makeToken(TOKEN_RIGHT_BRACE);
		case ';':
			return makeToken(TOKEN_SEMICOLON);
		case ',':
			return makeToken(TOKEN_COMMA);
		case '.':
			return makeToken(TOKEN_DOT);
		case '-':
			return makeToken(match('-') ? TOKEN_DEC : TOKEN_MINUS);
		case '+':
			return makeToken(TOKEN_PLUS);
		case '/':
			return makeToken(TOKEN_SLASH);
		case '*':
			return makeToken(TOKEN_STAR);
		case '~':
			return makeToken(TOKEN_TILDE);
		case '%':
			return makeToken(TOKEN_MOD);
		case '!':
			return makeToken(match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
		case '=':
			return makeToken(match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
		case '<':
			return makeToken(match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
		case '>':
			return makeToken(match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
		case '"':
			return string();
	}
	return errorToken("Unexpected character.");
}

const char* getTokenName(TokenType tokenType)
{
	static const char* s_tokenNames[] = {
		[TOKEN_LEFT_PAREN] = "TOKEN_LEFT_PAREN",
		[TOKEN_RIGHT_PAREN] = "TOKEN_RIGHT_PAREN",
		[TOKEN_LEFT_BRACE] = "TOKEN_LEFT_BRACE",
		[TOKEN_RIGHT_BRACE] = "TOKEN_RIGHT_BRACE",
		[TOKEN_COMMA] = "TOKEN_COMMA",
		[TOKEN_DOT] = "TOKEN_DOT",
		[TOKEN_MINUS] = "TOKEN_MINUS",
		[TOKEN_PLUS] = "TOKEN_PLUS",
		[TOKEN_SEMICOLON] = "TOKEN_SEMICOLON",
		[TOKEN_SLASH] = "TOKEN_SLASH",
		[TOKEN_STAR] = "TOKEN_STAR",
		[TOKEN_TILDE] = "TOKEN_TILDE",
		[TOKEN_MOD] = "TOKEN_MOD",
		[TOKEN_BANG] = "TOKEN_BANG",
		[TOKEN_BANG_EQUAL] = "TOKEN_BANG_EQUAL",
		[TOKEN_EQUAL] = "TOKEN_EQUAL",
		[TOKEN_EQUAL_EQUAL] = "TOKEN_EQUAL",
		[TOKEN_GREATER] = "TOKEN_GREATER",
		[TOKEN_GREATER_EQUAL] = "TOKEN_GREATER_EQUAL",
		[TOKEN_LESS] = "TOKEN_LESS",
		[TOKEN_LESS_EQUAL] = "TOKEN_LESS_EQUAL",
		[TOKEN_DEC] = "TOKEN_DEC",
		[TOKEN_IDENTIFIER] = "TOKEN_IDENTIFIER",
		[TOKEN_STRING] = "TOKEN_STRING",
		[TOKEN_NUMBER] = "TOKEN_NUMBER",
		[TOKEN_AND] = "TOKEN_AND",
		[TOKEN_CLASS] = "TOKEN_CLASS",
		[TOKEN_ELSE] = "TOKEN_ELSE",
		[TOKEN_FALSE] = "TOKEN_FALSE",
		[TOKEN_FUN] = "TOKEN_FUN",
		[TOKEN_FOR] = "TOKEN_FOR",
		[TOKEN_IF] = "TOKEN_IF",
		[TOKEN_INT] = "TOKEN_INT",
		[TOKEN_OR] = "TOKEN_OR",
		[TOKEN_PRINT] = "TOKEN_PRINT",
		[TOKEN_RETURN] = "TOKEN_RETURN",
		[TOKEN_SUPER] = "TOKEN_SUPER",
		[TOKEN_THIS] = "TOKEN_THIS",
		[TOKEN_TRUE] = "TOKEN_TRUE",
		[TOKEN_VOID] = "TOKEN_VOID",
		[TOKEN_WHILE] = "TOKEN_WHILE",
		[TOKEN_ERROR] = "TOKEN_ERROR",
		[TOKEN_EOF] = "TOKEN_EOF"
	};

	static_assert(sizeof(s_tokenNames) / sizeof(const char*) == (int32_t)TOKEN_COUNT, "Invalid Token");
	return s_tokenNames[tokenType];
}

const Token* scanTokens(void)
{
	Token* tokens = NULL;

	for (;;) {
		Token token = scanToken();
		if (token.type == TOKEN_ERROR) {
			int32_t len = token.length;
			char keyword[len+1];
			strncpy(keyword, token.start, token.length);
			keyword[token.length] = '\0';
			printf("Error: %s\n", keyword);
			exit(EXIT_FAILURE);
		}
		arrput(tokens, token);

		if (token.type == TOKEN_EOF) break;
	}

	return tokens;
}

