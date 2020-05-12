#include <ether.hpp>
#include <compiler.hpp>
#include <token.hpp>
#include <lexer.hpp>
#include <parser.hpp>
#include <ast_printer.hpp>
#include <linker.hpp>
#include <resolve.hpp>
#include <code_gen.hpp>

static FileDecl* file_decls = null;

Stmt** Compiler::compile(const char* in_file) {
	std::string current_file = std::string(in_file);
	if (!match_extension(current_file, "eth")) {
		ether_abort("%s: invalid source file extension;", in_file);
	}

	char* obj_fpath = change_extension(current_file, "o");
	
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
				parser_output.decls});
	parser.add_pending_imports();

	AstPrinter ast_printer;
	ast_printer.print(parser_output.stmts);

	Linker linker;
	error_code linker_error_code = linker.link(parser_output.stmts);
	if (linker_error_code == ETHER_ERROR) {
		ether_abort_no_args();
	}

	Resolve resolve;
	error_code resolve_error_code = resolve.resolve(parser_output.stmts);
	if (resolve_error_code == ETHER_ERROR) {
		ether_abort_no_args();
	}
	
	//CodeGenerator code_generator;
	//code_generator.generate(parser_output.stmts, const_cast<char*>(obj_fpath));

	return parser_output.decls;
}
