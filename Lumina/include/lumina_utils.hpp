#pragma once

#ifndef NDEBUG
#define DEBUG_INFORMATION std::string("    [") + __FUNCTION__ + "::" + std::to_string(__LINE__) + "]"
#else
#define DEBUG_INFORMATION std::string()
#endif

#ifndef NDEBUG
#define COMPOSED_DEBUG_INFORMATION(currentDebugInformation) currentDebugInformation + std::string(" ") + __FUNCTION__ + "::" + std::to_string(__LINE__)
#else
#define COMPOSED_DEBUG_INFORMATION(currentDebugInformation) std::string()
#endif

#define DEBUG_LINE() std::cout << __FUNCTION__ << "::" << __LINE__ << std::endl

#include <string>
#include <filesystem>

namespace Lumina
{
	std::string readFileAsString(const std::filesystem::path& p_path);
	std::filesystem::path composeFilePath(const std::string& fileName, const std::vector<std::filesystem::path>& additionnalPaths = {});
}