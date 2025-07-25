#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "token.h"

#define TRIE_CHARSET_SIZE 128 // ASCII charset
/*
typedef enum {
	TOKEN_IDENTIFIER,
	TOKEN_CLASS,
	TOKEN_RETURN,
	TOKEN_VAR,
	TOKEN_IF,
	TOKEN_ELSE,
	TOKEN_WHILE,
	TOKEN_PRINT,
	// Add more token types as needed
} TokenType;
*/
// Trie node definition
typedef struct TrieNode {
	TokenType token;                 // TokenType stored here (default: TOKEN_IDENTIFIER)
	struct TrieNode* children[TRIE_CHARSET_SIZE]; // Pointers to children nodes
	int isEnd;                       // Marks the end of a valid token keyword
} TrieNode;

// Create a new trie node
// returns - Newly allocated TrieNode initialised to empty.
TrieNode* createTrieNode(void) {
	TrieNode* node = (TrieNode*)malloc(sizeof(TrieNode));
	if (!node) {
		perror("Failed to allocate TrieNode");
		exit(EXIT_FAILURE);
	}
	node->isEnd = 0;
	node->token = TOKEN_IDENTIFIER; // Default token type
	for (int i = 0; i < TRIE_CHARSET_SIZE; i++) {
		node->children[i] = NULL;
	}
	return node;
}

// Insert a keyword and its associated TokenType into the trie
//
// root    - Root of the trie.
// keyword - Keyword string to insert.
// token   - Token type associated with the keyword.
void trieInsert(TrieNode* root, const char* keyword, TokenType token) {
	TrieNode* current = root;
	for (size_t i = 0; i < strlen(keyword); i++) {
		unsigned char c = (unsigned char)keyword[i];
		if (!current->children[c]) {
			current->children[c] = createTrieNode();
		}
		current = current->children[c];
	}
	current->isEnd = 1;
	current->token = token;
}

// Search for a keyword in the trie and return its TokenType
//
// root    - Trie root to search.
// keyword - String to lookup.
// returns - TOKEN_IDENTIFIER if not found or the keyword's token type.
TokenType trieSearch(TrieNode* root, const char* keyword) {
	TrieNode* current = root;
	for (size_t i = 0; i < strlen(keyword); i++) {
		unsigned char c = (unsigned char)keyword[i];
		if (!current->children[c]) {
			return TOKEN_IDENTIFIER; // Default: Not a keyword
		}
		current = current->children[c];
	}
	return current->isEnd ? current->token : TOKEN_IDENTIFIER;
}

// Free the trie recursively
//
// node - Node to free.
void freeTrie(TrieNode* node) {
	if (!node) return;
	for (int i = 0; i < TRIE_CHARSET_SIZE; i++) {
		if (node->children[i]) {
			freeTrie(node->children[i]);
		}
	}
	free(node);
}
