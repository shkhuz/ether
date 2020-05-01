#include <ether.hpp>
#include <parser.hpp>

#define CURRENT_ERROR u64 last_error_count = error_count
#define EXIT_ERROR if (error_count > last_error_count) return
#define CONTINUE_ERROR if (error_count > last_error_count) continue

#define CHECK_EOF(x)							\
	if (current()->type == T_EOF) {				\
		error("unexpected end of file;");		\
		return (x);								\
	}

#define CHECK_EOF_VOID_RETURN					\
	if (current()->type == T_EOF) {				\
		error("unexpected end of file;");		\
		return;									\
	}

#define CONSUME_IDENTIFIER(name)				\
	Token* name = null;							\
	{											\
		CURRENT_ERROR;							\
		name = consume_identifier();			\
		EXIT_ERROR null;						\
	}

#define CONSUME_DATA_TYPE(name)					\
	DataType* name = null;						\
	{											\
		CURRENT_ERROR;							\
		name = consume_data_type();				\
		EXIT_ERROR null;						\
	}

#define CONSUME_IDENTIFIER_CON(name)			\
	Token* name = null;							\
	{											\
		CURRENT_ERROR;							\
		name = consume_identifier();			\
		CONTINUE_ERROR;							\
	}

#define CONSUME_DATA_TYPE_CON(name)				\
	DataType* name = null;						\
	{											\
		CURRENT_ERROR;							\
		name = consume_data_type();				\
		CONTINUE_ERROR;							\
	}

ParserOutput Parser::parse(std::vector<Token*>* _tokens, SourceFile* _srcfile) {
	tokens = _tokens;
	srcfile = _srcfile;
	
	stmts = new std::vector<Stmt*>();
		
	token_idx = 0;
	tokens_len = tokens->size();
	
	error_count = 0;
	error_panic = false;
	error_loc = GLOBAL;
	error_brace_count = 0;
	error_lbrace_parsed = false;

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
	if (error_panic) {
		sync_to_next_statement();
		return null;
	}
	
	if (match_identifier()) {
		Token* identifier = previous();
		DataType* data_type;
		{
			CURRENT_ERROR;
			data_type = match_data_type();
			EXIT_ERROR null;
		}
		DataType* func_data_type = null;
		Expr* initializer = null;
		
		if (data_type) {
			error_loc = GLOBAL;
			if (match_double_colon()) {
				CURRENT_ERROR;
				initializer = expr();
				EXIT_ERROR null;
			}
			consume_semicolon();
			return var_decl_create(
				identifier,
				data_type,
				initializer);
		}
		
		else {
			consume_double_colon();

			bool has_params = false;
			bool empty_params = false;
			DataType* param_data_type_for_lookback = null;
			DataType* func_data_type_for_lookback = null;
			{
				CURRENT_ERROR;
				if (match_lparen()) {
					if (match_identifier()) {
						if ((param_data_type_for_lookback = match_data_type())) {
							has_params = true;
							previous_data_type(param_data_type_for_lookback);
						}
						goto_previous_token();
					}
					else if (match_rparen()) {
						if ((func_data_type_for_lookback = match_data_type())) {
							if (match_lbrace()) {
								has_params = true;
								empty_params = true;
								goto_previous_token();
							}
							previous_data_type(func_data_type_for_lookback);
						}
						goto_previous_token();
					}
					goto_previous_token();
				}
				EXIT_ERROR null;
			}
			
			std::vector<Stmt*>* params = null;
			error_loc = FUNCTION_HEADER;
			if (has_params) {
				if (!empty_params) {
					params = new std::vector<Stmt*>();
					consume_lparen();
					do {
						CONSUME_IDENTIFIER(p_identifier);
						CONSUME_DATA_TYPE(p_data_type);
						params->push_back(var_decl_create(
											  p_identifier,
											  p_data_type,
											  null));
						CHECK_EOF(null);
					} while (match_by_type(T_COMMA));
					consume_rparen();
				}
				else {
					consume_lparen();
					consume_rparen();
				}
			}

			if (has_params) {
				if (!(func_data_type = match_data_type())) {
					if (!match_lbrace()) {
						error("expect function return type;");
						goto got_lbrace;
					}
				}
			}
			else {
				func_data_type = match_data_type();
			}

			if (match_lbrace()) {
				/* function */
			got_lbrace:
				error_loc = FUNCTION_BODY;
				std::vector<Stmt*>* body = null;
				bool buffer_allocated = false;
				while (!match_rbrace()) {
					CURRENT_ERROR;
					Stmt* s = stmt();
					CONTINUE_ERROR;
					if (s) {
						if (!buffer_allocated) {
							body = new std::vector<Stmt*>();
							buffer_allocated = true;
						}
						body->push_back(s);
					}
					CHECK_EOF(null);
				}
				
				return func_decl_create(
					identifier,
					params,
					func_data_type,
					body);
			}
			else {
				error_loc = GLOBAL;
				previous_data_type(func_data_type);
				CURRENT_ERROR;
				initializer = expr();
				EXIT_ERROR null;
				consume_semicolon();
				return var_decl_create(
					identifier,
					data_type,
					initializer);
			}
		}
	}
	else if (match_keyword("struct")) {
		error_loc = STRUCT_HEADER;
		Token* identifier = null;
		{
			CURRENT_ERROR;
			identifier = consume_identifier();
			EXIT_ERROR null;
		}
		consume_lbrace();

		error_loc = STRUCT_BODY;
		std::vector<Stmt*>* fields = null;
		bool field_buffer_initialized = false;
		while (!match_rbrace()) {
			if (!field_buffer_initialized) {
				fields = new std::vector<Stmt*>();
				field_buffer_initialized = true;
			}

			CONSUME_IDENTIFIER_CON(f_identifier);
			CONSUME_DATA_TYPE_CON(f_data_type);
			consume_semicolon();
			fields->push_back(var_decl_create(
								  f_identifier,
								  f_data_type,
								  null));
			CHECK_EOF(null);
		}
		return struct_create(identifier,
							 fields);
	}	
	else {
		error("expect top-level decl statement;");
	}

	return null;
}

