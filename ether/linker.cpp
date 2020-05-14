#include <ether.hpp>
#include <linker.hpp>
#include <stmt.hpp>
#include <expr.hpp>
#include <data_type.hpp>
#include <token.hpp>

#define error_expr(e, fmt, ...) error_expr(this, e, fmt, ##__VA_ARGS__)
#define error_data_type(d, fmt, ...) error_data_type(this, d, fmt, ##__VA_ARGS__)
#define error_token(t, fmt, ...) error_token(this, t, fmt, ##__VA_ARGS__)

#define warning_expr(e, fmt, ...) warning_expr(this, e, fmt, ##__VA_ARGS__)
#define warning_data_type(d, fmt, ...) warning_data_type(this, d, fmt, ##__VA_ARGS__)
#define warning_token(t, fmt, ...) warning_token(this, t, fmt, ##__VA_ARGS__)

#define CHANGE_SCOPE(name)						\
	Scope* name = new Scope;					\
	scope->parent_scope = current_scope;		\
	scope->variables = null;					\
	current_scope = name;						\

#define REVERT_SCOPE(name)						\
	current_scope = name->parent_scope;

error_code Linker::link(Stmt** _stmts) {
	stmts = _stmts;
	
	defined_structs = null;
	defined_functions = null;
	global_scope = new Scope;
	global_scope->parent_scope = null;
	global_scope->variables = null;
	current_scope = global_scope;
	function_in = null;
	error_count = 0;

	add_structs();
	add_functions();
	add_variables();

	check_stmts();

	return (error_count == 0 ?
			ETHER_SUCCESS :
			ETHER_ERROR);
}

void Linker::add_structs() {
	buf_loop(stmts, s) {
		if (stmts[s]->type == S_STRUCT) {
			add_struct(stmts[s]);
		}
	}
}

void Linker::add_struct(Stmt* stmt) {
	buf_loop(defined_structs, i) {
		for (u64 bt = 0; bt < BUILT_IN_TYPES_LEN; bt++) {
			if (str_intern(stmt->struct_stmt.identifier->lexeme) ==
				str_intern(built_in_types[bt])) {
				error_token(stmt->struct_stmt.identifier,
							"redeclaration of built-in type ‘%s’ as a struct;",
							built_in_types[bt]);
				return;
			}
		}
		
		if (is_token_equal(stmt->struct_stmt.identifier,
						   defined_structs[i]->stmt->struct_stmt.identifier)) {
			error_token(stmt->struct_stmt.identifier,
						"redeclaration of struct ‘%s’;",
						stmt->struct_stmt.identifier->lexeme);
			return;
		}
	}

	StructFunctionMap* map = new StructFunctionMap;
	map->stmt = stmt;
	map->functions = null;
	buf_push(defined_structs, map);
}

void Linker::add_functions() {
	buf_loop(stmts, s) {
		if (stmts[s]->type == S_FUNC_DECL) {
			if (!stmts[s]->func_decl.struct_in) {
				add_function_global(stmts[s]);
			}
			else {
				add_function_struct(stmts[s]);
			}
		}
	}
}

void Linker::add_function_global(Stmt* stmt) {
	buf_loop(defined_functions, i) {
		if (is_token_equal(stmt->func_decl.identifier,
						   defined_functions[i]->func_decl.identifier)) {
			error_token(stmt->func_decl.identifier,
						"redefinition of function ‘%s’;",
						stmt->func_decl.identifier->lexeme);
			return;
		}
	}

	buf_push(defined_functions, stmt);
}

void Linker::add_function_struct(Stmt* stmt) {
	u64 idx = 0;
	buf_loop(defined_structs, i) {
		if (stmt->func_decl.struct_in ==
			defined_structs[i]->stmt) {
			idx = i;
			break;
		}
	}

	buf_loop(defined_structs[idx]->functions, f) {
		if (is_token_equal(stmt->func_decl.identifier,
						   defined_structs[idx]->functions[f]->func_decl.identifier)) {
			error_token(stmt->func_decl.identifier,
						"redefinition of struct function ‘%s’;",
						stmt->func_decl.identifier->lexeme);
			return;
		}
	}

	buf_push(defined_structs[idx]->functions, stmt);
}

