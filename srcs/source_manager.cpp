#include "source_manager.hpp"

#include "precompilation_parser.hpp"
#include "tokenizer.hpp"
#include "utils.hpp"

#include <system_error>
#include <unordered_map>

namespace
{
	std::unordered_map<std::filesystem::path, std::vector<Token>> alreadyLoadedFile;
	std::vector<std::filesystem::path> includeDirectories = readPathListFromEnv("LUMINA_INCLUDE_PATH");

	std::filesystem::path normalizePath(const std::filesystem::path &input)
	{
		std::error_code ec;
		std::filesystem::path normalized = std::filesystem::weakly_canonical(input, ec);
		if (ec)
		{
			normalized = std::filesystem::absolute(input, ec);
		}
		return normalized.empty() ? input : normalized;
	}
}

const std::vector<Token> &SourceManager::loadFile(const std::filesystem::path &p_path)
{
	const std::filesystem::path normalized = normalizePath(p_path);

	auto it = alreadyLoadedFile.find(normalized);
	if (it == alreadyLoadedFile.end())
	{
		Tokenizer tokenizer;
		std::vector<Token> tokens = tokenizer(normalized);

		PrecompilationParser precompilationParser(includeDirectories);
		precompilationParser(tokens);

		it = alreadyLoadedFile.emplace(normalized, std::move(tokens)).first;
	}

	return it->second;
}

void SourceManager::setIncludeDirectories(std::vector<std::filesystem::path> p_dirs)
{
	includeDirectories.clear();
	for (std::filesystem::path &dir : p_dirs)
	{
		if (!dir.empty())
		{
			includeDirectories.push_back(dir);
		}
	}
}

void SourceManager::addIncludeDirectory(const std::filesystem::path &p_dir)
{
	if (p_dir.empty())
	{
		return;
	}
	includeDirectories.push_back(p_dir);
}

const std::vector<std::filesystem::path> &SourceManager::getIncludeDirectories()
{
	return includeDirectories;
}
