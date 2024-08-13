#pragma once

#include <string>
#include <iostream>
#include <iomanip>
#include <vector>

namespace Lumina
{
	struct Token
	{
		struct Context
		{
			int line = 0;
			int column = 0;
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
			Operator, // Operators: +, -, *, /, etc.
			ComparatorOperator, // Operators: <, >, >=, <=, == and !=.
			Return, // "return"
			Discard, // "discard"
			BoolStatement, // "true" or "false"
			ConditionOperator, // Operator || and &&
			IfStatement, // "if"
			WhileStatement, // "while"
			ForStatement, // "for"
			ElseStatement, // "else"
			EndOfSentence, // ';'
			Assignator, // '=', "+=", "-=" etc
			Incrementor, // '++', '--'
			Comma, // ','
			OpenBracket, // '['
			CloseBracket // ']'
		};

		Type type = Type::Unknow;
		std::string content = "";
		Context context;

		Token() = default;

		Token(const std::string& p_content, Type p_type, int p_line, int p_column, const std::string& p_inputLine);
		Token(const std::string& p_content, Type p_type, const Context& p_context);

		friend std::ostream& operator << (std::ostream& p_os, const Token::Type& p_type);
		friend std::ostream& operator << (std::ostream& p_os, const Token& p_token);

		static Token merge(const std::vector<Token>& p_tokens, const Token::Type& p_type);
	};
	
	std::string to_string(Token::Type p_type);
}
