#pragma once

#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <vector>
#include <map>

#include <typedef.hpp>

#define true 1
#define false 0
#define null NULL

#define ETHER_SUCCESS 0
#define ETHER_ERROR 1

#define TAB_SIZE 4
#define KEYWORDS_LEN 4

#define array_len(arr) (sizeof(arr) / sizeof(arr[0]))

void ether_abort(const char* fmt, ...);
void ether_abort_no_args();

struct SourceFile {
	char* fpath;
	char* contents;
	uint len;
};

SourceFile* read_file(const char* fpath);
char* get_line_at(SourceFile* file, u64 line);
error_code print_file_line(SourceFile* file, u64 line);
error_code print_file_line_with_info(SourceFile* file, u64 line);
error_code print_marker_arrow_ln(SourceFile* file, u64 line, u64 column, u64 mark_len);
error_code print_marker_arrow_with_info_ln(SourceFile* file, u64 line, u64 column, u64 mark_len);
void print_tab(void);

void print_error_at(SourceFile* srcfile, u64 line, u64 column, u64 mark_len, const char* fmt, va_list ap);
void print_warning_at(SourceFile* srcfile, u64 line, u64 column, u64 mark_len, const char* fmt, va_list ap);

struct StrIntern {
	char* str;
	u64 len;
};

char* str_intern_range(char* start, char* end);
char* str_intern(char* str);

extern char* keywords[KEYWORDS_LEN];
