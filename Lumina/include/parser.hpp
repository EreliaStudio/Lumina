#pragma once

#include "lexer.hpp"
#include "shader_impl.hpp"

#include <list>
#include <deque>
#include <set>
#include <string>
#include <memory>

namespace Lumina
{
	class Parser
	{
	public:
		using Output = ShaderImpl;
		using Product = Lumina::Expected<Output>;

	private:
		const static std::map<std::string, std::string> _operatorNames;

		Parser();

		Product _product;

		std::vector<std::string> _nspaces;

		std::set<TypeImpl> _availibleTypes;
		std::set<FunctionImpl> _availibleFunctions;

		std::set<VariableImpl> _vertexVariables;
		std::set<VariableImpl> _fragmentVariables;
		std::set<VariableImpl> _globalVariables;

		TypeImpl _getType(const std::string& p_relativeName);
		TypeImpl _getType(const TypeInfo& p_typeName);

		std::string _composeName(const NameInfo& p_nameInfo);
		std::vector<size_t> _composeArraySizes(const ArraySizeInfo& p_arraySize);

		VariableImpl _composeVariable(const VariableInfo& p_variableInfo);
		VariableImpl _composePipelineFlowVariable(const PipelineFlowInfo& p_pipelineFlowInfo);

		ExpressionTypeImpl _composeExpressionTypeImpl(const ExpressionTypeInfo& p_expressionTypeInfo);

		ParameterImpl _composeParameter(const ParameterInfo& p_parameterInfo);

        std::string _composeStatement(std::set<VariableImpl>& p_variables, const StatementInfo& p_statementInfo);
        std::string _composeVariableDeclaration(std::set<VariableImpl>& p_variables, const VariableDeclarationStatementInfo& stmt);
        std::string _composeExpressionStatement(std::set<VariableImpl>& p_variables, const ExpressionStatementInfo& stmt);
        std::string _composeAssignmentStatement(std::set<VariableImpl>& p_variables, const AssignmentStatementInfo& stmt);
        std::string _composeReturnStatement(std::set<VariableImpl>& p_variables, const ReturnStatementInfo& stmt);
        std::string _composeRaiseExceptionStatement(std::set<VariableImpl>& p_variables, const RaiseExceptionStatementInfo& stmt);
        std::string _composeIfStatement(std::set<VariableImpl>& p_variables, const IfStatementInfo& stmt);
        std::string _composeWhileStatement(std::set<VariableImpl>& p_variables, const WhileStatementInfo& stmt);
        std::string _composeForStatement(std::set<VariableImpl>& p_variables, const ForStatementInfo& stmt);
        std::string _composeExpression(std::set<VariableImpl>& p_variables, const ExpressionInfo& expr);
		
        Token getExpressionToken(const ExpressionInfo& expr);

		ExpressionTypeImpl _deduceLiteralExpressionType(std::set<VariableImpl>& p_variables, const LiteralExpressionInfo& expr);
		ExpressionTypeImpl _deduceVariableExpressionType(std::set<VariableImpl>& p_variables, const VariableExpressionInfo& expr);
		ExpressionTypeImpl _deduceBinaryExpressionType(std::set<VariableImpl>& p_variables, const BinaryExpressionInfo& e);
		ExpressionTypeImpl _deduceUnaryExpressionType(std::set<VariableImpl>& p_variables, const UnaryExpressionInfo& e);
		ExpressionTypeImpl _deducePostfixExpressionType(std::set<VariableImpl>& p_variables, const PostfixExpressionInfo& e);
		ExpressionTypeImpl _deduceFunctionCallExpressionType(std::set<VariableImpl>& p_variables, const FunctionCallExpressionInfo& e);
		ExpressionTypeImpl _deduceMethodCallExpressionType(std::set<VariableImpl>& p_variables, const MethodCallExpressionInfo& e);
		ExpressionTypeImpl _deduceMemberAccessExpressionType(std::set<VariableImpl>& p_variables, const MemberAccessExpressionInfo& e);
		ExpressionTypeImpl _deduceArrayAccessExpressionType(std::set<VariableImpl>& p_variables, const ArrayAccessExpressionInfo& e);
		ExpressionTypeImpl _deduceExpressionType(std::set<VariableImpl>& p_variables, const ExpressionInfo& expr);

