#pragma once

#include <vector>
#include <memory>
#include <optional>
#include <variant>
#include <string>
#include "lumina_token.hpp"
#include "lumina_descriptors.hpp"

namespace Lumina
{
	struct Instruction {
		enum class Type {
			Unknown,
			VariableDeclaration,
			VariableAssignation,
			SymbolCall,
			IfStatement,
			WhileStatement,
			ForStatement,
			ReturnStatement,
			DiscardStatement,
			SymbolBody
		};

		Type type;

		Instruction(Type p_type) : type(p_type) {}
		virtual ~Instruction() = default;
	};

	struct SymbolBody : public Instruction
	{
		std::vector<std::shared_ptr<Instruction>> instructions;

		SymbolBody() : Instruction(Type::SymbolBody) {}
	};

	struct Expression : public Instruction {
		struct Result
		{
			const Type* type;
			std::vector<size_t> arraySizes;
		};

		struct Element : public Instruction {
			enum class Type {
				Unknown,
				Number,
				InnerExpression,
				Boolean,
				VariableDesignation,
				Operator,
				ComparaisonOperator,
				ConditionOperator,
				Incrementor,
				SymbolCall
			};

			Element::Type elementType;

			Element(Element::Type p_type) : elementType(p_type), Instruction(Instruction::Type::SymbolBody) {}
			virtual ~Element() = default;

			virtual Lumina::Token token() const = 0;
		};

		struct InnerExpression : public Element
		{
			std::shared_ptr<Expression> expression;

			InnerExpression() : Element(Type::InnerExpression) {}

			Lumina::Token token() const
			{
				return (expression->token());
			}
		};

		struct NumberElement : public Element {
			Lumina::Token value;

			NumberElement() : Element(Type::Number) {}

			Lumina::Token token() const
			{
				return (value);
			}
		};

		struct BooleanElement : public Element {
			Lumina::Token value;

			BooleanElement() : Element(Type::Boolean) {}

			Lumina::Token token() const
			{
				return (value);
			}
		};

		struct VariableDesignationElement : public Element {
			struct AccessorElement : public Instruction
			{
				AccessorElement() : Instruction(Instruction::Type::SymbolBody) {}
				virtual Lumina::Token token() const = 0;
			};

			struct VariableAccessorElement : public AccessorElement
			{
				Lumina::Token name;

				VariableAccessorElement(Lumina::Token p_name = Lumina::Token()) :
					AccessorElement(),
					name(p_name)
				{}

				Lumina::Token token() const
				{
					return (name);
				}

			};

			struct ArrayAccessorElement : public AccessorElement
			{
				std::shared_ptr<Expression> expression;

				ArrayAccessorElement(const std::shared_ptr<Expression>& p_expression = nullptr) :
					AccessorElement(),
					expression(p_expression)
				{}

				Lumina::Token token() const
				{
					return (expression->token());
				}

			};

			Lumina::Token signOperator;
			std::vector<Lumina::Token> namespaceChain;
			Lumina::Token name;
			std::vector<std::shared_ptr<AccessorElement>> accessors;

			VariableDesignationElement() : Element(Type::VariableDesignation) {}

			Lumina::Token token() const
			{
				std::vector<Lumina::Token> tokensToMerge;

				tokensToMerge.push_back(signOperator);
				for (const auto& token : namespaceChain)
				{
					tokensToMerge.push_back(token);
				}
				tokensToMerge.push_back(name);
				for (const auto& accessor : accessors)
				{
					tokensToMerge.push_back(accessor->token());
				}

				return (Lumina::Token::merge(tokensToMerge, Lumina::Token::Type::Identifier));
			}
		};

		struct OperatorElement : public Element {
			Lumina::Token operatorToken;

			OperatorElement() : Element(Type::Operator) {}

			Lumina::Token token() const
			{
				return (operatorToken);
			}
		};

		struct ComparatorOperatorElement : public Element {
			Lumina::Token operatorToken;

