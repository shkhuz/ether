#include <ether.hpp>

static std::vector<StrIntern> interns;

char* str_intern_range(char* start, char* end) {
	u64 len = end - start;
	for (auto it = interns.begin(); it != interns.end(); ++it) {
		if (it->len == len &&
			strncmp(it->str, start, len) == false) {
			return it->str;
		}
	}

	char* str = (char*)malloc(len + 1);
	memcpy(str, start, len);
	str[len] = 0;
	interns.emplace_back((StrIntern){ str, len });
	return str;
}

char* str_intern(char* str) {
	return str_intern_range(str, str + strlen(str));
}
