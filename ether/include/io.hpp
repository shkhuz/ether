#pragma once

struct SourceFile {
	char* fpath;
	char* contents;
	uint len;
};

SourceFile* read_file(const char* fpath);
bool file_exists(const char* fpath);
char* get_line_at(SourceFile* file, u64 line);
error_code print_file_line(SourceFile* file, u64 line);
error_code print_file_line_with_info(SourceFile* file, u64 line);
error_code print_marker_arrow_ln(SourceFile* file, u64 line, u64 column, u64 mark_len);
error_code print_marker_arrow_with_info_ln(SourceFile* file, u64 line, u64 column, u64 mark_len);
void print_tab(void);
