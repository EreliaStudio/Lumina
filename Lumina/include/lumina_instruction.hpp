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
		struct Element : public Instruction {
			enum class Type {
				Unknown,
				Number,
				Boolean,
				VariableDesignation,
				Operator,
				ComparaisonOperator,
				ConditionOperator,
				Incrementor,
				SymbolCall
			};

			Type type;

			Element(Type p_type) : type(p_type), Instruction(Instruction::Type::SymbolBody) {}
			virtual ~Element() = default;
		};

		struct NumberElement : public Element {
			Lumina::Token value;

			NumberElement() : Element(Type::Number) {}
		};

		struct BooleanElement : public Element {
			Lumina::Token value;

			BooleanElement() : Element(Type::Boolean) {}
		};

		struct VariableDesignationElement : public Element {
			struct AccessorElement : public Instruction
			{
				Lumina::Token name;

				AccessorElement() : Instruction(Instruction::Type::SymbolBody) {}
			};
			std::vector<Lumina::Token> namespaceChain;
			Lumina::Token name;
			std::vector<std::shared_ptr<Instruction>> accessors;

			VariableDesignationElement() : Element(Type::VariableDesignation) {}
		};

		struct OperatorElement : public Element {
			Lumina::Token operatorToken;

			OperatorElement() : Element(Type::Operator) {}
		};

		struct ComparatorOperatorElement : public Element {
			Lumina::Token operatorToken;

			ComparatorOperatorElement() : Element(Type::ComparaisonOperator) {}
		};

		struct ConditionOperatorElement : public Element {
			Lumina::Token operatorToken;

			ConditionOperatorElement() : Element(Type::ConditionOperator) {}
		};

		struct IncrementorElement : public Element {
			Lumina::Token operatorToken;

			IncrementorElement() : Element(Type::Incrementor) {}
		};

		struct SymbolCallElement : public Element {
			std::vector<Lumina::Token> namespaceChain;
			Lumina::Token functionName;
			std::vector<std::shared_ptr<Expression>> parameters;

			SymbolCallElement() : Element(Type::SymbolCall) {}
		};

		std::vector<std::shared_ptr<Instruction>> elements;

		Expression() : Instruction(Type::SymbolBody) {}
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
		std::optional<std::shared_ptr<Expression>> initialValue;

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
		std::vector<std::shared_ptr<Instruction>> body;

		WhileStatement() : Instruction(Type::WhileStatement) {}
	};

	struct ForStatement : public Instruction
	{
		std::shared_ptr<Instruction> initializer;
		std::shared_ptr<Expression> condition;
		std::shared_ptr<Instruction> increment;
		std::vector<std::shared_ptr<Instruction>> body;

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
