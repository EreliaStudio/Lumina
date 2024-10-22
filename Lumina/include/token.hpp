#pragma once

#include <filesystem>
#include <string>
#include <iostream>

namespace Lumina
{
	struct Token
	{
		struct Context
		{
			int line = 0;
			int column = 0;
			std::filesystem::path originFile = "";
			std::string inputLine = "";
		};

		enum class Type
		{
			Unknow,
			Include, // "#include"
			StringLitteral, // String contained inside a '"' on both sides
			IncludeLitteral, // String contained between '<' and '>'
			PipelineFlow, // "Input", "VertexPass", or "FragmentPass"
			PipelineFlowSeparator, // '->'
			NamespaceSeparator, // "::"
			Separator, // ':'
			Identifier, // Alphanumeric string
			Number, // Numeric value
			StructureBlock, // "struct"
			ThisKeyword, // "self", "this"
			AttributeBlock, // "AttributeBlock"
			ConstantBlock, // "ConstantBlock"
			Texture, // "Texture"
			Namespace, // "namespace"
			OpenCurlyBracket, // '{'
			CloseCurlyBracket, // '}'
			OpenParenthesis, // '('
			CloseParenthesis, // ')'
			Accessor, // '.'
			Comment, // Comments: "//" until end of line or "/* */"
			OperatorKeyword, // "operator"
			Operator, // Operators: +, -, *, /, etc.
			Return, // "return"
			Discard, // "discard"
			BoolStatement, // "true" or "false"
			ForStatement, // "for"
			IfStatement, // "if"
			WhileStatement, // "while"
			ElseStatement, // "else"
			EndOfSentence, // ';'
			Assignator, // '=', "+=", "-=" etc
			Incrementor, // '++', '--'
			Comma, // ','
			OpenBracket, // '['
			CloseBracket, // ']'
			Expression
		};

		Type type = Type::Unknow;
		std::string content = "";
		Context context;

		Token() = default;

		Token(const std::string& p_content, Type p_type, int p_line, int p_column, const std::filesystem::path& p_originFile, const std::string& p_inputLine);
		Token(Type p_type);
		Token(const std::string& p_content, Type p_type, const Context& p_context);

		bool operator == (const std::string& p_string) const;
		bool operator != (const std::string& p_string) const;

		friend std::ostream& operator << (std::ostream& p_os, const Token::Type& p_type);
		friend std::ostream& operator << (std::ostream& p_os, const Token& p_token);

		static std::string to_string(Token::Type p_type);

		Lumina::Token operator + (const Lumina::Token& p_toAdd) const;
		Lumina::Token operator += (const Lumina::Token& p_toAdd);
		void append(const Lumina::Token& p_toAdd);
		static Token merge(const std::vector<Token>& p_tokens, const Token::Type& p_type);
	};
}

namespace Lumina
{
	class TokenBasedError : public std::exception
	{
	private:
		std::string _what;

	public:
		TokenBasedError(const std::string& p_message, const Token& p_token);

		virtual const char* what() const noexcept override;
	};
}
