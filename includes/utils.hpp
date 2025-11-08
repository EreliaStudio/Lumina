#pragma once

#include "token.hpp"

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

struct Cursor
{
	size_t offset = 0;
	size_t line = 1;
	size_t column = 0;
};

std::string normalizeLineEndings(std::string input);

bool isWhitespace(char c);
bool isDigit(char c);
bool isHexDigit(char c);
bool isIdentifierStart(char c);
bool isIdentifierBody(char c);

void advanceCursor(Cursor &cursor, char c);

std::optional<Token::Type> lookupKeyword(std::string_view word);

std::vector<std::filesystem::path> splitPathList(const std::string &p_list);
std::vector<std::filesystem::path> readPathListFromEnv(const char *p_envName);
