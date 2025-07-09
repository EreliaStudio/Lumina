#pragma once

#include "token.hpp"

#include <filesystem>
#include <vector>
#include <unordered_map>
#include <fstream>

namespace 
{
	struct Cursor
	{
		const std::string &src;
		size_t pos{0}, row{1}, col{1};

		bool eof() const { return pos >= src.size(); }
		char cur() const { return eof() ? '\0' : src[pos]; }
		char peek(int o = 1) const { return (pos + o < src.size()) ? src[pos + o] : '\0'; }

		void advance(size_t n = 1)
		{
			while (n-- && pos < src.size())
			{
				if (src[pos] == '\n')
				{
					++row;
					col = 1;
				}
				else
				{
					++col;
				}
				++pos;
			}
		}
	};

	bool isAlpha(char c) { return std::isalpha(static_cast<unsigned char>(c)) || c == '_'; }
	bool isDigit(char c) { return std::isdigit(static_cast<unsigned char>(c)); }
	bool isAlnum(char c) { return isAlpha(c) || isDigit(c); }

	const std::unordered_map<std::string, Token::Type> kKeywordMap = {
			{"struct", Token::Type::KwStruct},
			{"namespace", Token::Type::KwNamespace},
			{"AttributeBlock", Token::Type::KwAttributeBlock},
			{"ConstantBlock", Token::Type::KwConstantBlock},
			{"Texture", Token::Type::KwTexture},
			{"Input", Token::Type::KwInput},
			{"VertexPass", Token::Type::KwVertexPass},
			{"FragmentPass", Token::Type::KwFragmentPass},
			{"Output", Token::Type::KwOutput},
			{"raiseException", Token::Type::KwRaiseException},
			{"discard", Token::Type::KwDiscard},
			{"if", Token::Type::KwIf},
			{"else", Token::Type::KwElse},
			{"while", Token::Type::KwWhile},
			{"do", Token::Type::KwDo},
			{"return", Token::Type::KwReturn},

			{"true", Token::Type::BoolLiteral},
			{"false", Token::Type::BoolLiteral}
		};

	const std::unordered_map<std::string, Token::Type> kDoubleOps = {
			{"++", Token::Type::Increment},
			{"--", Token::Type::Decrement},
			{"+=", Token::Type::PlusEqual},
			{"-=", Token::Type::MinusEqual},
			{"*=", Token::Type::StarEqual},
			{"/=", Token::Type::SlashEqual},
			{"%=", Token::Type::PercentEqual},
			{"==", Token::Type::EqualEqual},
			{"!=", Token::Type::NotEqual},
			{"<=", Token::Type::LessEqual},
			{">=", Token::Type::GreaterEqual},
			{"&&", Token::Type::LogicalAnd},
			{"||", Token::Type::LogicalOr},
			{"->", Token::Type::Arrow},
			{"::", Token::Type::DoubleColon}
		};

	const std::unordered_map<char, Token::Type> kSingleOps = {
			{'+', Token::Type::Plus},
			{'-', Token::Type::Minus},
			{'*', Token::Type::Star},
			{'/', Token::Type::Slash},
			{'%', Token::Type::Percent},
			{'=', Token::Type::Equal},
			{'<', Token::Type::Less},
			{'>', Token::Type::Greater},
			{'!', Token::Type::LogicalNot},
			{':', Token::Type::Colon},
			{',', Token::Type::Comma},
			{';', Token::Type::Semicolon},
			{'.', Token::Type::Dot},
			{'(', Token::Type::LeftParen},
			{')', Token::Type::RightParen},
			{'{', Token::Type::LeftBrace},
			{'}', Token::Type::RightBrace},
			{'[', Token::Type::LeftBracket},
			{']', Token::Type::RightBracket},
			{'#', Token::Type::Hash}
		};

	inline void push(std::vector<Token> &tks, Token::Type t, std::string_view lex, const Cursor &c, size_t sr, size_t sc)
	{
		tks.push_back(Token{t, std::string(lex), sr, sc});
	}

	bool skipWhitespaceAndComments(Cursor &c)
	{
		bool skipped = false;
		for (;;)
		{
			while (std::isspace(static_cast<unsigned char>(c.cur())))
			{
				c.advance();
				skipped = true;
			}

			if (c.cur() == '/' && c.peek() == '/')
			{
				while (c.cur() != '\n' && !c.eof())
					c.advance();
				skipped = true;
				continue;
			}

			if (c.cur() == '/' && c.peek() == '*')
			{
				c.advance(2);
				while (!c.eof() && !(c.cur() == '*' && c.peek() == '/'))
					c.advance();
				if (!c.eof())
					c.advance(2);
				skipped = true;
				continue;
			}

			break;
		}
		return skipped;
	}

	bool scanStringLit(Cursor &c, std::vector<Token> &out)
	{
		if (c.cur() != '"')
			return false;
		const size_t sr = c.row, sc = c.col, start = c.pos;
		c.advance();
		while (!c.eof())
		{
			if (c.cur() == '\\')
				c.advance(2);
			else if (c.cur() == '"')
			{
				c.advance();
				break;
			}
			else
				c.advance();
		}
		push(out, Token::Type::StringLiteral,
			 std::string_view(c.src.data() + start, c.pos - start), c, sr, sc);
		return true;
	}

