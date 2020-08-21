#pragma once

#include <typedef.hpp>

#define _DT_INTEGER_TYPE_COUNT 10

struct Token;

struct DataType {
	Token* identifier;
	u8 pointer_count;
	bool is_array;
	Token* array_elem_count;
	Token* start;
};

enum DataTypeMatch {
	DT_NOT_MATCH,
	DT_MATCH,
};

void sys_data_type_init();

DataType* data_type_create(Token* identifier, u8 pointer_count, bool is_array, Token* array_elem_count, Token* start);
DataType* data_type_from_string(char* type);
DataType* data_type_from_string_int(char* type, u8 pointer_count);

DataTypeMatch data_type_integer(DataType* data_type);
DataTypeMatch data_type_match(DataType* a, DataType* b);

struct PredefinedDataTypes {
	// INT BLOCK SHOULD BE PLACED FIRST
	// IF INT BLOCK IS CHANGED,
	// MODIFY THE DATA_TYPE_INTEGER CODE
	DataType* t_int;
	DataType* t_uint;
	DataType* t_u8;
	DataType* t_u16;
	DataType* t_u32;
	DataType* t_u64;
	DataType* t_i8;
	DataType* t_i16;
	DataType* t_i32;
	DataType* t_i64;
	// END INTEGER

	DataType* t_string;
	DataType* t_char;
	DataType* t_bool;
	DataType* t_void_pointer;
	DataType* t_f32;
	DataType* t_f64;
};

extern PredefinedDataTypes data_types;
