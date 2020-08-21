#pragma once

#include <typedef.hpp>

struct Stmt;
struct IfBranch;
struct SwitchBranch;

struct Resolve {
	Stmt** stmts;

	u64 error_count;
	char** data_type_strings;

	error_code resolve(Stmt** _stmts);

private:
	void destroy();
	
	void resolve_stmt(Stmt* stmt);
	void resolve_struct(Stmt* stmt);
	void resolve_func_decl(Stmt* stmt);
	void resolve_var_decl(Stmt* stmt);
	void resolve_if_stmt(Stmt* stmt);
	void resolve_if_branch(IfBranch* branch);
	void resolve_for_stmt(Stmt* stmt);
	void resolve_expr_stmt(Stmt* stmt);
	void resolve_block(Stmt* stmt);
	
	DataType* resolve_expr(Expr* expr);
	DataType* resolve_binary_expr(Expr* expr);
	DataType* resolve_arithmetic_expr(Expr* expr);
	DataType* resolve_logic_binary_expr(Expr* expr);
	DataType* resolve_bitwise_binary_expr(Expr* expr);
	DataType* resolve_comparison_expr(Expr* expr);
	DataType* resolve_bitshift_expr(Expr* expr);
	DataType* resolve_unary_expr(Expr* expr);
	DataType* resolve_cast_expr(Expr* expr);
	DataType* resolve_func_call(Expr* expr);
	DataType* resolve_array_access(Expr* expr);
	DataType* resolve_member_access(Expr* expr);
	DataType* resolve_variable_ref(Expr* expr);
	DataType* resolve_number_expr(Expr* expr);
	DataType* resolve_constant_expr(Expr* expr);

	char* data_type_to_string(DataType* data_type);

public:
	void error_root(SourceFile* srcfile, u64 line, u64 column, u64 char_count, const char* fmt, va_list ap);
	void warning_root(SourceFile* srcfile, u64 line, u64 column, u64 char_count, const char* fmt, va_list ap);
};
