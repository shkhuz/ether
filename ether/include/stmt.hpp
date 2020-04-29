#pragma once

#include <vector>

#include <data_type.hpp>
#include <token.hpp>

enum StmtType {
	S_FUNC_DECL,
	S_VAR_DECL,
	S_EXPR_STMT,
};

struct Expr;

struct Stmt {
	StmtType type;
	union {
		struct {
			Token* identifier;
			DataType* return_data_type;
			std::vector<Stmt*>* body;
		} func_decl;
		
		struct {
			Token* identifier;
			DataType* data_type;
			Expr* initializer;
		} var_decl;

		Expr* expr_stmt;
	};
};
