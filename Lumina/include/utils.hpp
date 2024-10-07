#pragma once

#include <string>
#include <filesystem>

namespace Lumina
{
	std::string readFileAsString(const std::filesystem::path& p_path);
	std::filesystem::path composeFilePath(const std::string& fileName, const std::vector<std::filesystem::path>& additionnalPaths = {});
}
