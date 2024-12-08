#include "utils.hpp"

#include <fstream>
#include <iostream>
#include <cstdlib>

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
	
	// Updated getEnvVar function
	std::string getEnvVar(const std::string& key)
	{
		const char* val = std::getenv(key.c_str());
		if (val == nullptr)
		{
			return "";
		}
		else
		{
			return std::string(val);
		}
	}

	// Updated composeFilePath function
	std::filesystem::path composeFilePath(const std::string& p_fileName, const std::vector<std::filesystem::path>& p_additionalPaths)
	{
		const char* pathEnv = std::getenv("PATH");
		if (!pathEnv)
		{
			std::cerr << "PATH environment variable not found." << std::endl;
			return std::filesystem::path();
		}
		std::string pathStr(pathEnv);

		std::vector<std::filesystem::path> paths;
		std::stringstream ss(pathStr);
		std::string path;

	#ifdef _WIN32
		const char delimiter = ';';
	#else
		const char delimiter = ':';
	#endif

		// Split the PATH environment variable
		while (std::getline(ss, path, delimiter))
		{
			std::filesystem::path dirPath(path);
			dirPath /= "includes";  // Append "includes" to the path
			paths.push_back(dirPath);
		}

		// Search in PATH/includes directories
		for (const auto& dir : paths)
		{
			std::filesystem::path filePath = dir / p_fileName;
			if (std::filesystem::exists(filePath) && std::filesystem::is_regular_file(filePath))
			{
				return filePath;
			}
		}

		// Search in additional paths provided
		for (const auto& dir : p_additionalPaths)
		{
			std::filesystem::path filePath = dir / p_fileName;
			if (std::filesystem::exists(filePath) && std::filesystem::is_regular_file(filePath))
			{
				return filePath;
			}
		}

		// Search in the current directory
		std::filesystem::path currentDir = std::filesystem::current_path();
		std::filesystem::path filePath = currentDir / p_fileName;
		if (std::filesystem::exists(filePath) && std::filesystem::is_regular_file(filePath))
		{
			return filePath;
		}

		// File not found
		return std::filesystem::path();
	}

	std::string arraySizeToString(const std::vector<size_t>& p_arraySize)
	{
		std::string result;

		for (const auto& size : p_arraySize)
		{
			result += "[" + std::to_string(size) + "]";
		}

		return (result);
	}
}
