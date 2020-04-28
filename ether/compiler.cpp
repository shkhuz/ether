#include <ether.hpp>
#include <compiler.hpp>
#include <lexer.hpp>
#include <token.hpp>
#include <parser.hpp>

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
		Token* token = *it;
		fprintf(stderr, " Token: %40s (%2d) at | %5lu col %5lu |\n",
				token->lexeme,
				token->type,
				token->line,
				token->column);
	}

	Parser parser;
	ParserOutput parser_output = parser.parse(lexer_output.tokens, srcfile);
	if (parser_output.error_occured == ETHER_ERROR) {
		ether_abort_no_args();
	}
}