			ComparatorOperatorElement() : Element(Type::ComparaisonOperator) {}

			Lumina::Token token() const
			{
				return (operatorToken);
			}
		};

		struct ConditionOperatorElement : public Element {
			Lumina::Token operatorToken;

			ConditionOperatorElement() : Element(Type::ConditionOperator) {}

			Lumina::Token token() const
			{
				return (operatorToken);
			}
		};

		struct IncrementorElement : public Element {
			Lumina::Token operatorToken;

			IncrementorElement() : Element(Type::Incrementor) {}

			Lumina::Token token() const
			{
				return (operatorToken);
			}
		};

		struct SymbolCallElement : public Element {
			std::vector<Lumina::Token> namespaceChain;
			Lumina::Token functionName;
			std::vector<std::shared_ptr<Expression>> parameters;

			SymbolCallElement() : Element(Type::SymbolCall) {}

			Lumina::Token token() const
			{
				std::vector<Lumina::Token> tokensToMerge;

				for (const auto& token : namespaceChain)
				{
					tokensToMerge.push_back(token);
				}
				tokensToMerge.push_back(functionName);
				for (const auto& parameter : parameters)
				{
					tokensToMerge.push_back(parameter->token());
				}

				return (Lumina::Token::merge(tokensToMerge, Lumina::Token::Type::Identifier));
			}
		};

		std::vector<std::shared_ptr<Expression::Element>> elements;

		Expression() : Instruction(Type::SymbolBody) {}

		Lumina::Token token() const
		{
			std::vector<Lumina::Token> tokensToMerge;

			for (const auto& element : elements)
			{
				tokensToMerge.push_back(element->token());
			}

			return (Lumina::Token::merge(tokensToMerge, Lumina::Token::Type::Identifier));

		}
	};

	struct ConditionalOperator : public Instruction
	{
		Lumina::Token token;

		ConditionalOperator() :
			Instruction(Instruction::Type::SymbolBody)
		{

		}
	};

	struct Condition
	{
		std::vector< std::shared_ptr<Instruction>> values;		
	};

	struct VariableDeclaration : public Instruction {
		VariableDescriptor descriptor;
		std::shared_ptr<Expression> initialValue;

		VariableDeclaration() : Instruction(Type::VariableDeclaration) {}
	};

	struct VariableAssignation : public Instruction {
		std::shared_ptr<Expression::VariableDesignationElement> target;
		std::shared_ptr<Expression> value;

		VariableAssignation() : Instruction(Type::VariableAssignation) {}
	};

	struct SymbolCall : public Instruction
	{
		std::vector<Lumina::Token> namespaceChain;
		Lumina::Token functionName;
		std::vector<std::shared_ptr<Expression>> parameters;

		SymbolCall() : Instruction(Type::SymbolCall) {}
	};

	struct ConditionalBranch
	{
		Condition condition;
		SymbolBody body;

		ConditionalBranch() = default;
	};

	struct IfStatement : public Instruction
	{
		std::vector<ConditionalBranch> branches;

		IfStatement() : Instruction(Type::IfStatement) {}
	};

	struct WhileStatement : public Instruction
	{
		std::shared_ptr<Expression> condition;
		SymbolBody body;

		WhileStatement() : Instruction(Type::WhileStatement) {}
	};

	struct ForStatement : public Instruction
	{
		std::shared_ptr<Instruction> initializer;
		std::shared_ptr<Expression> condition;
		std::shared_ptr<Instruction> increment;
		SymbolBody body;

		ForStatement() : Instruction(Type::ForStatement) {}
	};

	struct ReturnStatement : public Instruction
	{
		std::optional<std::shared_ptr<Expression>> returnValue;

		ReturnStatement() : Instruction(Type::ReturnStatement) {}
	};

	struct DiscardStatement : public Instruction
	{
		DiscardStatement() : Instruction(Type::DiscardStatement) {}
	};
}