Stmt* Parser::stmt() {
	if (error_panic) {
		sync_to_next_statement();
		return null;
	}
	
	error_loc = FUNCTION_BODY;
	if (match_identifier()) {
		Token* identifier = previous();
		DataType* data_type = match_data_type();
		Expr* initializer = null;
		if (!data_type) {
			if (!match_double_colon()) {
				goto_previous_token();
				return expr_stmt();
			}
			else {
				CURRENT_ERROR;
				initializer = expr();
				consume_semicolon();
				EXIT_ERROR null;
				return var_decl_create(
					identifier,
					data_type,
					initializer);				
			}
		}
		
		else {
			if (match_double_colon()) {
				CURRENT_ERROR;
				initializer = expr();
				EXIT_ERROR null;
			}
			consume_semicolon();
			return var_decl_create(
				identifier,
				data_type,
				initializer);
		}
	}
	return expr_stmt();
}

Stmt* Parser::expr_stmt() {
	Expr* e = expr();
	consume_semicolon();
	return expr_stmt_create(e);
}

#define STMT_CREATE(name) Stmt* name = new Stmt; 

Stmt* Parser::struct_create(Token* identifier, std::vector<Stmt*>* fields) {
	STMT_CREATE(stmt);
	stmt->type = S_STRUCT;
	stmt->struct_stmt.identifier = identifier;
	stmt->struct_stmt.fields = fields;
	return stmt;
}

Stmt* Parser::func_decl_create(Token* identifier, std::vector<Stmt*>* params, DataType* return_data_type, std::vector<Stmt*>* body) {
	STMT_CREATE(stmt);
	stmt->type = S_FUNC_DECL;
	stmt->func_decl.identifier = identifier;
	stmt->func_decl.params = params;
	stmt->func_decl.return_data_type = return_data_type;
	stmt->func_decl.body = body;
	return stmt;
}

