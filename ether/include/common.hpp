#pragma once

#include <string>

bool match_extension(std::string& str, const char* ext);
char* change_extension(std::string& str, char* ext);
