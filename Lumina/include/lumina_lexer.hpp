#pragma once

#include <fstream>
#include <vector>
#include <memory>
#include <filesystem>
#include "lumina_utils.hpp"
#include "lumina_token.hpp"
#include "lumina_tokenizer.hpp"
#include "lumina_exception.hpp"
#include "lumina_instructions.hpp"

namespace Lumina
{
	class LexerChecker
	{
	public:
		struct Result
		{
			std::vector<std::shared_ptr<Lumina::Instruction>> instructions;
			std::vector<Lumina::TokenBasedError> errors;
		};

		LexerChecker() = default;

		static Result checkSyntax(const std::filesystem::path& p_file, const std::vector<Lumina::Token>& p_tokens);

	private:
		std::filesystem::path _file;
		Result _result;
		Lumina::Token noToken;
		const std::vector<Lumina::Token>* _tokens;
		size_t _index = 0;

		bool hasTokenLeft() const;
		void backOff();
		void advance();
		const Lumina::Token& currentToken() const;
		const Lumina::Token& tokenAtIndex(size_t p_offset) const;
		const Lumina::Token& nextToken() const;
		void skipToken();
		void skipLine();
		const Lumina::Token& expect(Lumina::Token::Type p_expectedType, const std::string& p_errorMessage = "Unexpected token type.");
		const Lumina::Token& expect(std::vector<Lumina::Token::Type> p_expectedTypes, const std::string& p_errorMessage = "Unexpected token type.");

		bool describeSymbolCallName();
		bool describeVariableDeclarationInstruction();
		bool describeSymbolCallInstruction();
		bool describeVariableAssignationInstruction();

		std::shared_ptr<Lumina::IncludeInstruction> parseIncludeInstruction();
		std::shared_ptr<Lumina::TypeInstruction> parseTypeInstruction();
		std::shared_ptr<Lumina::PipelineFlowInstruction> parsePipelineFlowInstruction();
		std::shared_ptr<Lumina::BlockElementInstruction> parseBlockElementInstruction();
		std::shared_ptr<Lumina::StructureBlockInstruction> parseStructureBlockInstruction();
		std::shared_ptr<Lumina::AttributeBlockInstruction> parseAttributeBlockInstruction();
		std::shared_ptr<Lumina::ConstantBlockInstruction> parseConstantBlockInstruction();
		std::shared_ptr<Lumina::TextureInstruction> parseTextureInstruction();
		std::shared_ptr<Lumina::SymbolParameterInstruction> parseSymbolParameterInstruction();
		std::shared_ptr<Lumina::ComparatorOperatorExpressionInstruction> parseComparatorOperatorExpressionInstruction();
		std::shared_ptr<Lumina::OperatorExpressionInstruction> parseOperatorExpressionInstruction();
		std::shared_ptr<Lumina::BoolExpressionValueInstruction> parseBoolExpressionValueInstruction();
		std::shared_ptr<Lumina::NumberExpressionValueInstruction> parseNumberExpressionValueInstruction();
		std::shared_ptr<Lumina::StringLiteralsExpressionValueInstruction> parseStringLiteralsExpressionValueInstruction();
		std::shared_ptr<Lumina::VariableExpressionValueInstruction> parseVariableExpressionValueInstruction();
		std::shared_ptr<Lumina::SymbolNameInstruction> parseSymbolCallNameInstruction();
		std::shared_ptr<Lumina::SymbolCallInstruction> parseSymbolCallInstruction();
		std::shared_ptr<Lumina::ExpressionInstruction> parseExpression();
		std::shared_ptr<Lumina::VariableDeclarationInstruction> parseVariableDeclarationInstruction();
		std::shared_ptr<Lumina::ReturnInstruction> parseReturnInstruction();
		std::shared_ptr<Lumina::DiscardInstruction> parseDiscardInstruction();
		std::shared_ptr<Lumina::VariableDesignationInstruction> parseVariableDesignationInstruction();
		std::shared_ptr<Lumina::VariableAssignationInstruction> parseVariableAssignationInstruction();
		std::shared_ptr<Lumina::ConditionElementInstruction> parseConditionElementInstruction();
		std::shared_ptr<Lumina::ConditionInstruction> parseConditionInstruction();
		std::shared_ptr<Lumina::IfStatementInstruction> parseIfStatementInstruction();
		std::shared_ptr<Lumina::WhileLoopInstruction> parseWhileLoopInstruction();
		std::shared_ptr<Lumina::ForLoopInstruction> parseForLoopInstruction();
		std::shared_ptr<Lumina::SymbolBodyInstruction> parseSymbolBodyInstruction();
		std::shared_ptr<Lumina::SymbolInstruction> parseSymbolInstruction();
		std::shared_ptr<Lumina::NamespaceInstruction> parseNamespaceInstruction();
		std::shared_ptr<Lumina::PipelineBodyInstruction> parsePipelineBodyInstruction();

		Result parse(const std::filesystem::path& p_file, const std::vector<Lumina::Token>& p_tokens);
	};
}