Stmt* Parser::var_decl_create(Token* identifier, DataType* data_type, Expr* initializer) {
	STMT_CREATE(stmt);
	stmt->type = S_VAR_DECL;
	stmt->var_decl.identifier = identifier;
	stmt->var_decl.data_type = data_type;
	stmt->var_decl.initializer = initializer;
	return stmt;
}

Stmt* Parser::expr_stmt_create(Expr* expr) {
	STMT_CREATE(stmt);
	stmt->type = S_EXPR_STMT;
	stmt->expr_stmt = expr;
	return stmt;
}

Expr* Parser::expr() {
	return expr_assign();
}

Expr* Parser::expr_assign() {
	Expr* left = null;
	{
		CURRENT_ERROR;
		left = expr_binary_plus_minus();
		EXIT_ERROR null;
	}
	if (match_by_type(T_EQUAL)) {
		CURRENT_ERROR;
		Expr* value = expr_assign();
		EXIT_ERROR null;

		if (left->type == E_VARIABLE_REF) {
			return assign_create(left, value);
		}
		error_expr(left, "invalid assignment target;");
		return null;
	}
	return left;
}

Expr* Parser::expr_binary_plus_minus() {
	Expr* left = null;
	{
		CURRENT_ERROR;
		left = expr_cast();
		EXIT_ERROR null;
	}
	while (match_by_type(T_PLUS) ||
		   match_by_type(T_MINUS)) {
		Token* op = previous();
		
		CURRENT_ERROR;
		Expr* right = expr_binary_plus_minus();
		EXIT_ERROR null;
		
		left = binary_create(left, right, op);
		CHECK_EOF(null);
	}
	return left;
}

Expr* Parser::expr_cast() {
	if (match_langbkt()) {
		Token* start = previous();
		
		CONSUME_DATA_TYPE(cast_to);
		consume_rangbkt();

		CURRENT_ERROR;
		Expr* right = expr_grouping();
		EXIT_ERROR null;
		
		return cast_create(start, cast_to, right);
	}
	return expr_primary();
}

Expr* Parser::expr_primary() {
	if (match_identifier()) {
		Token* identifier = previous();
		if (match_lparen()) {
			std::vector<Expr*>* args = null;
			bool args_buffer_initialized = false;
			if (!match_rparen()) {
				do {
					if (!args_buffer_initialized) {
						args = new std::vector<Expr*>();
						args_buffer_initialized = true;
					}

					CURRENT_ERROR;
					Expr* arg = expr();
					EXIT_ERROR null;
					
					args->push_back(arg);
					CHECK_EOF(null);
				} while (match_by_type(T_COMMA));
				consume_rparen();
			}
			return func_call_create(identifier,
									args);
		}
		else {
			return variable_ref_create(identifier);
		}
	}	
	else if (match_by_type(T_INTEGER) ||
			 match_by_type(T_FLOAT32) ||
			 match_by_type(T_FLOAT64)) {
		return number_create(previous());
	}
	else if (match_by_type(T_STRING)) {
		return string_create(previous());
	}
	else if (match_by_type(T_CHAR)) {
		return char_create(previous());
	}
	else if (current()->type == T_LPAREN) {
		return expr_grouping();
	}
	else {
		error("unknown expression;");
	}
	return null;
}

Expr* Parser::expr_grouping() {
	consume_lparen();
	CURRENT_ERROR;
	Expr* e = expr();
	EXIT_ERROR null;
	CHECK_EOF(null);
	consume_rparen();
	return e;
}

#define EXPR_CREATE(name) Expr* name = new Expr();

Expr* Parser::assign_create(Expr* left, Expr* value) {
	EXPR_CREATE(expr);
	expr->type = E_ASSIGN;
	expr->head = left->head;
	expr->tail = value->tail;
	expr->assign.left = left;
	expr->assign.value = value;
	return expr;
}

