#pragma once

struct StrIntern {
	char* str;
	u64 len;
};

char* str_intern_range(char* start, char* end);
char* str_intern(char* str);
