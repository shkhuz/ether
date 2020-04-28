#pragma once

enum ExprType {
	E_VARIABLE_REF,
	E_NUMBER,
};

struct Stmt;

struct Expr {
	ExprType type;
	Token* head;
	union {
		struct {
			Token* identifier;
			Stmt* variable_refed;
		} variable_ref;

		Token* number;
	};
};
