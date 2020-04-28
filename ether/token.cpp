#include <ether.hpp>
#include <token.hpp>

Token* token_create(char* lexeme, TokenType type, SourceFile* file, u64 line, u64 column) {
	Token* token = new Token();
	token->lexeme = lexeme;
	token->type = type;
	token->file = file;
	token->line = line;
	token->column = column;
	return token;
}
