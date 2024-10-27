#include "parser.hpp"
#include <unordered_map>

namespace Lumina
{

	ShaderRepresentation::ArithmeticOperator _stringToOperator(const std::string& opStr)
	{
		static const std::unordered_map<std::string, ShaderRepresentation::ArithmeticOperator> operatorMap = {
			{ "+", ShaderRepresentation::ArithmeticOperator::Plus },
			{ "-", ShaderRepresentation::ArithmeticOperator::Minus },
			{ "*", ShaderRepresentation::ArithmeticOperator::Multiply },
			{ "/", ShaderRepresentation::ArithmeticOperator::Divide },
			{ "%", ShaderRepresentation::ArithmeticOperator::Modulo },
			{ "==", ShaderRepresentation::ArithmeticOperator::ConditionEqual },
			{ "!=", ShaderRepresentation::ArithmeticOperator::NotEqual },
			{ "<", ShaderRepresentation::ArithmeticOperator::Less },
			{ ">", ShaderRepresentation::ArithmeticOperator::Greater },
			{ "<=", ShaderRepresentation::ArithmeticOperator::LessEqual },
			{ ">=", ShaderRepresentation::ArithmeticOperator::GreaterEqual },
			{ "&&", ShaderRepresentation::ArithmeticOperator::LogicalAnd },
			{ "||", ShaderRepresentation::ArithmeticOperator::LogicalOr },
			{ "=", ShaderRepresentation::ArithmeticOperator::Equal },
			{ "+=", ShaderRepresentation::ArithmeticOperator::PlusEqual },
			{ "-=", ShaderRepresentation::ArithmeticOperator::MinusEqual },
			{ "*=", ShaderRepresentation::ArithmeticOperator::MultiplyEqual },
			{ "/=", ShaderRepresentation::ArithmeticOperator::DivideEqual },
			{ "%=", ShaderRepresentation::ArithmeticOperator::ModuloEqual }
		};

		auto it = operatorMap.find(opStr);
		if (it != operatorMap.end())
		{
			return it->second;
		}
		else
		{
		std::cout << "Unknown operator: " + opStr << std::endl;
			throw std::runtime_error("Unknown operator: " + opStr);
		}
	}

	ShaderRepresentation::AssignatorOperator _stringToAssignatorOperator(const std::string& opStr)
	{
		static const std::unordered_map<std::string, ShaderRepresentation::AssignatorOperator> assignatorOperatorMap = {
			{ "=", ShaderRepresentation::AssignatorOperator::Equal },
			{ "+=", ShaderRepresentation::AssignatorOperator::PlusEqual },
			{ "-=", ShaderRepresentation::AssignatorOperator::MinusEqual },
			{ "*=", ShaderRepresentation::AssignatorOperator::MultiplyEqual },
			{ "/=", ShaderRepresentation::AssignatorOperator::DivideEqual },
			{ "%=", ShaderRepresentation::AssignatorOperator::ModuloEqual }
		};

		auto it = assignatorOperatorMap.find(opStr);
		if (it != assignatorOperatorMap.end())
		{
			return it->second;
		}
		else
		{
			throw std::runtime_error("Unknown assignator operator: " + opStr);
		}
	}

	ShaderRepresentation::UnaryOperator _stringToUnaryOperator(const std::string& opStr)
	{
		static const std::unordered_map<std::string, ShaderRepresentation::UnaryOperator> unaryOperatorMap = {
			{ "--", ShaderRepresentation::UnaryOperator::Decrement },
			{ "++", ShaderRepresentation::UnaryOperator::Increment }
		};

		auto it = unaryOperatorMap.find(opStr);
		if (it != unaryOperatorMap.end())
		{
			return it->second;
		}
		else
		{
			throw std::runtime_error("Unknown unary operator: " + opStr);
		}
	}


