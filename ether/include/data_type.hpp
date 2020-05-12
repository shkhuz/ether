#pragma once

#include <typedef.hpp>

struct Token;

struct DataType {
	Token* identifier;
	u8 pointer_count;
	bool is_array;
	Token* array_elem_count;
	Token* start;
};

void sys_data_type_init();
DataType* data_type_create(Token* identifier, u8 pointer_count, bool is_array, Token* array_elem_count, Token* start);
DataType* data_type_from_string(char* type);
DataType* data_type_from_string_int(char* type, u8 pointer_count);

struct PredefinedDataTypes {
	DataType* t_int;
	DataType* t_uint;
	DataType* t_string;
	DataType* t_char;
	DataType* t_bool;
	DataType* t_f32;
	DataType* t_f64;
	
	DataType* t_u8;
	DataType* t_u16;
	DataType* t_u32;
	DataType* t_u64;
	DataType* t_i8;
	DataType* t_i16;
	DataType* t_i32;
	DataType* t_i64;	
};

extern PredefinedDataTypes data_types;
