#include "lumina_lexer.hpp"

namespace Lumina
{
	std::shared_ptr<NamespaceInstruction> LexerChecker::parseNamespaceInstruction()
	{
		std::shared_ptr<NamespaceInstruction> result = std::make_shared<NamespaceInstruction>();

		expect(Lumina::Token::Type::Namespace, "Expected a namespace token."+ DEBUG_INFORMATION);
		result->name = expect(Lumina::Token::Type::Identifier, "Expected an identifier name");
		expect(Lumina::Token::Type::OpenCurlyBracket, "Expected an open curly bracket."+ DEBUG_INFORMATION);
		while (currentToken().type != Lumina::Token::Type::CloseCurlyBracket)
		{
			try
			{
				switch (currentToken().type)
				{
				case Lumina::Token::Type::Comment:
					skipToken();
					break;
				case Lumina::Token::Type::StructureBlock:
					result->instructions.push_back(parseStructureBlockInstruction());
					break;
				case Lumina::Token::Type::AttributeBlock:
					result->instructions.push_back(parseAttributeBlockInstruction());
					break;
				case Lumina::Token::Type::ConstantBlock:
					result->instructions.push_back(parseConstantBlockInstruction());
					break;
				case Lumina::Token::Type::Texture:
					result->instructions.push_back(parseTextureInstruction());
					break;
				case Lumina::Token::Type::Namespace:
					result->instructions.push_back(parseNamespaceInstruction());
					break;
				case Lumina::Token::Type::Identifier:
					result->instructions.push_back(parseSymbolInstruction());
					break;
				default:
					throw Lumina::TokenBasedError(_file, "Unexpected token type : " + to_string(currentToken().type)+ DEBUG_INFORMATION, currentToken());
				}
			}
			catch (const Lumina::TokenBasedError& e)
			{
				_result.errors.push_back(e);
				skipLine();
			}
		}
		expect(Lumina::Token::Type::CloseCurlyBracket, "Expected a close curly bracket."+ DEBUG_INFORMATION);

		return result;
	}

	LexerChecker::Result LexerChecker::parse(const std::filesystem::path& p_file, const std::vector<Lumina::Token>& p_tokens)
	{
		_result = Result();
		_file = p_file;
		_tokens = &p_tokens;
		_index = 0;

		while (hasTokenLeft() == true)
		{
			try
			{
				switch (currentToken().type)
				{
				case Lumina::Token::Type::Comment:
				{
					skipToken();
					break;
				}
				case Lumina::Token::Type::Include:
				{
					_result.instructions.push_back(parseIncludeInstruction());
					break;
				}
				case Lumina::Token::Type::PipelineFlow:
				{
					if (nextToken().type == Lumina::Token::Type::OpenParenthesis)
					{
						_result.instructions.push_back(parsePipelineBodyInstruction());
					}
					else
					{
						_result.instructions.push_back(parsePipelineFlowInstruction());
					}
					break;
				}
				case Lumina::Token::Type::StructureBlock:
				{
					_result.instructions.push_back(parseStructureBlockInstruction());
					break;
				}
				case Lumina::Token::Type::AttributeBlock:
				{
					_result.instructions.push_back(parseAttributeBlockInstruction());
					break;
				}
				case Lumina::Token::Type::ConstantBlock:
				{
					_result.instructions.push_back(parseConstantBlockInstruction());
					break;
				}
				case Lumina::Token::Type::Texture:
				{
					_result.instructions.push_back(parseTextureInstruction());
					break;
				}
				case Lumina::Token::Type::Namespace:
				{
					_result.instructions.push_back(parseNamespaceInstruction());
					break;
				}
				case Lumina::Token::Type::Identifier:
				{
					_result.instructions.push_back(parseSymbolInstruction());
					break;
				}
				default:
				{
					throw Lumina::TokenBasedError(_file, "Unexpected token type : " + to_string(currentToken().type), currentToken());
					break;
				}
				}
			}
			catch (const Lumina::TokenBasedError& e)
			{
				_result.errors.push_back(e);

				skipLine();
			}
		}

		return (_result);
	}

	LexerChecker::Result LexerChecker::checkSyntax(const std::filesystem::path& p_file, const std::vector<Lumina::Token>& p_tokens)
	{
		return (LexerChecker().parse(p_file, p_tokens));
	}
}