	ShaderRepresentation::SymbolBody Parser::_composeSymbolBody(const SymbolBodyInfo& p_symbolBodyInfo)
	{
		ShaderRepresentation::SymbolBody result;

		for (const auto& statementVariant : p_symbolBodyInfo.statements)
		{
			auto statement = _composeStatement(statementVariant);
			result.statements.push_back(std::move(statement));
		}

		return result;
	}

	std::shared_ptr<ShaderRepresentation::Expression> Parser::_composeUnaryExpression(const UnaryExpressionInfo& p_unaryExpressionInfo)
	{
		auto expression = std::make_shared<ShaderRepresentation::UnaryExpression>();

		expression->op = _stringToUnaryOperator(p_unaryExpressionInfo.operatorToken.content);
		expression->operand = _composeExpression(*p_unaryExpressionInfo.operand);

		return expression;
	}
	std::shared_ptr<ShaderRepresentation::Expression> Parser::_composeUnaryExpression(const PostfixExpressionInfo& p_postfixExpressionInfo)
	{
		auto expression = std::make_shared<ShaderRepresentation::UnaryExpression>();

		expression->op = _stringToUnaryOperator(p_postfixExpressionInfo.operatorToken.content);
		expression->operand = _composeExpression(*p_postfixExpressionInfo.operand);

		return expression;
	}

	std::shared_ptr<ShaderRepresentation::Expression> Parser::_composeFunctionCallExpression(const FunctionCallExpressionInfo& p_functionCallExpressionInfo)
	{
		auto expression = std::make_shared<ShaderRepresentation::FunctionCallExpression>();

		for (const auto& nspace : p_functionCallExpressionInfo.namespacePath)
		{
			expression->functionName += nspace.content + "_";
		}
		expression->functionName += p_functionCallExpressionInfo.functionName.content;

		for (const auto& arg : p_functionCallExpressionInfo.arguments)
		{
			expression->arguments.push_back(_composeExpression(*arg));
		}

		return expression;
	}

	std::shared_ptr<ShaderRepresentation::Expression> Parser::_composeMemberAccessExpression(const MemberAccessExpressionInfo& p_memberAccessExpressionInfo)
	{
		auto expression = std::make_shared<ShaderRepresentation::MemberAccessExpression>();

		expression->object = _composeExpression(*p_memberAccessExpressionInfo.object);
		expression->memberName = p_memberAccessExpressionInfo.memberName.content;

		return expression;
	}

	std::shared_ptr<ShaderRepresentation::Expression> Parser::_composeArrayAccessExpression(const ArrayAccessExpressionInfo& p_arrayAccessExpressionInfo)
	{
		auto expression = std::make_shared<ShaderRepresentation::ArrayAccessExpression>();

		expression->array = _composeExpression(*p_arrayAccessExpressionInfo.array);
		expression->index = _composeExpression(*p_arrayAccessExpressionInfo.index);

		return expression;
	}

	std::shared_ptr<ShaderRepresentation::Statement> Parser::_composeReturnStatement(const ReturnStatementInfo& p_returnStatementInfo)
	{
		auto statement = std::make_shared<ShaderRepresentation::ReturnStatement>();

		if (p_returnStatementInfo.expression)
		{
			statement->expression = _composeExpression(*p_returnStatementInfo.expression);
		}

		return statement;
	}

	std::shared_ptr<ShaderRepresentation::Statement> Parser::_composeDiscardStatement(const DiscardStatementInfo& p_discardStatementInfo)
	{
		return std::make_shared<ShaderRepresentation::DiscardStatement>();
	}

	std::shared_ptr<ShaderRepresentation::Statement> Parser::_composeIfStatement(const IfStatementInfo& p_ifStatementInfo)
	{
		auto statement = std::make_shared<ShaderRepresentation::IfStatement>();

		for (const auto& branchInfo : p_ifStatementInfo.branches)
		{
			ShaderRepresentation::IfStatement::ConditionalBranch branch;

			branch.condition = _composeExpression(*branchInfo.condition);
			branch.body = _composeSymbolBody(branchInfo.body);

			statement->branches.push_back(std::move(branch));
		}

		statement->elseBody = _composeSymbolBody(p_ifStatementInfo.elseBody);

		return statement;
	}

