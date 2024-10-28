#include "shader_representation.hpp"

#include <sstream>

namespace Lumina
{

	std::ostream& operator<<(std::ostream& os, ShaderRepresentation::ArithmeticOperator op)
	{
		switch (op)
		{
		case ShaderRepresentation::ArithmeticOperator::Plus: os << "+"; break;
		case ShaderRepresentation::ArithmeticOperator::Minus: os << "-"; break;
		case ShaderRepresentation::ArithmeticOperator::Multiply: os << "*"; break;
		case ShaderRepresentation::ArithmeticOperator::Divide: os << "/"; break;
		case ShaderRepresentation::ArithmeticOperator::Modulo: os << "%"; break;
		case ShaderRepresentation::ArithmeticOperator::ConditionEqual: os << "=="; break;
		case ShaderRepresentation::ArithmeticOperator::NotEqual: os << "!="; break;
		case ShaderRepresentation::ArithmeticOperator::Less: os << "<"; break;
		case ShaderRepresentation::ArithmeticOperator::Greater: os << ">"; break;
		case ShaderRepresentation::ArithmeticOperator::LessEqual: os << "<="; break;
		case ShaderRepresentation::ArithmeticOperator::GreaterEqual: os << ">="; break;
		case ShaderRepresentation::ArithmeticOperator::LogicalAnd: os << "&&"; break;
		case ShaderRepresentation::ArithmeticOperator::LogicalOr: os << "||"; break;
		case ShaderRepresentation::ArithmeticOperator::Equal: os << "="; break;
		case ShaderRepresentation::ArithmeticOperator::PlusEqual: os << "+="; break;
		case ShaderRepresentation::ArithmeticOperator::MinusEqual: os << "-="; break;
		case ShaderRepresentation::ArithmeticOperator::MultiplyEqual: os << "*="; break;
		case ShaderRepresentation::ArithmeticOperator::DivideEqual: os << "/="; break;
		case ShaderRepresentation::ArithmeticOperator::ModuloEqual: os << "%="; break;
		default: os << "?Arithmetic"; break;
		}
		return os;
	}

	std::ostream& operator<<(std::ostream& os, ShaderRepresentation::UnaryOperator op)
	{
		switch (op)
		{
		case ShaderRepresentation::UnaryOperator::Increment: os << "++"; break;
		case ShaderRepresentation::UnaryOperator::Decrement: os << "--"; break;
		default: os << "?Unary"; break;
		}
		return os;
	}

	std::ostream& operator<<(std::ostream& os, ShaderRepresentation::AssignatorOperator op)
	{
		switch (op)
		{
		case ShaderRepresentation::AssignatorOperator::Equal: os << "="; break;
		case ShaderRepresentation::AssignatorOperator::PlusEqual: os << "+="; break;
		case ShaderRepresentation::AssignatorOperator::MinusEqual: os << "-="; break;
		case ShaderRepresentation::AssignatorOperator::MultiplyEqual: os << "*="; break;
		case ShaderRepresentation::AssignatorOperator::DivideEqual: os << "/="; break;
		case ShaderRepresentation::AssignatorOperator::ModuloEqual: os << "%="; break;
		default: os << "?Assignator"; break;
		}
		return os;
	}

	void printArraySizes(std::ostream& os, const std::vector<size_t>& arraySize)
	{
		if (!arraySize.empty())
		{
			os << "[";
			for (size_t i = 0; i < arraySize.size(); ++i)
			{
				if (i > 0) os << ", ";
				os << arraySize[i];
			}
			os << "]";
		}
	}

	std::ostream& operator<<(std::ostream& os, const ShaderRepresentation::Variable& var)
	{
		if (var.type == nullptr)
		{
			os << "[No type] " << var.name;
		}
		else
		{
			os << var.type->name << " " << var.name;
			printArraySizes(os, var.arraySize);
		}
		return os;
	}

	std::ostream& operator<<(std::ostream& os, const ShaderRepresentation::Parameter& param)
	{
		if (param.type == nullptr)
		{
			os << "[No type]";
		}
		else
		{
			os << param.type->name;
			printArraySizes(os, param.arraySize);
		}
		os << " " << (param.isReference ? "in " : "") << param.name;
		return os;
	}

