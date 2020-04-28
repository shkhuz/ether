#include <ether.hpp>
#include <token.hpp>

Token* token_create(char* lexeme, TokenType type, SourceFile* file, u64 line, u64 column, u64 char_count) {
	Token* token = new Token();
	token->lexeme = lexeme;
	token->type = type;
	token->file = file;
	token->line = line;
	token->column = column;
	token->char_count = char_count;
	return token;
}
