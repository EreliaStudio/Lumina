#pragma once

#include <string>
#include <filesystem>

#define DEBUG_INFORMATION (" " + std::string(__FUNCTION__) + "::" + std::to_string(__LINE__))

namespace Lumina
{
	std::string readFileAsString(const std::filesystem::path& p_path);
	std::filesystem::path composeFilePath(const std::string& fileName, const std::vector<std::filesystem::path>& additionnalPaths = {});
}
