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

#define CONSUME_DATA_TYPE_CON(name)				\
	DataType* name = null;						\
	{											\
		CURRENT_ERROR;							\
		name = consume_data_type();				\
		CONTINUE_ERROR;							\
	}

#define CONSUME_DATA_TYPE_REC(name)				\
	DataType* name = consume_data_type();		\
	RECOVER;									\

#define CONSUME_IDENTIFIER_CON(name)			\
	Token* name = null;							\
	{											\
		CURRENT_ERROR;							\
		name = consume_identifier();			\
		CONTINUE_ERROR;							\
	}

#define CONSUME_IDENTIFIER_REC(name)			\
	Token* name = consume_identifier();			\
	RECOVER;									\

#define CONSUME_SEMICOLON						\
	{											\
		CURRENT_ERROR;							\
		consume_semicolon();					\
		EXIT_ERROR null;						\
	}

#define CONSUME_SEMICOLON_CON					\
	{											\
		CURRENT_ERROR;							\
		consume_semicolon();					\
		CONTINUE_ERROR;							\
	}

#define CONSUME_LBRACE							\
	{											\
		CURRENT_ERROR;							\
		consume_lbrace();						\
		EXIT_ERROR null;						\
	}
	
#define CONSUME_LBRACE_REC						\
	consume_lbrace();							\
	RECOVER;

#define EXPR(name)								\
	Expr* name = null;							\
	{											\
		CURRENT_ERROR;							\
		name = expr();							\
		EXIT_ERROR null;						\
	}

/* custom initialization */
#define EXPR_CI(name, func)						\
	Expr* name = null;							\
	{											\
		CURRENT_ERROR;							\
		name = func();							\
		EXIT_ERROR null;						\
	}

/* no declaration */
#define EXPR_ND(name)							\
	{											\
		CURRENT_ERROR;							\
		name = expr();							\
		EXIT_ERROR null;						\
	}

/* no declaration, recover */
#define EXPR_ND_REC(name)						\
	name = expr();								\
	RECOVER;

#define STMT(name)								\
	Stmt* name = null;							\
	{											\
		CURRENT_ERROR;							\
		name = stmt();							\
		EXIT_ERROR null;						\
	}

#define STMT_CON(name)							\
	Stmt* name = null;							\
	{											\
		CURRENT_ERROR;							\
		name = stmt();							\
		CONTINUE_ERROR;							\
	}

#define DECL_CON(name)							\
	Stmt* name = null;							\
	{											\
		CURRENT_ERROR;							\
		name = decl();							\
		CONTINUE_ERROR;							\
	}

#define RECOVER									\
	while (error_panic) {						\
		sync_to_next_statement();				\
	} 

#define STMT_CREATE(name) Stmt* name = new Stmt; 

ParserOutput Parser::parse(Token** _tokens, SourceFile* _srcfile) {
	tokens = _tokens;
	srcfile = _srcfile;
	
	stmts = null;
		
	token_idx = 0;
	tokens_len = buf_len(_tokens);
	
	error_count = 0;
	error_panic = false;
	error_loc = GLOBAL;
	error_brace_count = 0;
	error_lbrace_parsed = false;

	while (current()->type != T_EOF) {
		Stmt* stmt = decl_global();
		if (stmt) {
			buf_push(stmts, stmt);
		}
	}

	ParserOutput output;
	output.stmts = stmts;
	output.error_occured = (error_count > 0 ?
							ETHER_ERROR :
							ETHER_SUCCESS);
	return output;
}

Stmt* Parser::decl_global() {
	if (error_panic) {
		sync_to_next_statement();
		return null;
	}

	if (match_keyword("extern")) {
		CONSUME_IDENTIFIER(identifier);
		if (match_lparen()) {
			// extern function
			Stmt** params = null;
			if (!match_rparen()) {
				do {
					CONSUME_IDENTIFIER(p_identifier);
					CONSUME_DATA_TYPE(p_data_type);
					buf_push(params, var_decl_create(
								 p_identifier,
								 p_data_type,
								 null,
								 true));
					CHECK_EOF(null);
				} while (match_by_type(T_COMMA));
				consume_rparen();
			}

			DataType* return_data_type = match_data_type();
			CONSUME_SEMICOLON;

			return func_decl_create(identifier,
									params,
									return_data_type,
									null,
									false,
									true);
		}

		CONSUME_DATA_TYPE(data_type);
		CONSUME_SEMICOLON;

		return var_decl_create(identifier,
							   data_type,
							   null,
							   false);
	}
	else {
		return decl();
	}
}

