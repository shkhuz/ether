#include <ether.hpp>
#include <resolve.hpp>
#include <stmt.hpp>
#include <expr.hpp>
#include <data_type.hpp>
#include <token.hpp>

#define CURRENT_ERROR u64 current_error_count = error_count;
#define EXIT_ERROR_VOID_RETURN if (error_count > current_error_count) return;
#define EXIT_ERROR(ret) if (error_count > current_error_count) return ret;

error_code Resolve::resolve(Stmt** _stmts) {
	stmts = _stmts;
	error_count = 0;
	data_type_strings = null;

	buf_loop(stmts, s) {
		resolve_stmt(stmts[s]);
	}
	
	destroy();
}

void Resolve::destroy() {
	buf_loop(data_type_strings, i) {
		free(data_type_strings[i]);
	}
	buf_free(data_type_strings);
}

void Resolve::resolve_stmt(Stmt* stmt) {
	switch (stmt->type) {
	case S_FUNC_DECL:
		resolve_func_decl(stmt);
		break;
	case S_VAR_DECL:
		resolve_var_decl(stmt);
		break;
	}
}

void Resolve::resolve_func_decl(Stmt* stmt) {
	if (!stmt->func_decl.is_function) {
		return;
	}

	buf_loop(stmt->func_decl.body, s) {
		resolve_stmt(stmt->func_decl.body[s]);
	}
}

void Resolve::resolve_var_decl(Stmt* stmt) {
	if (stmt->var_decl.initializer && stmt->var_decl.data_type) {
		CURRENT_ERROR;
		DataType* defined_type = stmt->var_decl.data_type;
		DataType* initializer_type = resolve_expr(stmt->var_decl.initializer);
		EXIT_ERROR_VOID_RETURN;

		DataTypeMatch match = data_type_match(defined_type, initializer_type);
		if (match == DT_NOT_MATCH) {
			error_expr(stmt->var_decl.initializer,
					   "cannot implicitly convert from ‘%s’ to ‘%s’;",
					   data_type_to_string(initializer_type),
					   data_type_to_string(defined_type));
			return;
		}
	}
	else if (!stmt->var_decl.data_type) {
		stmt->var_decl.data_type = resolve_expr(stmt->var_decl.initializer);
	}
}

DataType* Resolve::resolve_expr(Expr* expr) {
	switch (expr->type) {
	case E_BINARY:
		return resolve_binary_expr(expr);
	case E_UNARY:
		return resolve_unary_expr(expr);
	case E_CAST:
		return resolve_cast_expr(expr);
	case E_FUNC_CALL:
		return resolve_func_call(expr);
	case E_ARRAY_ACCESS:
		return resolve_array_access(expr);
	case E_MEMBER_ACCESS:
		return resolve_member_access(expr);
	case E_VARIABLE_REF:
		return resolve_variable_ref(expr);
	case E_NUMBER:
		return resolve_number_expr(expr);
	case E_STRING:
		return data_types.t_string;
	case E_CHAR:
		return data_types.t_char;
	case E_CONSTANT:
		return resolve_constant_expr(expr);
	}
}

DataType* Resolve::resolve_binary_expr(Expr* expr) {
	switch (expr->binary.op->type) {
	case T_PLUS:
	case T_MINUS:
	case T_ASTERISK:
	case T_SLASH:
	case T_PERCENT:
		return resolve_arithmetic_expr(expr);
		
	case T_AMPERSAND_AMPERSAND:
	case T_BAR_BAR:
		return resolve_logic_binary_expr(expr);

	case T_AMPERSAND:
	case T_BAR:
		return resolve_bitwise_binary_expr(expr);

	case T_EQUAL_EQUAL:
	case T_BANG_EQUAL:
	case T_LANGBKT:
	case T_LESS_EQUAL:
	case T_RANGBKT:
	case T_GREATER_EQUAL:
		return resolve_comparison_expr(expr);

	case T_LESS_LESS:
	case T_GREATER_GREATER:
		return resolve_bitshift_expr(expr);

	default:
		break;
	}
}

DataType* Resolve::resolve_arithmetic_expr(Expr* expr) {
	
}

DataType* Resolve::resolve_logic_binary_expr(Expr* expr) {

}

DataType* Resolve::resolve_bitwise_binary_expr(Expr* expr) {

}

DataType* Resolve::resolve_comparison_expr(Expr* expr) {

}

DataType* Resolve::resolve_bitshift_expr(Expr* expr) {

}

DataType* Resolve::resolve_unary_expr(Expr* expr) {

}

DataType* Resolve::resolve_cast_expr(Expr* expr) {

}

DataType* Resolve::resolve_func_call(Expr* expr) {

}

DataType* Resolve::resolve_array_access(Expr* expr) {

}

DataType* Resolve::resolve_member_access(Expr* expr) {

}

DataType* Resolve::resolve_variable_ref(Expr* expr) {

}

DataType* Resolve::resolve_number_expr(Expr* expr) {

}

DataType* Resolve::resolve_constant_expr(Expr* expr) {

}	

DataTypeMatch Resolve::data_type_match(DataType* a, DataType* b) {
	if (a && b) {
		if (a->is_array != b->is_array) {
			return DT_NOT_MATCH;
		}
		if (a->is_array) {
			if (!is_token_equal(a->array_elem_count, b->array_elem_count)) {
				return DT_NOT_MATCH;
			}
		}

		if (a->pointer_count != b->pointer_count) {
			return DT_NOT_MATCH;
		}

		if (!is_token_equal(a->identifier, b->identifier)) {
			return DT_NOT_MATCH;
		}

		return DT_MATCH;
	}
	
	return DT_NOT_MATCH;
}

char* Resolve::data_type_to_string(DataType* data_type) {
	/* TODO: this is highly inefficient; use maps */
	u64 main_type_len = strlen(data_type->identifier->lexeme);
	u64 len = main_type_len + data_type->pointer_count;
	u64 array_elem_count_len = 0;
	if (data_type->is_array) {
		array_elem_count_len = strlen(data_type->array_elem_count->lexeme);
		len += 2;
		len += array_elem_count_len;
	}

	u64 idx = 0;
	char* str = (char*)malloc(len + 1);
	
	if (data_type->is_array) {
		str[0] = '[';
		idx++;
		strncpy(str + idx, data_type->array_elem_count->lexeme, array_elem_count_len);
		idx += array_elem_count_len;
		str[idx] = ']';
	}

	for (u8 i = 0; i < data_type->pointer_count; ++i) {
		str[idx] = '^';
		idx++;
	}

	strncpy(str + idx, data_type->identifier->lexeme, main_type_len);
	
	str[len] = '\0';
	buf_push(data_type_strings, str);
	return str;

}

void Resolve::error_root(SourceFile* srcfile, u64 line, u64 column, u64 char_count, const char* fmt, va_list ap) {
	print_error_at(
		srcfile,
		line,
		column,
		char_count,
		fmt,
		ap);
	error_count++;
}

void Resolve::warning_root(SourceFile* srcfile, u64 line, u64 column, u64 char_count, const char* fmt, va_list ap) {
	print_warning_at(
		srcfile,
		line,
		column,
		char_count,
		fmt,
		ap);
}
