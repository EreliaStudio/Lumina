#pragma once

#include "utils.hpp"
#include "tokenizer.hpp"
#include "expected.hpp"

#include <iostream>

#include "lexer.hpp"
#include "parser.hpp"
#include "compiler.hpp"

#include <iostream>
#include <string>
#include "shader_info.hpp"

namespace Lumina
{
    void printIndentation(size_t p_tabulationSize)
    {
        std::cout << std::string(p_tabulationSize * 4, ' ');
    }

    void printTypeInfo(const TypeInfo& p_toPrint, size_t p_tabulationSize)
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

    void printNameInfo(const NameInfo& p_toPrint, size_t p_tabulationSize)
    {
        printIndentation(p_tabulationSize);
        std::cout << "NameInfo: " << p_toPrint.value.content << "\n";
    }

    void printArraySizeInfo(const ArraySizeInfo& p_toPrint, size_t p_tabulationSize)
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

    void printVariableInfo(const VariableInfo& p_toPrint, size_t p_tabulationSize)
    {
        printIndentation(p_tabulationSize);
        std::cout << "VariableInfo:\n";
        printTypeInfo(p_toPrint.type, p_tabulationSize + 1);
        printNameInfo(p_toPrint.name, p_tabulationSize + 1);
        printArraySizeInfo(p_toPrint.arraySizes, p_tabulationSize + 1);
    }

    void printTextureInfo(const TextureInfo& p_toPrint, size_t p_tabulationSize)
    {
        printIndentation(p_tabulationSize);
        std::cout << "TextureInfo:\n";
        printNameInfo(p_toPrint.name, p_tabulationSize + 1);
        printArraySizeInfo(p_toPrint.arraySizes, p_tabulationSize + 1);
    }

    void printParameterInfo(const ParameterInfo& p_toPrint, size_t p_tabulationSize)
    {
        printIndentation(p_tabulationSize);
        std::cout << "ParameterInfo:\n";
        printTypeInfo(p_toPrint.type, p_tabulationSize + 1);
        std::cout << "IsReference: " << (p_toPrint.isReference ? "true" : "false") << "\n";
        printNameInfo(p_toPrint.name, p_tabulationSize + 1);
        printArraySizeInfo(p_toPrint.arraySizes, p_tabulationSize + 1);
    }

    void printSymbolBodyInfo(const SymbolBodyInfo& p_toPrint, size_t p_tabulationSize);
    void printExpression(const Expression& expr, size_t p_tabulationSize);

    void printLiteralExpression(const LiteralExpression& expr, size_t p_tabulationSize)
    {
        printIndentation(p_tabulationSize);
        std::cout << "LiteralExpression: " << expr.value.content << "\n";
    }

    void printVariableExpression(const VariableExpression& expr, size_t p_tabulationSize)
    {
        printIndentation(p_tabulationSize);
        std::cout << "VariableExpression: ";
        for (const auto& token : expr.namespacePath)
        {
            std::cout << token.content << "::";
        }
        std::cout << expr.variableName.content << "\n";
    }

    void printBinaryExpression(const BinaryExpression& expr, size_t p_tabulationSize)
    {
        printIndentation(p_tabulationSize);
        std::cout << "BinaryExpression:\n";
        printExpression(*expr.left, p_tabulationSize + 1);
        printIndentation(p_tabulationSize + 1);
        std::cout << "Operator: " << expr.operatorToken.content << "\n";
        printExpression(*expr.right, p_tabulationSize + 1);
    }

    void printUnaryExpression(const UnaryExpression& expr, size_t p_tabulationSize)
    {
        printIndentation(p_tabulationSize);
        std::cout << "UnaryExpression:\n";
        printIndentation(p_tabulationSize + 1);
        std::cout << "Operator: " << expr.operatorToken.content << "\n";
        printExpression(*expr.operand, p_tabulationSize + 1);
    }

