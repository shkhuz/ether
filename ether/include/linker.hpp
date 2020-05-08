#pragma once

#include <typedef.hpp>

struct Stmt;

struct StructFunctionMap {
	Stmt* stmt;
	Stmt** functions;
};

struct Linker {
	Stmt** stmts;
	
	StructFunctionMap** defined_structs;
	Stmt** defined_functions;
	Stmt** defined_variables;
	u64 error_count;
	
	error_code link(Stmt** _stmts);

private:
	void add_structs();
	void add_struct(Stmt* stmt);
	void add_functions();
	void add_function_global(Stmt* stmt);
	void add_function_struct(Stmt* stmt);
	void add_variables();
	void add_variable(Stmt* stmt);

public:
	void error_root(SourceFile* srcfile, u64 line, u64 column, u64 char_count, const char* fmt, va_list ap);
};
