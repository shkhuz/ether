#include <ether.hpp>

void print_error_at(SourceFile* srcfile, u64 line, u64 column, u64 mark_len, const char* fmt, va_list ap) {
	va_list aq;
	va_copy(aq, ap);
	fprintf(stderr, "%s:%lu:%lu: error: ", 
			srcfile->fpath,
			line,
			column);
	vfprintf(stderr, fmt, aq);
	fprintf(stderr, "\n");
	va_end(aq);

	print_file_line_with_info(srcfile, line);
	print_marker_arrow_with_info_ln(srcfile, line, column, mark_len);
}

void print_warning_at(SourceFile* srcfile, u64 line, u64 column, u64 mark_len, const char* fmt, va_list ap) {
	va_list aq;
	va_copy(aq, ap);
	fprintf(stderr, "%s:%lu:%lu: warning: ", 
			srcfile->fpath,
			line,
			column);
	vfprintf(stderr, fmt, aq);
	fprintf(stderr, "\n");
	va_end(aq);

	print_file_line_with_info(srcfile, line);
	print_marker_arrow_with_info_ln(srcfile, line, column, mark_len);
}
