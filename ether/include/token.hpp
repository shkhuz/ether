#pragma once

enum TokenType {
	T_IDENTIFIER,
	T_KEYWORD,
	T_STRING,
	T_CHAR,
	T_INTEGER,
	T_FLOAT32,
	T_FLOAT64,

	T_DOUBLE_COLON,

	T_LPAREN,
	T_RPAREN,
	T_LBRACE,
	T_RBRACE,
	T_COLON,
	T_SEMICOLON,
	T_COMMA,
	T_DOT,

	T_EOF,
};

struct Token {
	char* lexeme;
	TokenType type;
	SourceFile* file;
	u64 line;
	u64 column;
}; 

Token* token_create(char* lexeme, TokenType type, SourceFile* file, u64 line, u64 column);
