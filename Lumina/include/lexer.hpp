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
		Lumina::Token _emptyToken;

		Lexer() = default;

		void parseInclude();

		NamespaceDesignation parseNamespaceDesignation();
		TypeInfo parseTypeInfo();
		NameInfo parseNameInfo();
		ArraySizeInfo parseArraySizeInfo();
		VariableInfo parseVariableInfo();
		TextureInfo parseTextureInfo();
		ParameterInfo parseParameterInfo();
		FunctionInfo parseFunctionInfo();
		ConstructorInfo parseConstructorInfo();
		OperatorInfo parseOperatorInfo();
		BlockInfo parseBlockInfo();
		PipelinePassInfo parsePipelinePassInfo();
		PipelineFlowInfo parsePipelineFlowInfo();
		NamespaceInfo parseNamespaceInfo();

		SymbolBodyInfo parseSymbolBodyInfo();
		StatementInfo parseStatementInfo();
		VariableDeclarationStatementInfo parseVariableDeclarationStatementInfo();
		ExpressionStatementInfo parseExpressionStatementInfo();
		AssignmentStatementInfo parseAssignmentStatementInfo();
		ReturnStatementInfo parseReturnStatementInfo();
		DiscardStatementInfo parseDiscardStatementInfo();
		IfStatementInfo parseIfStatementInfo();
		WhileStatementInfo parseWhileStatementInfo();
		ForStatementInfo parseForStatementInfo();
		CompoundStatementInfo parseCompoundStatementInfo();

		std::shared_ptr<ExpressionInfo> parseExpressionInfo();
		std::shared_ptr<ExpressionInfo> parseAssignmentExpressionInfo();
		std::shared_ptr<ExpressionInfo> parseBinaryExpressionInfo(int p_minPrecedence);
		std::shared_ptr<ExpressionInfo> parseUnaryExpressionInfo();
		std::shared_ptr<ExpressionInfo> parsePostfixExpressionInfo();
		std::shared_ptr<ExpressionInfo> parsePrimaryExpressionInfo();
		std::shared_ptr<ExpressionInfo> parseVariableOrFunctionCallExpressionInfo();

		bool isVariableDeclaration();
		bool isAssignmentStatement();
		int computeOperatorPriority(const Token& p_token);

		ShaderInfo parseShaderInfo();

		bool describeConstructor();
		bool describeFunction();
		bool describeOperator();
		bool describeVariableInfo();

		Product _lex(const std::vector<Lumina::Token>& p_tokens);

		const Lumina::Token& currentToken() const;
		void advance();
		const Lumina::Token& expect(Lumina::Token::Type p_expectedType, const std::string& p_errorMessage);
		const Lumina::Token& expect(const std::vector<Lumina::Token::Type>& p_expectedTypes, const std::string& p_errorMessage);
		void skipToken();
		void skipLine();
		bool hasTokenLeft(size_t p_offset = 0) const;
		const Lumina::Token& peekNext() const;
		const Lumina::Token& tokenAtOffset(size_t p_offset) const;
		void moveBack(size_t p_steps = 1);
	public:
		static Product lex(const std::vector<Lumina::Token>& p_tokens);
		static ConstructorInfo lexConstructorSourceCode(const std::string& p_sourceCode);
		static FunctionInfo lexFunctionSourceCode(const std::string& p_sourceCode);
		static OperatorInfo lexOperatorSourceCode(const std::string& p_sourceCode);
	};
}
