#include "printer.hpp"

#include <iostream>
#include <string>

namespace 
{
	void printIndentation(size_t p_tabulationSize)
	{
		std::cout << std::string(p_tabulationSize * 4, ' ');
	}

	void printTypeInfo(const Lumina::TypeInfo& p_toPrint, size_t p_tabulationSize)
	{
		printIndentation(p_tabulationSize);
		std::cout << "TypeInfo: ";
		for (const auto& token : p_toPrint.nspace)
		{
			std::cout << token.content;
			std::cout << "::";
		}
		std::cout << p_toPrint.value.content;
		std::cout << "\n";
	}

	void printExpressionTypeInfo(const Lumina::ExpressionTypeInfo& p_toPrint, size_t p_tabulationSize)
	{
		printIndentation(p_tabulationSize);
		std::cout << "ExpressionType: ";
		for (const auto& token : p_toPrint.type.nspace)
		{
			std::cout << token.content;
			std::cout << "::";
		}
		std::cout << p_toPrint.type.value.content;

		for (const auto& size : p_toPrint.arraySizes.dims)
		{
			std::cout << "[" << size << "]";
		}
		std::cout << "\n";
	}

	void printNameInfo(const Lumina::NameInfo& p_toPrint, size_t p_tabulationSize)
	{
		printIndentation(p_tabulationSize);
		std::cout << "NameInfo: " << p_toPrint.value.content << "\n";
	}

	void printArraySizeInfo(const Lumina::ArraySizeInfo& p_toPrint, size_t p_tabulationSize)
	{
		if (p_toPrint.dims.size() != 0)
		{
			printIndentation(p_tabulationSize);
			std::cout << "ArraySizeInfo: ";
			for (const auto& dim : p_toPrint.dims)
			{
				std::cout << "[" << dim.content << "]";
			}
			std::cout << "\n";
		}
	}

	void printVariableInfo(const Lumina::VariableInfo& p_toPrint, size_t p_tabulationSize)
	{
		printIndentation(p_tabulationSize);
		std::cout << "VariableInfo:\n";
		printTypeInfo(p_toPrint.type, p_tabulationSize + 1);
		printNameInfo(p_toPrint.name, p_tabulationSize + 1);
		printArraySizeInfo(p_toPrint.arraySizes, p_tabulationSize + 1);
	}

	void printTextureInfo(const Lumina::TextureInfo& p_toPrint, size_t p_tabulationSize)
	{
		printIndentation(p_tabulationSize);
		std::cout << "TextureInfo:\n";
		printNameInfo(p_toPrint.name, p_tabulationSize + 1);
		printArraySizeInfo(p_toPrint.arraySizes, p_tabulationSize + 1);
	}

	void printParameterInfo(const Lumina::ParameterInfo& p_toPrint, size_t p_tabulationSize)
	{
		printIndentation(p_tabulationSize);
		std::cout << "ParameterInfo:\n";
		printTypeInfo(p_toPrint.type, p_tabulationSize + 1);
		std::cout << "IsReference: " << (p_toPrint.isReference ? "true" : "false") << "\n";
		printNameInfo(p_toPrint.name, p_tabulationSize + 1);
		printArraySizeInfo(p_toPrint.arraySizes, p_tabulationSize + 1);
	}

	void printSymbolBodyInfo(const Lumina::SymbolBodyInfo& p_toPrint, size_t p_tabulationSize);
	void printExpression(const Lumina::ExpressionInfo& expr, size_t p_tabulationSize);

	void printLiteralExpression(const Lumina::LiteralExpressionInfo& expr, size_t p_tabulationSize)
	{
		printIndentation(p_tabulationSize);
		std::cout << "LiteralExpression: " << expr.value.content << "\n";
	}

	void printVariableExpression(const Lumina::VariableExpressionInfo& expr, size_t p_tabulationSize)
	{
		printIndentation(p_tabulationSize);
		std::cout << "VariableExpression: ";
		for (const auto& token : expr.namespacePath)
		{
			std::cout << token.content << "::";
		}
		std::cout << expr.variableName.content << "\n";
	}

