#include <ether.hpp>
#include <compiler.hpp>

#include <string>

char* invoker_compiler = null;

void ether_abort_no_args() {
	fprintf(stderr, "Compilation terminated.\n");
	exit(EXIT_FAILURE);
}

void ether_print_error_va(const char* fmt, va_list ap) {
	va_list aq;
	va_copy(aq, ap);
	fprintf(stderr, "%s: ", invoker_compiler);

	vfprintf(stderr, fmt, aq);
	fprintf(stderr, "\n");
	va_end(aq);
}

void ether_print_error(const char* fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	ether_print_error_va(fmt, ap);
	va_end(ap);
}

void ether_abort(const char* fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	ether_print_error_va(fmt, ap);
	va_end(ap);
	
	ether_abort_no_args();
}

static void buf_test() {
	int* buf = null;
	buf_push(buf, 23);
	buf_push(buf, 1097);
	buf_push(buf, 23912032);
	assert(buf[0] == 23);
	assert(buf[1] == 1097);
	assert(buf[2] == 23912032);
	assert(buf_last(buf) == 23912032);
	assert(!buf_empty(buf));

	char* literal = null;
	buf_push(literal, 'h');
	buf_push(literal, 'e');
	buf_push(literal, 'l');
	buf_push(literal, 'l');
	buf_push(literal, 'o');
	buf_push(literal, '\0');
	assert(strncmp(literal, "hello", 5) == 0);
	assert(buf_last(literal) == '\0');
	assert(!buf_empty(literal));
}

int main(int argc, char** argv) {
#ifdef _DEBUG
	buf_test();
#endif
	
	char* output_exec_fpath = "a.out";
	bool arg_parse_error = false;
	int opt;
	
	while ((opt = getopt(argc, argv, "o:")) != -1) {
		switch (opt) {
		case 'o': {
			output_exec_fpath = optarg;
		} break;
				
		case '?': {
			arg_parse_error = false;
		} break;
		}
	}

	invoker_compiler = argv[0];

	if (arg_parse_error) {
		ether_abort_no_args();
	}

	char** source_files = null;
	for (; optind < argc; optind++) {
		buf_push(source_files, argv[optind]);
	}

	if (source_files == null) {
		ether_abort("no files supplied;");
	}

	buf_loop(source_files, i) {		
		Compiler compiler;
		compiler.compile(source_files[i]);
	}
}
