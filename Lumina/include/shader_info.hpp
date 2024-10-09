#pragma once

#include <map>
#include <vector>
#include <variant>

#include "token.hpp"

namespace Lumina
{
	using NamespaceDesignation = std::vector<Lumina::Token>;

	struct TypeInfo
	{
		NamespaceDesignation nspace;
		Lumina::Token value;
	};

	struct NameInfo
	{
		Lumina::Token value;
	};

	struct ArraySizeInfo
	{
		std::vector<Lumina::Token> dims;
	};

	struct VariableInfo
	{
		TypeInfo type;
		NameInfo name;
		ArraySizeInfo arraySizes;
	};

	struct TextureInfo
	{
		NameInfo name;
		ArraySizeInfo arraySizes;
	};

	struct ParameterInfo
	{
		TypeInfo type;
		bool isReference;
		NameInfo name;
		ArraySizeInfo arraySizes;
	};

	//
	// ---------------------
	//

    // Forward declarations of expression structs
    struct LiteralExpression;
    struct VariableExpression;
    struct BinaryExpression;
    struct UnaryExpression;
    struct PostfixExpression;
    struct FunctionCallExpression;
    struct MemberAccessExpression;
    struct ArrayAccessExpression;
    struct CastExpression;

    using Expression = std::variant<
        LiteralExpression,
        VariableExpression,
        BinaryExpression,
        UnaryExpression,
        PostfixExpression,
        FunctionCallExpression,
        MemberAccessExpression,
        ArrayAccessExpression,
        CastExpression
    >;

    // Expression structs

    struct LiteralExpression
    {
        Token value;
    };

    struct VariableExpression
    {
        NamespaceDesignation namespacePath;
        Token variableName;
    };

    struct BinaryExpression
    {
        std::shared_ptr<Expression> left;
        Token operatorToken;
        std::shared_ptr<Expression> right;
    };

    struct UnaryExpression
    {
        Token operatorToken;
        std::shared_ptr<Expression> operand;
    };

    struct PostfixExpression
    {
        std::shared_ptr<Expression> operand;
        Token operatorToken;
    };

    struct FunctionCallExpression
    {
        NamespaceDesignation namespacePath;
        Token functionName;
        std::vector<std::shared_ptr<Expression>> arguments;
    };

    struct MemberAccessExpression
    {
        std::shared_ptr<Expression> object;
        Token memberName;
    };

    struct ArrayAccessExpression
    {
        std::shared_ptr<Expression> array;
        std::shared_ptr<Expression> index;
    };

    struct CastExpression
    {
        TypeInfo targetType;
        std::vector<std::shared_ptr<Expression>> arguments;
    };

    struct VariableDeclarationStatement;
    struct ExpressionStatement;
    struct AssignmentStatement;
    struct ReturnStatement;
    struct DiscardStatement;
    struct ConditionalBranch;
    struct IfStatement;
    struct WhileStatement;
    struct ForStatement;
    struct RaiseExceptionStatement;
    struct CompoundStatement;

    using Statement = std::variant<
        VariableDeclarationStatement,
        ExpressionStatement,
        AssignmentStatement,
        ReturnStatement,
        DiscardStatement,
        IfStatement,
        WhileStatement,
        ForStatement,
        RaiseExceptionStatement,
        CompoundStatement
    >;

    struct SymbolBodyInfo
    {
        std::vector<Statement> statements;
    };

    struct VariableDeclarationStatement
    {
        VariableInfo variable;
        std::shared_ptr<Expression> initializer;
    };

    struct ExpressionStatement
    {
        std::shared_ptr<Expression> expression;
    };

    struct AssignmentStatement
    {
        std::shared_ptr<Expression> target;
        Token operatorToken;
        std::shared_ptr<Expression> value;
    };

    struct ReturnStatement
    {
        std::shared_ptr<Expression> expression;
    };

    struct DiscardStatement
    {
    };

    struct ConditionalBranch
    {
        std::shared_ptr<Expression> condition;
        SymbolBodyInfo body;
    };

    struct IfStatement
    {
        std::vector<ConditionalBranch> branches;
        SymbolBodyInfo elseBody;
    };

    struct WhileStatement
    {
        ConditionalBranch loop;
    };

    struct ForStatement
    {
        std::shared_ptr<Statement> initializer;
        std::shared_ptr<Expression> condition;
        std::shared_ptr<Expression> increment;
        SymbolBodyInfo body;
    };

    struct RaiseExceptionStatement
    {
        std::shared_ptr<FunctionCallExpression> functionCall;
    };

    struct CompoundStatement
    {
        std::vector<Statement> statements;
    };

	//
	// ---------------------
	//

	struct ExpressionTypeInfo
	{
		TypeInfo type;
		ArraySizeInfo arraySizes;
	};

	struct FunctionInfo
	{
		ExpressionTypeInfo returnType;
		NameInfo name;
		std::vector<ParameterInfo> parameters;
		SymbolBodyInfo body;
	};

	struct OperatorInfo
	{
		ExpressionTypeInfo returnType;
		Lumina::Token opeType;
		std::vector<ParameterInfo> parameters;
		SymbolBodyInfo body;
	};

	struct BlockInfo
	{
        NameInfo name;
		std::vector<VariableInfo> attributes;
		std::map<std::string, std::vector<FunctionInfo>> methodInfos;
		std::map<std::string, std::vector<OperatorInfo>> operatorInfos;
	};

	struct NamespaceInfo
	{
        NameInfo name;
		std::vector<BlockInfo> structureBlocks;
		std::vector<BlockInfo> attributeBlocks;
		std::vector<BlockInfo> constantBlocks;

		std::vector<TextureInfo> textureInfos;

		std::map<std::string, std::vector<FunctionInfo>> functionInfos;

		std::vector<NamespaceInfo> nestedNamespaces;
	};

	struct PipelineFlowInfo
	{
		Lumina::Token input;
		Lumina::Token output;
		VariableInfo variable;
	};

	struct PipelinePassInfo
	{
		Lumina::Token name;
		SymbolBodyInfo body;
	};

	struct ShaderInfo
	{
		std::vector<PipelineFlowInfo> pipelineFlows;

		std::vector<PipelinePassInfo> pipelinePasses;

		NamespaceInfo anonymNamespace;
	};
}