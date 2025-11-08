#include "token.hpp"

#include <algorithm>
#include <atomic>
#include <fstream>
#include <iostream>
#include <string_view>
#include <vector>

namespace
{
	std::atomic<int> g_errorCount = 0;
}

void emitError(const std::string &p_message, const Token &p_token)
{
	++g_errorCount;

	const std::size_t lineNumber = p_token.start.line;
	std::cout << p_token.origin.string() << ":" << lineNumber << " : " << p_message << '\n';

	const auto loadLines = [](const std::filesystem::path &path, std::vector<std::string> &out) {
		std::ifstream file(path);
		if (!file)
		{
			return false;
		}

		std::string line;
		while (std::getline(file, line))
		{
			if (!line.empty() && line.back() == '\r')
			{
				line.pop_back();
			}
			out.emplace_back(std::move(line));
		}
		return true;
	};

	std::vector<std::string> fileLines;
	const bool hasFile = loadLines(p_token.origin, fileLines);
	const auto zeroBasedLine = [](std::size_t line) {
		return (line > 0) ? (line - 1) : static_cast<std::size_t>(0);
	};

	const size_t startLine = zeroBasedLine(p_token.start.line);
	const size_t endLine = zeroBasedLine(p_token.end.line);

	if (hasFile && startLine < fileLines.size())
	{
		const size_t lastLine = std::min(endLine, fileLines.size() - 1);
		for (size_t lineIndex = startLine; lineIndex <= lastLine; ++lineIndex)
		{
			const std::string &line = fileLines[lineIndex];
			std::cout << line << '\n';

			size_t indicatorStart = (lineIndex == startLine) ? p_token.start.column : 0;
			size_t indicatorEnd = (lineIndex == endLine) ? p_token.end.column : line.size();

			if (indicatorStart > line.size())
			{
				indicatorStart = line.size();
			}
			if (indicatorEnd > line.size())
			{
				indicatorEnd = line.size();
			}

			const size_t caretCount =
			    std::max<std::size_t>(1, (indicatorEnd > indicatorStart) ? (indicatorEnd - indicatorStart) : 0);

			std::string prefix;
			prefix.reserve(indicatorStart);
			for (size_t i = 0; i < indicatorStart; ++i)
			{
				const char ch = (i < line.size()) ? line[i] : ' ';
				prefix.push_back((ch == '\t') ? '\t' : ' ');
			}

			std::cout << prefix << std::string(caretCount, '^') << '\n';
		}
		return;
	}

	if (p_token.content.empty())
	{
		std::cout << '\n';
		return;
	}

	const size_t nbLines = (p_token.end.line - p_token.start.line) + 1;
	std::string_view src = p_token.content;
	size_t lineBegin = 0;

	for (size_t i = 0; i < nbLines; ++i)
	{
		size_t nl = src.find('\n', lineBegin);
		size_t lineEnd = (nl == std::string_view::npos) ? src.size() : nl;

		if (lineEnd > lineBegin && src[lineEnd - 1] == '\r')
		{
			--lineEnd;
		}

		std::string_view line = src.substr(lineBegin, lineEnd - lineBegin);

		std::cout << line << '\n';

		const size_t indicatorStart = (i == 0) ? p_token.start.column : 0;
		const size_t indicatorEnd = (i == nbLines - 1) ? p_token.end.column : line.size();
		const size_t caretCount = std::max<std::size_t>(1, (indicatorEnd > indicatorStart) ? (indicatorEnd - indicatorStart) : 0);

		std::cout << std::string(indicatorStart, ' ') << std::string(caretCount, '^') << '\n';

		if (nl == std::string_view::npos)
		{
			break;
		}
		lineBegin = nl + 1;
	}
}

void resetErrorCount()
{
	g_errorCount.store(0, std::memory_order_relaxed);
}

int getErrorCount()
{
	return g_errorCount.load(std::memory_order_relaxed);
}

