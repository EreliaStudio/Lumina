#include "precompilation_parser.hpp"

#include "tokenizer.hpp"
#include "utils.hpp"

#include <algorithm>
#include <sstream>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <unordered_map>
#include <utility>
#include <vector>

namespace
{
	struct Macro
	{
		std::vector<Token> replacement;
	};

	using MacroTable = std::unordered_map<std::string, Macro>;

	struct PreprocessorState
	{
		MacroTable macros;
		std::vector<std::string> macroExpansionStack;
		std::vector<std::filesystem::path> includeStack;
	};

	std::string makeErrorPrefix(const Token &token)
	{
		return token.origin.string() + ":" + std::to_string(token.start.line) + ":" + std::to_string(token.start.column) +
		       ": ";
	}

	void appendWithExpansion(const Token &token, std::vector<Token> &out, PreprocessorState &state)
	{
		if (token.type != Token::Type::Identifier)
		{
			out.push_back(token);
			return;
		}

		const auto macroIt = state.macros.find(token.content);
		if (macroIt == state.macros.end())
		{
			out.push_back(token);
			return;
		}

		if (std::find(state.macroExpansionStack.begin(), state.macroExpansionStack.end(), token.content) !=
		    state.macroExpansionStack.end())
		{
			std::ostringstream oss;
			oss << makeErrorPrefix(token) << "Recursive macro expansion of '" << token.content << "'";
			if (!state.macroExpansionStack.empty())
			{
				oss << " (expansion stack: ";
				for (size_t i = 0; i < state.macroExpansionStack.size(); ++i)
				{
					if (i > 0)
					{
						oss << " -> ";
					}
					oss << state.macroExpansionStack[i];
				}
				oss << ")";
			}
			throw std::runtime_error(oss.str());
		}

		state.macroExpansionStack.push_back(token.content);
		for (const Token &macroToken : macroIt->second.replacement)
		{
			appendWithExpansion(macroToken, out, state);
		}
		state.macroExpansionStack.pop_back();
	}

	size_t consumeDefineDirective(const std::vector<Token> &tokens, size_t hashIndex, PreprocessorState &state)
	{
		const Token &hashToken = tokens[hashIndex];
		const size_t directiveLine = hashToken.start.line;

		if (hashIndex + 2 >= tokens.size())
		{
			throw std::runtime_error(makeErrorPrefix(hashToken) + "Incomplete #define directive");
		}

		const Token &keywordToken = tokens[hashIndex + 1];
		const size_t nameIndex = hashIndex + 2;
		if (nameIndex >= tokens.size() || tokens[nameIndex].type != Token::Type::Identifier)
		{
			throw std::runtime_error(makeErrorPrefix(keywordToken) + "Expected identifier in #define directive");
		}

		const Token &nameToken = tokens[nameIndex];
		size_t replacementBegin = nameIndex + 1;
		size_t replacementEnd = replacementBegin;

		while (replacementEnd < tokens.size())
		{
			const Token &candidate = tokens[replacementEnd];
			if (candidate.type == Token::Type::EndOfFile || candidate.start.line != directiveLine)
			{
				break;
			}
			++replacementEnd;
		}

		std::vector<Token> replacement;
		replacement.reserve(replacementEnd - replacementBegin);
		for (size_t i = replacementBegin; i < replacementEnd; ++i)
		{
			replacement.push_back(tokens[i]);
		}

		state.macros[nameToken.content] = Macro{std::move(replacement)};
		return replacementEnd;
	}

	bool fileExists(const std::filesystem::path &path)
	{
		std::error_code ec;
		return std::filesystem::exists(path, ec) && std::filesystem::is_regular_file(path, ec);
	}

	std::filesystem::path canonicalizeExisting(const std::filesystem::path &path)
	{
		std::error_code ec;
		std::filesystem::path normalized = std::filesystem::weakly_canonical(path, ec);
		if (!ec)
		{
			return normalized;
		}
		normalized = std::filesystem::absolute(path, ec);
		return normalized.empty() ? path : normalized;
	}

	std::string unescapeStringLiteral(std::string_view body)
	{
		std::string result;
		result.reserve(body.size());

		for (size_t i = 0; i < body.size(); ++i)
		{
			const char c = body[i];
			if (c == '\\' && (i + 1) < body.size())
			{
				const char next = body[++i];
				switch (next)
				{
					case 'n':
						result.push_back('\n');
						break;
					case 'r':
						result.push_back('\r');
						break;
					case 't':
						result.push_back('\t');
						break;
					case '\\':
						result.push_back('\\');
						break;
					case '"':
						result.push_back('"');
						break;
					default:
						result.push_back(next);
						break;
				}
			}
			else
			{
				result.push_back(c);
			}
		}

		return result;
	}

	std::string decodeIncludeOperand(const Token &token)
	{
		const std::string &text = token.content;
		if (text.size() < 2)
		{
			throw std::runtime_error(makeErrorPrefix(token) + "Malformed include operand");
		}

		if (token.type == Token::Type::StringLiteral)
		{
			return unescapeStringLiteral(std::string_view(text).substr(1, text.size() - 2));
		}

		if (token.type == Token::Type::HeaderLiteral)
		{
			return text.substr(1, text.size() - 2);
		}

		throw std::runtime_error(makeErrorPrefix(token) + "Expected string or header literal");
	}

	std::optional<std::filesystem::path> tryResolveAgainst(
	    const std::filesystem::path &requested, const std::vector<std::filesystem::path> &dirs)
	{
		for (const std::filesystem::path &dir : dirs)
		{
			if (dir.empty())
			{
				continue;
			}

			std::filesystem::path candidate = dir / requested;
			if (fileExists(candidate))
			{
				return canonicalizeExisting(candidate);
			}
		}

		return std::nullopt;
	}

