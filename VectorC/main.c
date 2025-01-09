//
//  main.c
//  VectorC
//
//  Created by Claire Rogers on 29/12/2024.
//

#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"
#include "parser.h"
#include "translate.h"
#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

static char* readFile(const char * path) {
	FILE* file = fopen(path, "rb");
	if (file == NULL) {
		fprintf(stderr, "Cound not open file \"%s\",\n", path);
		exit(74);
	}
	fseek(file, 0L, SEEK_END);
	size_t fileSize = ftell(file);
	rewind(file);
	
	char* buffer = (char*)malloc(fileSize + 1);
	if (buffer == NULL) {
		fprintf(stderr, "Not enough memory to read \"%s\",\n", path);
		exit(74);
	}
	size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
	if (bytesRead < fileSize) {
		fprintf(stderr, "Could not read file \"%s\".\n", path);
		exit(74);
	}
	buffer[bytesRead] = '\0';
	
	fclose(file);
	return buffer;
}

/*
int main(void)
{
	return 2;
};
 
 */
int main(int argc, const char * argv[]) {
	// insert code here...
	const char* source = readFile(argv[1]); //"return_2.c");
	initLexer(source);
	const Token* tokens = scanTokens();
	size_t tokenCount = arrlenu(tokens);
	for (size_t t = 0; t < tokenCount; ++t) {
		Token token = tokens[t];
		int32_t len = token.length;
		char keyword[len+1];
		switch (token.type)
		{
			case TOKEN_IDENTIFIER:
			case TOKEN_NUMBER:
				strncpy(keyword, token.start, token.length);
				keyword[token.length] = '\0';
				printf("Token: %s (%s)\n", getTokenName(token.type), keyword);
				break;
			case TOKEN_ERROR:
				strncpy(keyword, token.start, token.length);
				keyword[token.length] = '\0';
				printf("Error: %s\n", keyword);
				exit(EXIT_FAILURE);
			default:
				printf("Token: %s\n", getTokenName(token.type));
		}
	}
#if 1
	const ProgramNode* cProgram = parseProgramTokens(tokens);
	printProgram(cProgram);
	
	Program asmProgram;

	Architecture arch = /*ARCH_X64; //*/ ARCH_ARM64;
	switch (arch)
	{
		case ARCH_X64:
			translateProgramToX64(cProgram, &asmProgram);
			break;
		case ARCH_ARM64:
			translateProgramToARM64(cProgram, &asmProgram);
			break;
		default:
			printf("Unsupported architecture`n");
	}
	
//	generateCode(&asmProgram.functions[0])
	printAsmProgram(&asmProgram);
#endif
	destroyLexer();
	return 0;
}

/*
 
./test_compiler ../VectorC/bin/Debug/VectorC --chapter 1
*/
