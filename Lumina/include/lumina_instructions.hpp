#pragma once

#include <string>
#include <vector>
#include "lumina_token.hpp"

namespace Lumina
{
	struct AbstractInstruction
	{
		enum class Type
		{
			Include,
			PipelineFlow,
			StructureBlock,
			AttributeBlock,
			ConstantBlock,
			Texture,
			Namespace,
			SymbolParameter,
			OperatorExpression,
			ComparatorOperatorExpression,
			BoolExpressionValue,
			NumberExpressionValue,
			StringLiteralsExpressionValue,
			VariableExpressionValue,
			Expression,
			VariableDesignation,
			VariableAssignation,
			VariableDeclaration,
			SymbolName,
			SymbolBody,
			SymbolCall,
			ResultAccessor,
			Return,
			Discard,
			ConditionElement,
			Condition,
			Else,
			IfStatement,
			WhileLoop,
			ForLoop,
			Symbol,
			PipelineBody
		};

		Type type;

		AbstractInstruction(const Type& p_type) :
			type(p_type)
		{

		}
		virtual ~AbstractInstruction()
		{

		}
		virtual Token mergedToken() const = 0;
	};

	using Instruction = AbstractInstruction;

	struct IdentifierInstruction
	{
		Lumina::Token token;

		Token mergedToken() const
		{
			return (token);
		}
	};

	struct TypeInstruction
	{
		std::vector<Lumina::Token> tokens;

		Token mergedToken() const
		{
			return Token::merge(tokens, Token::Type::Identifier);
		}
	};

	struct IncludeInstruction : public AbstractInstruction
	{
		Lumina::Token includeFile;

		IncludeInstruction() :
			AbstractInstruction(AbstractInstruction::Type::Include)
		{

		}

		Token mergedToken() const
		{
			return (includeFile);
		}
	};

	struct PipelineFlowInstruction : public AbstractInstruction
	{
		Lumina::Token inputPipeline;
		Lumina::Token outputPipeline;
		std::shared_ptr<TypeInstruction> type;
		Lumina::Token name;

		PipelineFlowInstruction() :
			AbstractInstruction(AbstractInstruction::Type::PipelineFlow)
		{

		}

		Token mergedToken() const
		{
			std::vector<Token> tokens = {
				inputPipeline, name
			};
			return Token::merge(tokens, Token::Type::Identifier);
		}
	};

	struct Expression;

	struct ArrayInstruction
	{
		std::shared_ptr<Expression> expression;
	};

	struct BlockElementInstruction
	{
		std::shared_ptr<TypeInstruction> type;
		Lumina::Token name;
		std::shared_ptr<ArrayInstruction> array;

		BlockElementInstruction()
		{

		}
	};

	struct BlockInstruction : public AbstractInstruction
	{
		Lumina::Token name;
		std::vector<std::shared_ptr<BlockElementInstruction>> elements;

		BlockInstruction(const Type& p_type) :
			AbstractInstruction(p_type)
		{

		}

		Token mergedToken() const
		{
			return name;
		}
	};

	struct StructureBlockInstruction : public BlockInstruction
	{
		StructureBlockInstruction() :
			BlockInstruction(AbstractInstruction::Type::StructureBlock)
		{

		}
	};

	struct AttributeBlockInstruction : public BlockInstruction
	{
		AttributeBlockInstruction() :
			BlockInstruction(AbstractInstruction::Type::AttributeBlock)
		{

		}
	};

	struct ConstantBlockInstruction : public BlockInstruction
	{
		ConstantBlockInstruction() :
			BlockInstruction(AbstractInstruction::Type::ConstantBlock)
		{

		}
	};

	struct TextureInstruction : public AbstractInstruction
	{
		Lumina::Token name;

		TextureInstruction() :
			AbstractInstruction(AbstractInstruction::Type::Texture)
		{

		}

		Token mergedToken() const
		{
			return (name);
		}
	};

	struct NamespaceInstruction : public AbstractInstruction
	{
		Lumina::Token name;
		std::vector<std::shared_ptr<Instruction>> instructions;

