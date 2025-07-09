#pragma once

#include <string>

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
			case Type::KwStruct         : return "struct";
			case Type::KwNamespace      : return "namespace";
			case Type::KwAttributeBlock : return "AttributeBlock";
			case Type::KwConstantBlock  : return "ConstantBlock";
			case Type::KwTexture        : return "Texture";
			case Type::KwInput          : return "Input";
			case Type::KwVertexPass     : return "VertexPass";
			case Type::KwFragmentPass   : return "FragmentPass";
			case Type::KwOutput         : return "Output";
			case Type::KwRaiseException : return "raiseException";
			case Type::KwDiscard        : return "discard";
			case Type::KwIf             : return "if";
			case Type::KwElse           : return "else";
			case Type::KwWhile          : return "while";
			case Type::KwDo             : return "do";
			case Type::KwReturn         : return "return";
			case Type::KwInclude        : return "#include";

			// — operators & punctuation —
			case Type::Plus             : return "+";
			case Type::Minus            : return "-";
			case Type::Star             : return "*";
			case Type::Slash            : return "/";
			case Type::Percent          : return "%";
			case Type::PlusEqual        : return "+=";
			case Type::MinusEqual       : return "-=";
			case Type::StarEqual        : return "*=";
			case Type::SlashEqual       : return "/=";
			case Type::PercentEqual     : return "%=";
			case Type::Increment        : return "++";
			case Type::Decrement        : return "--";
			case Type::Equal            : return "=";
			case Type::EqualEqual       : return "==";
			case Type::NotEqual         : return "!=";
			case Type::Less             : return "<";
			case Type::Greater          : return ">";
			case Type::LessEqual        : return "<=";
			case Type::GreaterEqual     : return ">=";
			case Type::LogicalAnd       : return "&&";
			case Type::LogicalOr        : return "||";
			case Type::LogicalNot       : return "!";
			case Type::Arrow            : return "->";
			case Type::Colon            : return ":";
			case Type::DoubleColon      : return "::";
			case Type::Comma            : return ",";
			case Type::Semicolon        : return ";";
			case Type::Dot              : return ".";
			case Type::LeftParen        : return "(";
			case Type::RightParen       : return ")";
			case Type::LeftBrace        : return "{";
			case Type::RightBrace       : return "}";
			case Type::LeftBracket      : return "[";
			case Type::RightBracket     : return "]";
			case Type::Hash             : return "#";

			default                     : return "Unknown";
		}
	}
};
