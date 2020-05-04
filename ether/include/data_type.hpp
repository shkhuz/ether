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

DataType* data_type_create(Token* identifier, u8 pointer_count, bool is_array, Token* array_elem_count, Token* start);