void Linker::add_variables() {
	buf_loop(stmts, s) {
		if (stmts[s]->type == S_VAR_DECL) {
			add_variable(stmts[s]);
		}
	}
}

void Linker::add_variable(Stmt* stmt) {
	buf_loop(global_scope->variables, i) {
		if (is_token_equal(stmt->var_decl.identifier,
						   global_scope->variables[i]->var_decl.identifier)) {
			error_token(stmt->var_decl.identifier,
					  "redeclaration of variable ‘%s’;",
					  stmt->var_decl.identifier->lexeme);
			return;
		}
	}
	if (stmt->var_decl.initializer) {
		check_expr(stmt->var_decl.initializer);
	}

	buf_push(global_scope->variables, stmt);
}

void Linker::check_stmts() {
	buf_loop(stmts, s) {
		check_stmt(stmts[s]);
	}
}

void Linker::check_stmt(Stmt* stmt) {
	switch (stmt->type) {
	case S_STRUCT:
		check_struct(stmt);
		break;
	case S_FUNC_DECL:
		if (!stmt->func_decl.struct_in) {
			check_func_decl(stmt);
		}
		break;
	case S_VAR_DECL:
		if (function_in) { // global vars have already been checked
			check_var_decl(stmt);
		}
		break;
	case S_IF:
		check_if_stmt(stmt);
		break;
	case S_FOR:
		check_for_stmt(stmt);
		break;
	case S_SWITCH:
		check_switch_stmt(stmt);
		break;
	case S_RETURN:
		check_return_stmt(stmt);
		break;
	case S_EXPR_STMT:
		check_expr_stmt(stmt);
		break;
	case S_BLOCK:
		check_block_stmt(stmt);
		break;
	}
}

void Linker::check_struct(Stmt* stmt) {
	CHANGE_SCOPE(scope);
	
	Stmt** fields = stmt->struct_stmt.fields;
	buf_loop(fields, f) {
		VariableScope scope_in_found = is_variable_in_scope(fields[f]);
		if (scope_in_found == VS_CURRENT_SCOPE) {
			error_token(fields[f]->var_decl.identifier,
						"redeclaration of struct variable ‘%s’;",
						fields[f]->var_decl.identifier->lexeme);
		}
		else if (scope_in_found == VS_OUTER_SCOPE) {
			warning_token(fields[f]->var_decl.identifier,
						  "variable declaration shadows another variable;");
			buf_push(scope->variables, fields[f]);
		}
		buf_push(scope->variables, fields[f]);
	}

	Stmt** functions = null;
	buf_loop(defined_structs, s) {
		if (defined_structs[s]->stmt == stmt) {
			functions = defined_structs[s]->functions;
		}
	}

	buf_loop(functions, f) {
		// struct functions are checked here because
		// struct variables should be in scope
		check_func_decl(functions[f]);
	}
	
	REVERT_SCOPE(scope);
}

void Linker::check_func_decl(Stmt* stmt) {
	CHANGE_SCOPE(scope);
	function_in = stmt;
	buf_loop(stmt->func_decl.params, p) {
		Stmt** params = stmt->func_decl.params;
		add_variable_to_scope(params[p]);
	}
		
	if (stmt->func_decl.return_data_type) {
		check_data_type(stmt->func_decl.return_data_type, true);
	}

	if (stmt->func_decl.is_function) {
		buf_loop(stmt->func_decl.body, s) {
			check_stmt(stmt->func_decl.body[s]);
		}
	}
	function_in = null;
	REVERT_SCOPE(scope);
}

void Linker::check_var_decl(Stmt* stmt) {
	add_variable_to_scope(stmt);

	if (stmt->var_decl.data_type) {
		check_data_type(stmt->var_decl.data_type, false);
	}

	if (stmt->var_decl.is_variable) {
		if (stmt->var_decl.initializer) {
			check_expr(stmt->var_decl.initializer);
		}
	}
}

void Linker::check_if_stmt(Stmt* stmt) {
	check_if_branch(stmt->if_stmt.if_branch);
	buf_loop(stmt->if_stmt.elif_branch, b) {
		check_if_branch(stmt->if_stmt.elif_branch[b]);
	}
	if (stmt->if_stmt.else_branch) {
		check_if_branch(stmt->if_stmt.else_branch);
	}
}