std::string_view tokenTypeToString(Token::Type p_type)
{
	switch (p_type)
	{
		case Token::Type::EndOfFile:
			return "EndOfFile";
		case Token::Type::Identifier:
			return "Identifier";
		case Token::Type::IntegerLiteral:
			return "IntegerLiteral";
		case Token::Type::FloatLiteral:
			return "FloatLiteral";
		case Token::Type::StringLiteral:
			return "StringLiteral";
		case Token::Type::HeaderLiteral:
			return "HeaderLiteral";
		case Token::Type::Hash:
			return "Hash";
		case Token::Type::Colon:
			return "Colon";
		case Token::Type::DoubleColon:
			return "DoubleColon";
		case Token::Type::Semicolon:
			return "Semicolon";
		case Token::Type::Comma:
			return "Comma";
		case Token::Type::Dot:
			return "Dot";
		case Token::Type::LeftParen:
			return "LeftParen";
		case Token::Type::RightParen:
			return "RightParen";
		case Token::Type::LeftBrace:
			return "LeftBrace";
		case Token::Type::RightBrace:
			return "RightBrace";
		case Token::Type::LeftBracket:
			return "LeftBracket";
		case Token::Type::RightBracket:
			return "RightBracket";
		case Token::Type::Less:
			return "Less";
		case Token::Type::LessEqual:
			return "LessEqual";
		case Token::Type::Greater:
			return "Greater";
		case Token::Type::GreaterEqual:
			return "GreaterEqual";
		case Token::Type::Arrow:
			return "Arrow";
		case Token::Type::Assign:
			return "Assign";
		case Token::Type::Equal:
			return "Equal";
		case Token::Type::Plus:
			return "Plus";
		case Token::Type::PlusEqual:
			return "PlusEqual";
		case Token::Type::PlusPlus:
			return "PlusPlus";
		case Token::Type::Minus:
			return "Minus";
		case Token::Type::MinusEqual:
			return "MinusEqual";
		case Token::Type::MinusMinus:
			return "MinusMinus";
		case Token::Type::Star:
			return "Star";
		case Token::Type::StarEqual:
			return "StarEqual";
		case Token::Type::Slash:
			return "Slash";
		case Token::Type::SlashEqual:
			return "SlashEqual";
		case Token::Type::Percent:
			return "Percent";
		case Token::Type::PercentEqual:
			return "PercentEqual";
		case Token::Type::Bang:
			return "Bang";
		case Token::Type::BangEqual:
			return "BangEqual";
		case Token::Type::Ampersand:
			return "Ampersand";
		case Token::Type::AmpersandAmpersand:
			return "AmpersandAmpersand";
		case Token::Type::AmpersandEqual:
			return "AmpersandEqual";
		case Token::Type::Pipe:
			return "Pipe";
		case Token::Type::PipePipe:
			return "PipePipe";
		case Token::Type::PipeEqual:
			return "PipeEqual";
		case Token::Type::Caret:
			return "Caret";
		case Token::Type::CaretEqual:
			return "CaretEqual";
		case Token::Type::Tilde:
			return "Tilde";
		case Token::Type::Question:
			return "Question";
		case Token::Type::KeywordInclude:
			return "KeywordInclude";
		case Token::Type::KeywordStruct:
			return "KeywordStruct";
		case Token::Type::KeywordNamespace:
			return "KeywordNamespace";
		case Token::Type::KeywordAttributeBlock:
			return "KeywordAttributeBlock";
		case Token::Type::KeywordConstantBlock:
			return "KeywordConstantBlock";
		case Token::Type::KeywordTexture:
			return "KeywordTexture";
		case Token::Type::KeywordDefine:
			return "KeywordDefine";
		case Token::Type::KeywordReturn:
			return "KeywordReturn";
		case Token::Type::KeywordIf:
			return "KeywordIf";
		case Token::Type::KeywordElse:
			return "KeywordElse";
		case Token::Type::KeywordFor:
			return "KeywordFor";
		case Token::Type::KeywordWhile:
			return "KeywordWhile";
		case Token::Type::KeywordDo:
			return "KeywordDo";
		case Token::Type::KeywordBreak:
			return "KeywordBreak";
		case Token::Type::KeywordContinue:
			return "KeywordContinue";
		case Token::Type::KeywordConst:
			return "KeywordConst";
		case Token::Type::KeywordDiscard:
			return "KeywordDiscard";
		case Token::Type::KeywordThis:
			return "KeywordThis";
		case Token::Type::KeywordInput:
			return "KeywordInput";
		case Token::Type::KeywordOutput:
			return "KeywordOutput";
		case Token::Type::KeywordVertexPass:
			return "KeywordVertexPass";
		case Token::Type::KeywordFragmentPass:
			return "KeywordFragmentPass";
		case Token::Type::KeywordTrue:
			return "KeywordTrue";
		case Token::Type::KeywordFalse:
			return "KeywordFalse";
	}

	return "Unknown";
}
