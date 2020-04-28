#include <ether.hpp>
#include <parser.hpp>

ParserOutput Parser::parse(std::vector<Token*>* _tokens, SourceFile* _srcfile) {
	tokens = _tokens;
	srcfile = _srcfile;
	
	stmts = new std::vector<Stmt*>();
	error_count = 0;
	
	token_idx = 0;
	tokens_len = tokens->size();

	while (current()->type != T_EOF) {
		Stmt* stmt = decl();
		if (stmt) {
			stmts->push_back(stmt);
		}
	}

	ParserOutput output;
	output.stmts = stmts;
	output.error_occured = (error_count > 0 ?
							ETHER_ERROR :
							ETHER_SUCCESS);
	return output;
}

Stmt* Parser::decl() {
	if (match_identifier()) {
		Token* identifier = previous();
		DataType* data_type = match_data_type();
		Expr* initializer = null;
		if (data_type != null) {
			if (match_double_colon()) {
				initializer = expr();
			}
		}
		else {
			consume_double_colon();
			initializer = expr();
		}
		consume_semicolon();
		return var_decl_create(
			data_type,
			identifier,
			initializer);
	}
	else {
		error("expect top-level decl statement;");
	}
	return null;
}

#define STMT_CREATE(name) Stmt* name = new Stmt();

Stmt* Parser::var_decl_create(DataType* data_type, Token* identifier, Expr* initializer) {
	STMT_CREATE(stmt);
	stmt->type = S_VAR_DECL;
	stmt->var_decl.data_type = data_type;
	stmt->var_decl.identifier = identifier;
	stmt->var_decl.initializer = initializer;
	return stmt;
}

Expr* Parser::expr() {
	return expr_primary();
}

Expr* Parser::expr_primary() {
	if (match_identifier()) {
		return variable_ref_create(previous());
	}
	else {
		error("unknown expression;");
	}
}

#define EXPR_CREATE(name) Expr* name = new Expr();

Expr* Parser::variable_ref_create(Token* identifier) {
	EXPR_CREATE(expr);
	expr->type = E_VARIABLE_REF;
	expr->head = identifier;
	expr->variable_ref.identifier = identifier;
	return expr;
}

bool Parser::match_identifier() {
	if (match_by_type(T_IDENTIFIER)) {
		return true;
	}
	return false;
}

bool Parser::match_by_type(TokenType type) {
	if (current()->type == type) {
		goto_next_token();
		return true;
	}
	return false;
}

bool Parser::match_double_colon() {
	if (match_by_type(T_DOUBLE_COLON)) {
		return true;
	}
	return false;
}

DataType* Parser::match_data_type() {
	u8 pointer_count = 0;
	bool pre_type_matched = false;
	while (match_by_type(T_CARET)) {
		pre_type_matched = true;
		pointer_count++;
	}

	Token* identifier = null;
	if (!match_identifier()) {
		if (pre_type_matched) {
			error("expect type name;");
		}
		return null;
	}
	identifier = previous();
	return data_type_create(identifier, pointer_count);
}

Token* Parser::consume_identifier() {
	expect_by_type(T_IDENTIFIER, "expect identifier;");
	return previous();
}

DataType* Parser::consume_data_type() {
	DataType* data_type = match_data_type();
	if (data_type == null) {
		error("expect data type;");
		return null;
	}
	return data_type;
}

void Parser::consume_double_colon() {
	expect_by_type(T_DOUBLE_COLON, "expect ‘::’;"); 
}

void Parser::consume_semicolon() {
	expect_by_type(T_SEMICOLON, "expect ‘;’;"); 
}

void Parser::expect_by_type(TokenType type, const char* fmt, ...) {
	if (match_by_type(type)) {
		return;
	}

	va_list ap;
	va_start(ap, fmt);
	verror(fmt, ap);
	va_end(ap);
}

Token* Parser::current() {
	if (token_idx >= tokens_len) {
		return null;
	}
	return (*tokens)[token_idx];
}

Token* Parser::previous() {
	if (token_idx >= tokens_len+1) {
		return null;
	}
	return (*tokens)[token_idx-1];
}

void Parser::goto_next_token() {
	if ((token_idx == 0 || (token_idx-1) < tokens_len) &&
		current()->type != T_EOF) {
		token_idx++;
	}
}

void Parser::goto_previous_token() {
	if (token_idx > 0) {
		token_idx--;
	}
}

void Parser::verror(const char* fmt, va_list ap) {
	va_list aq;
	va_copy(aq, ap);
	print_error_at(
		srcfile,
		current()->line,
		current()->column,
		current()->char_count,
		fmt,
		aq);
	va_end(aq);
	sync_to_next_statement();
	error_count++;
}

void Parser::error(const char* fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	verror(fmt, ap);
	va_end(ap);
}

void Parser::sync_to_next_statement() {
	while (current()->type != T_SEMICOLON &&
		   current()->type != T_EOF) {
		goto_next_token();
	}
	goto_next_token();
}
