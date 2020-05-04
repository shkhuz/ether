#include <ast_printer.hpp>

void AstPrinter::print(Stmt** _stmts) {
	stmts = _stmts;
	tab_count = 0;

	print_string("\n[─────────────────────────]\n");
	buf_loop(_stmts, s) {
		print_stmt(_stmts[s]);
	}
	print_string("[──────────────────────────]\n");
}

void AstPrinter::print_stmt(Stmt* stmt) {
	pre_stmt();
	switch (stmt->type) {
	case S_STRUCT:
		print_struct_stmt(stmt);
		break;
	case S_FUNC_DECL:
		print_func_decl(stmt);
		break;
	case S_VAR_DECL:
		print_var_decl(stmt);
		break;
	case S_IF:
		print_if_stmt(stmt);
		break;
	case S_FOR:
		print_for_stmt(stmt);
		break;
	case S_RETURN:
		print_return_stmt(stmt);
		break;
	case S_EXPR_STMT:
		print_expr_stmt(stmt);
		break;
	}
	print_newline();
}

void AstPrinter::pre_stmt() {
	print_tabs_by_indentation();
	print_tree_node_literal();
}

void AstPrinter::print_struct_stmt(Stmt* stmt) {
	print_string("STRUCT ");
	print_token(stmt->struct_stmt.identifier);
	print_newline();

	tab_count++;
	buf_loop(stmt->struct_stmt.fields, f) {
		print_stmt(stmt->struct_stmt.fields[f]);
	}
	tab_count--;
}

void AstPrinter::print_func_decl(Stmt* stmt) {
	if (!stmt->func_decl.is_function) {
		print_string("EXTERN ");
	}
	
	print_string("FUNC ");
	print_token(stmt->func_decl.identifier);
	print_space();

	if (stmt->func_decl.params) {
		print_lparen();
		Stmt** params = stmt->func_decl.params;
		buf_loop(params, p) {
			Stmt* param = params[p];
			print_token(param->var_decl.identifier);
			print_space();
			print_data_type(param->var_decl.data_type);
			if (p != buf_len(params)-1) {
				print_string(", ");
			}
		}
		print_rparen();
		print_space();
	}
	
	if (stmt->func_decl.return_data_type) {
		print_data_type(stmt->func_decl.return_data_type);
	}

	print_newline();

	if (stmt->func_decl.is_function) {
		tab_count++;
		Stmt** body = stmt->func_decl.body;
		if (body) {
			buf_loop(body, s) {
				print_stmt(body[s]);
			}
		}
		tab_count--;
	}
}

void AstPrinter::print_var_decl(Stmt* stmt) {
	if (!stmt->var_decl.is_variable) {
		print_string("EXTERN ");
	}
	
	print_string("VAR ");
	print_token(stmt->var_decl.identifier);
	if (stmt->var_decl.data_type) {
		print_space();
		print_data_type(stmt->var_decl.data_type);
	}
	if (stmt->var_decl.initializer) {
		print_string(" = ");
		print_expr(stmt->var_decl.initializer);
	}
}

void AstPrinter::print_if_stmt(Stmt* stmt) {
	print_if_branch(stmt->if_stmt.if_branch, IF_IF_BRANCH);

	buf_loop(stmt->if_stmt.elif_branch, b) {
		print_if_branch(stmt->if_stmt.elif_branch[b], IF_ELIF_BRANCH);
	}

	if (stmt->if_stmt.else_branch) {
		print_if_branch(stmt->if_stmt.else_branch, IF_ELSE_BRANCH);
	}
}

void AstPrinter::print_if_branch(IfBranch* branch, IfBranchType type) {
	if (type != IF_IF_BRANCH) {
		pre_stmt();
	}

	char* if_type_literal;
	switch (type) {
	case IF_IF_BRANCH:
		if_type_literal = "IF";
		break;
	case IF_ELIF_BRANCH:
		if_type_literal = "ELIF";
		break;
	case IF_ELSE_BRANCH:
		if_type_literal = "ELSE";
		break;
	}
	print_string(if_type_literal);
	print_space();

	if (type != IF_ELSE_BRANCH) {
		print_expr(branch->cond);
	}
	print_newline();

	tab_count++;
	Stmt** body = branch->body;
	if (body) {
		buf_loop(body, s) {
			print_stmt(body[s]);
		}
	}
	tab_count--;
}

