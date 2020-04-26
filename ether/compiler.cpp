#include <ether.h>
#include <compiler.h>

error_code Compiler::compile(const char* in_file, const char* out_file) {
	SourceFile* srcfile = read_file(in_file);
	if (!srcfile) {
		ether_abort("%s: no such file or directory", in_file);
		return ETHER_ERROR; /* unreachable */
	}
}