Stmt* Parser::decl() {
	if (error_panic) {
		sync_to_next_statement();
		return null;
	}
	
	if (match_identifier()) {
		Token* identifier = previous();
		DataType* data_type = match_data_type();
		
		DataType* func_data_type = null;
		bool is_public_function = false;
		Expr* initializer = null;
		
		if (data_type) {
			error_loc = GLOBAL;
			if (match_double_colon()) {
				EXPR_ND(initializer);
			}
			CONSUME_SEMICOLON;
			return var_decl_create(
				identifier,
				data_type,
				initializer,
				true);
		}
		
		else {
			if (!match_double_colon()) {
				error("expect ‘::’ or data type;");
				return null;
			}

			if (match_keyword("pub")) {
				is_public_function = true;				
			}
			
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
			
			Stmt** params = null;
			error_loc = FUNCTION_HEADER;
			if (has_params) {
				if (!empty_params) {
					consume_lparen();
					do {
						CONSUME_IDENTIFIER(p_identifier);
						CONSUME_DATA_TYPE(p_data_type);
						buf_push(params, var_decl_create(
									 p_identifier,
									 p_data_type,
									 null,
									 true));
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
					}
					goto got_lbrace;
				}
			}
			else {
				func_data_type = match_data_type();
			}

			if (match_lbrace()) {
				/* function */
			got_lbrace:
				error_loc = FUNCTION_BODY;
				Stmt** body = null;
				while (!match_rbrace()) {
					STMT_CON(s);
					if (s) {
						buf_push(body, s);
					}
					CHECK_EOF(null);
				}
				
				return func_decl_create(
					identifier,
					params,
					func_data_type,
					body,
					true,
					is_public_function);
			}
			else {
				error_loc = GLOBAL;
				previous_data_type(func_data_type);
				EXPR_ND(initializer);
				CONSUME_SEMICOLON;
				
				return var_decl_create(
					identifier,
					data_type,
					initializer,
					true);
			}
		}
	}
	else if (match_keyword("struct")) {
		error_loc = STRUCT_HEADER;
		CONSUME_IDENTIFIER(identifier);
		consume_lbrace();

		error_loc = STRUCT_BODY;
		Stmt** fields = null;
		while (!match_rbrace()) {
			DECL_CON(field);
			if (!field) {
				continue;
			}
			if (field->type == S_VAR_DECL) {
				bool error_here = false;
				if (field->var_decl.initializer) {
					error_expr(field->var_decl.initializer,
							   "field cannot have an initializer; ");
					error_here = true;
					error_panic = false;
				}
				if (!field->var_decl.data_type) {
					error_token(field->var_decl.identifier,
									"field must specify a type; ");
					error_here = true;
					error_panic = false;
				}

				if (error_here) {
					continue;
				}
			}
			buf_push(fields, field);
		}
		
		return struct_create(identifier,
							 fields);
	}
	
	else if (match_keyword("if") ||
			 match_keyword("elif") ||
			 match_keyword("else")) {
		error_loc = IF_HEADER;
		goto_previous_token();
		if (str_intern(current()->lexeme) ==
			str_intern("if")) {
			error("if statement requires function scope; ");
			RECOVER;
			return null;
		}
		else {
			error("%s requires preceding if statement and function scope;", current()->lexeme);
			RECOVER;
			return null;
		}
	}

	else if (match_keyword("for")) {
		error_loc = FOR_HEADER;
		goto_previous_token();
		error("for statement requires function scope;");
		RECOVER;
		return null;
	}

	else if (match_keyword("return")) {
		error_loc = GLOBAL;
		goto_previous_token();
		error("cannot return from global scope; ");
		RECOVER;
		return null;
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
	if (match_semicolon()) {
		return null;
	}
	
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
				EXPR_ND(initializer);
				CONSUME_SEMICOLON;
				
				return var_decl_create(
					identifier,
					data_type,
					initializer,
					true);				
			}
		}
		
		else {
			if (match_double_colon()) {
				EXPR_ND(initializer);
			}
			CONSUME_SEMICOLON;
			
			return var_decl_create(
				identifier,
				data_type,
				initializer,
				true);
		}
	}
	
	else if (match_keyword("if")) {
		STMT_CREATE(stmt);
		stmt->type = S_IF;
		stmt->if_stmt.elif_branch = null;
		stmt->if_stmt.else_branch = null;

		if (!if_branch(stmt, IF_IF_BRANCH)) {
			RECOVER;
		}

		while (match_keyword("elif")) {
			if_branch(stmt, IF_ELIF_BRANCH);
			CHECK_EOF(null);			
			RECOVER;
		}

		if (match_keyword("else")) {
			if_branch(stmt, IF_ELSE_BRANCH);
			RECOVER;
		}
		
		return stmt;
	}

	else if (match_keyword("for")) {
		error_loc = FOR_HEADER;
		Token* counter = null;
		Expr* counter_initializer = null;
		Expr* end = null;
		if (match_identifier()) {
			counter = previous();

			if (match_by_type(T_EQUAL)) {
				EXPR_ND_REC(counter_initializer);
			}
			
			if (!match_by_type(T_DOT_DOT)) {
				error("expect ‘..’;");
				RECOVER;
				return null;
			}
			EXPR_ND_REC(end);
		}

		CONSUME_LBRACE_REC;
		Stmt** body = null;
		error_loc = FOR_BODY;
		while (!match_rbrace()) {
			STMT_CON(s);
			if (s) {
				buf_push(body, s);
			}
			CHECK_EOF(null);		
		}

		return for_stmt_create(counter,
							   counter_initializer,
							   end,
							   body);
	}

	else if (match_keyword("return")) {
		Expr* to_return = null;
		if (!match_semicolon()) {
			EXPR_ND(to_return);
			CONSUME_SEMICOLON;
		}

		return return_stmt_create(to_return);
	}
	
	else if (match_keyword("elif") ||
			 match_keyword("else")) {
		error_loc = IF_HEADER;
		goto_previous_token();
		error("%s without preceding if statement;", current()->lexeme);
		RECOVER;
		return null;
	}
	
	return expr_stmt();
}

