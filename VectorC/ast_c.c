//
//  ast.c
//  VectorC
//
//  Created by Claire Rogers on 04/01/2025.
//

#include <stdio.h>
#include <string.h>

#include "ast_c.h"
#include "token.h"

// Create the top level program node which simply points at the first function.
//
// function - First function in the program.
// returns  - Newly allocated ProgramNode.
ProgramNode* createProgramNode(FunctionNode* function) {
	ProgramNode* node = (ProgramNode*)malloc(sizeof(ProgramNode));
	node->function = function;
	return node;
}

// Allocate and initialise a FunctionNode with the given name and body.
//
// name - Function identifier to duplicate and store.
// body - Root statement for the function body.
// returns - Newly allocated FunctionNode.
FunctionNode* createFunctionNode(const char* name, StatementNode* body) {
	FunctionNode* node = (FunctionNode*)malloc(sizeof(FunctionNode));
	node->name = strdup(name);  // Duplicate the name
	node->body = body;
	node->next = NULL;
	return node;
}

// Construct a return statement node containing the provided expression.
//
// expr   - Expression to return.
// returns - Newly allocated StatementNode of type STMT_RETURN.
StatementNode* createReturnStatementNode(ExpressionNode* expr) {
	StatementNode* node = (StatementNode*)malloc(sizeof(StatementNode));
	node->type = STMT_RETURN;
	node->expr = expr;
	return node;
}

// Utility to create a numeric constant expression.
//
// value   - Integer literal value.
// returns - ExpressionNode representing the constant.
ExpressionNode* createIntConstant(int value) {
	ExpressionNode* node = malloc(sizeof(ExpressionNode));
	node->type = EXP_CONSTANT;
	node->value.constant.intValue = value;
	return node;
}

// Create a double precision constant expression node.
//
// value   - Double literal value.
// returns - ExpressionNode for the constant.
ExpressionNode* createDoubleConstant(double value) {
	ExpressionNode* node = malloc(sizeof(ExpressionNode));
	node->type = EXP_CONSTANT;
	node->value.constant.doubleValue = value;
	return node;
}

// Build a unary expression node for operations like negation or complement.
//
// op      - Unary operator type.
// operand - Expression the operator is applied to.
// returns - Newly allocated ExpressionNode.
ExpressionNode* createUnaryNode(UnaryOperator op, ExpressionNode* operand) {
	ExpressionNode* node = malloc(sizeof(ExpressionNode));
	node->type = EXP_UNARY;
	node->value.unary.op = op;
	node->value.unary.operand = operand;
	return node;
}

// Construct a binary expression node combining two operands with an operator.
//
// op    - Binary operator type (add, subtract etc.).
// left  - Left hand side expression.
// right - Right hand side expression.
// returns - Newly allocated ExpressionNode.
ExpressionNode* createBinaryNode(BinaryOperator op, ExpressionNode* left, ExpressionNode* right) {
        ExpressionNode* node = malloc(sizeof(ExpressionNode));
        node->type = EXP_BINARY;
        node->value.binary.op = op;
        node->value.binary.left = left;
        node->value.binary.right = right;
        return node;
}

// Recursively print an expression tree with indentation for debugging.
//
// expr   - Expression to print.
// indent - Current indentation level.
void printExpression(const ExpressionNode* expr, int indent) {
	if (!expr) return;

	// Print indentation
	for (int i = 0; i < indent; i++) {
		printf("    "); // 4 spaces per indentation level
	}

	switch (expr->type) {
		case EXP_CONSTANT:
			printf("Constant(%d)\n", expr->value.constant.intValue);
			break;

		case EXP_UNARY:
			printf("Unary(%s)\n",
				expr->value.unary.op == UNARY_COMPLEMENT ? "~" : "-");
			printExpression(expr->value.unary.operand, indent + 1);
			break;
		case EXP_BINARY: {
			const char* opStr;
			switch (expr->value.binary.op) {
				case BINOP_ADD:
					opStr = "+";
					break;
				case BINOP_SUBTRACT:
					opStr = "-";
					break;
				case BINOP_MULTIPLY:
					opStr = "*";
					break;
				case BINOP_DIVIDE:
					opStr = "/";
					break;
				case BINOP_MODULO:
					opStr = "%";
					break;
				default:
					opStr = "?";
					break;
			}
			printf("Binary(%s)\n", opStr);
			printExpression(expr->value.binary.left, indent + 1);
			printExpression(expr->value.binary.right, indent + 1);
			break;
		}
	}
}

// Print a single statement node for debugging purposes.
//
// stmt - Statement to print.
void printStatement(const StatementNode* stmt) {
	if (!stmt) return;

	switch (stmt->type) {
		case STMT_RETURN:
			printf("Return(\n");
			printExpression(stmt->expr, 3);
			printf("        )\n");
			break;
	}
}

// Print a function node including its body statements.
//
// func - Function to print.
void printFunction(const FunctionNode* func) {
	if (!func) return;

	printf("    Function(\n        name=%s,\n        body=", func->name);
	printStatement(func->body);
	printf("    )\n");
}

// Print an entire program AST for debugging.
//
// program - Root program node.
void printProgram(const ProgramNode* program) {
	if (!program) return;

	printf("Program(\n");
	printFunction(program->function);
	printf(")\n");
}

// Recursively free an expression node.
//
// expr - Expression to destroy.
void freeExpression(ExpressionNode* expr) {
        if (!expr) return;
        free(expr);
}

// Free a statement node and any owned expressions.
//
// stmt - Statement to destroy.
void freeStatement(StatementNode* stmt) {
        if (!stmt) return;

        if (stmt->type == STMT_RETURN) {
                freeExpression(stmt->expr);
        }
        free(stmt);
}

// Release a FunctionNode and everything it owns.
//
// func - Function to free.
void freeFunction(FunctionNode* func) {
        if (!func) return;

        free(func->name); // Free the duplicated string
        freeStatement(func->body);
        free(func);
}

// Destroy a ProgramNode and its contained functions.
//
// program - Program to free.
void freeProgram(ProgramNode* program) {
        if (!program) return;

        freeFunction(program->function);
        free(program);
}