	bool scanNumber(Cursor &c, std::vector<Token> &out)
	{
		const bool startsWithDigit = std::isdigit(static_cast<unsigned char>(c.cur()));
		const bool dotThenDigit = (c.cur() == '.' && std::isdigit(static_cast<unsigned char>(c.peek())));
		if (!startsWithDigit && !dotThenDigit)
			return false;

		const size_t sr = c.row, sc = c.col, start = c.pos;
		bool isFloat = false;

		while (std::isdigit(static_cast<unsigned char>(c.cur())))
			c.advance();
		if (c.cur() == '.')
		{
			isFloat = true;
			c.advance();
			while (std::isdigit(static_cast<unsigned char>(c.cur())))
				c.advance();
		}
		if (c.cur() == 'f' || c.cur() == 'F')
		{
			isFloat = true;
			c.advance();
		}

		push(out, isFloat ? Token::Type::FloatLiteral : Token::Type::IntLiteral,
			 std::string_view(c.src.data() + start, c.pos - start), c, sr, sc);

		return true;
	}

	bool scanIdentifierOrKeyword(Cursor &c, std::vector<Token> &out)
	{
		auto isAlpha = [](char ch)
		{
			return std::isalpha(static_cast<unsigned char>(ch)) || ch == '_';
		};
		auto isAlnum = [&](char ch)
		{
			return isAlpha(ch) || std::isdigit(static_cast<unsigned char>(ch));
		};

		if (!isAlpha(c.cur()))
		{
			return false;
		}
		const size_t sr = c.row, sc = c.col, start = c.pos;
		c.advance();
		while (isAlnum(c.cur()))
		{
			c.advance();
		}

		std::string_view lex(c.src.data() + start, c.pos - start);
		auto it = kKeywordMap.find(std::string(lex));
		Token::Type type = (it != kKeywordMap.end()) ? it->second : Token::Type::Identifier;

		push(out, type, lex, c, sr, sc);

		return true;
	}

	bool scanInclude(Cursor &c, std::vector<Token> &out)
	{
		if (c.cur() != '#' || c.src.compare(c.pos, 8, "#include") != 0)
			return false;
		const size_t sr = c.row, sc = c.col;
		c.advance(8);
		push(out, Token::Type::KwInclude, "#include", c, sr, sc);
		return true;
	}

	bool scanOperator(Cursor &c, std::vector<Token> &out)
	{
		// longest match first – try 2-char
		std::string two{c.cur(), c.peek()};
		if (auto it = kDoubleOps.find(two); it != kDoubleOps.end())
		{
			const size_t sr = c.row, sc = c.col;
			c.advance(2);
			push(out, it->second, two, c, sr, sc);
			return true;
		}
		// single char
		if (auto it = kSingleOps.find(c.cur()); it != kSingleOps.end())
		{
			const size_t sr = c.row, sc = c.col;
			char ch = c.cur();
			c.advance();
			push(out, it->second, std::string(1, ch), c, sr, sc);
			return true;
		}
		return false;
	}
}

namespace Tokenizer
{
	std::vector<Token> tokenize(const std::filesystem::path &path)
	{
		std::ifstream f(path, std::ios::binary);
		if (!f)
			throw std::runtime_error("cannot open " + path.string());

		std::string src{std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>()};

		std::vector<Token> tokens;
		Cursor cur{src};

		while (!cur.eof())
		{
			if (skipWhitespaceAndComments(cur))
				continue;
			if (scanStringLit(cur, tokens))
				continue;
			if (scanNumber(cur, tokens))
				continue;
			if (scanIdentifierOrKeyword(cur, tokens))
				continue;
			if (scanInclude(cur, tokens))
				continue;
			if (scanOperator(cur, tokens))
				continue;

			// unknown byte – emit Unknown and move on
			size_t sr = cur.row, sc = cur.col;
			char bad = cur.cur();
			cur.advance();
			push(tokens, Token::Type::Unknown, std::string(1, bad), cur, sr, sc);
		}

		// 3) EOF sentinel
		tokens.push_back(Token{Token::Type::EndOfFile, "", cur.row, cur.col});
		return tokens;
	}

	void printTokens(const std::vector<Token> &tokens)
	{
		std::cout << "Tokenization complete. Number of tokens: " << tokens.size() << std::endl;

		size_t maxTypeSize = 4;
		size_t maxLexemeSize = 6;
		size_t maxRow = 3;
		size_t maxCol = 3;

		for (const auto &token : tokens)
		{
			maxTypeSize = std::max(maxTypeSize, Token::to_string(token.type).size());
			maxLexemeSize = std::max(maxLexemeSize, token.lexeme.size());
			maxRow = std::max(maxRow, std::to_string(token.row).size());
			maxCol = std::max(maxCol, std::to_string(token.col).size());
		}

		std::cout << "| " << std::left
					  << std::setw(maxTypeSize) << "Type" << " | "
					  << std::setw(maxLexemeSize) << "Content" << " | "
					  << std::setw(maxRow) << "Row" << " | "
					  << std::setw(maxCol) << "Col" << " |"
					  << std::endl;
		for (const auto &token : tokens)
		{
			std::cout << "| " << std::left
					  << std::setw(maxTypeSize) << Token::to_string(token.type) << " | "
					  << std::setw(maxLexemeSize) << token.lexeme << " | "
					  << std::setw(maxRow) << token.row << " | "
					  << std::setw(maxCol) << token.col << " |"
					  << std::endl;
		}
	}
}