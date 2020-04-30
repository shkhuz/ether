#include <data_type.hpp>

DataType* data_type_create(Token* identifier, u8 pointer_count, bool is_array, Token* array_elem_count) {
	DataType* data_type = new DataType();
	data_type->identifier = identifier;
	data_type->pointer_count = pointer_count;
	data_type->is_array = is_array;
	data_type->array_elem_count = array_elem_count;
	return data_type;
}