Expr* Parser::binary_create(Expr* left, Expr* right, Token* op) {
	EXPR_CREATE(expr);
	expr->type = E_BINARY;
	expr->head = left->head;
	expr->tail = right->tail;
	expr->binary.left = left;
	expr->binary.right = right;
	expr->binary.op = op;
	return expr;
}

Expr* Parser::cast_create(Token* start, DataType* cast_to, Expr* right) {
	EXPR_CREATE(expr);
	expr->type = E_CAST;
	expr->head = start;
	expr->tail = right->tail;
	expr->cast.cast_to = cast_to;
	expr->cast.right = right;
	return expr;
}

Expr* Parser::func_call_create(Token* callee, std::vector<Expr*>* args) {
	EXPR_CREATE(expr);
	expr->type = E_FUNC_CALL;
	expr->head = callee;
	if (args == null) {
		expr->tail = callee;
	}
	else {
		expr->tail = args->at(args->size()-1)->tail;
	}
	expr->func_call.callee = callee;
	expr->func_call.args = args;
	return expr;
}

Expr* Parser::variable_ref_create(Token* identifier) {
	EXPR_CREATE(expr);
	expr->type = E_VARIABLE_REF;
	expr->head = identifier;
	expr->tail = identifier;
	expr->variable_ref.identifier = identifier;
	return expr;
}

Expr* Parser::number_create(Token* number) {
	EXPR_CREATE(expr);
	expr->type = E_NUMBER;
	expr->head = number;
	expr->tail = number;
	expr->number = number;
	return expr;
}

Expr* Parser::string_create(Token* string) {
	EXPR_CREATE(expr);
	expr->type = E_STRING;
	expr->head = string;
	expr->tail = string;
	expr->string = string;
	return expr;
}

Expr* Parser::char_create(Token* chr) {
	EXPR_CREATE(expr);
	expr->type = E_CHAR;
	expr->head = chr;
	expr->tail = chr;
	expr->chr = chr;
	return expr;
}

bool Parser::match_identifier() {
	if (match_by_type(T_IDENTIFIER)) {
		return true;
	}
	return false;
}

bool Parser::match_keyword(char* keyword) {
	if (current()->type == T_KEYWORD) {
		for (u64 i = 0; i < KEYWORDS_LEN; ++i) {
			if (str_intern(keyword) ==
				str_intern(keywords[i])) {
				goto_next_token();
				return true;
			}
		}
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

bool Parser::match_lparen() {
	if (match_by_type(T_LPAREN)) {
		return true;
	}
	return false;
}

bool Parser::match_rparen() {
	if (match_by_type(T_RPAREN)) {
		return true;
	}
	return false;
}

bool Parser::match_lbrace() {
	if (match_by_type(T_LBRACE)) {
		return true;
	}
	return false;
}

bool Parser::match_rbrace() {
	if (match_by_type(T_RBRACE)) {
		return true;
	}
	return false;
}

bool Parser::match_lbracket() {
	if (match_by_type(T_LBRACKET)) {
		return true;
	}
	return false;
}

bool Parser::match_rbracket() {
	if (match_by_type(T_RBRACKET)) {
		return true;
	}
	return false;
}

bool Parser::match_langbkt() {
	if (match_by_type(T_LANGBKT)) {
		return true;
	}
	return false;
}

bool Parser::match_rangbkt() {
	if (match_by_type(T_RANGBKT)) {
		return true;
	}
	return false;
}

bool Parser::match_semicolon() {
	if (match_by_type(T_SEMICOLON)) {
		return true;
	}
	return false;
}

DataType* Parser::match_data_type() {
	bool is_array = false;
	Token* array_elem_count = null;

	if (match_lbracket()) {
		is_array = true;
		if (!match_by_type(T_INTEGER)) {
			if (current()->type == T_RBRACKET) {
				error("unspecified array length; ");
				return null;
			}
			else {
				error("expect integer literal (size should be known at compile-time);");
				return null;
			}
		}
		array_elem_count = previous();
		consume_rbracket();
	}
	
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
			return null;
		}
		return null;
	}
	identifier = previous();
	return data_type_create(identifier,
							pointer_count,
							is_array,
							array_elem_count);
}

