#pragma once

#include <ether.hpp>
#include <token.hpp>

struct LexerOutput {
	Token** tokens;
	error_code error_occured;
}; 

struct Lexer {
	SourceFile* srcfile;
	
	Token** tokens;
	u64 error_count;

	char* start;
	char* current;
	u64 line;

	char* last_newline;
	char* last_to_last_newline;

	LexerOutput lex(SourceFile* _srcfile);

private:
	void identifier();
	void number();
	void string();
	void chr();
	void newline();

	void escape_chars();
	void backslash();

	bool match(char c);
	bool is_at_end();

	void add(TokenType type);
	void add_in(TokenType type);
	void add_eof();

	u64 compute_column();
	u64 compute_column_on_current();

	void error(const char* fmt, ...);
	void error_at(u64 _line, u64 _column, const char* fmt, ...);

	void warning(const char* fmt, ...);
	void warning_at(u64 _line, u64 _column, const char* fmt, ...);
	void warning_at_rng(u64 _line, u64 _column, u64 mark_len, const char* fmt, ...);
};
