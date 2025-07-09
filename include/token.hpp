#pragma once

#include <string>
#include <vector>

struct Token
{
	enum class Type
	{
		EndOfFile,
		Identifier,
		IntLiteral,
		FloatLiteral,
		BoolLiteral,
		StringLiteral,

		KwStruct,
		KwNamespace,
		KwAttributeBlock,
		KwConstantBlock,
		KwTexture,
		KwInput,
		KwVertexPass,
		KwFragmentPass,
		KwOutput,
		KwRaiseException,
		KwDiscard,
		KwIf,
		KwElse,
		KwWhile,
		KwDo,
		KwReturn,
		KwInclude,

		Plus,
		Minus,
		Star,
		Slash,
		Percent,
		PlusEqual,
		MinusEqual,
		StarEqual,
		SlashEqual,
		PercentEqual,
		Increment,
		Decrement,
		Equal,
		EqualEqual,
		NotEqual,
		Less,
		Greater,
		LessEqual,
		GreaterEqual,
		LogicalAnd,
		LogicalOr,
		LogicalNot,
		Arrow,
		Colon,
		DoubleColon,
		Comma,
		Semicolon,
		Dot,
		LeftParen,
		RightParen,
		LeftBrace,
		RightBrace,
		LeftBracket,
		RightBracket,
		Hash,
		
		Unknown
	};

	Type type;
	std::string lexeme;
	size_t row;
	size_t col;

	static std::string to_string(Type type)
	{
		switch (type)
		{
			// — literals & identifiers —
			case Type::EndOfFile        : return "EndOfFile";
			case Type::Identifier       : return "Identifier";
			case Type::IntLiteral       : return "IntLiteral";
			case Type::FloatLiteral     : return "FloatLiteral";
			case Type::BoolLiteral      : return "BoolLiteral";
			case Type::StringLiteral    : return "StringLiteral";

			// — keywords —
			case Type::KwStruct         : return "Struct";
			case Type::KwNamespace      : return "Namespace";
			case Type::KwAttributeBlock : return "AttributeBlock";
			case Type::KwConstantBlock  : return "ConstantBlock";
			case Type::KwTexture        : return "Texture";
			case Type::KwInput          : return "Input";
			case Type::KwVertexPass     : return "VertexPass";
			case Type::KwFragmentPass   : return "FragmentPass";
			case Type::KwOutput         : return "Output";
			case Type::KwRaiseException : return "RaiseException";
			case Type::KwDiscard        : return "Discard";
			case Type::KwIf             : return "If";
			case Type::KwElse           : return "Else";
			case Type::KwWhile          : return "While";
			case Type::KwDo             : return "Do";
			case Type::KwReturn         : return "Return";
			case Type::KwInclude        : return "Include";

			// — operators & punctuation —
			case Type::Plus             : return "Plus";
			case Type::Minus            : return "Minus";
			case Type::Star             : return "Star";
			case Type::Slash            : return "Slash";
			case Type::Percent          : return "Percent";
			case Type::PlusEqual        : return "PlusEqual";
			case Type::MinusEqual       : return "MinusEqual";
			case Type::StarEqual        : return "StarEqual";
			case Type::SlashEqual       : return "SlashEqual";
			case Type::PercentEqual     : return "PercentEqual";
			case Type::Increment        : return "Increment";
			case Type::Decrement        : return "Decrement";
			case Type::Equal            : return "Equal";
			case Type::EqualEqual       : return "EqualEqual";
			case Type::NotEqual         : return "NotEqual";
			case Type::Less             : return "Less";
			case Type::Greater          : return "Greater";
			case Type::LessEqual        : return "LessEqual";
			case Type::GreaterEqual     : return "GreaterEqual";
			case Type::LogicalAnd       : return "LogicalAnd";
			case Type::LogicalOr        : return "LogicalOr";
			case Type::LogicalNot       : return "LogicalNot";
			case Type::Arrow            : return "Arrow";
			case Type::Colon            : return "Colon";
			case Type::DoubleColon      : return "DoubleColon";
			case Type::Comma            : return "Comma";
			case Type::Semicolon        : return "Semicolon";
			case Type::Dot              : return "Dot";
			case Type::LeftParen        : return "LeftParen";
			case Type::RightParen       : return "RightParen";
			case Type::LeftBrace        : return "LeftBrace";
			case Type::RightBrace       : return "RightBrace";
			case Type::LeftBracket      : return "LeftBracket";
			case Type::RightBracket     : return "RightBracket";
			case Type::Hash             : return "Hash";

			default                     : return "Unknown";
		}
	}
};

using TokenList = std::vector<Token>;