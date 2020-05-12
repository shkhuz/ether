#include <ether.hpp>
#include <token.hpp>

Token* token_create(char* lexeme, char* start, char* end, TokenType type, SourceFile* file, u64 line, u64 column, u64 char_count) {
	Token* token = new Token();
	token->lexeme = lexeme;
	token->start = start;
	token->end = end;
	token->type = type;
	token->file = file;
	token->line = line;
	token->column = column;
	token->char_count = char_count;
	return token;
}

Token* token_from_string(char* lexeme) {
	return token_create(lexeme,
						lexeme,
						lexeme,
						T_KEYWORD,
						null,
						0,
						0,
						0);
}

bool is_token_equal(Token* a, Token* b) {
	if (str_intern(a->lexeme) ==
		str_intern(b->lexeme)) {
		return true;	
	}
	return false;
}
