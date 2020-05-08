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
	error_count = 0;

	add_structs();

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
						   defined_structs[i]->struct_stmt.identifier)) {
			error_token(stmt->struct_stmt.identifier,
						"redeclaration of struct ‘%s’;",
						stmt->struct_stmt.identifier->lexeme);
			return;
		}
	}

	buf_push(defined_structs, stmt);
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