	std::shared_ptr<ShaderRepresentation::Statement> Parser::_composeWhileStatement(const WhileStatementInfo& p_whileStatementInfo)
	{
		auto statement = std::make_shared<ShaderRepresentation::WhileStatement>();

		statement->condition = _composeExpression(*(p_whileStatementInfo.loop.condition));
		statement->body = _composeSymbolBody(p_whileStatementInfo.loop.body);

		return statement;
	}

	std::shared_ptr<ShaderRepresentation::Statement> Parser::_composeForStatement(const ForStatementInfo& p_forStatementInfo)
	{
		auto statement = std::make_shared<ShaderRepresentation::ForStatement>();

		if (p_forStatementInfo.initializer)
		{
			statement->initializer = _composeStatement(*p_forStatementInfo.initializer);
		}

		if (p_forStatementInfo.condition)
		{
			statement->condition = _composeExpression(*p_forStatementInfo.condition);
		}

		if (p_forStatementInfo.increment)
		{
			statement->increment = _composeExpression(*p_forStatementInfo.increment);
		}

		statement->body = _composeSymbolBody(p_forStatementInfo.body);

		return statement;
	}

	std::shared_ptr<ShaderRepresentation::Statement> Parser::_composeRaiseExceptionStatement(const RaiseExceptionStatementInfo& p_raiseExceptionStatementInfo)
	{
		auto statement = std::make_shared<ShaderRepresentation::RaiseExceptionStatement>();

		return statement;
	}

	std::shared_ptr<ShaderRepresentation::Statement> Parser::_composeCompoundStatement(const CompoundStatementInfo& p_compoundStatementInfo)
	{
		auto statement = std::make_shared<ShaderRepresentation::CompoundStatement>();

		statement->body = _composeSymbolBody(p_compoundStatementInfo.body);

		return statement;
	}

	std::shared_ptr<ShaderRepresentation::Statement> Parser::_composeStatement(const StatementInfo& p_statementInfo)
	{
		std::shared_ptr<ShaderRepresentation::Statement> result = nullptr;

		switch (p_statementInfo.index())
		{
		case 0: // VariableDeclarationStatementInfo
			result = _composeVariableDeclarationStatement(std::get<0>(p_statementInfo));
			break;
		case 1: // ExpressionStatementInfo
			result = _composeExpressionStatement(std::get<1>(p_statementInfo));
			break;
		case 2: // AssignmentStatementInfo
			result = _composeAssignmentStatement(std::get<2>(p_statementInfo));
			break;
		case 3: // ReturnStatementInfo
			result = _composeReturnStatement(std::get<3>(p_statementInfo));
			break;
		case 4: // DiscardStatementInfo
			result = _composeDiscardStatement(std::get<4>(p_statementInfo));
			break;
		case 5: // IfStatementInfo
			result = _composeIfStatement(std::get<5>(p_statementInfo));
			break;
		case 6: // WhileStatementInfo
			result = _composeWhileStatement(std::get<6>(p_statementInfo));
			break;
		case 7: // ForStatementInfo
			result = _composeForStatement(std::get<7>(p_statementInfo));
			break;
		case 8: // RaiseExceptionStatementInfo
			result = _composeRaiseExceptionStatement(std::get<8>(p_statementInfo));
			break;
		case 9: // CompoundStatementInfo
			result = _composeCompoundStatement(std::get<9>(p_statementInfo));
			break;
		default:
			// Handle unexpected cases if necessary
			break;
		}

		return result;
	}

	std::shared_ptr<ShaderRepresentation::Statement> Parser::_composeVariableDeclarationStatement(const VariableDeclarationStatementInfo& p_variableDeclarationStatementInfo)
	{
		auto statement = std::make_shared<ShaderRepresentation::VariableDeclarationStatement>();

		statement->variable = _composeVariable(p_variableDeclarationStatementInfo.variable);

		if (p_variableDeclarationStatementInfo.initializer)
		{
			statement->initializer = _composeExpression(*p_variableDeclarationStatementInfo.initializer);
		}

		return statement;
	}

