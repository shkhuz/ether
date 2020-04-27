#include <ether.hpp>
#include <lexer.hpp>

LexerOutput Lexer::lex(SourceFile* _srcfile) {
	srcfile = _srcfile;
	tokens = new std::vector<Token*>();
	error_count = 0;
	start = srcfile->contents;
	current = start;
	line = 1;
	last_newline = srcfile->contents;
	last_to_last_newline = null;
	
	for (current = srcfile->contents; current != (srcfile->contents + srcfile->len);) {
		start = current;

		switch (*current) {
		case ';':
			add(T_SEMICOLON); 
			break;
		case '\n':	
			newline(); 
			break;
		default: 
			error("invalid literal: ‘%c’ (dec: %d);", *current, (int)(*current));
			current++;
			break;
		}
	}

	LexerOutput output;
	output.tokens = tokens;
	output.error_occured = (error_count > 0 ?
							ETHER_ERROR :
							ETHER_SUCCESS);
	return output;
}

void Lexer::newline() {
	last_to_last_newline = last_newline;
	last_newline = current;
	line++;
	current++;
}

void Lexer::add(TokenType type) {
	Token* token = new Token();
	token->lexeme = str_intern_range(start, ++current);
	token->type = type;
	token->file = srcfile;
	token->line = line;
	token->column = compute_column();

	tokens->push_back(token);
}

u64 Lexer::compute_column() {
	u64 column = start - last_newline;
	if (line == 1) {
		column++;
	}
	return column;
}

void Lexer::error(const char* fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	print_error_at(srcfile, line, compute_column(), fmt, ap);
	va_end(ap);
	error_count++;
}
