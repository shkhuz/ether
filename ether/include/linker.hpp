#pragma once

#include <typedef.hpp>

struct Stmt;

struct Linker {
	Stmt** stmts;
	
	Stmt** defined_structs;
	u64 error_count;
	
	error_code link(Stmt** _stmts);

private:
	void add_structs();
	void add_struct(Stmt* stmt);

public:
	void error_root(SourceFile* srcfile, u64 line, u64 column, u64 char_count, const char* fmt, va_list ap);
};
