#pragma once

#include <typedef.hpp>
#include <vector>

enum ExprType {
	E_ASSIGN,
	E_BINARY,
	E_FUNC_CALL,
	E_VARIABLE_REF,
	E_NUMBER,
	E_STRING,
	E_CHAR,
};

struct SourceFile;
struct Stmt;
struct Token;

struct Expr {
	ExprType type;
	Token* head;
	Token* tail;
	union {
		struct {
			Token* identifier;
			Stmt* variable_refed;
		} variable_ref;

		struct {
			Token* callee;
			std::vector<Expr*>* args;
			Stmt* function_called;
		} func_call;
		
		struct {
			Expr* left;
			Expr* value;
		} assign;

		struct {
			Expr* left;
			Expr* right;
			Token* op;
		} binary;
		
		Token* number;
		Token* string;
		Token* chr;
	};
};

u64 get_expr_char_count(Expr* expr);