    void printPostfixExpression(const PostfixExpression& expr, size_t p_tabulationSize)
    {
        printIndentation(p_tabulationSize);
        std::cout << "PostfixExpression:\n";
        printExpression(*expr.operand, p_tabulationSize + 1);
        printIndentation(p_tabulationSize + 1);
        std::cout << "Operator: " << expr.operatorToken.content << "\n";
    }

    void printFunctionCallExpression(const FunctionCallExpression& expr, size_t p_tabulationSize)
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

    void printMemberAccessExpression(const MemberAccessExpression& expr, size_t p_tabulationSize)
    {
        printIndentation(p_tabulationSize);
        std::cout << "MemberAccessExpression:\n";
        printExpression(*expr.object, p_tabulationSize + 1);
        printIndentation(p_tabulationSize + 1);
        std::cout << "Member: " << expr.memberName.content << "\n";
    }

    void printArrayAccessExpression(const ArrayAccessExpression& expr, size_t p_tabulationSize)
    {
        printIndentation(p_tabulationSize);
        std::cout << "ArrayAccessExpression:\n";
        printExpression(*expr.array, p_tabulationSize + 1);
        printExpression(*expr.index, p_tabulationSize + 1);
    }

    void printCastExpression(const CastExpression& expr, size_t p_tabulationSize)
    {
        printIndentation(p_tabulationSize);
        std::cout << "CastExpression:\n";
        printTypeInfo(expr.targetType, p_tabulationSize + 1);
        printIndentation(p_tabulationSize + 1);
        std::cout << "Arguments:\n";
        for (const auto& arg : expr.arguments)
        {
            printExpression(*arg, p_tabulationSize + 2);
        }
    }

    void printExpression(const Expression& expr, size_t p_tabulationSize)
    {
        std::visit([&](const auto& expression) {
            using T = std::decay_t<decltype(expression)>;
            if constexpr (std::is_same_v<T, LiteralExpression>)
                printLiteralExpression(expression, p_tabulationSize);
            else if constexpr (std::is_same_v<T, VariableExpression>)
                printVariableExpression(expression, p_tabulationSize);
            else if constexpr (std::is_same_v<T, BinaryExpression>)
                printBinaryExpression(expression, p_tabulationSize);
            else if constexpr (std::is_same_v<T, UnaryExpression>)
                printUnaryExpression(expression, p_tabulationSize);
            else if constexpr (std::is_same_v<T, PostfixExpression>)
                printPostfixExpression(expression, p_tabulationSize);
            else if constexpr (std::is_same_v<T, FunctionCallExpression>)
                printFunctionCallExpression(expression, p_tabulationSize);
            else if constexpr (std::is_same_v<T, MemberAccessExpression>)
                printMemberAccessExpression(expression, p_tabulationSize);
            else if constexpr (std::is_same_v<T, ArrayAccessExpression>)
                printArrayAccessExpression(expression, p_tabulationSize);
            else if constexpr (std::is_same_v<T, CastExpression>)
                printCastExpression(expression, p_tabulationSize);
            }, expr);
    }

    void printVariableDeclarationStatement(const VariableDeclarationStatement& p_toPrint, size_t p_tabulationSize)
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

    void printExpressionStatement(const ExpressionStatement& p_toPrint, size_t p_tabulationSize)
    {
        printIndentation(p_tabulationSize);
        std::cout << "ExpressionStatement:\n";
        printExpression(*p_toPrint.expression, p_tabulationSize + 1);
    }

    void printAssignmentStatement(const AssignmentStatement& p_toPrint, size_t p_tabulationSize)
    {
        printIndentation(p_tabulationSize);
        std::cout << "AssignmentStatement:\n";
        printExpression(*p_toPrint.target, p_tabulationSize + 1);
        printIndentation(p_tabulationSize + 1);
        std::cout << "Operator: " << p_toPrint.operatorToken.content << "\n";
        printExpression(*p_toPrint.value, p_tabulationSize + 1);
    }

