#pragma once

struct Compiler {
	error_code compile(const char* in_file, const char* out_file);
};
