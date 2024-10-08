#pragma once

#include "expected.hpp"
#include "token.hpp"
#include "shader_info.hpp"

namespace Lumina
{
	struct Lexer
	{
	public:
		using Output = ShaderInfo;
		using Product = Lumina::Expected<Output>;

	private:
		std::vector<Lumina::Token> _tokens;
		size_t _index = 0;
		Product _product;
		static const Lumina::Token _emptyToken;

		Lexer() = default;

		TypeInfo parseTypeInfo();
		NameInfo parseNameInfo();
		ArraySizeInfo parseArraySizeInfo();
		VariableInfo parseVariableInfo();
		TextureInfo parseTextureInfo();
		SymbolBodyInfo parseSymbolBody();
		ParameterInfo parseParameterInfo();
		FunctionInfo parseFunctionInfo();
		OperatorInfo parseOperatorInfo();
		BlockInfo parseBlockInfo();
		PipelinePassInfo parsePipelinePassInfo();
		PipelineFlowInfo parsePipelineFlowInfo();
		NamespaceInfo parseNamespaceInfo();
		ShaderInfo parseShaderInfo();

		bool describeFunction();
		bool describeVariableInfo();

		Product _lex(const std::vector<Lumina::Token>& p_tokens);

		const Lumina::Token& currentToken() const;
		void advance();
		const Lumina::Token& expect(Lumina::Token::Type p_expectedType, const std::string& p_errorMessage);
		const Lumina::Token& expect(const std::vector<Lumina::Token::Type>& p_expectedTypes, const std::string& p_errorMessage);
		void skipToken();
		void skipLine();
		bool hasTokenLeft(size_t p_offset = 1) const;
		const Lumina::Token& peekNext() const;
		const Lumina::Token& tokenAtOffset(size_t p_offset) const;
		void moveBack(size_t p_steps = 1);
	public:
		static Product lex(const std::vector<Lumina::Token>& p_tokens);
	};
}