    void printReturnStatement(const ReturnStatement& p_toPrint, size_t p_tabulationSize)
    {
        printIndentation(p_tabulationSize);
        std::cout << "ReturnStatement:\n";
        if (p_toPrint.expression)
        {
            printExpression(*p_toPrint.expression, p_tabulationSize + 1);
        }
    }

    void printDiscardStatement(const DiscardStatement&, size_t p_tabulationSize)
    {
        printIndentation(p_tabulationSize);
        std::cout << "DiscardStatement\n";
    }

    void printIfStatement(const IfStatement& p_toPrint, size_t p_tabulationSize)
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

    void printWhileStatement(const WhileStatement& p_toPrint, size_t p_tabulationSize)
    {
        printIndentation(p_tabulationSize);
        std::cout << "WhileStatement:\n";
        printExpression(*p_toPrint.loop.condition, p_tabulationSize + 1);
        printSymbolBodyInfo(p_toPrint.loop.body, p_tabulationSize + 1);
    }

    void printStatement(const Statement& p_toPrint, size_t p_tabulationSize);

    void printForStatement(const ForStatement& p_toPrint, size_t p_tabulationSize)
    {
        printIndentation(p_tabulationSize);
        std::cout << "ForStatement:\n";
        printIndentation(p_tabulationSize + 1);
        std::cout << "Initializer:\n";
        if (p_toPrint.initializer)
        {
            printStatement(*p_toPrint.initializer, p_tabulationSize + 2);
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

    void printRaiseExceptionStatement(const RaiseExceptionStatement& p_toPrint, size_t p_tabulationSize)
    {
        printIndentation(p_tabulationSize);
        std::cout << "RaiseExceptionStatement:\n";
        printExpression(*p_toPrint.functionCall, p_tabulationSize + 1);
    }

    void printStatement(const Statement& p_toPrint, size_t p_tabulationSize)
    {
        std::visit([&](const auto& p_toPrint) {
            using T = std::decay_t<decltype(p_toPrint)>;
            if constexpr (std::is_same_v<T, VariableDeclarationStatement>)
                printVariableDeclarationStatement(p_toPrint, p_tabulationSize);
            else if constexpr (std::is_same_v<T, ExpressionStatement>)
                printExpressionStatement(p_toPrint, p_tabulationSize);
            else if constexpr (std::is_same_v<T, AssignmentStatement>)
                printAssignmentStatement(p_toPrint, p_tabulationSize);
            else if constexpr (std::is_same_v<T, ReturnStatement>)
                printReturnStatement(p_toPrint, p_tabulationSize);
            else if constexpr (std::is_same_v<T, DiscardStatement>)
                printDiscardStatement(p_toPrint, p_tabulationSize);
            else if constexpr (std::is_same_v<T, IfStatement>)
                printIfStatement(p_toPrint, p_tabulationSize);
            else if constexpr (std::is_same_v<T, WhileStatement>)
                printWhileStatement(p_toPrint, p_tabulationSize);
            else if constexpr (std::is_same_v<T, ForStatement>)
                printForStatement(p_toPrint, p_tabulationSize);
            else if constexpr (std::is_same_v<T, RaiseExceptionStatement>)
                printRaiseExceptionStatement(p_toPrint, p_tabulationSize);
            else if constexpr (std::is_same_v<T, CompoundStatement>)
                printCompoundStatement(p_toPrint, p_tabulationSize);
            }, p_toPrint);
    }

    void printCompoundStatement(const CompoundStatement& p_toPrint, size_t p_tabulationSize)
    {
        printIndentation(p_tabulationSize);
        std::cout << "CompoundStatement:\n";
        for (const auto& subStmt : p_toPrint.statements)
        {
            printStatement(subStmt, p_tabulationSize + 1);
        }
    }


    void printSymbolBodyInfo(const SymbolBodyInfo& p_toPrint, size_t p_tabulationSize)
    {
        printIndentation(p_tabulationSize);
        std::cout << "Body:\n";
        printIndentation(p_tabulationSize);
        std::cout << "{\n";
        for (const auto& statement : p_toPrint.statements)
        {
            printStatement(statement, p_tabulationSize + 1);
        }
        printIndentation(p_tabulationSize);
        std::cout << "}\n";
    }


    void printFunctionInfo(const FunctionInfo& p_toPrint, size_t p_tabulationSize)
    {
        printIndentation(p_tabulationSize);
        std::cout << "FunctionInfo:\n";
        printTypeInfo(p_toPrint.returnType.type, p_tabulationSize + 1);
        printNameInfo(p_toPrint.name, p_tabulationSize + 1);
        printIndentation(p_tabulationSize + 1);
        std::cout << "Parameters:\n";
        for (const auto& param : p_toPrint.parameters)
        {
            printParameterInfo(param, p_tabulationSize + 2);
        }
        printSymbolBodyInfo(p_toPrint.body, p_tabulationSize + 1);
    }

    void printOperatorInfo(const OperatorInfo& p_toPrint, size_t p_tabulationSize)
    {
        printIndentation(p_tabulationSize);
        std::cout << "OperatorInfo:\n";
        printTypeInfo(p_toPrint.returnType.type, p_tabulationSize + 1);
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

    void printBlockInfo(const BlockInfo& p_toPrint, size_t p_tabulationSize)
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

    void printNamespaceInfo(const NamespaceInfo& p_toPrint, size_t p_tabulationSize)
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

    void printPipelineFlowInfo(const PipelineFlowInfo& p_toPrint, size_t p_tabulationSize)
    {
        printIndentation(p_tabulationSize);
        std::cout << "PipelineFlowInfo:\n";
        printIndentation(p_tabulationSize + 1);
        std::cout << "Input: " << p_toPrint.input.content << "\n";
        printIndentation(p_tabulationSize + 1);
        std::cout << "Output: " << p_toPrint.output.content << "\n";
        printVariableInfo(p_toPrint.variable, p_tabulationSize + 1);
    }

    void printPipelinePassInfo(const PipelinePassInfo& p_toPrint, size_t p_tabulationSize)
    {
        printIndentation(p_tabulationSize);
        std::cout << "PipelinePassInfo:\n";
        printIndentation(p_tabulationSize + 1);
        std::cout << "Pass: " << p_toPrint.name.content << "\n";
        printSymbolBodyInfo(p_toPrint.body, p_tabulationSize + 1);
        
    }

    void printShaderInfo(const ShaderInfo& p_toPrint, size_t p_tabulationSize)
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
        printNamespaceInfo(p_toPrint.anonymNamespace, p_tabulationSize + 1);
    }
}

int main(int argc, char** argv)
{
	if (argc != 3)
	{
		std::cout << "Usage : " << argv[0] << " [path to your lumina shader code] [path to your compiled shader file]" << std::endl;
		return (0);
	}

	std::string rawCode = Lumina::readFileAsString(argv[1]);

	std::vector<Lumina::Token> tokens = Lumina::Tokenizer::tokenize(argv[1], rawCode);

	if (tokens.size() == 0)
	{
		std::cout << "Empty source file [" << argv[1] << "]" << std::endl;
		return (-1);
	}

	Lumina::Lexer::Product lexerProduct = Lumina::Lexer::lex(tokens);

	if (lexerProduct.errors.size() != 0)
	{
		for (const auto& error : lexerProduct.errors)
		{
			std::cerr << error.what() << std::endl;
		}
		return (-1);
	}

    printShaderInfo(lexerProduct.value, 0);

	Lumina::Parser::Product parserProduct = Lumina::Parser::parse(lexerProduct.value);

	if (parserProduct.errors.size() != 0)
	{
		for (const auto& error : parserProduct.errors)
		{
			std::cerr << error.what() << std::endl;
		}
		return (-1);
	}

	Lumina::ShaderImpl compilerProduct = Lumina::Compiler::compile(parserProduct.value);

	std::cout << "Shader : " << std::endl << compilerProduct << std::endl;

	return (0);
}
