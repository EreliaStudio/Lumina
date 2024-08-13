#include "lumina_lexer.hpp"

namespace Lumina
{
	bool LexerChecker::describeSymbolCallName()
	{
		size_t offset = 0;

		if (tokenAtIndex(offset).type == Lumina::Token::Type::NamespaceSeparator)
			offset++;

		if (tokenAtIndex(offset).type == Lumina::Token::Type::Identifier)
		{
			offset++;

			while (tokenAtIndex(offset).type == Lumina::Token::Type::NamespaceSeparator)
			{
				offset++;
				if (tokenAtIndex(offset).type != Lumina::Token::Type::Identifier)
				{
					return false;
				}
				offset++;
			}

			if (tokenAtIndex(offset).type == Lumina::Token::Type::OpenParenthesis)
			{
				return true;
			}
		}

		return false;
	}

	bool LexerChecker::describeVariableDeclarationInstruction()
	{
		size_t offset = 0;

		if (tokenAtIndex(offset).type == Lumina::Token::Type::NamespaceSeparator)
			offset++;

		if (tokenAtIndex(offset).type == Lumina::Token::Type::Identifier)
		{
			offset++;

			while (tokenAtIndex(offset).type == Lumina::Token::Type::NamespaceSeparator)
			{
				offset++;
				if (tokenAtIndex(offset).type != Lumina::Token::Type::Identifier)
				{
					return false;
				}
				offset++;
			}

			if (tokenAtIndex(offset).type == Lumina::Token::Type::Identifier)
			{
				offset++;

				if (tokenAtIndex(offset).type == Lumina::Token::Type::Assignator)
				{
					return true;
				}

				return true;
			}
		}

		return false;
	}

	bool LexerChecker::describeSymbolCallInstruction()
	{
		size_t offset = 0;

		if (tokenAtIndex(offset).type == Lumina::Token::Type::NamespaceSeparator)
			offset++;

		if (tokenAtIndex(offset).type == Lumina::Token::Type::Identifier)
		{
			offset++;

			while (tokenAtIndex(offset).type == Lumina::Token::Type::NamespaceSeparator)
			{
				offset++;
				if (tokenAtIndex(offset).type != Lumina::Token::Type::Identifier)
				{
					return false;
				}
				offset++;
			}

			if (tokenAtIndex(offset).type == Lumina::Token::Type::OpenParenthesis)
			{
				return true;
			}
		}

		return false;
	}


	bool LexerChecker::describeVariableAssignationInstruction()
	{
		size_t offset = 0;

		if (tokenAtIndex(offset).type == Lumina::Token::Type::Identifier)
		{
			offset++;

			while (tokenAtIndex(offset).type == Lumina::Token::Type::NamespaceSeparator ||
				tokenAtIndex(offset).type == Lumina::Token::Type::Accessor)
			{
				offset++;

				if (tokenAtIndex(offset).type != Lumina::Token::Type::Identifier)
				{
					return false;
				}

				offset++;
			}

			if (tokenAtIndex(offset).type == Lumina::Token::Type::Assignator)
			{
				return true;
			}
		}

		return false;
	}
}