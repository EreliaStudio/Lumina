#pragma once

#include <string>
#include <filesystem>
#include <vector>

#define DEBUG_INFORMATION std::string(" ") + std::string(__FUNCTION__) + "::" + std::to_string(__LINE__)

namespace Lumina
{
	std::string readFileAsString(const std::filesystem::path& p_path);
	std::string getEnvVar(const std::string& key);
	std::filesystem::path composeFilePath(const std::string& p_fileName, const std::vector<std::filesystem::path>& p_additionnalPaths = {});
	std::string arraySizeToString(const std::vector<size_t>& p_arraySize);
}