Stmt* Parser::if_branch(Stmt* if_stmt, IfBranchType type) {
	Expr* cond = null;
	error_loc = IF_HEADER;
	if (type != IF_ELSE_BRANCH) {
		EXPR_ND(cond);
	}

	CONSUME_LBRACE;
	Stmt** body = null;
	error_loc = IF_BODY;
	while (!match_rbrace()) {
		STMT_CON(s);
		if (s) {
			buf_push(body, s);
		}
		CHECK_EOF(null);		
	}

	IfBranch* branch = new IfBranch();
	branch->cond = cond;
	branch->body = body;
	
	switch (type) {
	case IF_IF_BRANCH:
		if_stmt->if_stmt.if_branch = branch;
		break;
	case IF_ELIF_BRANCH:
		buf_push(if_stmt->if_stmt.elif_branch, branch);
		break;
	case IF_ELSE_BRANCH:
		if_stmt->if_stmt.else_branch = branch;
		break;
	}

	return if_stmt;
}

Stmt* Parser::expr_stmt() {
	EXPR(e);
	CONSUME_SEMICOLON;
	return expr_stmt_create(e);
}

Stmt* Parser::struct_create(Token* identifier, Stmt** fields) {
	STMT_CREATE(stmt);
	stmt->type = S_STRUCT;
	stmt->struct_stmt.identifier = identifier;
	stmt->struct_stmt.fields = fields;
	buf_loop(fields, f) {
		Stmt* field = fields[f];
		if (field->type == S_FUNC_DECL) {
			field->func_decl.struct_in = stmt;
		}
	}
	return stmt;
}

