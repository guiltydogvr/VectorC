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

ExpressionNode* createIntConstant(int value) {
	ExpressionNode* node = malloc(sizeof(ExpressionNode));
	node->type = EXP_CONSTANT;
	node->value.constant.intValue = value;
	return node;
}

ExpressionNode* createDoubleConstant(double value) {
	ExpressionNode* node = malloc(sizeof(ExpressionNode));
	node->type = EXP_CONSTANT;
	node->value.constant.doubleValue = value;
	return node;
}

ExpressionNode* createUnaryNode(UnaryOperator op, ExpressionNode* operand) {
	ExpressionNode* node = malloc(sizeof(ExpressionNode));
	node->type = EXP_UNARY;
	node->value.unary.op = op;
	node->value.unary.operand = operand;
	return node;
}

ExpressionNode* createBinaryNode(BinaryOperator op, ExpressionNode* left, ExpressionNode* right) {
	ExpressionNode* node = malloc(sizeof(ExpressionNode));
	node->type = EXP_BINARY;
	node->value.binary.op = op;
	node->value.binary.left = left;
	node->value.binary.right = right;
	return node;
}

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