void AstPrinter::print_for_stmt(Stmt* stmt) {
	print_string("FOR ");
	if (stmt->for_stmt.counter) {
		print_token(stmt->for_stmt.counter);
		print_space();

		if (stmt->for_stmt.counter_initializer) {
			print_string("= ");
			print_expr(stmt->for_stmt.counter_initializer);
			print_space();
		}

		print_string(".. ");
		print_expr(stmt->for_stmt.end);
	}
	print_newline();
	
	tab_count++;
	Stmt** body = stmt->for_stmt.body;
	if (body) {
		buf_loop(body, s) {
			print_stmt(body[s]);
		}
	}
	tab_count--;
}

void AstPrinter::print_return_stmt(Stmt* stmt) {
	print_string("RETURN ");
	if (stmt->return_stmt.to_return) {
		print_expr(stmt->return_stmt.to_return);
	}
}

void AstPrinter::print_expr_stmt(Stmt* stmt) {
	print_string("EXPR ");
	print_expr(stmt->expr_stmt);
}

void AstPrinter::print_expr(Expr* expr) {
	switch (expr->type) {
	case E_BINARY:
		print_binary(expr);
		break;
	case E_UNARY:
		print_unary(expr);
		break;
	case E_CAST:
		print_cast(expr);
		break;
	case E_FUNC_CALL:
		print_func_call(expr);
		break;
	case E_ARRAY_ACCESS:
		print_array_access(expr);
		break;
	case E_MEMBER_ACCESS:
		print_member_access(expr);
		break;
	case E_VARIABLE_REF:
		print_token(expr->variable_ref.identifier);
		break;
	case E_NUMBER:
		print_token(expr->number);
		break;
	case E_STRING:
		print_string(expr);
		break;
	case E_CHAR:
		print_char(expr);
		break;
	}
}

void AstPrinter::print_binary(Expr* expr) {
	print_lparen();
	print_expr(expr->binary.left);
	print_space();
	print_token(expr->binary.op);
	print_space();
	print_expr(expr->binary.right);
	print_rparen();
}

void AstPrinter::print_unary(Expr* expr) {
	print_lparen();
	print_token(expr->unary.op);
	print_expr(expr->unary.right);
	print_rparen();
}

void AstPrinter::print_cast(Expr* expr) {
	print_string("cast<");
	print_data_type(expr->cast.cast_to);
	print_string(">(");
	print_expr(expr->cast.right);
	print_rparen();
}

void AstPrinter::print_func_call(Expr* expr) {
	print_expr(expr->func_call.left);
	print_lparen();
	if (expr->func_call.args) {
		auto args = expr->func_call.args;
		for (u64 i = 0; i < args->size(); ++i) {
			Expr* arg = args->at(i);
			print_expr(arg);
			if (i != args->size()-1) {
				print_string(", ");
			}
		}
	}
	print_rparen();
}

void AstPrinter::print_array_access(Expr* expr) {
	print_expr(expr->array_access.left);
	print_char('[');
	print_expr(expr->array_access.index);
	print_char(']');
}

void AstPrinter::print_member_access(Expr* expr) {
	print_lparen();
	print_expr(expr->member_access.left);
	print_char('.');
	print_token(expr->member_access.right);
	print_rparen();
}

void AstPrinter::print_string(Expr* expr) {
	print_char('"');
	print_token(expr->string);
	print_char('"');
}

void AstPrinter::print_char(Expr* expr) {
	print_char('\'');
	print_token(expr->chr);
	print_char('\'');
}

void AstPrinter::print_data_type(DataType* data_type) {
	if (data_type->is_array) {
		print_char('[');
		print_token(data_type->array_elem_count);
		print_char(']');
	}
	for (u8 i = 0; i < data_type->pointer_count; ++i) {
		print_char('^');
	}
	print_token(data_type->identifier);
}

void AstPrinter::print_token(Token* token) {
	print_string(token->lexeme);
}

void AstPrinter::print_string(char* str) {
	printf("%s", str);
}

void AstPrinter::print_lparen() {
	print_char('(');
}

void AstPrinter::print_rparen() {
	print_char(')');
}

void AstPrinter::print_space() {
	print_char(' ');
}

void AstPrinter::print_newline() {
	print_char('\n');
}

void AstPrinter::print_char(char c) {
	printf("%c", c);
}

void AstPrinter::print_tabs_by_indentation() {
	for (u64 i = 0; i < tab_count; ++i) {
		printf("   ");
	}
}

void AstPrinter::print_tree_node_literal() {
	print_string("└─ ");
}
