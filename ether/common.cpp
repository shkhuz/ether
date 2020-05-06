#include <common.hpp>
#include <typedef.hpp>
#include <str_intern.hpp>

#include <string>

bool match_extension(std::string& str, const char* ext) {
	u64 dot_index = std::string::npos;
	if ((dot_index = str.find_last_of('.')) == std::string::npos) {
		return false;
	}

	std::string ext_str = str.substr(dot_index + 1, std::string::npos);
	std::string target_ext_str = std::string(ext);
	if (ext_str != target_ext_str) {
		return false;
	}

	return true;
}

char* change_extension(std::string& str, char* ext) {
	u64 dot_index = std::string::npos;
	if ((dot_index = str.find_last_of('.')) == std::string::npos) {
		return null;
	}

	std::string file_without_ext = str.substr(0, dot_index + 1);
	file_without_ext.append(ext);
	return str_intern(const_cast<char*>(file_without_ext.c_str()));
}
