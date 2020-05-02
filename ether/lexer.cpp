#include <ether.hpp>
#include <lexer.hpp>

LexerOutput Lexer::lex(SourceFile* _srcfile) {
	srcfile = _srcfile;
	
	tokens = null;
	error_count = 0;
	
	start = srcfile->contents;
	current = start;
	line = 1;
	
	last_newline = srcfile->contents;
	last_to_last_newline = null;
	
	for (current = srcfile->contents; current != (srcfile->contents + srcfile->len);) {
		start = current;
		switch (*current) {
		case '<': {
			if (match('<')) {
				if (match('=')) {
					add(T_LESS_LESS_EQUAL);
				}
				else {
					add(T_LESS_LESS);
				}
			}
			else if (match('=')) {
				add(T_LESS_EQUAL);
			}
			else {
				add(T_LANGBKT);
			}
		} break;

		case '>': {
			if (match('>')) {
				if (match('=')) {
					add(T_GREATER_GREATER_EQUAL);
				}
				else {
					add(T_GREATER_GREATER);
				}
			}
			else if (match('=')) {
				add(T_GREATER_EQUAL);
			}
			else {
				add(T_RANGBKT);
			}
		} break;

		case '&': {
			if (match('&')) {
				add(T_AMPERSAND_AMPERSAND);
			}
			else if (match('=')) {
				add(T_AMPERSAND_EQUAL);
			}
			else {
				add(T_AMPERSAND);
			}
		} break;

		case '|': {
			if (match('|')) {
				add(T_BAR_BAR);
			}
			else if (match('=')) {
				add(T_BAR_EQUAL);
			}
			else {
				add(T_BAR);
			}
		} break;

		case ':':	match(':') ? add(T_DOUBLE_COLON) :   add(T_COLON); break;
		case '+':	match('=') ? add(T_PLUS_EQUAL)   :   add(T_PLUS); break;
		case '-':	match('=') ? add(T_MINUS_EQUAL)	 :   add(T_MINUS); break;
		case '*':	match('=') ? add(T_ASTERISK_EQUAL) : add(T_ASTERISK); break;
		case '/':	match('=') ? add(T_SLASH_EQUAL) :    add(T_SLASH); break;
		case '%': 	match('=') ? add(T_PERCENT_EQUAL) :  add(T_PERCENT); break;
		case '!':	match('=') ? add(T_BANG_EQUAL) :	 add(T_BANG); break;
		case '=':	match('=') ? add(T_EQUAL_EQUAL) :	 add(T_EQUAL); break;
		case '.':	match('.') ? add(T_DOT_DOT) :		 add(T_DOT); break;

		case '(':	add(T_LPAREN); break;
		case ')':	add(T_RPAREN); break;
		case '{':	add(T_LBRACE); break;
		case '}':	add(T_RBRACE); break;
		case '[':	add(T_LBRACKET); break;
		case ']':	add(T_RBRACKET); break;
		case ';':	add(T_SEMICOLON); break;
		case ',':	add(T_COMMA); break;
		case '^':	add(T_CARET); break;
		case '~':	add(T_TILDE); break;

		case 'A': case 'B': case 'C': case 'D':
		case 'E': case 'F': case 'G': case 'H':
		case 'I': case 'J': case 'K': case 'L':
		case 'M': case 'N': case 'O': case 'P':
		case 'Q': case 'R': case 'S': case 'T':
		case 'U': case 'V': case 'W': case 'X':
		case 'Y': case 'Z': 
		case 'a': case 'b': case 'c': case 'd':
		case 'e': case 'f': case 'g': case 'h':
		case 'i': case 'j': case 'k': case 'l':
		case 'm': case 'n': case 'o': case 'p':
		case 'q': case 'r': case 's': case 't':
		case 'u': case 'v': case 'w': case 'x':
		case 'y': case 'z':
		case '_':
			identifier();
			break;

		case '0': case '1':
		case '2': case '3':
		case '4': case '5':
		case '6': case '7':
		case '8': case '9':
			number();
			break;

		case '"':
			string();
			break;

		case '\'':
			chr();
			break;

		case '\\':
			backslash();
			break;

		case '\n':
			newline();
			break;

		case '\t':
		case '\r':
		case ' ':
			current++;
			break;

		default: 
			error("invalid literal: ‘%c’ (ASCII: %d);", *current, (int)(*current));
			current++;
			break;
		}
	}
	add_eof();

	LexerOutput output;
	output.tokens = tokens;
	output.error_occured = (error_count > 0 ?
							ETHER_ERROR :
							ETHER_SUCCESS);
	return output;
}

