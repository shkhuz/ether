#include <data_type.hpp>
#include <token.hpp>

PredefinedDataTypes data_types;

void sys_data_type_init() {
	data_types.t_int = data_type_from_string("int");
	data_types.t_uint = data_type_from_string("uint");
	data_types.t_string = data_type_from_string_int("char", 1);
	data_types.t_char = data_type_from_string("char");
	data_types.t_bool = data_type_from_string("bool");
	data_types.t_void_pointer = data_type_from_string_int("void", 1);
	data_types.t_f32 = data_type_from_string("f32");
	data_types.t_f64 = data_type_from_string("f64");
	
	data_types.t_u8 = data_type_from_string("u8");
	data_types.t_u16 = data_type_from_string("u16");
	data_types.t_u32 = data_type_from_string("u32");
	data_types.t_u64 = data_type_from_string("u64");
	data_types.t_i8 = data_type_from_string("i8");
	data_types.t_i16 = data_type_from_string("i16");
	data_types.t_i32 = data_type_from_string("i32");
	data_types.t_i64 = data_type_from_string("i64");
}

DataType* data_type_create(Token* identifier, u8 pointer_count, bool is_array, Token* array_elem_count, Token* start) {
	DataType* data_type = new DataType();
	data_type->identifier = identifier;
	data_type->pointer_count = pointer_count;
	data_type->is_array = is_array;
	data_type->array_elem_count = array_elem_count;
	data_type->start = start;
	return data_type;
}

DataType* data_type_from_string(char* type) {
	Token* identifier = token_from_string(type);
	return data_type_create(identifier,
							0,
							false,
							null,
							identifier);
}

DataType* data_type_from_string_int(char* type, u8 pointer_count) {
	Token* identifier = token_from_string(type);
	return data_type_create(identifier,
							pointer_count,
							false,
							null,
							identifier);
}

DataTypeMatch data_type_match(DataType* a, DataType* b) {
	if (a && b) {
		if (a->is_array != b->is_array) {
			return DT_NOT_MATCH;
		}
		if (a->is_array) {
			if (!is_token_equal(a->array_elem_count, b->array_elem_count)) {
				return DT_NOT_MATCH;
			}
		}

		if (a->pointer_count != b->pointer_count) {
			return DT_NOT_MATCH;
		}

		if (!is_token_equal(a->identifier, b->identifier)) {
			return DT_NOT_MATCH;
		}

		return DT_MATCH;
	}
	
	return DT_NOT_MATCH;
}

DataTypeMatch data_type_integer(DataType* data_type) {
	if (data_type) {
		DataType** predef_data_types_array = &(data_types.t_int);
		bool integer_match = false;
		for (u64 i = 0; i < _DT_INTEGER_TYPE_COUNT; i++) {
			DataTypeMatch match = data_type_match(data_type, predef_data_types_array[i]);
			if (match != DT_NOT_MATCH) {
				integer_match = true;
				break;
			}
		}

		if (integer_match) {
			return DT_MATCH;
		}
	}

	return DT_NOT_MATCH;
}
