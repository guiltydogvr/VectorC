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
	EXP_CONSTANT,  // Integer or double constant
	EXP_UNARY      // Unary operation (e.g., ~x, -x)
} ExpressionType;

typedef enum {
	UNARY_COMPLEMENT,  // Bitwise NOT (~x)
	UNARY_NEGATE       // Negation (-x)
} UnaryOperator;

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
	char* name;						// Function identifier
	struct StatementNode* body;		// Body of the function
	struct FunctionNode* next; // Allows linking multiple functions
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
	ExpressionType type;  // EXP_CONSTANT or EXP_UNARY
	
	union {
		struct {                 // For unary expressions
			UnaryOperator op;    // e.g., ~ or -
			struct ExpressionNode* operand;
		} unary;

		struct {                 // For constant values
			int intValue;
			double doubleValue;
		} constant;
	} value;
} ExpressionNode;

ProgramNode* createProgramNode(FunctionNode* function);
FunctionNode* createFunctionNode(const char* name, StatementNode* body);
StatementNode* createReturnStatementNode(ExpressionNode* expr);
ExpressionNode* createIntConstant(int value);
ExpressionNode* createUnaryNode(UnaryOperator op, ExpressionNode* operand);

void printProgram(const ProgramNode* program);

#endif /* ast_h */
