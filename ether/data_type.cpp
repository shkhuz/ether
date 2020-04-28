#include <data_type.hpp>

DataType* data_type_create(Token* identifier, u8 pointer_count) {
	DataType* data_type = new DataType();
	data_type->identifier = identifier;
	data_type->pointer_count = pointer_count;
	return data_type;
}
