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
	case S_STRUCT:
		resolve_struct(stmt);
		break;
	case S_FUNC_DECL:
		resolve_func_decl(stmt);
		break;
	case S_VAR_DECL:
		resolve_var_decl(stmt);
		break;
	case S_IF:
		resolve_if_stmt(stmt);
		break;
	case S_FOR:
		resolve_for_stmt(stmt);
		break;
	case S_EXPR_STMT:
		resolve_expr_stmt(stmt);
		break;
	case S_BLOCK:
		resolve_block(stmt);
		break;
	}
}

void Resolve::resolve_struct(Stmt* stmt) {
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

void Resolve::resolve_if_stmt(Stmt* stmt) {
	resolve_if_branch(stmt->if_stmt.if_branch);

	buf_loop(stmt->if_stmt.elif_branch, b) {
		resolve_if_branch(stmt->if_stmt.elif_branch[b]);
	}

	if (stmt->if_stmt.else_branch) {
		resolve_if_branch(stmt->if_stmt.else_branch);
	}
}

void Resolve::resolve_if_branch(IfBranch* branch) {
	if (branch->cond) {
		CURRENT_ERROR;
		DataType* cond_type = resolve_expr(branch->cond);
		EXIT_ERROR_VOID_RETURN;

		DataTypeMatch match = data_type_match(cond_type, data_types.t_bool);
		if (match == DT_NOT_MATCH) {
			error_expr(branch->cond,
					   "expect ‘bool’ but got ’%s’;",
					   data_type_to_string(cond_type));
		}
	}

	buf_loop(branch->body, s) {
		resolve_stmt(branch->body[s]);
	}
}

void Resolve::resolve_for_stmt(Stmt* stmt) {
	CURRENT_ERROR;
	if (!stmt->for_stmt.counter->var_decl.initializer &&
		!stmt->for_stmt.counter->var_decl.data_type) {
		stmt->for_stmt.counter->var_decl.data_type = data_types.t_int;
	}
	
	resolve_var_decl(stmt->for_stmt.counter);
	DataType* counter_type = stmt->for_stmt.counter->var_decl.data_type;
	if (error_count == current_error_count) {
		DataTypeMatch match = data_type_integer(counter_type);
		if (match == DT_NOT_MATCH) {
			error_token(stmt->for_stmt.counter->var_decl.identifier,
						"expect integer type;");
		}

		DataType* end_type = resolve_expr(stmt->for_stmt.end);
		match = data_type_match(end_type, counter_type);
		if (match == DT_NOT_MATCH) {
			error_expr(stmt->for_stmt.end,
					   "expect ‘%s’ type;",
					   data_type_to_string(counter_type));
		}
	}

	buf_loop(stmt->for_stmt.body, s) {
		resolve_stmt(stmt->for_stmt.body[s]);
	}
}

void Resolve::resolve_expr_stmt(Stmt* stmt) {
	resolve_expr(stmt->expr_stmt);	
}

void Resolve::resolve_block(Stmt* stmt) {
	buf_loop(stmt->block, s) {
		resolve_stmt(stmt->block[s]);
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
	CURRENT_ERROR;
	DataType* left_type = resolve_expr(expr->binary.left);
	DataType* right_type = resolve_expr(expr->binary.right);
	EXIT_ERROR(null);

	DataTypeMatch match = data_type_match(left_type, right_type);
	if (match == DT_NOT_MATCH) {
		char* operation;
		switch (expr->binary.op->type) {
		case T_PLUS: operation = "add"; break;
		case T_MINUS: operation = "subtract"; break;
		case T_ASTERISK: operation = "multiply"; break;
		case T_SLASH: operation = "divide"; break;
		case T_PERCENT: operation = "mod"; break;
		default: assert(0); break;
		}
		error_token(expr->binary.op,
					"cannot %s operand types ‘%s’ and ‘%s’;",
					operation,
					data_type_to_string(left_type),
					data_type_to_string(right_type));
		return null;
	}
	return left_type;
	// TODO don't let operate on bools or custom types without pointers
}

DataType* Resolve::resolve_logic_binary_expr(Expr* expr) {
	bool error_here = false;
	for (int c = 0; c < 2; c++) {
		CURRENT_ERROR;
		Expr* current_expr = null;
		if (c == 0) {
			current_expr = expr->binary.left;
		}
		else if (c == 1) {
			current_expr = expr->binary.right;
		}
		DataType* type = resolve_expr(current_expr);
		EXIT_ERROR(null);
		// TODO: continue error ???

		DataTypeMatch match = data_type_match(type, data_types.t_bool);
		if (match == DT_NOT_MATCH) {
			error_here = true;
			error_expr(current_expr,
					   "operator ‘%s’ expects boolean;",
					   expr->binary.op->lexeme);
		}
	}
	
	if (error_here) return null;
	return data_types.t_bool;
}

DataType* Resolve::resolve_bitwise_binary_expr(Expr* expr) {
}

DataType* Resolve::resolve_comparison_expr(Expr* expr) {
	CURRENT_ERROR;
	DataType* left_type = resolve_expr(expr->binary.left);
	DataType* right_type = resolve_expr(expr->binary.right);
	EXIT_ERROR(null);

	DataTypeMatch match = data_type_match(left_type, right_type);
	if (match == DT_NOT_MATCH) {
		error_token(expr->binary.op,
					"operator ‘%s’ cannot operate on conflicting types ‘%s’ and ‘%s’;",
					expr->binary.op->lexeme,
					data_type_to_string(left_type),
					data_type_to_string(right_type));
		return null;
	}
	return data_types.t_bool;
	// TODO don't let operate on bools or custom types without pointers
}

DataType* Resolve::resolve_bitshift_expr(Expr* expr) {
}

DataType* Resolve::resolve_unary_expr(Expr* expr) {	
}

DataType* Resolve::resolve_cast_expr(Expr* expr) {
	CURRENT_ERROR;
	DataType* cast_type = expr->cast.cast_to;
	DataType* right_type = resolve_expr(expr->cast.right);
	EXIT_ERROR(null);

	if (cast_type->pointer_count == right_type->pointer_count) {
		if (cast_type->pointer_count == 0) {
			bool cast_is_custom_type = true;
			bool right_is_custom_type = true;
			for (u64 i = 0; i < BUILT_IN_TYPES_LEN; i++) {
				if (str_intern(cast_type->identifier->lexeme) ==
					str_intern(built_in_types[i])) {
					cast_is_custom_type = false;
				}

				if (str_intern(right_type->identifier->lexeme) ==
					str_intern(built_in_types[i])) {
					right_is_custom_type = false;
				}				
			}

			if (cast_is_custom_type) {
				error_data_type(cast_type,
								"cannot cast to custom type;");
			}
			if (right_is_custom_type) {
				error_expr(expr->cast.right,
						   "cannot cast a custom type;");
			}
			return cast_type;
		}
		else {
			return cast_type;
		}
	}
	
	error_expr(expr->cast.right,
			   "cannot cast from ‘%s’ to ‘%s’;",
			   data_type_to_string(cast_type),
			   data_type_to_string(right_type));
	return null;
}

DataType* Resolve::resolve_func_call(Expr* expr) {
	Stmt** params = expr->func_call.function_called->func_decl.params;
	Expr** args = expr->func_call.args;
	bool error_here = false;
	buf_loop(args, i) {
		CURRENT_ERROR;
		DataType* param_type = params[i]->var_decl.data_type;
		DataType* arg_type = resolve_expr(args[i]);
		EXIT_ERROR(null);

		DataTypeMatch match = data_type_match(param_type, arg_type);
		if (match == DT_NOT_MATCH) {
			error_here = true;
			error_expr(args[i],
					   "expected type ‘%s’, but got ‘%s’;",
					   data_type_to_string(param_type),
					   data_type_to_string(arg_type));
		}
	}

	if (error_here) return null;
	return expr->func_call.function_called->func_decl.return_data_type;
}

DataType* Resolve::resolve_array_access(Expr* expr) {	
}

DataType* Resolve::resolve_member_access(Expr* expr) {
}

DataType* Resolve::resolve_variable_ref(Expr* expr) {
	return expr->variable_ref.variable_refed->var_decl.data_type;
}

DataType* Resolve::resolve_number_expr(Expr* expr) {
	switch (expr->number->type) {
	case T_INTEGER:
		return data_types.t_int;
	case T_FLOAT64:
		return data_types.t_f64;
	default: break;
	}
	return null;
}

DataType* Resolve::resolve_constant_expr(Expr* expr) {
	if (str_intern(expr->constant->lexeme) ==
		str_intern("true") ||
		str_intern(expr->constant->lexeme) ==
		str_intern("false")) {
		return data_types.t_bool;
	}
	else if (str_intern(expr->constant->lexeme) ==
			 str_intern("null")) {
		return data_types.t_void_pointer;
	}
	return null;
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