	void printBinaryExpression(const Lumina::BinaryExpressionInfo& expr, size_t p_tabulationSize)
	{
		printIndentation(p_tabulationSize);
		std::cout << "BinaryExpression:\n";
		printExpression(*expr.left, p_tabulationSize + 1);
		printIndentation(p_tabulationSize + 1);
		std::cout << "Operator: " << expr.operatorToken.content << "\n";
		printExpression(*expr.right, p_tabulationSize + 1);
	}

	void printUnaryExpression(const Lumina::UnaryExpressionInfo& expr, size_t p_tabulationSize)
	{
		printIndentation(p_tabulationSize);
		std::cout << "UnaryExpression:\n";
		printIndentation(p_tabulationSize + 1);
		std::cout << "Operator: " << expr.operatorToken.content << "\n";
		printExpression(*expr.operand, p_tabulationSize + 1);
	}

	void printPostfixExpression(const Lumina::PostfixExpressionInfo& expr, size_t p_tabulationSize)
	{
		printIndentation(p_tabulationSize);
		std::cout << "PostfixExpression:\n";
		printExpression(*expr.operand, p_tabulationSize + 1);
		printIndentation(p_tabulationSize + 1);
		std::cout << "Operator: " << expr.operatorToken.content << "\n";
	}

	void printFunctionCallExpression(const Lumina::FunctionCallExpressionInfo& expr, size_t p_tabulationSize)
	{
		printIndentation(p_tabulationSize);
		std::cout << "FunctionCallExpression: ";
		for (const auto& token : expr.namespacePath)
		{
			std::cout << token.content << "::";
		}
		std::cout << expr.functionName.content << "\n";
		printIndentation(p_tabulationSize + 1);
		std::cout << "Arguments:\n";
		for (const auto& arg : expr.arguments)
		{
			printExpression(*arg, p_tabulationSize + 2);
		}
	}

	void printMemberAccessExpression(const Lumina::MemberAccessExpressionInfo& expr, size_t p_tabulationSize)
	{
		printIndentation(p_tabulationSize);
		std::cout << "MemberAccessExpression:\n";
		printExpression(*expr.object, p_tabulationSize + 1);
		printIndentation(p_tabulationSize + 1);
		std::cout << "Member: " << expr.memberName.content << "\n";
	}

	void printArrayAccessExpression(const Lumina::ArrayAccessExpressionInfo& expr, size_t p_tabulationSize)
	{
		printIndentation(p_tabulationSize);
		std::cout << "ArrayAccessExpression:\n";
		printExpression(*expr.array, p_tabulationSize + 1);
		printExpression(*expr.index, p_tabulationSize + 1);
	}

	void printExpression(const Lumina::ExpressionInfo& expr, size_t p_tabulationSize)
	{
		std::visit([&](const auto& expression) {
			using T = std::decay_t<decltype(expression)>;
			if constexpr (std::is_same_v<T, Lumina::LiteralExpressionInfo>)
				printLiteralExpression(expression, p_tabulationSize);
			else if constexpr (std::is_same_v<T, Lumina::VariableExpressionInfo>)
				printVariableExpression(expression, p_tabulationSize);
			else if constexpr (std::is_same_v<T, Lumina::BinaryExpressionInfo>)
				printBinaryExpression(expression, p_tabulationSize);
			else if constexpr (std::is_same_v<T, Lumina::UnaryExpressionInfo>)
				printUnaryExpression(expression, p_tabulationSize);
			else if constexpr (std::is_same_v<T, Lumina::PostfixExpressionInfo>)
				printPostfixExpression(expression, p_tabulationSize);
			else if constexpr (std::is_same_v<T, Lumina::FunctionCallExpressionInfo>)
				printFunctionCallExpression(expression, p_tabulationSize);
			else if constexpr (std::is_same_v<T, Lumina::MemberAccessExpressionInfo>)
				printMemberAccessExpression(expression, p_tabulationSize);
			else if constexpr (std::is_same_v<T, Lumina::ArrayAccessExpressionInfo>)
				printArrayAccessExpression(expression, p_tabulationSize);
			}, expr);
	}

