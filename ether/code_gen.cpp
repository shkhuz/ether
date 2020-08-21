#include <code_gen.hpp>
#include <stmt.hpp>
#include <expr.hpp>
#include <token.hpp>
#include <data_type.hpp>

void CodeGenerator::generate(Stmt** _stmts, char* _obj_fpath) {
	printf("Generating %s...\n", _obj_fpath);
	stmts = _stmts;
	obj_fpath = _obj_fpath;
	
	output_code = null;
	tab_count = 0;

	buf_loop(stmts, s) {
		gen_stmt(stmts[s]);		
	}
	buf_push(output_code, '\0');
	printf("%s", output_code);
}

void CodeGenerator::gen_stmt(Stmt* stmt) {
	print_tabs_by_indentation();
	switch (stmt->type) {
	case S_STRUCT:
		gen_struct(stmt);
		break;
	case S_VAR_DECL:
		gen_var_decl(stmt);
		break;
	}
	print_newline();
}

void CodeGenerator::gen_struct(Stmt* stmt) {
	print_string("typedef struct ");
	print_token(stmt->struct_stmt.identifier);
	print_string(" {");
	print_newline();

	tab_count++;
	buf_loop(stmt->struct_stmt.fields, f) {
		gen_stmt(stmt->struct_stmt.fields[f]);
	}
	tab_count--;
	print_char('}');
}

void CodeGenerator::gen_var_decl(Stmt* stmt) {
	print_data_type(stmt->var_decl.data_type);
	print_space();
	print_token(stmt->var_decl.identifier);
	print_semicolon();
}

void CodeGenerator::print_data_type(DataType* data_type) {
	print_token(data_type->identifier);
	for (u8 p = 0; p < data_type->pointer_count; ++p) {
		print_char('*');
	}
}

void CodeGenerator::print_tabs_by_indentation() {
	for (u64 i = 0; i < tab_count; ++i) {
		print_tab_with_spaces();
	}
}

void CodeGenerator::print_newline() {
	print_char('\n');
}

void CodeGenerator::print_space() {
	print_char(' ');
}

void CodeGenerator::print_semicolon() {
	print_char(';');
}

void CodeGenerator::print_tab_with_spaces() {
	for (u64 i = 0; i < TAB_SIZE; ++i) {
		print_space();
	}
}

void CodeGenerator::print_token(Token* token) {
	print_string(token->lexeme);
}

void CodeGenerator::print_string(char* str) {
	char* _str = str;
	while (*_str != '\0') {
		print_char(*_str);
		_str++;
	}
}

void CodeGenerator::print_char(char ch) {
	buf_push(output_code, ch);
}
