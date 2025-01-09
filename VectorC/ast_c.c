//
//  ast.c
//  VectorC
//
//  Created by Claire Rogers on 04/01/2025.
//

#include <stdio.h>
#include <string.h>

#include "ast_c.h"

ProgramNode* createProgramNode(FunctionNode* function) {
	ProgramNode* node = (ProgramNode*)malloc(sizeof(ProgramNode));
	node->function = function;
	return node;
}

FunctionNode* createFunctionNode(const char* name, StatementNode* body) {
	FunctionNode* node = (FunctionNode*)malloc(sizeof(FunctionNode));
	node->name = strdup(name);  // Duplicate the name
	node->body = body;
	return node;
}

StatementNode* createReturnStatementNode(ExpressionNode* expr) {
	StatementNode* node = (StatementNode*)malloc(sizeof(StatementNode));
	node->type = STMT_RETURN;
	node->expr = expr;
	return node;
}

ExpressionNode* createConstantNode(int value) {
	// Allocate memory for the constant expression node
	ExpressionNode* node = (ExpressionNode*)malloc(sizeof(ExpressionNode));
	if (!node) {
		perror("Failed to allocate memory for ConstantNode");
		exit(EXIT_FAILURE); // Exit if memory allocation fails
	}

	// Initialize the node as a constant expression
	node->type = EXP_CONSTANT;
	node->value.intValue = value;

	return node;
}

void printExpression(const ExpressionNode* expr) {
	if (!expr) return;

	switch (expr->type) {
		case EXP_CONSTANT:
			printf("            Constant(%d)\n", expr->value.intValue);
			break;
	}
}

void printStatement(const StatementNode* stmt) {
	if (!stmt) return;

	switch (stmt->type) {
		case STMT_RETURN:
			printf("Return(\n");
			printExpression(stmt->expr);
			printf("        )\n");
			break;
	}
}

void printFunction(const FunctionNode* func) {
	if (!func) return;

	printf("    Function(\n        name=%s,\n        body=", func->name);
	printStatement(func->body);
	printf("    )\n");
}

void printProgram(const ProgramNode* program) {
	if (!program) return;

	printf("Program(\n");
	printFunction(program->function);
	printf(")\n");
}

void freeExpression(ExpressionNode* expr) {
	if (!expr) return;
	free(expr);
}

void freeStatement(StatementNode* stmt) {
	if (!stmt) return;

	if (stmt->type == STMT_RETURN) {
		freeExpression(stmt->expr);
	}
	free(stmt);
}

void freeFunction(FunctionNode* func) {
	if (!func) return;

	free(func->name); // Free the duplicated string
	freeStatement(func->body);
	free(func);
}

void freeProgram(ProgramNode* program) {
	if (!program) return;

	freeFunction(program->function);
	free(program);
}

/*
int main() {
	// Create the AST: program = Program(Function("main", Return(Constant(42))))
	ExpressionNode* constant = createConstantNode(42);
	StatementNode* returnStmt = createReturnStatementNode(constant);
	FunctionNode* mainFunc = createFunctionNode("main", returnStmt);
	ProgramNode* program = createProgramNode(mainFunc);

	// Print the AST
	printf("AST: ");
	printProgram(program);
	printf("\n");

	// Clean up memory
	freeProgram(program);

	return 0;
}
*/