	void printVariableDeclarationStatementInfo(const Lumina::VariableDeclarationStatementInfo& p_toPrint, size_t p_tabulationSize)
	{
		printIndentation(p_tabulationSize);
		std::cout << "VariableDeclarationStatement:\n";
		printVariableInfo(p_toPrint.variable, p_tabulationSize + 1);
		if (p_toPrint.initializer)
		{
			printIndentation(p_tabulationSize + 1);
			std::cout << "Initializer:\n";
			printExpression(*p_toPrint.initializer, p_tabulationSize + 2);
		}
	}

	void printExpressionStatementInfo(const Lumina::ExpressionStatementInfo& p_toPrint, size_t p_tabulationSize)
	{
		printIndentation(p_tabulationSize);
		std::cout << "ExpressionStatement:\n";
		printExpression(*p_toPrint.expression, p_tabulationSize + 1);
	}

	void printAssignmentStatementInfo(const Lumina::AssignmentStatementInfo& p_toPrint, size_t p_tabulationSize)
	{
		printIndentation(p_tabulationSize);
		std::cout << "AssignmentStatement:\n";
		printExpression(*p_toPrint.target, p_tabulationSize + 1);
		printIndentation(p_tabulationSize + 1);
		std::cout << "Operator: " << p_toPrint.operatorToken.content << "\n";
		printExpression(*p_toPrint.value, p_tabulationSize + 1);
	}

	void printReturnStatementInfo(const Lumina::ReturnStatementInfo& p_toPrint, size_t p_tabulationSize)
	{
		printIndentation(p_tabulationSize);
		std::cout << "ReturnStatement:\n";
		if (p_toPrint.expression)
		{
			printExpression(*p_toPrint.expression, p_tabulationSize + 1);
		}
	}

	void printDiscardStatementInfo(const Lumina::DiscardStatementInfo&, size_t p_tabulationSize)
	{
		printIndentation(p_tabulationSize);
		std::cout << "DiscardStatement\n";
	}

	void printIfStatementInfo(const Lumina::IfStatementInfo& p_toPrint, size_t p_tabulationSize)
	{
		printIndentation(p_tabulationSize);
		std::cout << "IfStatement:\n";
		for (const auto& branch : p_toPrint.branches)
		{
			printIndentation(p_tabulationSize + 1);
			std::cout << "Condition:\n";
			printExpression(*branch.condition, p_tabulationSize + 2);
			printSymbolBodyInfo(branch.body, p_tabulationSize + 1);
		}
	}

	void printWhileStatementInfo(const Lumina::WhileStatementInfo& p_toPrint, size_t p_tabulationSize)
	{
		printIndentation(p_tabulationSize);
		std::cout << "WhileStatement:\n";
		printExpression(*p_toPrint.loop.condition, p_tabulationSize + 1);
		printSymbolBodyInfo(p_toPrint.loop.body, p_tabulationSize + 1);
	}

	void printStatementInfo(const Lumina::StatementInfo& p_toPrint, size_t p_tabulationSize);

	void printForStatementInfo(const Lumina::ForStatementInfo& p_toPrint, size_t p_tabulationSize)
	{
		printIndentation(p_tabulationSize);
		std::cout << "ForStatement:\n";
		printIndentation(p_tabulationSize + 1);
		std::cout << "Initializer:\n";
		if (p_toPrint.initializer)
		{
			printStatementInfo(*p_toPrint.initializer, p_tabulationSize + 2);
		}
		printIndentation(p_tabulationSize + 1);
		std::cout << "Condition:\n";
		if (p_toPrint.condition)
		{
			printExpression(*p_toPrint.condition, p_tabulationSize + 2);
		}
		printIndentation(p_tabulationSize + 1);
		std::cout << "Increment:\n";
		if (p_toPrint.increment)
		{
			printExpression(*p_toPrint.increment, p_tabulationSize + 2);
		}
		printSymbolBodyInfo(p_toPrint.body, p_tabulationSize + 1);
	}

