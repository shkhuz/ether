#pragma once

#include <ether.hpp>

struct LexerOutput {
	std::vector<Token*>* tokens;
	error_code error_occured;
}; 

struct Lexer {
	SourceFile* srcfile;
	std::vector<Token*>* tokens;
	u64 error_count;

	char* start;
	char* current;
	u64 line;

	char* last_newline;
	char* last_to_last_newline;
	
	LexerOutput lex(SourceFile* _srcfile);

private:
	void newline();

	void add(TokenType type);
	u64 compute_column();

	void error(const char* fmt, ...);
};
