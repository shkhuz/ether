#include <ether.hpp>
#include <linker.hpp>
#include <stmt.hpp>
#include <expr.hpp>
#include <data_type.hpp>
#include <token.hpp>

#define error_expr(e, fmt, ...) error_expr(this, e, fmt, ##__VA_ARGS__)
#define error_data_type(d, fmt, ...) error_data_type(this, d, fmt, ##__VA_ARGS__)
#define error_token(t, fmt, ...) error_token(this, t, fmt, ##__VA_ARGS__)

error_code Linker::link(Stmt** _stmts) {
	stmts = _stmts;
	
	defined_structs = null;
	defined_functions = null;
	defined_variables = null;
	error_count = 0;

	add_structs();
	add_functions();
	add_variables();

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
	buf_loop(defined_variables, i) {
		if (is_token_equal(stmt->var_decl.identifier,
						   defined_variables[i]->var_decl.identifier)) {
			error_token(stmt->var_decl.identifier,
					  "redeclaration of variable ‘%s’;",
					  stmt->var_decl.identifier->lexeme);
			return;
		}
	}

	buf_push(defined_variables, stmt);
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