	void printRaiseExceptionStatementInfo(const Lumina::RaiseExceptionStatementInfo& p_toPrint, size_t p_tabulationSize)
	{
		printIndentation(p_tabulationSize);
		std::cout << "RaiseExceptionStatement:\n";
		printExpression(*p_toPrint.functionCall, p_tabulationSize + 1);
	}

	void printCompoundStatementInfo(const Lumina::CompoundStatementInfo& p_toPrint, size_t p_tabulationSize)
	{
		printIndentation(p_tabulationSize);
		std::cout << "CompoundStatement:\n";
		printSymbolBodyInfo(p_toPrint.body, p_tabulationSize + 1);
	}

	void printStatementInfo(const Lumina::StatementInfo& p_toPrint, size_t p_tabulationSize)
	{
		std::visit([&](const auto& p_toPrint) {
			using T = std::decay_t<decltype(p_toPrint)>;
			if constexpr (std::is_same_v<T, Lumina::VariableDeclarationStatementInfo>)
				printVariableDeclarationStatementInfo(p_toPrint, p_tabulationSize);
			else if constexpr (std::is_same_v<T, Lumina::ExpressionStatementInfo>)
				printExpressionStatementInfo(p_toPrint, p_tabulationSize);
			else if constexpr (std::is_same_v<T, Lumina::AssignmentStatementInfo>)
				printAssignmentStatementInfo(p_toPrint, p_tabulationSize);
			else if constexpr (std::is_same_v<T, Lumina::ReturnStatementInfo>)
				printReturnStatementInfo(p_toPrint, p_tabulationSize);
			else if constexpr (std::is_same_v<T, Lumina::DiscardStatementInfo>)
				printDiscardStatementInfo(p_toPrint, p_tabulationSize);
			else if constexpr (std::is_same_v<T, Lumina::IfStatementInfo>)
				printIfStatementInfo(p_toPrint, p_tabulationSize);
			else if constexpr (std::is_same_v<T, Lumina::WhileStatementInfo>)
				printWhileStatementInfo(p_toPrint, p_tabulationSize);
			else if constexpr (std::is_same_v<T, Lumina::ForStatementInfo>)
				printForStatementInfo(p_toPrint, p_tabulationSize);
			else if constexpr (std::is_same_v<T, Lumina::RaiseExceptionStatementInfo>)
				printRaiseExceptionStatementInfo(p_toPrint, p_tabulationSize);
			else if constexpr (std::is_same_v<T, Lumina::CompoundStatementInfo>)
				printCompoundStatementInfo(p_toPrint, p_tabulationSize);
			}, p_toPrint);
	}


	void printSymbolBodyInfo(const Lumina::SymbolBodyInfo& p_toPrint, size_t p_tabulationSize)
	{
		printIndentation(p_tabulationSize);
		std::cout << "Body:\n";
		printIndentation(p_tabulationSize);
		std::cout << "{\n";
		for (const auto& statement : p_toPrint.statements)
		{
			printStatementInfo(statement, p_tabulationSize + 1);
		}
		printIndentation(p_tabulationSize);
		std::cout << "}\n";
	}


	void printFunctionInfo(const Lumina::FunctionInfo& p_toPrint, size_t p_tabulationSize)
	{
		printIndentation(p_tabulationSize);
		std::cout << "FunctionInfo:\n";
		printExpressionTypeInfo(p_toPrint.returnType, p_tabulationSize + 1);
		printNameInfo(p_toPrint.name, p_tabulationSize + 1);
		printIndentation(p_tabulationSize + 1);
		std::cout << "Parameters:\n";
		for (const auto& param : p_toPrint.parameters)
		{
			printParameterInfo(param, p_tabulationSize + 2);
		}
		printSymbolBodyInfo(p_toPrint.body, p_tabulationSize + 1);
	}

