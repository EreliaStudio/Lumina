#include "lumina_instructions.hpp"

std::string to_string(Lumina::AbstractInstruction::Type p_type)
{
	switch (p_type)
	{
	case Lumina::AbstractInstruction::Type::Identifier:
		return "Identifier";
	case Lumina::AbstractInstruction::Type::Type:
		return "Type";
	case Lumina::AbstractInstruction::Type::Include:
		return "Include";
	case Lumina::AbstractInstruction::Type::PipelineFlow:
		return "PipelineFlow";
	case Lumina::AbstractInstruction::Type::StructureBlock:
		return "StructureBlock";
	case Lumina::AbstractInstruction::Type::AttributeBlock:
		return "AttributeBlock";
	case Lumina::AbstractInstruction::Type::ConstantBlock:
		return "ConstantBlock";
	case Lumina::AbstractInstruction::Type::Texture:
		return "Texture";
	case Lumina::AbstractInstruction::Type::Namespace:
		return "Namespace";
	case Lumina::AbstractInstruction::Type::SymbolParameter:
		return "SymbolParameter";
	case Lumina::AbstractInstruction::Type::OperatorExpression:
		return "OperatorExpression";
	case Lumina::AbstractInstruction::Type::NumberExpressionValue:
		return "NumberExpressionValue";
	case Lumina::AbstractInstruction::Type::StringLiteralsExpressionValue:
		return "StringLiteralsExpressionValue";
	case Lumina::AbstractInstruction::Type::BoolExpressionValue:
		return "BoolExpressionValue";
	case Lumina::AbstractInstruction::Type::VariableExpressionValue:
		return "VariableExpressionValue";
	case Lumina::AbstractInstruction::Type::Expression:
		return "Expression";
	case Lumina::AbstractInstruction::Type::VariableDesignation:
		return "VariableDesignation";
	case Lumina::AbstractInstruction::Type::VariableAssignation:
		return "VariableAssignation";
	case Lumina::AbstractInstruction::Type::VariableDeclaration:
		return "VariableDeclaration";
	case Lumina::AbstractInstruction::Type::SymbolName:
		return "SymbolName";
	case Lumina::AbstractInstruction::Type::SymbolBody:
		return "SymbolBody";
	case Lumina::AbstractInstruction::Type::SymbolCall:
		return "SymbolCall";
	case Lumina::AbstractInstruction::Type::Return:
		return "Return";
	case Lumina::AbstractInstruction::Type::Discard:
		return "Discard";
	case Lumina::AbstractInstruction::Type::ConditionElement:
		return "ConditionElement";
	case Lumina::AbstractInstruction::Type::Condition:
		return "Condition";
	case Lumina::AbstractInstruction::Type::Else:
		return "Else";
	case Lumina::AbstractInstruction::Type::IfStatement:
		return "IfStatement";
	case Lumina::AbstractInstruction::Type::WhileLoop:
		return "WhileLoop";
	case Lumina::AbstractInstruction::Type::ForLoop:
		return "ForLoop";
	case Lumina::AbstractInstruction::Type::Symbol:
		return "Symbol";
	case Lumina::AbstractInstruction::Type::PipelineBody:
		return "PipelineBody";
	default:
		return "Unknown";
	}
}

namespace Lumina
{
	Token Lumina::ExpressionInstruction::mergedToken() const
	{
		std::vector<Token> tokens;

		for (const auto& element : elements)
		{
			tokens.push_back(element->mergedToken());
		}

		return (Token::merge(tokens, Token::Type::Identifier));
	}
}
