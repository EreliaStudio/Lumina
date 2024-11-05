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
		std::map<TypeImpl, std::set<TypeImpl>> _convertionTable;
		std::set<FunctionImpl> _availibleFunctions;

		std::set<VariableImpl> _vertexVariables;
		std::set<VariableImpl> _fragmentVariables;
		std::set<VariableImpl> _globalVariables;

		TypeImpl _getType(const std::string& p_relativeName);
		TypeImpl _getType(const TypeInfo& p_typeName);

		Token _getExpressionToken(const ExpressionInfo& expr);

		std::string _namespacePrefix();

		std::string _composeName(const NameInfo& p_nameInfo);
		std::vector<size_t> _composeArraySizes(const ArraySizeInfo& p_arraySize);

		VariableImpl _composeVariable(const VariableInfo& p_variableInfo);
		VariableImpl _composePipelineFlowVariable(const PipelineFlowInfo& p_pipelineFlowInfo);

		ExpressionTypeImpl _composeExpressionTypeImpl(const ExpressionTypeInfo& p_expressionTypeInfo);

		ParameterImpl _composeParameter(const ParameterInfo& p_parameterInfo);

		std::string _composeStatement(std::set<VariableImpl>& p_variables, const StatementInfo& p_statementInfo, std::vector<FunctionImpl>& calledFunctions, std::vector<TypeImpl>& usedTypes);
		std::string _composeVariableDeclaration(std::set<VariableImpl>& p_variables, const VariableDeclarationStatementInfo& p_statement, std::vector<FunctionImpl>& calledFunctions, std::vector<TypeImpl>& usedTypes);
		std::string _composeExpressionStatement(std::set<VariableImpl>& p_variables, const ExpressionStatementInfo& p_statement, std::vector<FunctionImpl>& calledFunctions, std::vector<TypeImpl>& usedTypes);
		std::string _composeAssignmentStatement(std::set<VariableImpl>& p_variables, const AssignmentStatementInfo& p_statement, std::vector<FunctionImpl>& calledFunctions, std::vector<TypeImpl>& usedTypes);
		std::string _composeReturnStatement(std::set<VariableImpl>& p_variables, const ReturnStatementInfo& p_statement, std::vector<FunctionImpl>& calledFunctions, std::vector<TypeImpl>& usedTypes);
		std::string _composeRaiseExceptionStatement(std::set<VariableImpl>& p_variables, const RaiseExceptionStatementInfo& p_statement, std::vector<FunctionImpl>& calledFunctions, std::vector<TypeImpl>& usedTypes);
		std::string _composeIfStatement(std::set<VariableImpl>& p_variables, const IfStatementInfo& p_statement, std::vector<FunctionImpl>& calledFunctions, std::vector<TypeImpl>& usedTypes);
		std::string _composeWhileStatement(std::set<VariableImpl>& p_variables, const WhileStatementInfo& p_statement, std::vector<FunctionImpl>& calledFunctions, std::vector<TypeImpl>& usedTypes);
		std::string _composeForStatement(std::set<VariableImpl>& p_variables, const ForStatementInfo& p_statement, std::vector<FunctionImpl>& calledFunctions, std::vector<TypeImpl>& usedTypes);

		ExpressionTypeImpl _deduceLiteralExpressionType(std::set<VariableImpl>& p_variables, const LiteralExpressionInfo& p_expression);
		ExpressionTypeImpl _deduceVariableExpressionType(std::set<VariableImpl>& p_variables, const VariableExpressionInfo& p_expression);
		ExpressionTypeImpl _deduceBinaryExpressionType(std::set<VariableImpl>& p_variables, const BinaryExpressionInfo& p_expression);
		ExpressionTypeImpl _deduceUnaryExpressionType(std::set<VariableImpl>& p_variables, const UnaryExpressionInfo& p_expression);
		ExpressionTypeImpl _deducePostfixExpressionType(std::set<VariableImpl>& p_variables, const PostfixExpressionInfo& p_expression);
		ExpressionTypeImpl _deduceFunctionCallExpressionType(std::set<VariableImpl>& p_variables, const FunctionCallExpressionInfo& p_expression);
		ExpressionTypeImpl _deduceMethodCallExpressionType(std::set<VariableImpl>& p_variables, const MethodCallExpressionInfo& p_expression);
		ExpressionTypeImpl _deduceMemberAccessExpressionType(std::set<VariableImpl>& p_variables, const MemberAccessExpressionInfo& p_expression);
		ExpressionTypeImpl _deduceArrayAccessExpressionType(std::set<VariableImpl>& p_variables, const ArrayAccessExpressionInfo& p_expression);
		ExpressionTypeImpl _deduceExpressionType(std::set<VariableImpl>& p_variables, const ExpressionInfo& p_expression);

		std::string _composeExpression(std::set<VariableImpl>& p_variables, const ExpressionInfo& p_expression, std::vector<FunctionImpl>& calledFunctions, std::vector<TypeImpl>& usedTypes);
		std::string _composeLiteralExpression(std::set<VariableImpl>& p_variables, const LiteralExpressionInfo& p_expression, std::vector<TypeImpl>& usedTypes);
		std::string _composeVariableExpression(std::set<VariableImpl>& p_variables, const VariableExpressionInfo& p_expression, std::vector<TypeImpl>& usedTypes);
		std::string _composeBinaryExpression(std::set<VariableImpl>& p_variables, const BinaryExpressionInfo& p_expression, std::vector<FunctionImpl>& calledFunctions, std::vector<TypeImpl>& usedTypes);
		std::string _composeUnaryExpression(std::set<VariableImpl>& p_variables, const UnaryExpressionInfo& p_expression, std::vector<FunctionImpl>& calledFunctions, std::vector<TypeImpl>& usedTypes);
		std::string _composePostfixExpression(std::set<VariableImpl>& p_variables, const PostfixExpressionInfo& p_expression, std::vector<FunctionImpl>& calledFunctions, std::vector<TypeImpl>& usedTypes);
		std::string _composeFunctionCallExpression(std::set<VariableImpl>& p_variables, const FunctionCallExpressionInfo& p_expression, std::vector<FunctionImpl>& calledFunctions, std::vector<TypeImpl>& usedTypes);
		std::string _composeMethodCallExpression(std::set<VariableImpl>& p_variables, const MethodCallExpressionInfo& p_expression, std::vector<FunctionImpl>& calledFunctions, std::vector<TypeImpl>& usedTypes);
		std::string _composeMemberAccessExpression(std::set<VariableImpl>& p_variables, const MemberAccessExpressionInfo& p_expression, std::vector<FunctionImpl>& calledFunctions, std::vector<TypeImpl>& usedTypes);
		std::string _composeArrayAccessExpression(std::set<VariableImpl>& p_variables, const ArrayAccessExpressionInfo& p_expression, std::vector<FunctionImpl>& calledFunctions, std::vector<TypeImpl>& usedTypes);

		FunctionImpl* _findFunctionWithConversions(const std::string& name, const std::vector<ExpressionTypeImpl>& p_argumentTypes);
		FunctionImpl _findOperatorFunction(std::set<VariableImpl>& p_variables, const ExpressionTypeImpl& p_lhs, const std::string& p_op, const ExpressionTypeImpl& p_rhs, bool p_isAssignment = false);
		FunctionImpl _findUnaryOperatorFunction(std::set<VariableImpl>& p_variables, const std::string& p_op, const ExpressionTypeImpl& p_operand);
		FunctionImpl _findPostfixOperatorFunction(std::set<VariableImpl>& p_variables, const std::string& p_op, const ExpressionTypeImpl& p_operand);
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
		void _parseBlockInfo(const BlockInfo& p_blockInfo, std::vector<TypeImpl>& p_destination, bool p_needInstanciation);
		void _parseBlockArray(const std::vector<BlockInfo>& p_blockInfos, std::vector<TypeImpl>& p_destination, bool p_needInstanciation);
		void _parseTextures(const std::vector<TextureInfo>& p_textureInfos);
		void _parseFunctions(const std::vector<FunctionInfo>& p_functionInfos);
		void _parseFunctionMap(const std::map<std::string, std::vector<FunctionInfo>>& p_functionInfosMap);
		void _parseNamespace(const NamespaceInfo& p_namespaceInfo);
		void _parse(const Lexer::Output& p_input);

	public:
		static Product parse(const Lexer::Output& p_input);
	};
}
