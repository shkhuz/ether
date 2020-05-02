#include <ether.hpp>
#include <str_intern.hpp>

static StrIntern* interns;

char* str_intern_range(char* start, char* end) {
	u64 len = end - start;
	buf_loop(interns, i) {
		if (interns[i].len == len &&
			strncmp(interns[i].str, start, len) == false) {
			return interns[i].str;
		}
	}

	char* str = (char*)malloc(len + 1);
	memcpy(str, start, len);
	str[len] = 0;
	buf_push(interns, (StrIntern){ str, len });
	return str;
}

char* str_intern(char* str) {
	return str_intern_range(str, str + strlen(str));
}