	void printOperatorInfo(const Lumina::OperatorInfo& p_toPrint, size_t p_tabulationSize)
	{
		printIndentation(p_tabulationSize);
		std::cout << "OperatorInfo:\n";
		printExpressionTypeInfo(p_toPrint.returnType, p_tabulationSize + 1);
		printIndentation(p_tabulationSize);
		std::cout << "Operator: " << p_toPrint.opeType.content << "\n";
		printIndentation(p_tabulationSize + 1);
		std::cout << "Parameters:\n";
		for (const auto& param : p_toPrint.parameters)
		{
			printParameterInfo(param, p_tabulationSize + 2);
		}
		printSymbolBodyInfo(p_toPrint.body, p_tabulationSize + 1);
	}

	void printBlockInfo(const Lumina::BlockInfo& p_toPrint, size_t p_tabulationSize)
	{
		printIndentation(p_tabulationSize);
		std::cout << "BlockInfo:\n";
		printNameInfo(p_toPrint.name, p_tabulationSize + 1);
		printIndentation(p_tabulationSize + 1);
		std::cout << "Variables:\n";
		for (const auto& attribute : p_toPrint.attributes)
		{
			printVariableInfo(attribute, p_tabulationSize + 2);
		}
		printIndentation(p_tabulationSize + 1);
		std::cout << "Methods:\n";
		for (const auto& methods : p_toPrint.methodInfos)
		{
			for (const auto& method : methods.second)
			{
				printFunctionInfo(method, p_tabulationSize + 2);
			}
		}
		printIndentation(p_tabulationSize + 1);
		std::cout << "Operators:\n";
		for (const auto& operators : p_toPrint.operatorInfos)
		{
			for (const auto& ope : operators.second)
			{
				printOperatorInfo(ope, p_tabulationSize + 2);
			}
		}
	}

	void printNamespaceInfo(const Lumina::NamespaceInfo& p_toPrint, size_t p_tabulationSize)
	{
		printIndentation(p_tabulationSize);
		std::cout << "NamespaceInfo:\n";
		printNameInfo(p_toPrint.name, p_tabulationSize + 1);
		for (const auto& block : p_toPrint.structureBlocks)
		{
			printBlockInfo(block, p_tabulationSize + 1);
		}
		for (const auto& block : p_toPrint.attributeBlocks)
		{
			printBlockInfo(block, p_tabulationSize + 1);
		}
		for (const auto& block : p_toPrint.constantBlocks)
		{
			printBlockInfo(block, p_tabulationSize + 1);
		}
	}

	void printPipelineFlowInfo(const Lumina::PipelineFlowInfo& p_toPrint, size_t p_tabulationSize)
	{
		printIndentation(p_tabulationSize);
		std::cout << "PipelineFlowInfo:\n";
		printIndentation(p_tabulationSize + 1);
		std::cout << "Input: " << p_toPrint.input.content << "\n";
		printIndentation(p_tabulationSize + 1);
		std::cout << "Output: " << p_toPrint.output.content << "\n";
		printVariableInfo(p_toPrint.variable, p_tabulationSize + 1);
	}

	void printPipelinePassInfo(const Lumina::PipelinePassInfo& p_toPrint, size_t p_tabulationSize)
	{
		printIndentation(p_tabulationSize);
		std::cout << "PipelinePassInfo:\n";
		printIndentation(p_tabulationSize + 1);
		std::cout << "Pass: " << p_toPrint.name.content << "\n";
		printSymbolBodyInfo(p_toPrint.body, p_tabulationSize + 1);

	}

	void printShaderInfo(const Lumina::ShaderInfo& p_toPrint, size_t p_tabulationSize)
	{
		std::cout << "ShaderInfo:\n";
		for (const auto& flow : p_toPrint.pipelineFlows)
		{
			printPipelineFlowInfo(flow, p_tabulationSize + 1);
		}
		for (const auto& pass : p_toPrint.pipelinePasses)
		{
			printPipelinePassInfo(pass, p_tabulationSize + 1);
		}
		for (const auto& nspace : p_toPrint.namespaces)
		{
			printNamespaceInfo(nspace, p_tabulationSize + 1);
		}
	}
}

namespace Lumina
{
	void Printer::print(const ShaderInfo& p_shaderInfo)
	{
		printShaderInfo(p_shaderInfo, 0);
	}
}