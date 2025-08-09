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

//
// initLexer
// ---------
// Prepare the lexer to scan a new source string.
//
// Parameters:
//   source - Null-terminated source code to lex.
//
// Returns:
//   None.
//
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

//
// destroyLexer
// ------------
// Release resources associated with the lexer.
//
// Parameters: none
// Returns:    none
//
void destroyLexer(void) {
	freeTrie(s_keywordsTrie);
}

// Check whether a character is alphabetic or underscore.
//
// c - Character to test.
// Returns: true if the character is a letter or underscore.
static bool isAlpha(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

// Determine if a character represents a decimal digit.
//
// c - Character to test.
// Returns: true if c is between '0' and '9'.
static bool isDigit(char c) {
	return c >= '0' && c <= '9';
}

// Check if the lexer has reached end of input.
//
// Returns: true if there is no more source to read.
static bool isAtEnd(void) {
	return *lexer.current == '\0';
}

// Consume the next character in the source stream.
//
// Returns: the consumed character.
static char advance(void) {
	lexer.current++;
	return lexer.current[-1];
}

// Look at the current character without consuming it.
//
// Returns: current character.
static char peek(void) {
	return *lexer.current;
}

// Peek one character ahead without consuming it.
//
// Returns: next character, or '\0' if at end of input.
static char peekNext(void) {
	if (isAtEnd()) {
		return '\0';
	}
	return lexer.current[1];
}

// Try to consume the next character if it matches the expected one.
//
// expected - Character to match.
// Returns: true if the character was consumed, false otherwise.
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

// Build a generic token using the current lexeme boundaries.
//
// type - Token type to assign.
// Returns: populated token structure.
static Token makeToken(TokenType type) {
	Token token;
	token.type = type;
	token.start = lexer.start;
	token.length = (int32_t)(lexer.current - lexer.start);
	token.line = lexer.line;
	return token;
}

// Create a token representing an integer literal.
//
// type  - Token type (e.g., TOKEN_NUMBER).
// value - Numeric value of the literal.
// Returns: populated token with integer value stored.
static Token makeNumberToken(TokenType type, int32_t value) {
	Token token;
	token.type = type;
	token.start = lexer.start;
	token.length = (int32_t)(lexer.current - lexer.start);
	token.line = lexer.line;
	token.value.intValue = value;
	return token;
}

// Helper used by generic macros for creating integer tokens.
// lexeme is unused but kept for symmetry with makeDoubleToken.
Token makeIntToken(TokenType type, const char* lexeme, int value) {
	Token token;
	token.type = type;
	token.start = lexer.start;
	token.length = (int32_t)(lexer.current - lexer.start);
	token.line = lexer.line;
	token.value.intValue = value;
	return token;
}

// Helper used by generic macros for creating double tokens.
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

// Construct an error token containing the provided message.
// message - Error description.
// Returns: token of type TOKEN_ERROR.
static Token errorToken(const char* message) {
	Token token;
	token.type = TOKEN_ERROR;
	token.start = message;
	token.length = (int32_t)strlen(message);
	token.line = lexer.line;
	return token;
}

// Skip over whitespace and comments.
// Returns: none.
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

// Determine whether the identifier under construction is a keyword and return
// its corresponding TokenType.
static TokenType identifierType(void) {
	size_t len = lexer.current - lexer.start;
	char keyword[len+1];
	strncpy(keyword, lexer.start, len);
	keyword[len] = '\0';

	return trieSearch(s_keywordsTrie, keyword);
}

// Consume a sequence of alphanumeric characters and produce an identifier token.
static Token identifier(void) {
	while (isAlpha(peek()) || isDigit(peek())) {
		advance();
	}
	return makeToken(identifierType());
}

// Parse digits into a number token. Rejects identifiers starting with digits.
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

// Parse a double quoted string literal.
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

// Public interface to scan a single token from the source.
// Advances the lexer and returns the next token in the stream.
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
//			return makeToken(match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
			if (match('=')) return makeToken(TOKEN_LESS_EQUAL);
			if (match('<')) return makeToken(TOKEN_SHIFT_LEFT);
			return makeToken(TOKEN_LESS);
		case '>':
//			return makeToken(match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
			if (match('=')) return makeToken(TOKEN_GREATER_EQUAL);
			if (match('>')) return makeToken(TOKEN_SHIFT_RIGHT);
			return makeToken(TOKEN_GREATER);
		case '&':
			return makeToken(TOKEN_AND);
		case '|':
			return makeToken(TOKEN_OR);
		case '^':
			return makeToken(TOKEN_XOR);
		case '"':
			return string();
	}
	return errorToken("Unexpected character.");
}

// Convert a TokenType to a human readable string.
// tokenType - token enumeration to describe.
// Returns: string literal name of the token.
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
		[TOKEN_EQUAL_EQUAL] = "TOKEN_EQUAL_EQUAL",
		[TOKEN_GREATER] = "TOKEN_GREATER",
		[TOKEN_GREATER_EQUAL] = "TOKEN_GREATER_EQUAL",
		[TOKEN_LESS] = "TOKEN_LESS",
		[TOKEN_LESS_EQUAL] = "TOKEN_LESS_EQUAL",
		[TOKEN_SHIFT_LEFT] = "TOKEN_SHIFT_LEFT",
		[TOKEN_SHIFT_RIGHT] = "TOKEN_SHIFT_RIGHT",
		[TOKEN_DEC] = "TOKEN_DEC",
		[TOKEN_IDENTIFIER] = "TOKEN_IDENTIFIER",
		[TOKEN_STRING] = "TOKEN_STRING",
		[TOKEN_NUMBER] = "TOKEN_NUMBER",
		[TOKEN_AND] = "TOKEN_AND",
		[TOKEN_ELSE] = "TOKEN_ELSE",
		[TOKEN_FALSE] = "TOKEN_FALSE",
		[TOKEN_FOR] = "TOKEN_FOR",
		[TOKEN_IF] = "TOKEN_IF",
		[TOKEN_INT] = "TOKEN_INT",
		[TOKEN_OR] = "TOKEN_OR",
		[TOKEN_RETURN] = "TOKEN_RETURN",
		[TOKEN_VOID] = "TOKEN_VOID",
		[TOKEN_WHILE] = "TOKEN_WHILE",
		[TOKEN_XOR] = "TOKEN_XOR",
		[TOKEN_ERROR] = "TOKEN_ERROR",
		[TOKEN_EOF] = "TOKEN_EOF"
	};

	static_assert(sizeof(s_tokenNames) / sizeof(const char*) == (int32_t)TOKEN_COUNT, "Invalid Token");
	return s_tokenNames[tokenType];
}

// Scan the entire source returning an array of tokens terminated by TOKEN_EOF.
// Returns: stb_ds dynamic array containing all tokens.
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

