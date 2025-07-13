#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast_c.h"
#include "tacky.h"
#include "stb_ds.h"

static int currentFunctionTempCounter = 0;
static const char* currentFunctionName = NULL;

static const char* newTempVarName() {
	char* name = malloc(32);
	if (currentFunctionName) {
		sprintf(name, "%s.tmp.%d", currentFunctionName, currentFunctionTempCounter++);
	} else {
		sprintf(name, "tmp.%d", currentFunctionTempCounter++);
	}
	return name;
}

static TackyValue translateExpression(const ExpressionNode* expr, TackyFunction* func) {
	if (!expr) {
		fprintf(stderr, "Null expression node encountered\n");
		exit(EXIT_FAILURE);
	}

	if (expr->type == EXP_CONSTANT) {
		return (TackyValue){ .type = TACKY_VAL_CONSTANT, .constantValue = expr->value.constant.intValue };
	}
	else if (expr->type == EXP_UNARY) {
		TackyValue src = translateExpression(expr->value.unary.operand, func);

		const char* dstName = newTempVarName();
		TackyValue dst = { .type = TACKY_VAL_VAR, .varName = dstName };

		TackyInstruction instr = {
			.type = TACKY_INSTR_UNARY,
			.unary = {
				.op = (expr->value.unary.op == UNARY_COMPLEMENT) ? TACKY_COMPLEMENT : TACKY_NEGATE,
				.src = src,
				.dst = dst
			}
		};
		arrput(func->instructions, instr);

		return dst;
	}
	else if (expr->type == EXP_BINARY) {
		TackyValue lhs = translateExpression(expr->value.binary.left, func);
		TackyValue rhs = translateExpression(expr->value.binary.right, func);

		const char* dstName = newTempVarName();
		TackyValue dst = { .type = TACKY_VAL_VAR, .varName = dstName };

		TackyBinaryOperator op;
		switch (expr->value.binary.op) {
			case BINOP_ADD:
				op = TACKY_ADD;
				break;
			case BINOP_SUBTRACT:
				op = TACKY_SUBTRACT;
				break;
			case BINOP_MULTIPLY:
				op = TACKY_MULTIPLY;
				break;
			case BINOP_DIVIDE:
				op = TACKY_DIVIDE;
				break;
			case BINOP_MODULO:
				op = TACKY_MODULO;
				break;
			default:
				fprintf(stderr, "Unknown binary operator in TACKY generation\n");
				exit(EXIT_FAILURE);
		}

		TackyInstruction instr = {
			.type = TACKY_INSTR_BINARY,
			.binary = {
				.op = op,
				.lhs = lhs,
				.rhs = rhs,
				.dst = dst
			}
		};
		arrput(func->instructions, instr);

		return dst;
	}
	else {
		fprintf(stderr, "Unsupported expression type in TACKY generation\n");
		exit(EXIT_FAILURE);
	}
}

TackyProgram* generateTackyFromAst(const ProgramNode* ast) {
	if (!ast) return NULL;

	TackyProgram* program = (TackyProgram*)malloc(sizeof(TackyProgram));
	program->functions = NULL;

	for (FunctionNode* funcNode = ast->function; funcNode != NULL; funcNode = funcNode->next) {
		currentFunctionName = funcNode->name;
		currentFunctionTempCounter = 0;
		TackyFunction func = {0};
		func.name = funcNode->name;
		func.instructions = NULL;
		
		if (funcNode->body && funcNode->body->type == STMT_RETURN) {
			ExpressionNode* returnExpr = funcNode->body->expr;

			TackyValue retVal = translateExpression(returnExpr, &func);

			TackyInstruction retInstr = {
				.type = TACKY_INSTR_RETURN,
				.ret = { .value = retVal }
			};
			arrput(func.instructions, retInstr);
		}

		arrput(program->functions, func);
	}

	return program;
}

void printTackyProgram(const TackyProgram* program) {
	if (!program) {
		printf("TackyProgram(NULL)\n");
		return;
	}

	printf("TackyProgram(\n");
	for (size_t i = 0; i < arrlenu(program->functions); ++i) {
		const TackyFunction* func = &program->functions[i];
		printf("    Function(name=%s\n", func->name);
		for (size_t j = 0; j < arrlenu(func->instructions); ++j) {
			const TackyInstruction* instr = &func->instructions[j];
			switch (instr->type) {
				case TACKY_INSTR_RETURN:
					printf("        Return(");
					if (instr->ret.value.type == TACKY_VAL_CONSTANT)
						printf("%d", instr->ret.value.constantValue);
					else
						printf("%s", instr->ret.value.varName);
					printf(")\n");
					break;

				case TACKY_INSTR_UNARY:
					printf("        Unary(%s, ",
						   instr->unary.op == TACKY_COMPLEMENT ? "Complement" : "Negate");
					if (instr->unary.src.type == TACKY_VAL_CONSTANT)
						printf("%d, ", instr->unary.src.constantValue);
					else
						printf("%s, ", instr->unary.src.varName);
					printf("%s)\n", instr->unary.dst.varName);
					break;

				case TACKY_INSTR_BINARY:
				{
					char* opString = NULL;
					switch (instr->binary.op)
					{
						case TACKY_ADD:
							opString = "Add";
							break;
						case TACKY_SUBTRACT:
							opString = "Subtrack";
							break;
						case TACKY_MULTIPLY:
							opString = "Multiply";
							break;
						case TACKY_DIVIDE:
							opString = "Divide";
							break;
						case TACKY_MODULO:
							opString = "Modulo";
							break;
					}
					printf("        Binary(%s, ", opString);
					if (instr->binary.lhs.type == TACKY_VAL_CONSTANT)
						printf("%d, ", instr->binary.lhs.constantValue);
					else
						printf("%s, ", instr->binary.lhs.varName);
					if (instr->binary.rhs.type == TACKY_VAL_CONSTANT)
						printf("%d, ", instr->binary.rhs.constantValue);
					else
						printf("%s, ", instr->binary.rhs.varName);
					printf("%s)\n", instr->binary.dst.varName);
					break;
				}

			}
		}
		printf("    )\n");
	}
	printf(")\n");
}