	std::ostream& operator<<(std::ostream& os, const ShaderRepresentation::Expression& expr)
	{
		if (auto literalExpr = dynamic_cast<const ShaderRepresentation::LiteralExpression*>(&expr))
		{
			std::visit([&os](auto&& value) {
				os << value;
				}, literalExpr->value);
		}
		else if (auto varExpr = dynamic_cast<const ShaderRepresentation::VariableExpression*>(&expr))
		{
			os << varExpr->variableName;
		}
		else if (auto binaryExpr = dynamic_cast<const ShaderRepresentation::BinaryExpression*>(&expr))
		{
			os << "(" << *binaryExpr->left << " " << binaryExpr->op << " " << *binaryExpr->right << ")";
		}
		else if (auto unaryExpr = dynamic_cast<const ShaderRepresentation::UnaryExpression*>(&expr))
		{
			os << unaryExpr->op << *unaryExpr->operand;
		}
		else if (auto funcCallExpr = dynamic_cast<const ShaderRepresentation::FunctionCallExpression*>(&expr))
		{
			os << funcCallExpr->functionName << "(";
			for (size_t i = 0; i < funcCallExpr->arguments.size(); ++i)
			{
				if (i > 0) os << ", ";
				os << *funcCallExpr->arguments[i];
			}
			os << ")";
		}
		else if (auto memberAccessExpr = dynamic_cast<const ShaderRepresentation::MemberAccessExpression*>(&expr))
		{
			os << *memberAccessExpr->object << "." << memberAccessExpr->memberName;
		}
		else if (auto arrayAccessExpr = dynamic_cast<const ShaderRepresentation::ArrayAccessExpression*>(&expr))
		{
			os << *arrayAccessExpr->array << "[" << *arrayAccessExpr->index << "]";
		}
		else
		{
			os << "[Unknown Expression]";
		}
		return os;
	}

	std::ostream& operator<<(std::ostream& os, const ShaderRepresentation::SymbolBody& body);

	std::ostream& operator<<(std::ostream& os, const ShaderRepresentation::Statement& stmt)
	{
		const std::string indent = "    ";
		if (auto varDecl = dynamic_cast<const ShaderRepresentation::VariableDeclarationStatement*>(&stmt))
		{
			if (varDecl->variable.type)
			{
				os << indent << varDecl->variable.type->name << " ";
			}
			else
			{
				os << indent << "[Unknown Type] ";
			}
			os << varDecl->variable.name;
			printArraySizes(os, varDecl->variable.arraySize);
			if (varDecl->initializer)
			{
				os << " = ";
				os << *varDecl->initializer;
			}
			os << ";\n";
		}
		else if (auto exprStmt = dynamic_cast<const ShaderRepresentation::ExpressionStatement*>(&stmt))
		{
			os << indent << *exprStmt->expression << ";\n";
		}
		else if (auto assignStmt = dynamic_cast<const ShaderRepresentation::AssignmentStatement*>(&stmt))
		{
			os << indent << *(assignStmt->target) << " " << assignStmt->op << " " << *(assignStmt->value) << ";\n";
		}
		else if (auto returnStmt = dynamic_cast<const ShaderRepresentation::ReturnStatement*>(&stmt))
		{
			os << indent << "return";
			if (returnStmt->expression)
			{
				os << " " << *returnStmt->expression;
			}
			os << ";\n";
		}
		else if (dynamic_cast<const ShaderRepresentation::DiscardStatement*>(&stmt))
		{
			os << indent << "discard;\n";
		}
		else if (auto ifStmt = dynamic_cast<const ShaderRepresentation::IfStatement*>(&stmt))
		{
			for (size_t i = 0; i < ifStmt->branches.size(); ++i)
			{
				const auto& branch = ifStmt->branches[i];
				if (i == 0)
				{
					os << indent << "if (" << *branch.condition << ") {\n";
				}
				else
				{
					os << indent << "else if (" << *branch.condition << ") {\n";
				}
				os << branch.body;
				os << indent << "}\n";
			}
			if (!ifStmt->elseBody.statements.empty())
			{
				os << indent << "else {\n";
				os << ifStmt->elseBody;
				os << indent << "}\n";
			}
		}
		else if (auto whileStmt = dynamic_cast<const ShaderRepresentation::WhileStatement*>(&stmt))
		{
			os << indent << "while (" << *whileStmt->condition << ") {\n";
			os << whileStmt->body;
			os << indent << "}\n";
		}
		else if (auto forStmt = dynamic_cast<const ShaderRepresentation::ForStatement*>(&stmt))
		{
			os << indent << "for (";
			if (forStmt->initializer)
			{
				std::ostringstream initStream;
				initStream << *forStmt->initializer;
				std::string initStr = initStream.str();
				initStr.pop_back();
				os << initStr;
			}
			else
			{
				os << ";";
			}
			os << " ";
			if (forStmt->condition)
			{
				os << *forStmt->condition;
			}
			os << "; ";
			if (forStmt->increment)
			{
				os << *forStmt->increment;
			}
			os << ") {\n";
			os << forStmt->body;
			os << indent << "}\n";
		}
		else if (auto compoundStmt = dynamic_cast<const ShaderRepresentation::CompoundStatement*>(&stmt))
		{
			os << indent << "{\n";
			os << compoundStmt->body;
			os << indent << "}\n";
		}
		else
		{
			os << indent << "[Unknown Statement]\n";
		}
		return os;
	}

