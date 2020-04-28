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

	Stmt* var_decl_create(DataType* data_type, Token* identifier, Expr* initializer);

	Expr* expr();
	Expr* expr_primary();

	Expr* variable_ref_create(Token* identifier);
	
	bool match_identifier();
	bool match_by_type(TokenType type);
	bool match_double_colon();
	DataType* match_data_type();

	Token* consume_identifier();
	DataType* consume_data_type();
	void consume_double_colon();
	void consume_semicolon();

	void expect_by_type(TokenType type, const char* fmt, ...);
	
	Token* current();
	Token* previous();

	void goto_next_token();
	void goto_previous_token();

	void error(const char* fmt, ...);
	void verror(const char* fmt, va_list ap);
	void sync_to_next_statement();
}; 