Stmt* Parser::func_decl_create(Token* identifier, Stmt** params, DataType* return_data_type, Stmt** body, bool is_function, bool is_public) {
	STMT_CREATE(stmt);
	stmt->type = S_FUNC_DECL;
	stmt->func_decl.identifier = identifier;
	stmt->func_decl.params = params;
	stmt->func_decl.return_data_type = return_data_type;
	stmt->func_decl.body = body;
	stmt->func_decl.is_function = is_function;
	stmt->func_decl.is_public = is_public;
	stmt->func_decl.struct_in = null;
	return stmt;
}

Stmt* Parser::var_decl_create(Token* identifier, DataType* data_type, Expr* initializer, bool is_variable) {
	STMT_CREATE(stmt);
	stmt->type = S_VAR_DECL;
	stmt->var_decl.identifier = identifier;
	stmt->var_decl.data_type = data_type;
	stmt->var_decl.initializer = initializer;
	stmt->var_decl.is_variable = is_variable;
	return stmt;
}

Stmt* Parser::for_stmt_create(Token* counter, Expr* counter_initializer, Expr* end, Stmt** body) {
	STMT_CREATE(stmt);
	stmt->type = S_FOR;
	stmt->for_stmt.counter = counter;
	stmt->for_stmt.counter_initializer = counter_initializer;
	stmt->for_stmt.end = end;
	stmt->for_stmt.body = body;
	return stmt;
}

Stmt* Parser::return_stmt_create(Expr* to_return) {
	STMT_CREATE(stmt);
	stmt->type = S_RETURN;
	stmt->return_stmt.to_return = to_return;
	return stmt;
}

Stmt* Parser::expr_stmt_create(Expr* expr) {
	STMT_CREATE(stmt);
	stmt->type = S_EXPR_STMT;
	stmt->expr_stmt = expr;
	return stmt;
}

Expr* Parser::expr() {
	return expr_precedence_14();
}

Expr* Parser::expr_precedence_14() {
	EXPR_CI(left, expr_precedence_12);
	if (match_by_type(T_EQUAL)) {
		Token* equal_token = previous();
		EXPR_CI(value, expr_precedence_14);

		if (left->type == E_VARIABLE_REF ||
			left->type == E_ARRAY_ACCESS ||
			left->type == E_MEMBER_ACCESS ||
			(left->type == E_UNARY && left->unary.op->type == T_CARET)) {
			return binary_create(left, value, equal_token);
		}
		error_expr(left, "invalid assignment target;");
		return null;
	}
	return left;
}

Expr* Parser::expr_precedence_12() {
	EXPR_CI(left, expr_precedence_11);
	while (match_by_type(T_BAR_BAR)) {
		Token* op = previous();
		EXPR_CI(right, expr_precedence_11);
		left = binary_create(left, right, op);
	}
	return left;
}

Expr* Parser::expr_precedence_11() {
	EXPR_CI(left, expr_precedence_9);
	while (match_by_type(T_AMPERSAND_AMPERSAND)) {
		Token* op = previous();
		EXPR_CI(right, expr_precedence_9);
		left = binary_create(left, right, op);
	}
	return left;
}

Expr* Parser::expr_precedence_9() {
	EXPR_CI(left, expr_precedence_8);
	while (match_by_type(T_BAR)) {
		Token* op = previous();
		EXPR_CI(right, expr_precedence_8);
		left = binary_create(left, right, op);
	}
	return left;
}

Expr* Parser::expr_precedence_8() {
	EXPR_CI(left, expr_precedence_7);
	while (match_by_type(T_AMPERSAND)) {
		Token* op = previous();
		EXPR_CI(right, expr_precedence_7);
		left = binary_create(left, right, op);
	}
	return left;
}

Expr* Parser::expr_precedence_7() {
	EXPR_CI(left, expr_precedence_6);
	while (match_by_type(T_EQUAL_EQUAL) ||
		   match_by_type(T_BANG_EQUAL)) {
		Token* op = previous();
		EXPR_CI(right, expr_precedence_6);
		left = binary_create(left, right, op);
	}
	return left;
}

