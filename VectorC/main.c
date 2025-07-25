//
//  main.c
//  VectorC
//
//  Created by Claire Rogers on 29/12/2024.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "lexer.h"
#include "parser.h"
#include "tacky.h"
#include "ast_x64.h"
#include "ast_arm64.h"

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

// Read an entire file into a newly allocated null terminated buffer.
// The caller is responsible for freeing the returned string.
//
// path - Path to the file on disk.
// returns - Heap allocated buffer containing the file contents.
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

// Entry point for the compiler front-end. Handles command line parsing and
// drives each stage of the compilation pipeline.
int main(int argc, const char * argv[]) {
	// insert code here...
	bool bLex = false, bParse = false, bTacky = false, bCodegen = false, bVerbose = false;
	Architecture arch = ARCH_X64;

	// The source filename (if any)
	const char *inputFilename = NULL;

	// Start parsing from argv[1] onwards (argv[0] is typically the program name).
	for (int i = 1; i < argc; i++) {
		// First check if it's one of our known switches.
		
		// 1) --lex
		if (strcmp(argv[i], "--lex") == 0) {
			bLex = true;
		}
		// 2) --parse
		else if (strcmp(argv[i], "--parse") == 0) {
			bParse = true;
		}
		// 3) --tacky
		else if (strcmp(argv[i], "--tacky") == 0) {
			bTacky = true;
		}
		// 4) --codegen
		else if (strcmp(argv[i], "--codegen") == 0) {
			bCodegen = true;
		}
		// 5) -arch=???
		else if (strncmp(argv[i], "-arch=", 6) == 0) {
			const char* archValue = argv[i] + 6; // the part after '-arch='
			
			if (strcmp(archValue, "x64") == 0) {
				arch = ARCH_X64;
			} else if (strcmp(archValue, "arm64") == 0) {
				arch = ARCH_ARM64;
			} else {
				fprintf(stderr, "Error: Unknown architecture '%s'\n", archValue);
				return 1;
			}
		}
		// 6) -v
		else if (strcmp(argv[i], "-v") == 0) {
			bVerbose = true;
		}
		// Otherwise, we treat it as the source filename (or error if we already have one).
		else {
			// If we already have a source filename, raise an error or handle as you see fit.
			if (inputFilename != NULL) {
				fprintf(stderr, "Error: Multiple source files specified ('%s' and '%s')\n", inputFilename, argv[i]);
				return 1;
			}
			inputFilename = argv[i];
		}
	}

	// If no source filename was found, we can decide what to do.
	// For example, error out, or continue if your app doesn't need a file, etc.
	if (inputFilename == NULL) {
		// We can consider no file an error or just proceed differently:
		fprintf(stderr, "No source filename provided.\n");
		return EXIT_FAILURE;
	}


	char preprocessedFilename[256];
	strcpy(preprocessedFilename, inputFilename);
	strncpy(preprocessedFilename + (strlen(preprocessedFilename) - 1), "i", 1);

	char sourceFilename[256];
	strcpy(sourceFilename, inputFilename);
	strncpy(sourceFilename + (strlen(sourceFilename) - 1), "s", 1);

	char outFilename[256];
	char* find = strchr(inputFilename, '.');
	assert(find != NULL);
	size_t len = find - inputFilename;
	strncpy(outFilename, inputFilename, len);
#ifdef _WIN32
	strncpy(outFilename + len, ".exe", 4);
	len += 4;
#endif

	outFilename[len] = '\0';

	char commandline[2048] = "";
	sprintf(commandline, "clang -E -P %s -o %s", inputFilename, preprocessedFilename);
	if (bVerbose) {
		printf("Running: %s\n", commandline);
	}
	int32_t res = system(commandline);
	if (res == -1) {
		perror("Error executing system command");
		return EXIT_FAILURE;
	}
	const char* source = readFile(preprocessedFilename);
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
	if (bLex) {
		return EXIT_SUCCESS;
	}

	const ProgramNode* cProgram = parseProgramTokens(tokens);
	printProgram(cProgram);
	if (bParse) {
		return EXIT_SUCCESS;
	}

	TackyProgram* tackyProgram = generateTackyFromAst(cProgram);
	printTackyProgram(tackyProgram);
	if (bTacky) {
		return EXIT_SUCCESS;
	}

	Program asmProgram = { 0 };
	Program finalAsmProgram = { 0 };
	switch (arch)
	{
		case ARCH_X64:
// Pass 1.
			translateTackyToX64(tackyProgram, &asmProgram);
// Pass 2.
			replacePseudoRegistersX64(&asmProgram);
// Pass 3.
			fixupIllegalInstructionsX64(&asmProgram, &finalAsmProgram);
			break;
		case ARCH_ARM64:
// Pass 1.
			translateTackyToARM64(tackyProgram, &asmProgram);
// Pass 2.
			replacePseudoRegistersARM64(&asmProgram);
// Pass 3.
			fixupIllegalInstructionsARM64(&asmProgram, &finalAsmProgram);
			break;
		default:
			printf("Unsupported architecture`n");
	}
	if (bCodegen) {
		return EXIT_SUCCESS;
	}
	printAsmProgram(&finalAsmProgram);

	generateCode(&finalAsmProgram, sourceFilename);
	
	const char* archString = getArchitectureName(arch);
	
#ifdef __APPLE__
	sprintf(commandline, "clang -arch %s %s -o %s", archString, sourceFilename, outFilename);
#else
	sprintf(commandline, "clang %s -o %s", sourceFilename, outFilename);
#endif
	if (bVerbose) {
		printf("Running: %s\n", commandline);
	}
	res = system(commandline);
	if (res == -1) {
		perror("Error executing system command");
		return EXIT_FAILURE;
	}

	destroyLexer();
	return EXIT_SUCCESS;
}