		NamespaceInstruction() :
			AbstractInstruction(AbstractInstruction::Type::Namespace)
		{

		}

		Token mergedToken() const
		{
			return (name);
		}
	};

	struct SymbolParameterInstruction : public AbstractInstruction
	{
		std::shared_ptr<TypeInstruction> type;
		Lumina::Token name;

		SymbolParameterInstruction() :
			AbstractInstruction(AbstractInstruction::Type::SymbolParameter)
		{

		}

		Token mergedToken() const
		{
			return (Token::merge({ type->mergedToken(), name }, Token::Type::Identifier));
		}
	};

	struct Expression;

	struct ExpressionElement : public AbstractInstruction
	{
		ExpressionElement(const AbstractInstruction::Type& p_type) :
			AbstractInstruction(p_type)
		{

		}
	};

	struct OperatorExpression : public ExpressionElement
	{
		Lumina::Token token;

		OperatorExpression() :
			ExpressionElement(AbstractInstruction::Type::OperatorExpression)
		{

		}

		Token mergedToken() const
		{
			return (token);
		}
	};

	struct ComparatorOperatorExpression : public ExpressionElement
	{
		Lumina::Token token;

		ComparatorOperatorExpression() :
			ExpressionElement(AbstractInstruction::Type::ComparatorOperatorExpression)
		{

		}

		Token mergedToken() const
		{
			return (token);
		}
	};

	struct BoolExpressionValueInstruction : public ExpressionElement
	{
		Lumina::Token token;

		BoolExpressionValueInstruction() :
			ExpressionElement(AbstractInstruction::Type::BoolExpressionValue)
		{

		}

		Token mergedToken() const
		{
			return (token);
		}
	};

	struct NumberExpressionValueInstruction : public ExpressionElement
	{
		Lumina::Token token;

		NumberExpressionValueInstruction() :
			ExpressionElement(AbstractInstruction::Type::NumberExpressionValue)
		{

		}

		Token mergedToken() const
		{
			return (token);
		}
	};

	struct StringLiteralsExpressionValueInstruction : public ExpressionElement
	{
		Lumina::Token token;

		StringLiteralsExpressionValueInstruction() :
			ExpressionElement(AbstractInstruction::Type::StringLiteralsExpressionValue)
		{

		}

		Token mergedToken() const
		{
			return (token);
		}
	};

	struct VariableExpressionValueInstruction : public ExpressionElement
	{
		std::vector<Lumina::Token> tokens;
		std::shared_ptr<Expression> arrayAccessorExpression = nullptr;

		VariableExpressionValueInstruction() :
			ExpressionElement(AbstractInstruction::Type::VariableExpressionValue)
		{

		}

		Token mergedToken() const
		{
			return (Token::merge(tokens, Token::Type::Identifier));
		}
	};

	struct Expression : public AbstractInstruction
	{
		std::vector<std::shared_ptr<ExpressionElement>> elements;

		Expression() :
			AbstractInstruction(AbstractInstruction::Type::Expression)
		{

		}

		Token mergedToken() const;
	};

	struct SymbolNameInstruction : public AbstractInstruction
	{
		std::vector<Lumina::Token> tokens;

		SymbolNameInstruction() :
			AbstractInstruction(AbstractInstruction::Type::SymbolName)
		{

		}

		Token mergedToken() const
		{
			return (Token::merge(tokens, Token::Type::Identifier));
		}
	};


	struct SymbolCallInstruction : public ExpressionElement
	{
		struct ResultAccessor : public AbstractInstruction
		{
			std::vector<Token> tokens;

			ResultAccessor() :
				AbstractInstruction(AbstractInstruction::Type::ResultAccessor)
			{

			}

			Token mergedToken() const
			{
				return (Token::merge(tokens, Token::Type::Identifier));
			}
		};

		std::shared_ptr<SymbolNameInstruction> name;
		std::vector<std::shared_ptr<Expression>> arguments;
		std::shared_ptr<ResultAccessor> resultAccessor;