void Linker::check_if_branch(IfBranch* branch) {
	CHANGE_SCOPE(scope);
	if (branch->cond) {
		check_expr(branch->cond);
	}
	buf_loop(branch->body, s) {
		check_stmt(branch->body[s]);
	}
	REVERT_SCOPE(scope);
}

void Linker::check_for_stmt(Stmt* stmt) {
	CHANGE_SCOPE(scope);
	if (stmt->for_stmt.counter) {
		check_var_decl(stmt->for_stmt.counter);
	}
	if (stmt->for_stmt.end) {
		check_expr(stmt->for_stmt.end);
	}

	buf_loop(stmt->for_stmt.body, s) {
		check_stmt(stmt->for_stmt.body[s]);
	}
	REVERT_SCOPE(scope);
}

void Linker::check_switch_stmt(Stmt* stmt) {
	check_expr(stmt->switch_stmt.cond);
	buf_loop(stmt->switch_stmt.branches, b) {
		check_switch_branch(stmt->switch_stmt.branches[b]);
	}
}

void Linker::check_switch_branch(SwitchBranch* branch) {
	buf_loop(branch->conds, c) {
		check_expr(branch->conds[c]);
	}
	check_stmt(branch->stmt);
}

void Linker::check_return_stmt(Stmt* stmt) {
	if (stmt->return_stmt.to_return) {
		check_expr(stmt->return_stmt.to_return);
	}
	assert(function_in);
	stmt->return_stmt.function_refed = function_in;
}

void Linker::check_block_stmt(Stmt* stmt) {
	CHANGE_SCOPE(scope);
	buf_loop(stmt->block, s) {
		check_stmt(stmt->block[s]);
	}
	REVERT_SCOPE(scope);
}

void Linker::check_expr_stmt(Stmt* stmt) {
	check_expr(stmt->expr_stmt);
}

void Linker::check_expr(Expr* expr) {
	switch (expr->type) {
	case E_BINARY:
		check_binary_expr(expr);
		break;
	case E_UNARY:
		check_unary_expr(expr);
		break;
	case E_CAST:
		check_cast_expr(expr);
		break;
	case E_FUNC_CALL:
		check_func_call(expr);
		break;
	case E_ARRAY_ACCESS:
		check_array_access(expr);
		break;
	case E_VARIABLE_REF:
		check_variable_ref(expr);
		break;
	case E_NUMBER:
	case E_STRING:
	case E_CHAR:
	case E_CONSTANT:
	case E_MEMBER_ACCESS:
		break;	
	}
}

void Linker::check_binary_expr(Expr* expr) {
	check_expr(expr->binary.left);
	check_expr(expr->binary.right);
}

void Linker::check_unary_expr(Expr* expr) {
	check_expr(expr->unary.right);
}

void Linker::check_cast_expr(Expr* expr) {
	check_data_type(expr->cast.cast_to, false);
	check_expr(expr->cast.right);
}

void Linker::check_func_call(Expr* expr) {
	if (expr->func_call.left->type == E_VARIABLE_REF) {
		buf_loop(defined_functions, f) {
			if (is_token_equal(expr->func_call.left->variable_ref.identifier,
							   defined_functions[f]->func_decl.identifier)) {
				expr->func_call.function_called = defined_functions[f];
				break;
			}
		}

		if (!expr->func_call.function_called) {
			error_expr(expr->func_call.left,
					   "undefined function ‘%s’;",
					   expr->func_call.left->variable_ref.identifier->lexeme);
			return;
		}
	}

	// TODO: remove this if statement
	if (expr->func_call.left->type == E_VARIABLE_REF) {
		assert(expr->func_call.function_called);
		u64 arg_len = buf_len(expr->func_call.args);
		u64 param_len = buf_len(expr->func_call.function_called->func_decl.params);
		if (arg_len != param_len) {
			error_expr(expr->func_call.left,
					   "function ‘%s’ expects %lu arguments, but given %lu;",
					   expr->func_call.left->variable_ref.identifier->lexeme,
					   param_len,
					   arg_len);
			return;
		}
	}
	
	buf_loop(expr->func_call.args, a) {
		check_expr(expr->func_call.args[a]);
	}
}

