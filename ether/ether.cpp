#include <ether.hpp>
#include <compiler.hpp>

char* invoker_compiler = null;

void ether_abort_no_args() {
	fprintf(stderr, "Compilation terminated.\n");
	exit(EXIT_FAILURE);
}

void ether_abort(const char* fmt, ...) {
	fprintf(stderr, "%s: ", invoker_compiler);

	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	va_end(ap);

	ether_abort_no_args();
}

int main(int argc, char** argv) {
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
	
	std::vector<char*> source_files;
	for (; optind < argc; optind++) {
		source_files.emplace_back(argv[optind]);
	}

	if (source_files.empty()) {
		ether_abort("no files specified; consider adding a ‘main.eth’ source file");
	}

	for (auto it = source_files.begin(); it != source_files.end(); ++it) {
		Compiler compiler;
		compiler.compile(*it, "out");
	}
}