void Parser::previous_data_type(DataType* data_type) {
	if (!data_type) {
		return;
	}

	goto_previous_token(); // identifier

	for (u8 p = 0; p < data_type->pointer_count; p++) {
		goto_previous_token();
	}

	if (data_type->is_array) {
		goto_previous_token();
		goto_previous_token();
		goto_previous_token();
	}
}

Token* Parser::consume_identifier() {
	expect_by_type(T_IDENTIFIER, "expect identifier;");
	return previous();
}

DataType* Parser::consume_data_type() {
	CURRENT_ERROR;
	DataType* data_type = match_data_type();
	EXIT_ERROR null;
	if (data_type == null) {
		error("expect data type;");
		return null;
	}
	return data_type;
}

void Parser::consume_double_colon() {
	expect_by_type(T_DOUBLE_COLON, "expect ‘::’;"); 
}

void Parser::consume_lparen() {
	expect_by_type(T_LPAREN, "expect ‘(’;"); 
}

void Parser::consume_rparen() {
	expect_by_type(T_RPAREN, "expect ‘)’;"); 
}

void Parser::consume_lbrace() {
	expect_by_type(T_LBRACE, "expect ‘{’;"); 
}

void Parser::consume_rbrace() {
	expect_by_type(T_RBRACE, "expect ‘}’;"); 
}

void Parser::consume_lbracket() {
	expect_by_type(T_LBRACKET, "expect ‘[’;"); 
}

void Parser::consume_rbracket() {
	expect_by_type(T_RBRACKET, "expect ‘]’;"); 
}

void Parser::consume_langbkt() {
	expect_by_type(T_LANGBKT, "expect ‘<’;"); 
}

void Parser::consume_rangbkt() {
	expect_by_type(T_RANGBKT, "expect ‘>’;"); 
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

void Parser::error_root(u64 line, u64 column, u64 char_count, const char* fmt, va_list ap) {
	if (error_panic) {
		error_count++;		
		sync_to_next_statement();
		return;
	}
	error_panic = true;
	
	print_error_at(
		srcfile,
		line,
		column,
		char_count,
		fmt,
		ap);
	sync_to_next_statement();
	error_count++;		
}

void Parser::verror(const char* fmt, va_list ap) {
	va_list aq;
	va_copy(aq, ap);
	error_root(
		current()->line,
		current()->column,
		current()->char_count,
		fmt,
		aq);
	va_end(aq);
}

void Parser::error(const char* fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	verror(fmt, ap);
	va_end(ap);
}

void Parser::error_expr(Expr* expr, const char* fmt, ...) {
	u64 char_count = get_expr_char_count(expr);
	va_list ap;
	va_start(ap, fmt);
	error_root(expr->head->line,
			   expr->head->column,
			   char_count,
			   fmt,
			   ap);
	va_end(ap);
}

void Parser::sync_to_next_statement() {
	switch (error_loc) {
	case GLOBAL:
	case FUNCTION_BODY:
	case STRUCT_BODY:
		if (match_semicolon()) {
			error_panic = false;
			error_brace_count = 0;
			error_lbrace_parsed = false;
			return;
		}
		break;
		
	case STRUCT_HEADER:
	case FUNCTION_HEADER:
		if (current()->type == T_LBRACE) {
			if (!error_lbrace_parsed) {
				error_lbrace_parsed = true;
			}
			error_brace_count++;
		}
		else if (current()->type == T_RBRACE) {
			error_brace_count--;
		}
		
		if (error_brace_count == 0 && error_lbrace_parsed) {
			error_panic = false;
			error_brace_count = 0;
			error_lbrace_parsed = false;
			goto_next_token();
			return;
		}
		break;
	}
	goto_next_token();
}
