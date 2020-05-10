#pragma once

#include <expr.hpp>
#include <token.hpp>
#include <data_type.hpp>
#include <token.hpp>

struct SourceFile;

template <typename T>
void error_expr(T* ins, Expr* expr, const char* fmt, ...) {
	u64 char_count = get_expr_char_count(expr);
	va_list ap;
	va_start(ap, fmt);
	error_root(ins,
		 expr->head->file,
		 expr->head->line,
		 expr->head->column,
		 char_count,
		 fmt,
		 ap);
	va_end(ap);
}

template <typename T>
void error_data_type(T* ins, DataType* data_type, const char* fmt, ...) {
	assert(data_type);
	u64 char_count = data_type->identifier->end - data_type->start->start;
	va_list ap;
	va_start(ap, fmt);
	error_root(ins,
		 data_type->identifier->file,
		 data_type->identifier->line,
		 data_type->identifier->column,
		 char_count,
		 fmt,
		 ap);
	va_end(ap);
}

template <typename T>
void error_token(T* ins, Token* token, const char* fmt, ...) {
	u64 char_count = token->end - token->start;
	va_list ap;
	va_start(ap, fmt);
	error_root(ins,
		 token->file,
		 token->line,
		 token->column,
		 char_count,
		 fmt,
		 ap);
	va_end(ap);	
}

template <typename T>
static void error_root(T* ins, SourceFile* srcfile, u64 line, u64 column, u64 char_count, const char* fmt, va_list ap) {
	ins->error_root(srcfile,
					line,
					column,
					char_count,
					fmt,
					ap);
}

template <typename T>
void warning_expr(T* ins, Expr* expr, const char* fmt, ...) {
	u64 char_count = get_expr_char_count(expr);
	va_list ap;
	va_start(ap, fmt);
	warning_root(ins,
		 expr->head->file,
		 expr->head->line,
		 expr->head->column,
		 char_count,
		 fmt,
		 ap);
	va_end(ap);
}

template <typename T>
void warning_data_type(T* ins, DataType* data_type, const char* fmt, ...) {
	assert(data_type);
	u64 char_count = data_type->identifier->end - data_type->start->start;
	va_list ap;
	va_start(ap, fmt);
	warning_root(ins,
		 data_type->identifier->file,
		 data_type->identifier->line,
		 data_type->identifier->column,
		 char_count,
		 fmt,
		 ap);
	va_end(ap);
}

template <typename T>
void warning_token(T* ins, Token* token, const char* fmt, ...) {
	u64 char_count = token->end - token->start;
	va_list ap;
	va_start(ap, fmt);
	warning_root(ins,
		 token->file,
		 token->line,
		 token->column,
		 char_count,
		 fmt,
		 ap);
	va_end(ap);	
}

template <typename T>
static void warning_root(T* ins, SourceFile* srcfile, u64 line, u64 column, u64 char_count, const char* fmt, va_list ap) {
	ins->warning_root(srcfile,
					line,
					column,
					char_count,
					fmt,
					ap);
}

void print_error_at(SourceFile* srcfile, u64 line, u64 column, u64 mark_len, const char* fmt, va_list ap);
void print_warning_at(SourceFile* srcfile, u64 line, u64 column, u64 mark_len, const char* fmt, va_list ap);

#define error_expr(e, fmt, ...) error_expr(this, e, fmt, ##__VA_ARGS__)
#define error_data_type(d, fmt, ...) error_data_type(this, d, fmt, ##__VA_ARGS__)
#define error_token(t, fmt, ...) error_token(this, t, fmt, ##__VA_ARGS__)

#define warning_expr(e, fmt, ...) warning_expr(this, e, fmt, ##__VA_ARGS__)
#define warning_data_type(d, fmt, ...) warning_data_type(this, d, fmt, ##__VA_ARGS__)
#define warning_token(t, fmt, ...) warning_token(this, t, fmt, ##__VA_ARGS__)
