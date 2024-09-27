#include "lumina_compiler.hpp"

namespace Lumina
{
	std::string Compiler::parseNumberElement(const std::shared_ptr<Expression::NumberElement>& element)
	{
		std::string result = "";

		result += element->value.content;

		return result;
	}

	std::string Compiler::parseBooleanElement(const std::shared_ptr<Expression::BooleanElement>& element)
	{
		std::string result = "";

		result += element->value.content;

		return result;
	}

	std::string Compiler::parseVariableDesignationElement(const std::shared_ptr<Expression::VariableDesignationElement>& element)
	{
		std::string result = "";

		if (element->signOperator.type != Lumina::Token::Type::Unknow)
		{
			result += element->signOperator.content;
		}

		for (const auto& ns : element->namespaceChain)
		{
			result += ns.content + "::";
		}

		result += element->name.content;

		for (const auto& accessor : element->accessors)
		{
			if (accessor->type == Instruction::Type::SymbolBody)
			{
				auto castedAccessor = std::static_pointer_cast<Expression::VariableDesignationElement::AccessorElement>(accessor);
				result += "." + castedAccessor->name.content;
			}
		}
		return result;
	}

	std::string Compiler::parseOperatorElement(const std::shared_ptr<Expression::OperatorElement>& element)
	{
		std::string result = "";
		result += element->operatorToken.content;
		return result;
	}

	std::string Compiler::parseComparatorOperatorElement(const std::shared_ptr<Expression::ComparatorOperatorElement>& element)
	{
		std::string result = "";
		result += element->operatorToken.content;
		return result;
	}

	std::string Compiler::parseConditionOperatorElement(const std::shared_ptr<Expression::ConditionOperatorElement>& element)
	{
		std::string result = "";
		result += element->operatorToken.content;
		return result;
	}

	std::string Compiler::parseIncrementorElement(const std::shared_ptr<Expression::IncrementorElement>& element)
	{
		std::string result = "";
		result += element->operatorToken.content;
		return result;
	}

	std::string Compiler::parseSymbolCallElement(const std::shared_ptr<Expression::SymbolCallElement>& element)
	{
		std::string result = "";

		for (const auto& ns : element->namespaceChain)
		{
			result += ns.content + "::";
		}

		result += element->functionName.content + "(";

		for (size_t i = 0; i < element->parameters.size(); ++i)
		{
			result += parseExpression(element->parameters[i]);
			if (i != element->parameters.size() - 1)
			{
				result += ", ";
			}
		}

		result += ")";
		return result;
	}

	std::string Compiler::parseExpression(const std::shared_ptr<Expression> p_expression)
	{
		std::string result = "";

		for (const auto& element : p_expression->elements)
		{
			try
			{
				switch (element->elementType)
				{
				case Expression::Element::Type::Number:
					result += parseNumberElement(std::static_pointer_cast<Expression::NumberElement>(element));
					break;
				case Expression::Element::Type::Boolean:
					result += parseBooleanElement(std::static_pointer_cast<Expression::BooleanElement>(element));
					break;
				case Expression::Element::Type::VariableDesignation:
					result += parseVariableDesignationElement(std::static_pointer_cast<Expression::VariableDesignationElement>(element));
					break;
				case Expression::Element::Type::Operator:
					result += parseOperatorElement(std::static_pointer_cast<Expression::OperatorElement>(element));
					break;
				case Expression::Element::Type::ComparaisonOperator:
					result += parseComparatorOperatorElement(std::static_pointer_cast<Expression::ComparatorOperatorElement>(element));
					break;
				case Expression::Element::Type::ConditionOperator:
					result += parseConditionOperatorElement(std::static_pointer_cast<Expression::ConditionOperatorElement>(element));
					break;
				case Expression::Element::Type::Incrementor:
					result += parseIncrementorElement(std::static_pointer_cast<Expression::IncrementorElement>(element));
					break;
				case Expression::Element::Type::SymbolCall:
					result += parseSymbolCallElement(std::static_pointer_cast<Expression::SymbolCallElement>(element));
					break;
				default:
					throw TokenBasedError("Unknown element type", Token());
				}
			}
			catch (TokenBasedError& e)
			{
				_result.errors.push_back(e);
			}
		}

		return result;
	}
}