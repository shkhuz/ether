#pragma once

#include <ether.hpp>
#include <token.hpp>
#include <stmt.hpp>
#include <expr.hpp>
#include <data_type.hpp>

struct ParserOutput {
	Stmt** stmts;
	error_code error_occured;
};

enum ParserErrorLocation {
	STRUCT_HEADER,
	STRUCT_BODY,
	FUNCTION_HEADER,
	FUNCTION_BODY,
	IF_HEADER,
	IF_BODY,
	GLOBAL,
};

struct Parser {
	Token** tokens;
	SourceFile* srcfile;

	Stmt** stmts;
	
	u64 token_idx;
	u64 tokens_len;

	u64 error_count;
	bool error_panic;
	ParserErrorLocation error_loc;
	u64 error_brace_count;
	bool error_lbrace_parsed = false;
	
	ParserOutput parse(Token** _tokens, SourceFile* _srcfile);

private:
	Stmt* decl();

	Stmt* stmt();
	Stmt* if_branch(Stmt* if_stmt, IfBranchType type);
	Stmt* expr_stmt();

	Stmt* struct_create(Token* identifier, Stmt** fields);
	Stmt* func_decl_create(Token* identifier, Stmt** params, DataType* return_data_type, Stmt** body);
	Stmt* var_decl_create(Token* identifier, DataType* data_type, Expr* initializer);
	Stmt* expr_stmt_create(Expr* expr);

	Expr* expr();
	Expr* expr_assign();
	Expr* expr_binary_plus_minus();
	Expr* expr_cast();
	Expr* expr_func_call_array_access();
	Expr* expr_primary();
	Expr* expr_grouping();

	Expr* assign_create(Expr* left, Expr* value);
	Expr* binary_create(Expr* left, Expr* right, Token* op);
	Expr* cast_create(Token* start, DataType* cast_to, Expr* right);
	Expr* func_call_create(Expr* left, std::vector<Expr*>* args);
	Expr* array_access_create(Expr* left, Expr* index, Token* end);
	Expr* variable_ref_create(Token* identifier);
	Expr* number_create(Token* number);
	Expr* string_create(Token* string);
	Expr* char_create(Token* chr);
	
	bool match_identifier();
	bool match_keyword(char* keyword);
	bool match_by_type(TokenType type);
	bool match_double_colon();
	bool match_lparen();
	bool match_rparen();
	bool match_lbrace();
	bool match_rbrace();
	bool match_lbracket();
	bool match_rbracket();
	bool match_langbkt();
	bool match_rangbkt();
	bool match_semicolon();
	DataType* match_data_type();
	void previous_data_type(DataType* data_type);
	
	Token* consume_identifier();
	DataType* consume_data_type();
	void consume_double_colon();
	void consume_lparen();
	void consume_rparen();
	void consume_lbrace();
	void consume_rbrace();
	void consume_lbracket();
	void consume_rbracket();
	void consume_langbkt();
	void consume_rangbkt();
	void consume_semicolon();

	void expect_by_type(TokenType type, const char* fmt, ...);
	
	Token* current();
	Token* previous();

	void goto_next_token();
	void goto_previous_token();

	void error_root(u64 line, u64 column, u64 char_count, const char* fmt, va_list ap);
	void verror(const char* fmt, va_list ap);
	void error(const char* fmt, ...);
	void error_expr(Expr* expr, const char* fmt, ...);
	void sync_to_next_statement();
}; 
