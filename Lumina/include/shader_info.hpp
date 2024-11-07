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

	struct ExpressionTypeInfo
	{
		TypeInfo type;
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

	struct LiteralExpressionInfo;
	struct VariableExpressionInfo;
	struct BinaryExpressionInfo;
	struct UnaryExpressionInfo;
	struct PostfixExpressionInfo;
	struct FunctionCallExpressionInfo;
	struct MethodCallExpressionInfo;
	struct MemberAccessExpressionInfo;
	struct ArrayAccessExpressionInfo;
	struct ArrayDefinitionExpressionInfo;

	using ExpressionInfo = std::variant<
		LiteralExpressionInfo,
		VariableExpressionInfo,
		BinaryExpressionInfo,
		UnaryExpressionInfo,
		PostfixExpressionInfo,
		FunctionCallExpressionInfo,
		MethodCallExpressionInfo,
		MemberAccessExpressionInfo,
		ArrayAccessExpressionInfo,
		ArrayDefinitionExpressionInfo
	>;

	struct LiteralExpressionInfo
	{
		Token value;
	};

	struct VariableExpressionInfo
	{
		NamespaceDesignation namespacePath;
		Token variableName;
	};

	struct BinaryExpressionInfo
	{
		std::shared_ptr<ExpressionInfo> left;
		Token operatorToken;
		std::shared_ptr<ExpressionInfo> right;
	};

	struct UnaryExpressionInfo
	{
		Token operatorToken;
		std::shared_ptr<ExpressionInfo> operand;
	};

	struct PostfixExpressionInfo
	{
		std::shared_ptr<ExpressionInfo> operand;
		Token operatorToken;
	};

	struct FunctionCallExpressionInfo
	{
		NamespaceDesignation namespacePath;
		Token functionName;
		std::vector<std::shared_ptr<ExpressionInfo>> arguments;
	};

	struct MemberAccessExpressionInfo
	{
		std::shared_ptr<ExpressionInfo> object;
		Token memberName;
	};
	
	struct MethodCallExpressionInfo
	{
		std::shared_ptr<ExpressionInfo> object;
		Token name;
		std::vector<std::shared_ptr<ExpressionInfo>> arguments;
	};

	struct ArrayAccessExpressionInfo
	{
		std::shared_ptr<ExpressionInfo> array;
		std::shared_ptr<ExpressionInfo> index;
	};

	struct ArrayDefinitionExpressionInfo
	{
		std::vector<std::shared_ptr<ExpressionInfo>> elements;
	};

	struct VariableDeclarationStatementInfo;
	struct ExpressionStatementInfo;
	struct AssignmentStatementInfo;
	struct ReturnStatementInfo;
	struct DiscardStatementInfo;
	struct ConditionalBranch;
	struct IfStatementInfo;
	struct WhileStatementInfo;
	struct ForStatementInfo;
	struct RaiseExceptionStatementInfo;
	struct CompoundStatementInfo;

	using StatementInfo = std::variant<
		VariableDeclarationStatementInfo,
		ExpressionStatementInfo,
		AssignmentStatementInfo,
		ReturnStatementInfo,
		DiscardStatementInfo,
		IfStatementInfo,
		WhileStatementInfo,
		ForStatementInfo,
		RaiseExceptionStatementInfo,
		CompoundStatementInfo
	>;

	struct SymbolBodyInfo
	{
		std::vector<StatementInfo> statements;
	};

	struct VariableDeclarationStatementInfo
	{
		VariableInfo variable;
		std::shared_ptr<ExpressionInfo> initializer;
	};

	struct ExpressionStatementInfo
	{
		std::shared_ptr<ExpressionInfo> expression;
	};

	struct AssignmentStatementInfo
	{
		std::shared_ptr<ExpressionInfo> target;
		Token operatorToken;
		std::shared_ptr<ExpressionInfo> value;
	};

	struct ReturnStatementInfo
	{
		std::shared_ptr<ExpressionInfo> expression;
	};

	struct DiscardStatementInfo
	{
	};

	struct ConditionalBranch
	{
		std::shared_ptr<ExpressionInfo> condition;
		SymbolBodyInfo body;
	};

	struct IfStatementInfo
	{
		std::vector<ConditionalBranch> branches;
		SymbolBodyInfo elseBody;
	};

	struct WhileStatementInfo
	{
		ConditionalBranch loop;
	};

	struct ForStatementInfo
	{
		std::shared_ptr<StatementInfo> initializer;
		std::shared_ptr<ExpressionInfo> condition;
		std::shared_ptr<ExpressionInfo> increment;
		SymbolBodyInfo body;
	};

	struct RaiseExceptionStatementInfo
	{
		std::shared_ptr<FunctionCallExpressionInfo> functionCall;
	};

	struct CompoundStatementInfo
	{
		SymbolBodyInfo body;
	};

	struct FunctionInfo
	{
		bool isPrototype;
		ExpressionTypeInfo returnType;
		NameInfo name;
		std::vector<ParameterInfo> parameters;
		SymbolBodyInfo body;
	};

	struct OperatorInfo
	{
		bool isPrototype;
		ExpressionTypeInfo returnType;
		Lumina::Token opeType;
		std::vector<ParameterInfo> parameters;
		SymbolBodyInfo body;
	};

	struct ConstructorInfo
	{
		bool isPrototype;
		std::vector<ParameterInfo> parameters;
		SymbolBodyInfo body;
	};

	struct BlockInfo
	{
		NameInfo name;
		std::vector<VariableInfo> attributes;
		std::vector<ConstructorInfo> constructorInfos;
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