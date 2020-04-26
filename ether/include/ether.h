#pragma once

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <vector>

#define true 1
#define false 0
#define null NULL

#define ETHER_SUCCESS 0
#define ETHER_ERROR 1

#define TAB_SIZE 4

typedef unsigned int  uint;
typedef unsigned char uchar;
typedef signed char schar;

typedef uint8_t	 u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t	i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef int error_code;

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
error_code print_marker_arrow_ln(SourceFile* file, u64 line, u32 column);
error_code print_marker_arrow_with_info_ln(SourceFile* file, u64 line, u32 column);
void print_tab(void);
