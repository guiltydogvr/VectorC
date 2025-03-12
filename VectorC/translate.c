//
//  translate.c
//  VectorC
//
//  Created by Claire Rogers on 05/01/2025.
//

#include "translate.h"

void translateFunctionToX64(const FunctionNode* cFunction, Function* asmFunction) {
	// Example: Translate a function like `int main() { return 42; }`
	Operand src = createImmOperand(42);
	Operand dst = createRegisterOperand("eax");

	X64Instruction* instructions = malloc(sizeof(X64Instruction) * 2);
	instructions[0] = (X64Instruction){X64_MOV, src, dst};
	instructions[1] = (X64Instruction){X64_RET};

	*asmFunction = createFunction(cFunction->name, instructions, 2, ARCH_X64);
}

void translateFunctionToARM64(const FunctionNode* cFunction, Function* asmFunction) {
	Operand src = createImmOperand(42);
	Operand dst = createRegisterOperand("x0");

	ARM64Instruction* instructions = malloc(sizeof(ARM64Instruction) * 2);
	instructions[0] = (ARM64Instruction){ARM64_MOV, src, dst};
	instructions[1] = (ARM64Instruction){ARM64_RET};

	*asmFunction = createFunction(cFunction->name, instructions, 2, ARCH_ARM64);
}

typedef struct {
	Architecture arch;            // Architecture flag
	int currentRegister;    // Tracks the next available register
} TranslationContext;

// Initialize context
TranslationContext createTranslationContext(Architecture arch) {
	TranslationContext ctx;
	ctx.arch = arch;
	ctx.currentRegister = 0;
	return ctx;
}

// Allocate a register (simplified example)
const char* allocateRegister(TranslationContext* ctx) {
	static const char* x64Registers[] = {"eax", "ebx", "ecx", "edx"};
	static const char* arm64Registers[] = {"x0", "x1", "x2", "x3"};
	switch (ctx->arch)
	{
		case ARCH_X64:
			return x64Registers[ctx->currentRegister++];
			break;
		case ARCH_ARM64:
			return arm64Registers[ctx->currentRegister++];
			break;
		default:
			printf("Unsupported architecture\n");
	}
	return NULL;
}

// Translate a `return` statement into x64
void translateReturnToX64(const StatementNode* stmt, X64Instruction** instructions, size_t* count)
{
	if (stmt->type == STMT_RETURN) {
		const ExpressionNode* expr = stmt->expr;

		if (expr->type == EXP_CONSTANT) {
			Operand src = createImmOperand(expr->value.intValue);
			Operand dst = createRegisterOperand("%eax"); // Return value in EAX for x64
			(*instructions)[(*count)++] = (X64Instruction){X64_MOV, src, dst};
			(*instructions)[(*count)++] = (X64Instruction){X64_RET};
		}
	}
}

// Translate a `return` statement into ARM64
void translateReturnToARM64(const StatementNode* stmt, ARM64Instruction** instructions, size_t* count)
{
	if (stmt->type == STMT_RETURN) {
		const ExpressionNode* expr = stmt->expr;

		if (expr->type == EXP_CONSTANT) {
			Operand src = createImmOperand(expr->value.intValue);
			Operand dst = createRegisterOperand("x0"); // Return value in X0 for ARM64
			(*instructions)[(*count)++] = (ARM64Instruction){ARM64_MOV, src, dst};
			(*instructions)[(*count)++] = (ARM64Instruction){ARM64_RET};
		}
	}
}

void translateProgramToX64(const ProgramNode* cProgram, Program* asmProgram)
{
	size_t functionCount = 1; //cProgram->functionCount;
	asmProgram->functions = malloc(sizeof(Function) * functionCount);
	asmProgram->functionCount = functionCount;

	for (size_t i = 0; i < functionCount; i++) {
		const FunctionNode* cFunction = cProgram->function; //[i];
		size_t instructionCapacity = 16; // Initial capacity for instructions
		size_t instructionCount = 0;

		X64Instruction* instructions = malloc(sizeof(X64Instruction) * instructionCapacity);

		// Translate function body (statements)
		const StatementNode* stmt = cFunction->body;
		while (stmt) {
			if (instructionCount + 2 >= instructionCapacity) {
				instructionCapacity *= 2;
				instructions = realloc(instructions, sizeof(X64Instruction) * instructionCapacity);
			}
			translateReturnToX64(stmt, &instructions, &instructionCount);
			stmt = NULL; //stmt->next;
		}

		// Create the ASM function
		asmProgram->functions[i] = createFunction(cFunction->name, instructions, instructionCount, ARCH_X64);
	}
}

void translateProgramToARM64(const ProgramNode* cProgram, Program* asmProgram)
{
	size_t functionCount = 1; //cProgram->functionCount;
	asmProgram->functions = malloc(sizeof(Function) * functionCount);
	asmProgram->functionCount = functionCount;

	for (size_t i = 0; i < functionCount; i++) {
		const FunctionNode* cFunction = cProgram->function; //s[i];
		size_t instructionCapacity = 16; // Initial capacity for instructions
		size_t instructionCount = 0;

		ARM64Instruction* instructions = malloc(sizeof(ARM64Instruction) * instructionCapacity);

		// Translate function body (statements)
		const StatementNode* stmt = cFunction->body;
		while (stmt) {
			if (instructionCount + 2 >= instructionCapacity) {
				instructionCapacity *= 2;
				instructions = realloc(instructions, sizeof(ARM64Instruction) * instructionCapacity);
			}
			translateReturnToARM64(stmt, &instructions, &instructionCount);
			stmt = NULL; //stmt->next;
		}

		// Create the ASM function
		asmProgram->functions[i] = createFunction(cFunction->name, instructions, instructionCount, ARCH_ARM64);
	}
}