void Linker::check_array_access(Expr* expr) {
	check_expr(expr->array_access.left);
	check_expr(expr->array_access.index);
}

void Linker::check_variable_ref(Expr* expr) {
	VariableScope scope_in_found = is_variable_ref_in_scope(expr);
	if (scope_in_found == VS_NO_SCOPE) {
		error_expr(expr,
				   "undefined variable ‘%s’;",
				   expr->variable_ref.identifier->lexeme);
	}
}

void Linker::check_data_type(DataType* data_type, bool is_return_data_type) {
	if (str_intern(data_type->identifier->lexeme) ==
		str_intern("void") &&
		!is_return_data_type) {
		error_data_type(data_type,
						"invalid use of ‘void’ data type;");
		return;
	}
	
	bool found = false;
	buf_loop(defined_structs, i) {
		if (is_token_equal(defined_structs[i]->stmt->struct_stmt.identifier,
						   data_type->identifier)) {
			found = true;
			break;
		}
	}

	if (!found) {
		for (u64 i = 0; i < BUILT_IN_TYPES_LEN; ++i) {
			if (str_intern(built_in_types[i]) ==
				str_intern(data_type->identifier->lexeme)) {
				found = true;
				break;
			}
		}
	}
	
	if (!found) {
		error_token(data_type->identifier,
					"undefined type ‘%s’;",
					data_type->identifier->lexeme);
	}
}

void Linker::add_variable_to_scope(Stmt* stmt) {
	VariableScope scope_in_found = is_variable_in_scope(stmt);
	if (scope_in_found == VS_CURRENT_SCOPE) {
		error_token(stmt->var_decl.identifier,
					"redeclaration of variable ‘%s’;",
					stmt->var_decl.identifier->lexeme);
	}
	else if (scope_in_found == VS_OUTER_SCOPE) {
		warning_token(stmt->var_decl.identifier,
					  "variable declaration shadows another variable;");
		buf_push(current_scope->variables, stmt);
	}
	buf_push(current_scope->variables, stmt);
}

VariableScope Linker::is_variable_ref_in_scope(Expr* expr) {
	VariableScope scope_in_found = VS_NO_SCOPE;
	bool first_iter = true;
	Scope* scope = current_scope;
	
	while (scope != null) {
		buf_loop(scope->variables, v) {
			if (is_token_equal(expr->variable_ref.identifier,
							   scope->variables[v]->var_decl.identifier)) {
				if (first_iter) {
					scope_in_found = VS_CURRENT_SCOPE;										
				}
				else {
					scope_in_found = VS_OUTER_SCOPE;
				}
				expr->variable_ref.variable_refed = scope->variables[v];
				break;
			}
		}
		
		if (scope_in_found != VS_NO_SCOPE) break;
		scope = scope->parent_scope;
		first_iter = false;
	}
	return scope_in_found;
}

VariableScope Linker::is_variable_in_scope(Stmt* stmt) {
	VariableScope scope_in_found = VS_NO_SCOPE;
	bool first_iter = true;
	Scope* scope = current_scope;

	while (scope != null) {
		buf_loop(scope->variables, v) {
			if (is_token_equal(stmt->var_decl.identifier,
							   scope->variables[v]->var_decl.identifier)) {
				if (first_iter) {
					scope_in_found = VS_CURRENT_SCOPE;
				}
				else {
					scope_in_found = VS_OUTER_SCOPE;
				}
				break;
			}
		}

		if (scope_in_found != VS_NO_SCOPE) break;
		scope = scope->parent_scope;
		first_iter = false;
	}
	return scope_in_found;
}

void Linker::error_root(SourceFile* srcfile, u64 line, u64 column, u64 char_count, const char* fmt, va_list ap) {
	print_error_at(
		srcfile,
		line,
		column,
		char_count,
		fmt,
		ap);
	error_count++;
}

void Linker::warning_root(SourceFile* srcfile, u64 line, u64 column, u64 char_count, const char* fmt, va_list ap) {
	print_warning_at(
		srcfile,
		line,
		column,
		char_count,
		fmt,
		ap);
}