Expr* Parser::expr_precedence_6() {
	EXPR_CI(left, expr_precedence_5);
	while (match_by_type(T_LANGBKT) ||
		   match_by_type(T_LESS_EQUAL) ||
		   match_by_type(T_RANGBKT) ||
		   match_by_type(T_GREATER_EQUAL)) {
		Token* op = previous();
		EXPR_CI(right, expr_precedence_5);
		left = binary_create(left, right, op);
	}
	return left;
}

Expr* Parser::expr_precedence_5() {
	EXPR_CI(left, expr_precedence_4);
	while (match_by_type(T_LESS_LESS) ||
		   match_by_type(T_GREATER_GREATER)) {
		Token* op = previous();
		EXPR_CI(right, expr_precedence_4);
		left = binary_create(left, right, op);
	}
	return left;
}

Expr* Parser::expr_precedence_4() {
	EXPR_CI(left, expr_precedence_3);
	while (match_by_type(T_PLUS) ||
		   match_by_type(T_MINUS)) {
		Token* op = previous();
		EXPR_CI(right, expr_precedence_3);
		left = binary_create(left, right, op);
	}
	return left;
}

Expr* Parser::expr_precedence_3() {
	EXPR_CI(left, expr_precedence_2);
	while (match_by_type(T_ASTERISK) ||
		   match_by_type(T_SLASH) ||
		   match_by_type(T_PERCENT)) {
		Token* op = previous();
		EXPR_CI(right, expr_precedence_2);
		left = binary_create(left, right, op);
	}
	return left;
}

Expr* Parser::expr_precedence_2() {
	if (match_by_type(T_PLUS) ||
		match_by_type(T_MINUS)) {
		Token* op = previous();
		EXPR_CI(right, expr_precedence_2);		
		return unary_create(op, right);
	}

	if (match_by_type(T_BANG) ||
		match_by_type(T_TILDE)) {
		Token* op = previous();
		EXPR_CI(right, expr_precedence_2);
		return unary_create(op, right);
	}
	
	if (match_langbkt()) {
		Token* start = previous();		
		CONSUME_DATA_TYPE(cast_to);
		consume_rangbkt();
		EXPR_CI(right, expr_grouping);
		return cast_create(start, cast_to, right);
	}

	if (match_by_type(T_CARET)) {
		Token* op = previous();
		EXPR_CI(right, expr_precedence_2);
		return unary_create(op, right);
	}

	if (match_by_type(T_AMPERSAND)) {
		Token* op = previous();
		EXPR_CI(right, expr_precedence_2);
		return unary_create(op, right);
	}
	return expr_precedence_1();
}

Expr* Parser::expr_precedence_1() {
	EXPR_CI(left, expr_precedence_0);
	while (match_lparen() ||
		   match_lbracket() ||
		   match_by_type(T_DOT)) {
		
		if (previous()->type == T_LPAREN) {
			if (left->type != E_VARIABLE_REF &&
				left->type != E_MEMBER_ACCESS) {
				error_expr(left, "invalid operand to call expression; ");
				return null;
			}
			std::vector<Expr*>* args = null;
			bool args_buffer_initialized = false;
			if (!match_rparen()) {
				do {
					if (!args_buffer_initialized) {
						args = new std::vector<Expr*>();
						args_buffer_initialized = true;
					}

					EXPR_CI(arg, expr);	
					args->push_back(arg);
					CHECK_EOF(null);
				} while (match_by_type(T_COMMA));
				consume_rparen();
			}
			left = func_call_create(left, args);
		}

		else if (previous()->type == T_LBRACKET) {
			EXPR_CI(array_subscript, expr);
			consume_rbracket();
			left = array_access_create(left, array_subscript, previous());
		}

		else if (previous()->type == T_DOT) {
			CONSUME_IDENTIFIER(right);
			left = member_access_create(left, right);
		}
	}
	return left;
}