		SymbolCallInstruction() :
			ExpressionElement(AbstractInstruction::Type::SymbolCall)
		{

		}

		Token mergedToken() const
		{
			std::vector<Token> tokens;

			for (const auto& token : name->tokens)
			{
				tokens.push_back(token);
			}

			for (const auto& argument : arguments)
			{
				tokens.push_back(argument->mergedToken());
			}

			return (Token::merge(tokens, Token::Type::Identifier));
		}
	};

	struct VariableDesignationInstruction : public AbstractInstruction
	{
		std::vector<Lumina::Token> tokens;
		std::shared_ptr<Expression> arrayAccessorExpression = nullptr;

		VariableDesignationInstruction() :
			AbstractInstruction(AbstractInstruction::Type::VariableDesignation)
		{

		}

		Token mergedToken() const
		{
			return (Token::merge(tokens, Token::Type::Identifier));
		}
	};

	struct VariableAssignationInstruction : public AbstractInstruction
	{
		std::shared_ptr<VariableDesignationInstruction> name;
		std::shared_ptr<Expression> initializer;

		VariableAssignationInstruction() :
			AbstractInstruction(AbstractInstruction::Type::VariableAssignation)
		{

		}

		Token mergedToken() const
		{
			return (Token::merge({ name->mergedToken(), initializer->mergedToken() }, Token::Type::Identifier));
		}
	};

	struct VariableDeclarationInstruction : public AbstractInstruction
	{
		std::shared_ptr<TypeInstruction> type;
		Lumina::Token name;
		size_t size;
		std::shared_ptr<Expression> initializer;

		VariableDeclarationInstruction() :
			AbstractInstruction(AbstractInstruction::Type::VariableDeclaration)
		{

		}

		Token mergedToken() const
		{
			return (Token::merge({ type->mergedToken(), initializer->mergedToken() }, Token::Type::Identifier));
		}
	};

	struct SymbolBodyInstruction : public AbstractInstruction
	{
		std::vector<Token> completeBodyTokens;
		std::vector<std::shared_ptr<Instruction>> elements;

		SymbolBodyInstruction() :
			AbstractInstruction(AbstractInstruction::Type::SymbolBody)
		{

		}

		Token mergedToken() const
		{
			std::vector<Token> tokens;

			for (const auto& element : elements)
			{
				tokens.push_back(element->mergedToken());
			}

			return (Token::merge(tokens, Token::Type::Identifier));
		}
	};

	struct ReturnInstruction : public AbstractInstruction
	{
		std::shared_ptr<Expression> argument;

		ReturnInstruction() :
			AbstractInstruction(AbstractInstruction::Type::Return)
		{

		}

		Token mergedToken() const
		{
			return (argument->mergedToken());
		}
	};

	struct DiscardInstruction : public AbstractInstruction
	{
		DiscardInstruction() :
			AbstractInstruction(AbstractInstruction::Type::Discard)
		{

		}

		Token mergedToken() const
		{
			return (Token());
		}
	};

	struct ConditionElementInstruction : public AbstractInstruction
	{
		std::shared_ptr<Expression> lhs;
		Token comparatorToken;
		std::shared_ptr<Expression> rhs;

		ConditionElementInstruction() :
			AbstractInstruction(AbstractInstruction::Type::ConditionElement)
		{

		}

		Token mergedToken() const
		{
			return (Token::merge({ lhs->mergedToken(), rhs->mergedToken() }, Token::Type::Identifier));
		}
	};

	struct ConditionInstruction : public AbstractInstruction
	{
		std::vector<std::shared_ptr<ConditionElementInstruction>> elements;

		ConditionInstruction() :
			AbstractInstruction(AbstractInstruction::Type::Condition)
		{

		}

		Token mergedToken() const
		{
			std::vector<Token> tokens;

			for (const auto& element : elements)
			{
				tokens.push_back(element->mergedToken());
			}

			return (Token::merge(tokens, Token::Type::Identifier));
		}
	};

