#pragma once

#include "lumina_tokenizer.hpp"
#include "lumina_exception.hpp"
#include "lumina_utils.hpp"
#include "lumina_metatokenizer.hpp"

namespace Lumina
{
struct Shader
{
	std::string layouts;
	std::string constants;
	std::string attributes;
	std::string textures;
	std::string vertexShaderCode;
	std::string fragmentShaderCode;
};

struct Compiler
{
private:
	Expected<Shader> _result;
	std::vector<std::shared_ptr<MetaToken>> _metaTokens;
	size_t _index;

	Expected<Shader> _compile(const std::vector<std::shared_ptr<MetaToken>>& p_metaTokens)
	{
		_result = Expected<Shader>();

		_metaTokens = p_metaTokens;
		_index = 0;

		while (hasMetaTokenLeft() == true)
		{
			switch (currentMetaToken()->type)
			{

			}
			_index++;
		}

		return (_result);
	}

	const std::shared_ptr<MetaToken> currentMetaToken() const
	{
		return (_metaTokens[_index]);
	}

	bool hasMetaTokenLeft() const
	{
		return (_index < _metaTokens.size());
	}

public:
	static Expected<Shader> compile(const std::vector<std::shared_ptr<MetaToken>>& p_metaTokens)
	{
		return (Compiler()._compile(p_metaTokens));
	}
};
}

int main(int argc, char** argv)
{
	if (argc != 3)
	{
		std::cout << "Usage : " << argv[0] << " [path to your lumina shader code] [path to your compiled shader file]" << std::endl;
		return (0);
	}

	std::vector<Lumina::Token> tokens = Lumina::Tokenizer::tokenize(argv[1]);

	Lumina::MetaTokenizer::Product metaTokens = Lumina::MetaTokenizer::analyse(tokens);

	if (metaTokens.errors.size() != 0)
	{
		for (const auto& error : metaTokens.errors)
		{
			std::cout << error.what() << std::endl;
		}
		return (-1);
	}

	return (0);
}
