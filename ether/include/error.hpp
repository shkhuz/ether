#pragma once

struct SourceFile;

void print_error_at(SourceFile* srcfile, u64 line, u64 column, u64 mark_len, const char* fmt, va_list ap);
void print_warning_at(SourceFile* srcfile, u64 line, u64 column, u64 mark_len, const char* fmt, va_list ap);
