#pragma once

#include <ether.hpp>
#include <stmt.hpp>
#include <expr.hpp>
#include <data_type.hpp>

struct AstPrinter {
	Stmt** stmts;
	u64 tab_count;
	
	void print(Stmt** _stmts);

private:
	void print_stmt(Stmt* stmt);
	void pre_stmt();
	void print_struct_stmt(Stmt* stmt);
	void print_func_decl(Stmt* stmt);
	void print_var_decl(Stmt* stmt);
	void print_if_stmt(Stmt* stmt);
	void print_if_branch(IfBranch* branch, IfBranchType type);
	void print_for_stmt(Stmt* stmt);
	void print_return_stmt(Stmt* stmt);
	void print_expr_stmt(Stmt* stmt);

	void print_expr(Expr* expr);
	void print_binary(Expr* expr);
	void print_unary(Expr* expr);
	void print_cast(Expr* expr);
	void print_func_call(Expr* expr);
	void print_array_access(Expr* expr);
	void print_member_access(Expr* expr);
	void print_string(Expr* expr);
	void print_char(Expr* expr);

	void print_data_type(DataType* data_type);
	void print_token(Token* token);
	void print_string(char* str);
	void print_lparen();
	void print_rparen();
	// TODO: add other brackets and braces
	void print_space();
	void print_newline();
	void print_char(char c);
	void print_tabs_by_indentation();
	void print_tree_node_literal();
};
