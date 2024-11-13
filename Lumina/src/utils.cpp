#include "utils.hpp"

#include <fstream>
#include <iostream>

namespace Lumina
{
	std::string readFileAsString(const std::filesystem::path& p_path)
	{
		std::fstream inputFile;
		inputFile.open(p_path, std::ios_base::in);

		std::string line;
		std::string result = "";
		while (std::getline(inputFile, line))
		{
			result += line + "\n";
		}

		inputFile.close();

		std::string tab = "\t";
		std::string spaces = "    ";
		size_t pos = 0;

		while ((pos = result.find(tab, pos)) != std::string::npos)
		{
			result.replace(pos, tab.length(), spaces);
			pos += spaces.length();
		}

		return (result);
	}

	std::string getEnvVar(const std::string& key)
	{
#ifdef _WIN32
		// Windows implementation using _dupenv_s
		char* value = nullptr;
		size_t len = 0;
		errno_t err = _dupenv_s(&value, &len, key.c_str());
		if (err || value == nullptr)
		{
			return "";
		}
		else
		{
			std::string result(value);
			free(value);
			return result;
		}
#else
		// POSIX implementation using getenv
		const char* val = std::getenv(key.c_str());
		if (val == nullptr)
		{
			return "";
		}
		else
		{
			return std::string(val);
		}
#endif
	}

	std::filesystem::path composeFilePath(const std::string& p_fileName, const std::vector<std::filesystem::path>& p_additionnalPaths)
	{
		std::string pathStr;

#ifdef _WIN32
		char* pathEnv = nullptr;
		size_t len = 0;
		if (_dupenv_s(&pathEnv, &len, "PATH") != 0 || pathEnv == nullptr)
		{
			std::cerr << "PATH environment variable not found." << std::endl;
			return std::filesystem::path();
		}
		pathStr = std::string(pathEnv);
		free(pathEnv);
#else
		const char* pathEnv = std::getenv("PATH");
		if (!pathEnv)
		{
			std::cerr << "PATH environment variable not found." << std::endl;
			return std::filesystem::path();
		}
		pathStr = std::string(pathEnv);
#endif

		std::vector<std::filesystem::path> paths;
		std::stringstream ss(pathStr);
		std::string path;

#ifdef _WIN32
		const char delimiter = ';';
#else
		const char delimiter = ':';
#endif
	
		while (std::getline(ss, path, delimiter))
		{
			std::string fuzedPath = path + "\\includes";
			paths.push_back(fuzedPath);
		}

		for (const auto& dir : paths)
		{
			std::filesystem::path filePath = dir / p_fileName;
			if (std::filesystem::exists(filePath) && std::filesystem::is_regular_file(filePath))
			{
				return filePath;
			}
		}

		for (const auto& dir : p_additionnalPaths)
		{
			std::filesystem::path filePath = dir / p_fileName;
			if (std::filesystem::exists(filePath) && std::filesystem::is_regular_file(filePath))
			{
				return filePath;
			}
		}

		std::filesystem::path currentDir = std::filesystem::current_path();
		std::filesystem::path filePath = currentDir / p_fileName;
		if (std::filesystem::exists(filePath) && std::filesystem::is_regular_file(filePath))
		{
			return filePath;
		}

		return std::filesystem::path();
	}
}
