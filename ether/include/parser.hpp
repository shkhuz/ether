#pragma once

#include <ether.hpp>
#include <token.hpp>
#include <stmt.hpp>
#include <expr.hpp>
#include <data_type.hpp>

struct ParserOutput {
	std::vector<Stmt*>* stmts;
	error_code error_occured;
};

struct Parser {
	std::vector<Token*>* tokens;
	SourceFile* srcfile;

	std::vector<Stmt*>* stmts;
	u64 error_count;

	u64 token_idx;
	u64 tokens_len;
	
	ParserOutput parse(std::vector<Token*>* _tokens, SourceFile* _srcfile);

private:
	Stmt* decl();

	Stmt* stmt();
	Stmt* expr_stmt();
	
	Stmt* var_decl_create(Token* identifier, DataType* data_type, Expr* initializer);
	Stmt* func_decl_create(Token* identifier, std::vector<Stmt*>* params, DataType* return_data_type, std::vector<Stmt*>* body);
	Stmt* expr_stmt_create(Expr* expr);

	Expr* expr();
	Expr* expr_assign();
	Expr* expr_binary_plus_minus();
	Expr* expr_primary();

	Expr* assign_create(Expr* left, Expr* value);
	Expr* binary_create(Expr* left, Expr* right, Token* op);
	Expr* variable_ref_create(Token* identifier);
	Expr* number_create(Token* number);
	
	bool match_identifier();
	bool match_by_type(TokenType type);
	bool match_double_colon();
	bool match_lparen();
	bool match_rparen();
	bool match_lbrace();
	bool match_rbrace();
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