	std::shared_ptr<ShaderRepresentation::Statement> Parser::_composeExpressionStatement(const ExpressionStatementInfo& p_expressionStatementInfo)
	{
		auto statement = std::make_shared<ShaderRepresentation::ExpressionStatement>();

		statement->expression = _composeExpression(*p_expressionStatementInfo.expression);

		return statement;
	}

	std::shared_ptr<ShaderRepresentation::Statement> Parser::_composeAssignmentStatement(const AssignmentStatementInfo& p_assignmentStatementInfo)
	{
		auto statement = std::make_shared<ShaderRepresentation::AssignmentStatement>();

		statement->target = _composeExpression(*p_assignmentStatementInfo.target);
		statement->op = _stringToAssignatorOperator(p_assignmentStatementInfo.operatorToken.content);
		statement->value = _composeExpression(*p_assignmentStatementInfo.value);

		return statement;
	}

	std::shared_ptr<ShaderRepresentation::Expression> Parser::_composeExpression(const ExpressionInfo& p_expressionInfo)
	{
		std::shared_ptr<ShaderRepresentation::Expression> result = nullptr;

		switch (p_expressionInfo.index())
		{
		case 0:
		{
			result = _composeLiteralExpression(std::get<LiteralExpressionInfo>(p_expressionInfo));
			break;
		}
		case 1:
		{
			result = _composeVariableExpression(std::get<VariableExpressionInfo>(p_expressionInfo));
			break;
		}
		case 2:
		{
			result = _composeBinaryExpression(std::get<BinaryExpressionInfo>(p_expressionInfo));
			break;
		}
		case 3:
		{
			result = _composeUnaryExpression(std::get<UnaryExpressionInfo>(p_expressionInfo));
			break;
		}
		case 4:
		{
			result = _composeUnaryExpression(std::get<PostfixExpressionInfo>(p_expressionInfo));
			break;
		}
		case 5:
		{
			result = _composeFunctionCallExpression(std::get<FunctionCallExpressionInfo>(p_expressionInfo));
			break;
		}
		case 6:
		{
			result = _composeMemberAccessExpression(std::get<MemberAccessExpressionInfo>(p_expressionInfo));
			break;
		}
		case 7:
		{
			result = _composeArrayAccessExpression(std::get<ArrayAccessExpressionInfo>(p_expressionInfo));
			break;
		}
		default:
			break;
		}

		return result;
	}

	std::shared_ptr<ShaderRepresentation::Expression> Parser::_composeLiteralExpression(const LiteralExpressionInfo& p_literalExpressionInfo)
	{
		auto expression = std::make_shared<ShaderRepresentation::LiteralExpression>();

		expression->value = p_literalExpressionInfo.value.content;

		return expression;
	}

	std::shared_ptr<ShaderRepresentation::Expression> Parser::_composeVariableExpression(const VariableExpressionInfo& p_variableExpressionInfo)
	{
		auto expression = std::make_shared<ShaderRepresentation::VariableExpression>();

		for (const auto& nspace : p_variableExpressionInfo.namespacePath)
		{
			expression->variableName += nspace.content + "_";
		}
		expression->variableName += p_variableExpressionInfo.variableName.content;

		return expression;
	}

	std::shared_ptr<ShaderRepresentation::Expression> Parser::_composeBinaryExpression(const BinaryExpressionInfo& p_binaryExpressionInfo)
	{
		auto expression = std::make_shared<ShaderRepresentation::BinaryExpression>();

		expression->left = _composeExpression(*p_binaryExpressionInfo.left);
		expression->op = _stringToOperator(p_binaryExpressionInfo.operatorToken.content);
		expression->right = _composeExpression(*p_binaryExpressionInfo.right);

		return expression;
	}
}
