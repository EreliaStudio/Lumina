#pragma once

#include "token.hpp"

#include <filesystem>
#include <vector>

struct SourceManager
{
	static const std::vector<Token> &loadFile(const std::filesystem::path &p_path);

	static void setIncludeDirectories(std::vector<std::filesystem::path> p_dirs);
	static void addIncludeDirectory(const std::filesystem::path &p_dir);
	static const std::vector<std::filesystem::path> &getIncludeDirectories();
};
