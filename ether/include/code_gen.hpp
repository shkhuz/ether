#pragma once

#include <ether.hpp>

struct Stmt;

struct CodeGenerator {
	void generate(Stmt** _stmts, char* _obj_fpath);
};