	const std::vector<std::filesystem::path> &systemPathDirectories()
	{
		static const std::vector<std::filesystem::path> dirs = readPathListFromEnv("PATH");
		return dirs;
	}

	std::filesystem::path resolveIncludePath(const Token &operand,
	    const std::vector<std::filesystem::path> &includeDirs)
	{
		const std::string rawText = decodeIncludeOperand(operand);
		if (rawText.empty())
		{
			throw std::runtime_error(makeErrorPrefix(operand) + "#include target cannot be empty");
		}

		std::filesystem::path requested(rawText);
		if (requested.is_absolute())
		{
			if (!fileExists(requested))
			{
				throw std::runtime_error(makeErrorPrefix(operand) + "Cannot find include file '" + rawText + "'");
			}
			return canonicalizeExisting(requested);
		}

		std::vector<std::filesystem::path> searchDirs;
		searchDirs.reserve(includeDirs.size() + 1);
		const std::filesystem::path baseDir = operand.origin.parent_path();
		if (!baseDir.empty())
		{
			searchDirs.push_back(baseDir);
		}
		searchDirs.insert(searchDirs.end(), includeDirs.begin(), includeDirs.end());

		if (const std::optional<std::filesystem::path> resolved = tryResolveAgainst(requested, searchDirs))
		{
			return *resolved;
		}

		if (const std::optional<std::filesystem::path> resolved = tryResolveAgainst(requested, systemPathDirectories()))
		{
			return *resolved;
		}

		throw std::runtime_error(makeErrorPrefix(operand) + "Cannot find include file '" + rawText + "'");
	}

	void processTokens(const std::vector<Token> &tokens, std::vector<Token> &out, PreprocessorState &state,
	    const std::vector<std::filesystem::path> &includeDirs);

	size_t handleIncludeDirective(const std::vector<Token> &tokens, size_t hashIndex, std::vector<Token> &out,
	    PreprocessorState &state, const std::vector<std::filesystem::path> &includeDirs)
	{
		const Token &hashToken = tokens[hashIndex];
		const size_t directiveLine = hashToken.start.line;

		if (hashIndex + 2 >= tokens.size())
		{
			throw std::runtime_error(makeErrorPrefix(hashToken) + "Incomplete #include directive");
		}

		const Token &operandToken = tokens[hashIndex + 2];
		if (operandToken.type != Token::Type::StringLiteral && operandToken.type != Token::Type::HeaderLiteral)
		{
			throw std::runtime_error(makeErrorPrefix(operandToken) + "Expected file literal in #include");
		}

		const std::filesystem::path resolved = resolveIncludePath(operandToken, includeDirs);

		if (std::find(state.includeStack.begin(), state.includeStack.end(), resolved) != state.includeStack.end())
		{
			std::ostringstream oss;
			oss << makeErrorPrefix(operandToken) << "Recursive include detected for '" << resolved.string() << "'";
			throw std::runtime_error(oss.str());
		}

		Tokenizer tokenizer;
		std::vector<Token> includedTokens;
		try
		{
			includedTokens = tokenizer(resolved);
		}
		catch (const std::exception &e)
		{
			throw std::runtime_error(makeErrorPrefix(operandToken) + "Failed to include '" + resolved.string() +
			                         "': " + e.what());
		}

		state.includeStack.push_back(resolved);
		processTokens(includedTokens, out, state, includeDirs);
		state.includeStack.pop_back();

		size_t nextIndex = hashIndex + 3;
		while (nextIndex < tokens.size())
		{
			const Token &candidate = tokens[nextIndex];
			if (candidate.type == Token::Type::EndOfFile || candidate.start.line != directiveLine)
			{
				break;
			}
			++nextIndex;
		}

		return nextIndex;
	}

	void processTokens(const std::vector<Token> &tokens, std::vector<Token> &out, PreprocessorState &state,
	    const std::vector<std::filesystem::path> &includeDirs)
	{
		for (size_t index = 0; index < tokens.size();)
		{
			const Token &token = tokens[index];

			if (token.type == Token::Type::Hash && (index + 1) < tokens.size())
			{
				const Token::Type directive = tokens[index + 1].type;
				if (directive == Token::Type::KeywordDefine)
				{
					index = consumeDefineDirective(tokens, index, state);
					continue;
				}
				if (directive == Token::Type::KeywordInclude)
				{
					index = handleIncludeDirective(tokens, index, out, state, includeDirs);
					continue;
				}
			}

			if (token.type == Token::Type::EndOfFile)
			{
				break;
			}

			appendWithExpansion(token, out, state);
			++index;
		}
	}
}

PrecompilationParser::PrecompilationParser() = default;

PrecompilationParser::PrecompilationParser(std::vector<std::filesystem::path> p_includeDirs)
    : m_includeDirectories(std::move(p_includeDirs))
{
}

void PrecompilationParser::operator()(std::vector<Token> &p_rawTokens)
{
	if (p_rawTokens.empty())
	{
		return;
	}

	PreprocessorState state;
	std::vector<Token> processed;
	processed.reserve(p_rawTokens.size());

	processTokens(p_rawTokens, processed, state, m_includeDirectories);

	if (processed.empty() || processed.back().type != Token::Type::EndOfFile)
	{
		Token eof = p_rawTokens.back();
		eof.type = Token::Type::EndOfFile;
		eof.content.clear();
		processed.push_back(eof);
	}

	p_rawTokens = std::move(processed);
}
