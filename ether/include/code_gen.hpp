#pragma once

#include <ether.hpp>

struct Stmt;
struct DataType;
struct Token;

struct CodeGenerator {
	Stmt** stmts;
	char* obj_fpath;
	
	char* output_code;
	u64 tab_count;
	
	void generate(Stmt** _stmts, char* _obj_fpath);

private:
	void gen_stmt(Stmt* stmt);
	void gen_struct(Stmt* stmt);
	void gen_var_decl(Stmt* stmt);
	void gen_expr(Expr* expr);

	void print_data_type(DataType* data_type);
	void print_tabs_by_indentation();
	void print_newline();
	void print_space();
	void print_semicolon();
	void print_tab_with_spaces();
	void print_token(Token* token);
	void print_string(char* str);
	void print_char(char ch);
};
