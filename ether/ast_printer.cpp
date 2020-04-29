#include <ast_printer.hpp>

void AstPrinter::print(std::vector<Stmt*>* _stmts) {
	stmts = _stmts;
	tab_count = 0;

	print_string("\n[─────────────────────────]\n");
	for (auto it = stmts->begin(); it != stmts->end(); ++it) {
		Stmt* stmt = *it;
		print_stmt(stmt);
	}
	print_string("[──────────────────────────]\n");
}

void AstPrinter::print_stmt(Stmt* stmt) {
	print_tabs_by_indentation();
	print_string("└─ ");
	switch (stmt->type) {
	case S_FUNC_DECL:
		print_func_decl(stmt);
		break;
	case S_VAR_DECL:
		print_var_decl(stmt);
		break;
	case S_EXPR_STMT:
		print_expr_stmt(stmt);
		break;
	}
	print_newline();
}

void AstPrinter::print_func_decl(Stmt* stmt) {
	print_string("FUNC ");
	print_token(stmt->func_decl.identifier);
	print_space();

	if (stmt->func_decl.params) {
		print_lparen();
		auto params = stmt->func_decl.params;
		for (u64 i = 0; i < params->size(); ++i) {
			Stmt* param = (*params)[i];
			print_token(param->var_decl.identifier);
			print_space();
			print_data_type(param->var_decl.data_type);
			if (i != params->size()-1) {
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

	tab_count++;
	auto body = stmt->func_decl.body;
	if (body) {
		for (auto it = body->begin(); it != body->end(); ++it) {
			Stmt* s = *it;
			print_stmt(s);
		}
	}
	tab_count--;
}

void AstPrinter::print_var_decl(Stmt* stmt) {
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

void AstPrinter::print_expr_stmt(Stmt* stmt) {
	print_string("EXPR ");
	print_expr(stmt->expr_stmt);
}

void AstPrinter::print_expr(Expr* expr) {
	switch (expr->type) {
	case E_ASSIGN:
		print_assign(expr);
		break;
	case E_BINARY:
		print_binary(expr);
		break;
	case E_VARIABLE_REF:
		print_token(expr->variable_ref.identifier);
		break;
	case E_NUMBER:
		print_token(expr->number);
		break;
	}
}

void AstPrinter::print_assign(Expr* expr) {
	print_expr(expr->assign.left);
	print_string(" = ");
	print_expr(expr->assign.value);
}

void AstPrinter::print_binary(Expr* expr) {
	print_expr(expr->binary.left);
	print_space();
	print_token(expr->binary.op);
	print_space();
	print_expr(expr->binary.right);
}

void AstPrinter::print_data_type(DataType* data_type) {
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
