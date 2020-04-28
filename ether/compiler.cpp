#include <ether.hpp>
#include <compiler.hpp>
#include <lexer.hpp>
#include <token.hpp>

error_code Compiler::compile(const char* in_file, const char* out_file) {
	SourceFile* srcfile = read_file(in_file);
	if (!srcfile) {
		ether_abort("%s: no such file or directory", in_file);
		return ETHER_ERROR; /* unreachable */
	}

	Lexer lexer;
	LexerOutput lexer_output = lexer.lex(srcfile);
	if (lexer_output.error_occured == ETHER_ERROR) {
		ether_abort_no_args();
	}

	for (auto it = lexer_output.tokens->begin();
		 it != lexer_output.tokens->end(); 
		 ++it) {
		fprintf(stderr, "Lexeme: | %10s  Type: %d\n", (*it)->lexeme, (*it)->type);
	}
}
