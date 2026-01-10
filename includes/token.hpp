#pragma once

#include <cstddef>
#include <filesystem>
#include <string>
#include <string_view>

struct Token
{
	struct Location
	{
		size_t line;
		size_t column;
	};

	enum class Type
	{
		EndOfFile,
		Identifier,
		IntegerLiteral,
		FloatLiteral,
		StringLiteral,
		HeaderLiteral,

		// Directives / punctuation
		Hash,
		Colon,
		DoubleColon,
		Semicolon,
		Comma,
		Dot,

		LeftParen,
		RightParen,
		LeftBrace,
		RightBrace,
		LeftBracket,
		RightBracket,

		Less,
		LessEqual,
		Greater,
		GreaterEqual,
		ShiftLeft,
		ShiftRight,
		Arrow, // ->

		Assign,
		Equal,

		Plus,
		PlusEqual,
		PlusPlus,
		Minus,
		MinusEqual,
		MinusMinus,
		Star,
		StarEqual,
		Slash,
		SlashEqual,
		Percent,
		PercentEqual,

		Bang,
		BangEqual,
		Ampersand,
		AmpersandAmpersand,
		AmpersandEqual,
		Pipe,
		PipePipe,
		PipeEqual,
		Caret,
		CaretEqual,
		ShiftLeftEqual,
		ShiftRightEqual,
		Tilde,
		Question,

		// Keywords
		KeywordInclude,
		KeywordStruct,
		KeywordNamespace,
		KeywordAttributeBlock,
		KeywordConstantBlock,
		KeywordDataBlock,
		KeywordTexture,
		KeywordAs,
		KeywordConstant,
		KeywordAttribute,
		KeywordDefine,
		KeywordReturn,
		KeywordIf,
		KeywordElse,
		KeywordFor,
		KeywordWhile,
		KeywordDo,
		KeywordBreak,
		KeywordContinue,
		KeywordConst,
		KeywordDiscard,
		KeywordThis,
		KeywordInput,
		KeywordOutput,
		KeywordVertexPass,
		KeywordFragmentPass,
		KeywordTrue,
		KeywordFalse
	};

	std::filesystem::path origin;
	Type type;
	std::string content;
	Location start;
	Location end;
};

void emitError(const std::string &p_message, const Token &p_token);

void resetErrorCount();
int getErrorCount();

std::string_view tokenTypeToString(Token::Type p_type);
