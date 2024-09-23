#include "lumina_metatokenizer.hpp"

namespace Lumina
{
	MetaTokenizer::Product MetaTokenizer::_analyse(const std::vector<Lumina::Token>& p_tokens)
	{
		_result = Product();
		_tokens = p_tokens;

		while (hasTokenLeft())
		{
			try
			{
				switch (currentToken().type)
				{
				case TokenType::Comment:
				{
					skipToken();
					break;
				}
				case TokenType::Include:
				{
					expendInclude();
					break;
				}
				case TokenType::StructureBlock:
				case TokenType::ConstantBlock:
				case TokenType::AttributeBlock:
				{
					_result.value.push_back(parseBlockMetaToken(currentToken().type));
					break;
				}
				case TokenType::PipelineFlow:
				{
					if (nextToken().type == TokenType::PipelineFlowSeparator)
					{
						_result.value.push_back(parsePipelineFlowMetaToken());
						break;
					}
					else
					{
						_result.value.push_back(parsePipelineBodyMetaToken());
						break;
					}
				}
				case TokenType::Identifier:
				{
					_result.value.push_back(parseFunctionMetaToken());
					break;
				}
				case TokenType::Texture:
				{
					_result.value.push_back(parseTextureMetaToken());
					break;
				}
				case TokenType::Namespace:
				{
					_result.value.push_back(parseNamespaceMetaToken());
					break;
				}
				default:
					throw Lumina::TokenBasedError("Invalid token type [" + Lumina::to_string(currentToken().type) + "].", currentToken());
				}
			}
			catch (const Lumina::TokenBasedError& e)
			{
				_result.errors.push_back(e);
				skipLine();
			}
		}

		return _result;
	}

	MetaTokenizer::Product MetaTokenizer::analyse(const std::vector<Lumina::Token>& p_tokens)
	{
		return MetaTokenizer()._analyse(p_tokens);
	}
}