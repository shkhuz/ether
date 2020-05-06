#pragma once

enum TokenType {
	T_IDENTIFIER,
	T_KEYWORD,
	T_STRING,
	T_CHAR,
	T_INTEGER,
	T_FLOAT32,
	T_FLOAT64,

	T_LPAREN,
	T_RPAREN,
	T_LBRACE,
	T_RBRACE,
	T_LBRACKET,
	T_RBRACKET,
	T_LANGBKT,
	T_RANGBKT,
	T_PLUS,
	T_MINUS,
	T_ASTERISK,
	T_SLASH,
	T_PERCENT,
	T_BANG,
	T_EQUAL,
	T_AMPERSAND,
	T_BAR,
	T_TILDE,
	T_COLON,
	T_SEMICOLON,
	T_COMMA,
	T_DOT,
	T_CARET,
	T_POUND,

	T_DOUBLE_COLON,
	T_PLUS_EQUAL,
	T_MINUS_EQUAL,
	T_ASTERISK_EQUAL,
	T_SLASH_EQUAL,
	T_PERCENT_EQUAL,
	T_BANG_EQUAL,
	T_LESS_EQUAL,
	T_GREATER_EQUAL,
	T_LESS_LESS,
	T_GREATER_GREATER,
	T_EQUAL_EQUAL,
	T_AMPERSAND_AMPERSAND,
	T_BAR_BAR,
	T_AMPERSAND_EQUAL,
	T_BAR_EQUAL,
	T_DOT_DOT,
	T_ARROW,

	T_LESS_LESS_EQUAL,
	T_GREATER_GREATER_EQUAL,

	T_EOF,
};

struct Token {
	char* lexeme;
	char* start;
	char* end;
	TokenType type;
	SourceFile* file;
	u64 line;
	u64 column;
	u64 char_count;
}; 

Token* token_create(char* lexeme, char* start, char* end, TokenType type, SourceFile* file, u64 line, u64 column, u64 char_count);
