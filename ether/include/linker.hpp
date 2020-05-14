#pragma once

#include <typedef.hpp>

struct Stmt;
struct IfBranch;
struct SwitchBranch;

struct StructFunctionMap {
	Stmt* stmt;
	Stmt** functions;
};

struct Scope {
	Scope* parent_scope;
	Stmt** variables;
};

enum VariableScope {
	VS_CURRENT_SCOPE,
	VS_OUTER_SCOPE,
	VS_NO_SCOPE,
};

struct Linker {
	Stmt** stmts;
	
	StructFunctionMap** defined_structs;
	Stmt** defined_functions;
	Scope* current_scope;
	Scope* global_scope;
	Stmt* function_in;
	u64 error_count;
	
	error_code link(Stmt** _stmts);

private:
	void add_structs();
	void add_struct(Stmt* stmt);
	void add_functions();
	void add_function_global(Stmt* stmt);
	void add_function_struct(Stmt* stmt);
	void add_variables();
	void add_variable(Stmt* stmt);

	void check_stmts();
	void check_stmt(Stmt* stmt);
	void check_struct(Stmt* stmt);
	void check_func_decl(Stmt* stmt);
	void check_var_decl(Stmt* stmt);
	void check_if_stmt(Stmt* stmt);
	void check_if_branch(IfBranch* branch);
	void check_for_stmt(Stmt* stmt);
	void check_switch_stmt(Stmt* stmt);
	void check_switch_branch(SwitchBranch* branch);
	void check_return_stmt(Stmt* stmt);
	void check_expr_stmt(Stmt* stmt);
	void check_block_stmt(Stmt* stmt);
	void check_expr(Expr* expr);
	void check_binary_expr(Expr* expr);
	void check_unary_expr(Expr* expr);
	void check_cast_expr(Expr* expr);
	void check_func_call(Expr* expr);
	void check_array_access(Expr* expr);
	void check_variable_ref(Expr* expr);
	void check_data_type(DataType* data_type, bool is_return_data_type);
	void add_variable_to_scope(Stmt* stmt);
	VariableScope is_variable_ref_in_scope(Expr* expr);
	VariableScope is_variable_in_scope(Stmt* stmt);
	
public:
	void error_root(SourceFile* srcfile, u64 line, u64 column, u64 char_count, const char* fmt, va_list ap);
	void warning_root(SourceFile* srcfile, u64 line, u64 column, u64 char_count, const char* fmt, va_list ap);
};
