#pragma once

#include "lumina_metatokens.hpp"
#include "lumina_descriptors.hpp"
#include "lumina_exception.hpp"
#include "lumina_instruction.hpp"

namespace Lumina
{
	class MetaTokenizer
	{
	public:
		using TokenType = Lumina::Token::Type;
		using Product = Lumina::Expected<std::vector<std::shared_ptr<MetaToken>>>;

	private:
		Product _result;
		std::vector<Lumina::Token> _tokens;
		size_t _index = 0;
		Lumina::Token noToken;

		MetaTokenizer();

		void expendInclude();
		TypeDescriptor parseTypeDescriptor();
		size_t parseArraySize();
		VariableDescriptor parseVariableDescriptor();
		Condition parseCondition();

		std::shared_ptr<PipelineFlowMetaToken> parsePipelineFlowMetaToken();
		std::shared_ptr<BlockMetaToken> parseBlockMetaToken(const TokenType& p_tokenType);
		std::shared_ptr<TextureMetaToken> parseTextureMetaToken();
		ReturnTypeDescriptor parseReturnTypeDescriptor();
		std::shared_ptr<FunctionMetaToken> parseFunctionMetaToken();
		std::shared_ptr<PipelineBodyMetaToken> parsePipelineBodyMetaToken();
		std::shared_ptr<NamespaceMetaToken> parseNamespaceMetaToken();

		std::shared_ptr<VariableDeclaration> parseVariableDeclaration();
		std::shared_ptr<Expression::NumberElement> parseNumberElement();
		std::shared_ptr<Expression::BooleanElement> parseBooleanElement();
		std::shared_ptr<Expression::OperatorElement> parseOperatorElement();
		std::shared_ptr<Expression::ComparatorOperatorElement> parseComparatorOperatorElement();
		std::shared_ptr<Expression::ConditionOperatorElement> parseConditionOperatorElement();
		std::shared_ptr<Expression::SymbolCallElement> parseSymbolCallElement();
		std::shared_ptr<Expression::IncrementorElement> parseIncrementor();
		std::shared_ptr<Expression::VariableDesignationElement> parseVariableDesignation();
		std::shared_ptr<VariableAssignation> parseVariableAssignation();
		std::shared_ptr<Instruction> parseExpressionElement();
		std::shared_ptr<Expression> parseExpression();
		std::shared_ptr<SymbolCall> parseSymbolCall();
		std::shared_ptr<ConditionalOperator> parseConditionalOperator();
		std::shared_ptr<IfStatement> parseIfStatement();
		std::shared_ptr<WhileStatement> parseWhileStatement();
		std::shared_ptr<ForStatement> parseForStatement();
		std::shared_ptr<ReturnStatement> parseReturnStatement();
		std::shared_ptr<DiscardStatement> parseDiscardStatement();
		bool isDeclaration() const;
		bool isAssignation() const;
		bool isSymbolCall() const;
		SymbolBody parseSymbolBody();

		Lumina::Token composeToken(size_t p_startingIndex, size_t p_endIndex, Lumina::Token::Type p_type);
		Product _analyse(const std::vector<Lumina::Token>& p_tokens);
		bool hasTokenLeft() const;
		void backOff();
		void advance();
		const Lumina::Token& currentToken() const;
		const Lumina::Token& tokenAtIndex(size_t p_index) const;
		const Lumina::Token& nextToken() const;
		void skipToken();
		void skipLine();
		void skipUntilReach(const TokenType& p_type);
		void skipUntilReach(const std::vector<TokenType>& p_types);
		const Lumina::Token& expect(Lumina::Token::Type p_expectedType, const std::string& p_errorMessage);
		const Lumina::Token& expect(std::vector<Lumina::Token::Type> p_expectedTypes, const std::string& p_errorMessage);

	public:
		static Product analyse(const std::vector<Lumina::Token>& p_tokens);
	};
}
