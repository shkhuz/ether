#pragma once

#include <vector>

#include <data_type.hpp>
#include <token.hpp>

enum StmtType {
	S_STRUCT,
	S_FUNC_DECL,
	S_VAR_DECL,
	S_IF,
	S_FOR,
	S_SWITCH,
	S_RETURN,
	S_EXPR_STMT,
	S_BLOCK,
};

struct Expr;
struct IfBranch;
struct SwitchBranch;

struct Stmt {
	StmtType type;
	union {
		struct {
			Token* identifier;
			Stmt** fields; // fields is a vector of Stmts
			Stmt** functions;
		} struct_stmt;
		
		struct {
			Token* identifier;
			Stmt** params; // params is a vector of Stmts, to ease scope later
			DataType* return_data_type;
			Stmt** body;
			bool is_function;
			bool is_public;
			Stmt* struct_in; // null if global
		} func_decl;
		
		struct {
			Token* identifier;
			DataType* data_type;
			Expr* initializer;
			bool is_variable;
		} var_decl;

		struct {
			IfBranch* if_branch;
			IfBranch** elif_branch;
			IfBranch* else_branch;
		} if_stmt;

		struct {
			Stmt* counter;
			Expr* end;
			Stmt** body;
		} for_stmt;

		struct {
			Expr* cond;
			SwitchBranch** branches;
		} switch_stmt;

		struct {
			Expr* to_return;
			Stmt* function_refed;
		} return_stmt;
		
		Expr* expr_stmt;
		Stmt** block;
	};
};

struct IfBranch {
	Expr* cond;
	Stmt** body;
};

enum IfBranchType {
	IF_IF_BRANCH,
	IF_ELIF_BRANCH,
	IF_ELSE_BRANCH,
};

struct SwitchBranch {
	Expr** conds;
	Stmt* stmt;
};