		std::string _composeLiteralExpression(std::set<VariableImpl>& p_variables, const LiteralExpressionInfo& e);
		std::string _composeVariableExpression(std::set<VariableImpl>& p_variables, const VariableExpressionInfo& e);
        std::string _composeBinaryExpression(std::set<VariableImpl>& p_variables, const BinaryExpressionInfo& e);
        std::string _composeUnaryExpression(std::set<VariableImpl>& p_variables, const UnaryExpressionInfo& e);
        std::string _composePostfixExpression(std::set<VariableImpl>& p_variables, const PostfixExpressionInfo& e);
		std::string _composeFunctionCallExpression(std::set<VariableImpl>& p_variables, const FunctionCallExpressionInfo& e);
		std::string _composeMethodCallExpression(std::set<VariableImpl>& p_variables, const MethodCallExpressionInfo& e);
		std::string _composeMemberAccessExpression(std::set<VariableImpl>& p_variables, const MemberAccessExpressionInfo& e);
		std::string _composeArrayAccessExpression(std::set<VariableImpl>& p_variables, const ArrayAccessExpressionInfo& e);


		FunctionImpl _findOperatorFunction(std::set<VariableImpl>& p_variables, const ExpressionTypeImpl& lhs, const std::string& op, const ExpressionTypeImpl& rhs, bool isAssignment = false);
		std::string _findOperatorFunctionName(std::set<VariableImpl>& p_variables, const ExpressionTypeImpl& lhs, const std::string& op, const ExpressionTypeImpl& rhs, bool isAssignment = false);
        std::string _findUnaryOperatorFunctionName(std::set<VariableImpl>& p_variables, const std::string& op, const ExpressionTypeImpl& operand);
        std::string _findPostfixOperatorFunctionName(std::set<VariableImpl>& p_variables, const std::string& op, const ExpressionTypeImpl& operand);
		SymbolBodyImpl _composeSymbolBody(std::set<VariableImpl>& p_variables, const SymbolBodyInfo& p_symbolBodyInfo);

		VariableImpl _composeTexture(const TextureInfo& p_textureInfo);

		FunctionImpl _composeFunction(const FunctionInfo& p_functionInfo);

		TypeImpl _composeTypeImpl(const BlockInfo& p_blockInfo);

		std::vector<FunctionImpl> _composeConstructors(const BlockInfo& p_blockInfo);
		std::vector<FunctionImpl> _composeMethods(const BlockInfo& p_blockInfo);
		std::vector<FunctionImpl> _composeOperators(const BlockInfo& p_blockInfo);

		PipelinePassImpl _composePipelinePass(const PipelinePassInfo& p_pipelinePassInfo);

		void _parsePipelineFlow(const PipelineFlowInfo& p_pipelineFlow);
		void _parsePipelinePass(const PipelinePassInfo& p_pipelinePass);
		void _parseBlockInfo(const BlockInfo& p_blockInfo, std::vector<TypeImpl>& p_destination);
		void _parseBlockArray(const std::vector<BlockInfo>& p_blockInfos, std::vector<TypeImpl>& p_destination);
		void _parseTextures(const std::vector<TextureInfo>& p_textureInfos);
		void _parseFunctions(const std::vector<FunctionInfo>& p_functionInfos);
		void _parseFunctionMap(const std::map<std::string, std::vector<FunctionInfo>>& p_functionInfosMap);
		void _parseNamespace(const NamespaceInfo& p_namespaceInfo);
		void _parse(const Lexer::Output& p_input);

	public:
		static Product parse(const Lexer::Output& p_input);
	};
}