void Lexer::identifier() {
	TokenType type = T_IDENTIFIER;
	current++;
	while (isalnum(*current) || *current == '_') {
		current++;
	}

	char* lexed_identifier = str_intern_range(start, current);
	for (u64 kw_idx = 0; kw_idx < KEYWORDS_LEN; ++kw_idx) {
		char* keyword = keywords[kw_idx];
		if (lexed_identifier ==
			str_intern(keyword)) {
			type = T_KEYWORD;
		}
	}

	add_in(type);
}

void Lexer::number() {
	u64 number_start_line = line;
	u64 number_start_column = compute_column_on_current();
	TokenType type = T_INTEGER;
	while (isdigit(*current)) {
		current++;
	}

	if (*current == '.') {
		current++;
		if (!isdigit(*current)) {
			error("expect numeral literal after ‘.’;");
		}

		type = T_FLOAT64;
		while (isdigit(*current)) {
			current++;
		}
	}

	if (type == T_INTEGER) {
		u64 converted_value = strtoul(str_intern_range(start, current), null, 10);
		if (converted_value == ULONG_MAX && errno == ERANGE) {
			warning_at_rng(
					number_start_line, 
					number_start_column, 
					(current - start), 
					"integer overflow; value will be wrapped;");
		}
	}

	add_in(type);
}

void Lexer::string() {
	u64 quote_line = line;
	u64 quote_column = compute_column();

	start++;	// don't include the double quotes
	current++;
	while (*current != '"') {
		if (is_at_end()) {
			error_at(quote_line,
					 quote_column,
					 "missing terminating ‘\"’;");
			return;
		}
		current++;
	}

	add_in(T_STRING);
	current++;
}

void Lexer::chr() {
	start++;	// don't include the double quotes
	current++;

	if (*current == '\\') {
		escape_chars();
	}
	else {
		current++;
	}

	if (*current != '\'') {
		error("char literals can only hold a single character;");
		return;
	}

	add_in(T_CHAR);
	current++;
}

void Lexer::newline() {
	last_to_last_newline = last_newline;
	last_newline = current;
	line++;
	current++;
}

void Lexer::escape_chars() {
	current++;
	switch (*current) {
	case '\'':
	case '"':
	case 'n':
	case 'r':
	case 't':
		break;
	
	default:
		error("invalid escape character ‘%c’;", *current);
		break;
	}

	current++;
}

void Lexer::backslash() {
	current++;
	switch (*current) {
	case ' ':
	case '\n':
	case '\t':
	case '\r':
		break;
	
	default:
		error("invalid escape character ‘%c’;", *current);
		break;
	}

	current++;
}

bool Lexer::match(char c) {
	if (!is_at_end() && current < (srcfile->contents + srcfile->len)) {
		if (*(current + 1) == c) {
			current++;
			return true;
		}
	}
	return false;
}

bool Lexer::is_at_end() {
	if (current >= (srcfile->contents + srcfile->len) ||
		*current == '\0') {
		return true;
	}
	return false;
}

void Lexer::add(TokenType type) {
	current++;
	add_in(type);
}

void Lexer::add_in(TokenType type) {
	buf_push(tokens, token_create(
				 str_intern_range(start, current),
				 start,
				 current,
				 type,
				 srcfile,
				 line, 
				 compute_column(),
				 current - start));
}

void Lexer::add_eof() {
	Token* last_token = null;
	char* eof_string = "*EOF*";
	if (!buf_empty(tokens)) {
		last_token = buf_last(tokens);
		buf_push(tokens, token_create(
					 eof_string,
					 last_token->end,
					 last_token->end,
					 T_EOF,
					 srcfile,
					 last_token->line, 
					 last_token->column +
					 last_token->char_count,
					 1));
	}
	
	else {
		buf_push(tokens, token_create(
					 eof_string,
					 null,
					 null,
					 T_EOF,
					 srcfile,
					 1,
					 1,
					 1));
	}
}

u64 Lexer::compute_column() {
	u64 column = start - last_newline;
	if (line == 1) {
		column++;
	}
	return column;
}

u64 Lexer::compute_column_on_current() {
	u64 column = current - last_newline;
	if (line == 1) {
		column++;
	}
	return column;
}

void Lexer::error(const char* fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	print_error_at(srcfile, line, compute_column_on_current(), 1, fmt, ap);
	va_end(ap);
	error_count++;
}

void Lexer::error_at(u64 _line, u64 _column, const char* fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	print_error_at(srcfile, _line, _column, 1, fmt, ap);
	va_end(ap);
	error_count++;
}

void Lexer::warning(const char* fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	print_warning_at(srcfile, line, compute_column_on_current(), 1, fmt, ap);
	va_end(ap);
}

void Lexer::warning_at(u64 _line, u64 _column, const char* fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	print_warning_at(srcfile, _line, _column, 1, fmt, ap);
	va_end(ap);
}

void Lexer::warning_at_rng(u64 _line, u64 _column, u64 mark_len, const char* fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	print_warning_at(srcfile, _line, _column, mark_len, fmt, ap);
	va_end(ap);
}
