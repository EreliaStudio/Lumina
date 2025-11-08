#pragma once

#include "token.hpp"

#include <filesystem>
#include <vector>

struct PrecompilationParser
{
	PrecompilationParser();
	explicit PrecompilationParser(std::vector<std::filesystem::path> p_includeDirs);

	void operator()(std::vector<Token> &p_rawTokens);

private:
	std::vector<std::filesystem::path> m_includeDirectories;
};
