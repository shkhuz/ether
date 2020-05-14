#pragma once

#include <assert.h>
#include <limits.h>
#include <math.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#include <common.hpp>
#include <ds.hpp>
#include <error.hpp>
#include <io.hpp>
#include <str_intern.hpp>
#include <typedef.hpp>

#define ETHER_SUCCESS 0
#define ETHER_ERROR 1

#define TAB_SIZE 4
#define KEYWORDS_LEN 14
#define BUILT_IN_TYPES_LEN 4

#define PRINT_TOKEN 0
#define PRINT_AST 0

void ether_abort(const char* fmt, ...);
void ether_abort_no_args();

extern char* keywords[KEYWORDS_LEN];
extern char* built_in_types[BUILT_IN_TYPES_LEN];
