#pragma once

#include <data_type.hpp>
#include <token.hpp>

enum StmtType {
	S_VAR_DECL,
};

struct Expr;

struct Stmt {
	StmtType type;
	union {
		struct {
			DataType* data_type;
			Token* identifier;
			Expr* initializer;
		} var_decl;
	};
};