	std::ostream& operator<<(std::ostream& os, const ShaderRepresentation::SymbolBody& body)
	{
		for (const auto& stmt : body.statements)
		{
			os << *stmt;
		}
		return os;
	}

	std::ostream& operator<<(std::ostream& os, const ShaderRepresentation::Type::Constructor& constructor)
	{
		os << "(";
		for (size_t i = 0; i < constructor.parameters.size(); ++i)
		{
			if (i != 0)
			{
				os << ", ";
			}
			os << constructor.parameters[i];
		}
		os << ")";
		if (constructor.isPrototype == false)
		{
			os << " {\n";
			os << constructor.body;
			os << "}";
		}
		else
		{
			os << ";";
		}
		return os;
	}

	std::ostream& operator<<(std::ostream& os, const ShaderRepresentation::ExpressionType& exprType)
	{
		if (exprType.type == nullptr)
		{
			os << "[No type]";
		}
		else
		{
			os << exprType.type->name;
			printArraySizes(os, exprType.arraySize);
		}
		return os;
	}

	std::ostream& operator<<(std::ostream& os, const ShaderRepresentation::Function& func)
	{
		os << func.returnType << " " << func.name << "(";
		for (size_t i = 0; i < func.parameters.size(); ++i)
		{
			if (i != 0)
			{
				os << ", ";
			}
			os << func.parameters[i];
		}
		os << ")";
		if (func.isPrototype == false)
		{
			os << " {\n";
			os << func.body;
			os << "}";
		}
		else
		{
			os << ";";
		}
		return os;
	}

	std::ostream& operator<<(std::ostream& os, const ShaderRepresentation::Type& type)
	{
		os << "Type: " << type.name << "\n";

		if (!type.attributes.empty())
		{
			os << "    Attributes:\n";
			for (const auto& attr : type.attributes)
			{
				os << "        " << attr << "\n";
			}
		}

		if (!type.constructors.empty())
		{
			os << "    Constructors:\n";
			for (const auto& constructor : type.constructors)
			{
				os << "        " << type.name << constructor << "\n";
			}
		}

		if (!type.methods.empty())
		{
			os << "    Methods:\n";
			for (const auto& methodPair : type.methods)
			{
				for (const auto& method : methodPair.second)
				{
					os << "        " << method << "\n";
				}
			}
		}

		if (!type.operators.empty())
		{
			os << "    Operators:\n";
			for (const auto& operatorPair : type.operators)
			{
				for (const auto& op : operatorPair.second)
				{
					os << "        " << op << "\n";
				}
			}
		}

		if (!type.acceptedConvertions.empty())
		{
			os << "    Convertions:\n";
			for (const auto& convertion : type.acceptedConvertions)
			{
				os << "        " << type.name << " -> " << convertion->name << "\n";
			}
		}

		return os;
	}

	std::ostream& operator<<(std::ostream& os, const ShaderRepresentation& shaderRep)
	{
		os << "Available Types:\n";
		for (const auto& type : shaderRep.availableTypes)
		{
			os << "    " << type << "\n";
		}

		os << "\nGlobal Variables:\n";
		for (const auto& var : shaderRep.globalVariables)
		{
			os << "    " << var << "\n";
		}

		os << "\nVertex Variables:\n";
		for (const auto& var : shaderRep.vertexVariables)
		{
			os << "    " << var << "\n";
		}

		os << "\nFragment Variables:\n";
		for (const auto& var : shaderRep.fragmentVariables)
		{
			os << "    " << var << "\n";
		}

		os << "\nFunctions:\n";
		for (const auto& funcPair : shaderRep.availableFunctions)
		{
			for (const auto& func : funcPair.second)
			{
				os << "    " << func << "\n";
			}
		}

		os << "\nVertexPass main() {\n";
		os << shaderRep.vertexPassMain.body;
		os << "}\n";

		os << "\nFragmentPass main() {\n";
		os << shaderRep.fragmentPassMain.body;
		os << "}\n";

		os << "\nAttribute Types:\n";
		for (const auto& attrType : shaderRep.attributesTypes)
		{
			if (attrType == nullptr)
			{
				os << "    Inserted a nullptr type in attribute\n";
			}
			else
			{
				os << "    " << attrType->name << "\n";
			}
		}

		os << "\nConstants Types:\n";
		for (const auto& constType : shaderRep.constantsTypes)
		{
			if (constType == nullptr)
			{
				os << "    Inserted a nullptr type in Constant\n";
			}
			else
			{
				os << "    " << constType->name << "\n";
			}
		}

		return os;
	}

	std::string ShaderRepresentation::SymbolBody::toString() const
	{
		std::ostringstream oss;
		oss << *this;
		return oss.str();
	}
}
