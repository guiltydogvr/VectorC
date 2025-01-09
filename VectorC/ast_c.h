//
//  ast.h
//  VectorC
//
//  Created by Claire Rogers on 04/01/2025.
//

#ifndef ast_h
#define ast_h

#include <stdlib.h>

typedef enum {
	NODE_PROGRAM,
	NODE_FUNCTION,
	NODE_STATEMENT,
	NODE_EXPRESSION,
} NodeType;

typedef enum {
	EXP_CONSTANT,
} ExpressionType;

typedef enum {
	STMT_RETURN,
} StatementType;


// Forward declarations for nested structures
struct FunctionNode;
struct StatementNode;
struct ExpressionNode;

// Program node
typedef struct ProgramNode {
	struct FunctionNode* function;
} ProgramNode;

// Function node
typedef struct FunctionNode {
	char* name;                      // Function identifier
	struct StatementNode* body;      // Body of the function
} FunctionNode;

// Statement node
typedef struct StatementNode {
	StatementType type;              // Statement type (e.g., STMT_RETURN)
	union {
		struct ExpressionNode* expr; // Expression for Return statements
	};
} StatementNode;

// Expression node
typedef struct ExpressionNode {
	ExpressionType type;             // Expression type (e.g., EXP_CONSTANT)
	union {               			// Union for different value types
		int intValue;
		double doubleValue;
		// Add more types here as needed
	} value;
} ExpressionNode;

ProgramNode* createProgramNode(FunctionNode* function);
FunctionNode* createFunctionNode(const char* name, StatementNode* body);
StatementNode* createReturnStatementNode(ExpressionNode* expr);
ExpressionNode* createConstantNode(int value);

void printProgram(const ProgramNode* program);

#endif /* ast_h */