	struct ElseInstruction : public AbstractInstruction
	{
		std::shared_ptr<ConditionInstruction> condition;
		std::shared_ptr<SymbolBodyInstruction> body;

		ElseInstruction() :
			AbstractInstruction(AbstractInstruction::Type::Else)
		{

		}

		Token mergedToken() const
		{
			std::vector<Token> tokens;

			tokens.push_back(condition->mergedToken());
			tokens.push_back(body->mergedToken());

			return (Token::merge(tokens, Token::Type::Identifier));
		}
	};

	struct IfStatementInstruction : public AbstractInstruction
	{
		std::shared_ptr<ConditionInstruction> condition;
		std::shared_ptr<SymbolBodyInstruction> body;
		std::vector<std::shared_ptr<ElseInstruction>> elseBlocks;

		IfStatementInstruction() :
			AbstractInstruction(AbstractInstruction::Type::IfStatement)
		{

		}

		Token mergedToken() const
		{
			std::vector<Token> tokens;

			tokens.push_back(condition->mergedToken());
			tokens.push_back(body->mergedToken());

			for (const auto& element : elseBlocks)
			{
				tokens.push_back(element->mergedToken());
			}

			return (Token::merge(tokens, Token::Type::Identifier));
		}
	};

	struct WhileLoopInstruction : public AbstractInstruction
	{
		std::shared_ptr<ConditionInstruction> condition;
		std::shared_ptr<SymbolBodyInstruction> body;

		WhileLoopInstruction() :
			AbstractInstruction(AbstractInstruction::Type::WhileLoop)
		{

		}

		Token mergedToken() const
		{
			std::vector<Token> tokens;

			tokens.push_back(condition->mergedToken());
			tokens.push_back(body->mergedToken());

			return (Token::merge(tokens, Token::Type::Identifier));
		}
	};

	struct ForLoopInstruction : public AbstractInstruction
	{
		std::shared_ptr<AbstractInstruction> initializer;
		std::shared_ptr<ConditionInstruction> condition;
		std::shared_ptr<Expression> increment;
		std::shared_ptr<SymbolBodyInstruction> body;

		ForLoopInstruction() :
			AbstractInstruction(AbstractInstruction::Type::ForLoop)
		{

		}

		Token mergedToken() const
		{
			std::vector<Token> tokens;

			tokens.push_back(initializer->mergedToken());
			tokens.push_back(condition->mergedToken());
			tokens.push_back(increment->mergedToken());
			tokens.push_back(body->mergedToken());

			return (Token::merge(tokens, Token::Type::Identifier));
		}
	};

	struct SymbolInstruction : public AbstractInstruction
	{
		std::shared_ptr<TypeInstruction> returnType;
		Lumina::Token name;
		std::vector<std::shared_ptr<SymbolParameterInstruction>> parameters;
		std::shared_ptr<SymbolBodyInstruction> body;

		SymbolInstruction() :
			AbstractInstruction(AbstractInstruction::Type::Symbol)
		{

		}

		Token mergedToken() const
		{
			std::vector<Token> tokens;

			tokens.push_back(returnType->mergedToken());
			tokens.push_back(name);

			for (const auto& parameter : parameters)
			{
				tokens.push_back(parameter->mergedToken());
			}

			tokens.push_back(body->mergedToken());

			return (Token::merge(tokens, Token::Type::Identifier));
		}
	};

	struct PipelineBodyInstruction : public AbstractInstruction
	{
		Lumina::Token pipelineToken;
		std::shared_ptr<SymbolBodyInstruction> body;

		PipelineBodyInstruction() :
			AbstractInstruction(AbstractInstruction::Type::PipelineBody)
		{

		}

		Token mergedToken() const
		{
			std::vector<Token> tokens;

			tokens.push_back(pipelineToken);
			tokens.push_back(body->mergedToken());

			return (Token::merge(tokens, Token::Type::Identifier));
		}
	};
}

std::string to_string(Lumina::AbstractInstruction::Type p_type);