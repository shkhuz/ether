#include <ether.hpp>
#include <compiler.hpp>
#include <lexer.hpp>
#include <token.hpp>
#include <parser.hpp>
#include <ast_printer.hpp>

static FileDecl* file_decls = null;

Stmt** Compiler::compile(const char* in_file, const char* out_file) {
	buf_loop(file_decls, f) {
		if (str_intern(file_decls[f].fpath) ==
			str_intern(const_cast<char*>(in_file))) {
			return file_decls[f].decls;
		}
	}
	
	SourceFile* srcfile = read_file(in_file);
	if (!srcfile) {
		ether_abort("%s: no such file or directory", in_file);
		return null; /* unreachable */
	}

	Lexer lexer;
	LexerOutput lexer_output = lexer.lex(srcfile);
	if (lexer_output.error_occured == ETHER_ERROR) {
		ether_abort_no_args();
	}

#if PRINT_TOKEN
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
#endif

	Parser parser;
	ParserOutput parser_output = parser.parse(lexer_output.tokens, srcfile);
	if (parser_output.error_occured == ETHER_ERROR) {
		ether_abort_no_args();
	}
	buf_push(file_decls, (FileDecl){ const_cast<char*>(in_file),
									 parser_output.decls });
	parser.add_pending_imports();

	AstPrinter ast_printer;
	ast_printer.print(parser_output.stmts);

	return parser_output.decls;
}
