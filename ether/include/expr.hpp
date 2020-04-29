#pragma once

#include <typedef.hpp>

enum ExprType {
	E_ASSIGN,
	E_BINARY,
	E_VARIABLE_REF,
	E_NUMBER,
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
			Expr* left;
			Expr* value;
		} assign;

		struct {
			Expr* left;
			Expr* right;
			Token* op;
		} binary;
		
		Token* number;
	};
};

u64 get_expr_char_count(Expr* expr);
