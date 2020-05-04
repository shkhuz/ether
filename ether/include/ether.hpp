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

#include <typedef.hpp>
#include <ds.hpp>
#include <io.hpp>
#include <error.hpp>
#include <str_intern.hpp>

#define true 1
#define false 0
#define null NULL

#define ETHER_SUCCESS 0
#define ETHER_ERROR 1

#define TAB_SIZE 4
#define KEYWORDS_LEN 8

#define PRINT_TOKEN 0

void ether_abort(const char* fmt, ...);
void ether_abort_no_args();

extern char* keywords[KEYWORDS_LEN];