Expr* Parser::expr_precedence_0() {
	if (match_identifier()) {
		return variable_ref_create(previous());
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
	EXPR_CI(e, expr);
	CHECK_EOF(null);
	consume_rparen();
	return e;
}

#define EXPR_CREATE(name) Expr* name = new Expr();

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

Expr* Parser::unary_create(Token* op, Expr* right) {
	EXPR_CREATE(expr);
	expr->type = E_UNARY;
	expr->head = op;
	expr->tail = right->tail;
	expr->unary.op = op;
	expr->unary.right = right;
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

Expr* Parser::func_call_create(Expr* left, std::vector<Expr*>* args) {
	EXPR_CREATE(expr);
	expr->type = E_FUNC_CALL;
	expr->head = left->head;
	if (args == null) {
		expr->tail = left->tail;
	}
	else {
		expr->tail = args->at(args->size()-1)->tail;
	}
	expr->func_call.left = left;
	expr->func_call.args = args;
	return expr;
}

Expr* Parser::array_access_create(Expr* left, Expr* index, Token* end) {
	EXPR_CREATE(expr);
	expr->type = E_ARRAY_ACCESS;
	expr->head = left->head;
	expr->tail = end;
	expr->array_access.left = left;
	expr->array_access.index = index;
	return expr;
}

Expr* Parser::member_access_create(Expr* left, Token* right) {
	EXPR_CREATE(expr);
	expr->type = E_MEMBER_ACCESS;
	expr->head = left->head;
	expr->tail = right;
	expr->member_access.left = left;
	expr->member_access.right = right;
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
	if (current()->type == T_KEYWORD &&
		(str_intern(current()->lexeme) ==
		 str_intern(keyword))) {
		goto_next_token();
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
	bool array_matched = false;
	bool pointer_matched = false;
	Token* array_elem_count = null;
	Token* start = null;

	if (match_lbracket()) {
		if (!start) {
			start = previous();
		}
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
		if (current()->type != T_RBRACKET) {
			for (u8 i = 0; i < 2; ++i) {
				goto_previous_token();
			}
			return null;			
		}
		array_matched = true;
		array_elem_count = previous();
		consume_rbracket();
	}
	
	u8 pointer_count = 0;
	while (match_by_type(T_CARET)) {
		if (!start) {
			start = previous();
		}
		pointer_matched = true;
		pointer_count++;
	}

	Token* identifier = null;
	if (!match_identifier()) {
		if (array_matched) {
			for (u8 i = 0; i < 3; ++i) {
				goto_previous_token();
			}
			return null;
		}
		else if (pointer_matched) {
			error("expect type name;");
			return null;
		}
		return null;
	}
	
	identifier = previous();
	if (!start) {
		start = previous();
	}	
	return data_type_create(identifier,
							pointer_count,
							is_array,
							array_elem_count,
							start);
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
	return tokens[token_idx];
}

Token* Parser::previous() {
	if (token_idx >= tokens_len+1) {
		return null;
	}
	return tokens[token_idx-1];
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

void Parser::error_data_type(DataType* data_type, const char* fmt, ...) {
	assert(data_type);
	u64 char_count = data_type->identifier->end - data_type->start->start;
	va_list ap;
	va_start(ap, fmt);
	error_root(data_type->identifier->line,
			   data_type->identifier->column,
			   char_count,
			   fmt,
			   ap);
	va_end(ap);
}

void Parser::error_token(Token* token, const char* fmt, ...) {
	u64 char_count = token->end - token->start;
	va_list ap;
	va_start(ap, fmt);
	error_root(token->line,
			   token->column,
			   char_count,
			   fmt,
			   ap);
	va_end(ap);	
}

void Parser::sync_to_next_statement() {
	switch (error_loc) {
	case GLOBAL:
	case STRUCT_BODY:
	case FUNCTION_BODY:
	case IF_BODY:
	case FOR_BODY:
		if (match_semicolon()) {
			error_panic = false;
			error_brace_count = 0;
			error_lbrace_parsed = false;
			return;
		}
		break;
		
	case STRUCT_HEADER:
	case FUNCTION_HEADER:
	case IF_HEADER:
	case FOR_HEADER:
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
