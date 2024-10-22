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

		// Parsing methods for statements
		SymbolBodyInfo parseSymbolBody();
		Statement parseStatement();
		VariableDeclarationStatement parseVariableDeclarationStatement();
		ExpressionStatement parseExpressionStatement();
		AssignmentStatement parseAssignmentStatement();
		ReturnStatement parseReturnStatement();
		DiscardStatement parseDiscardStatement();
		IfStatement parseIfStatement();
		WhileStatement parseWhileStatement();
		ForStatement parseForStatement();
		CompoundStatement parseCompoundStatement();

		// Parsing methods for expressions
		std::shared_ptr<Expression> parseExpression();
		std::shared_ptr<Expression> parseAssignmentExpression();
		std::shared_ptr<Expression> parseBinaryExpression(int minPrecedence);
		std::shared_ptr<Expression> parseUnaryExpression();
		std::shared_ptr<Expression> parsePostfixExpression();
		std::shared_ptr<Expression> parsePrimaryExpression();
		std::shared_ptr<Expression> parseVariableOrFunctionCallExpression();

		// Helper methods
		bool isVariableDeclaration();
		bool isAssignmentStatement();
		int computeOperatorPriority(const Token& token);

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
	};
}
