#include "utils.hpp"

#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <unordered_map>

std::string normalizeLineEndings(std::string input)
{
	std::string normalized;
	normalized.reserve(input.size());

	for (size_t i = 0; i < input.size(); ++i)
	{
		const char c = input[i];
		if (c == '\r')
		{
			normalized.push_back('\n');
			if (i + 1 < input.size() && input[i + 1] == '\n')
			{
				++i;
			}
		}
		else
		{
			normalized.push_back(c);
		}
	}

	return normalized;
}

bool isWhitespace(char c)
{
	return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f';
}

bool isDigit(char c)
{
	return c >= '0' && c <= '9';
}

bool isHexDigit(char c)
{
	return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

bool isIdentifierStart(char c)
{
	return (c == '_') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool isIdentifierBody(char c)
{
	return isIdentifierStart(c) || isDigit(c);
}

void advanceCursor(Cursor &cursor, char c)
{
	++cursor.offset;
	if (c == '\n')
	{
		++cursor.line;
		cursor.column = 0;
	}
	else
	{
		++cursor.column;
	}
}

std::optional<Token::Type> lookupKeyword(std::string_view word)
{
	static const std::unordered_map<std::string_view, Token::Type> keywords = {
	    {"include", Token::Type::KeywordInclude},
	    {"struct", Token::Type::KeywordStruct},
	    {"namespace", Token::Type::KeywordNamespace},
	    {"AttributeBlock", Token::Type::KeywordAttributeBlock},
	    {"ConstantBlock", Token::Type::KeywordConstantBlock},
	    {"Texture", Token::Type::KeywordTexture},
	    {"define", Token::Type::KeywordDefine},
	    {"return", Token::Type::KeywordReturn},
	    {"if", Token::Type::KeywordIf},
	    {"else", Token::Type::KeywordElse},
	    {"for", Token::Type::KeywordFor},
	    {"while", Token::Type::KeywordWhile},
	    {"do", Token::Type::KeywordDo},
	    {"break", Token::Type::KeywordBreak},
	    {"continue", Token::Type::KeywordContinue},
	    {"const", Token::Type::KeywordConst},
	    {"discard", Token::Type::KeywordDiscard},
	    {"this", Token::Type::KeywordThis},
	    {"Input", Token::Type::KeywordInput},
	    {"Output", Token::Type::KeywordOutput},
	    {"VertexPass", Token::Type::KeywordVertexPass},
	    {"FragmentPass", Token::Type::KeywordFragmentPass},
	    {"true", Token::Type::KeywordTrue},
	    {"false", Token::Type::KeywordFalse},
	};

	const auto it = keywords.find(word);
	if (it != keywords.end())
	{
		return it->second;
	}
	return std::nullopt;
}

namespace
{
	char pathListSeparator()
	{
#if defined(_WIN32)
		return ';';
#else
		return ':';
#endif
	}
}

std::vector<std::filesystem::path> splitPathList(const std::string &p_list)
{
	std::vector<std::filesystem::path> result;
	if (p_list.empty())
	{
		return result;
	}

	const char separator = pathListSeparator();
	size_t start = 0;

	while (start <= p_list.size())
	{
		const size_t end = p_list.find(separator, start);
		const size_t length = (end == std::string::npos) ? std::string::npos : (end - start);
		std::string entry = p_list.substr(start, length);

		size_t first = 0;
		while (first < entry.size() && std::isspace(static_cast<unsigned char>(entry[first])))
		{
			++first;
		}

		size_t last = entry.size();
		while (last > first && std::isspace(static_cast<unsigned char>(entry[last - 1])))
		{
			--last;
		}

		if (last > first)
		{
			result.emplace_back(entry.substr(first, last - first));
		}

		if (end == std::string::npos)
		{
			break;
		}
		start = end + 1;
	}

	return result;
}

std::vector<std::filesystem::path> readPathListFromEnv(const char *p_envName)
{
	if (p_envName == nullptr)
	{
		return {};
	}

	auto fetchEnv = [](const char *name) -> std::optional<std::string> {
#if defined(_WIN32)
		char *buffer = nullptr;
		size_t length = 0;
		if (_dupenv_s(&buffer, &length, name) != 0 || buffer == nullptr)
		{
			return std::nullopt;
		}
		std::string value(buffer, (length > 0) ? length - 1 : 0);
		free(buffer);
		return value;
#else
		const char *raw = std::getenv(name);
		if (raw == nullptr)
		{
			return std::nullopt;
		}
		return std::string(raw);
#endif
	};

	const std::optional<std::string> value = fetchEnv(p_envName);
	if (!value || value->empty())
	{
		return {};
	}

	return splitPathList(*value);
}
