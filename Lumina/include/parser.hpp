#pragma once

#include "lexer.hpp"
#include "shader_representation.hpp"
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
		ShaderRepresentation::Function composeMethod(const ShaderRepresentation::Type* p_originatorType, const std::string& p_sourceCode);
		ShaderRepresentation::Type::Constructor composeConstructor(const ShaderRepresentation::Type* p_originatorType, const std::string& p_sourceCode);
		ShaderRepresentation::Function composeOperator(const ShaderRepresentation::Type* p_originatorType, const std::string& p_sourceCode);

		void composeStandardTypes();
		void composePredefinedTypes();
		void composeComplexStandardTypes();

		Parser();

		Product _product;

		// ShaderRepresentation instance to build up the parsed shader
		ShaderRepresentation _shaderRepresentation;

		// Namespace tracking
		std::deque<std::string> _nspaces;

		// Helper methods for composing various elements
		std::string _composeTypeName(const TypeInfo& p_typeInfo);
		std::vector<size_t> _composeSizeArray(const ArraySizeInfo& p_arraySizeInfo);
		std::string currentNamespace();
		std::string _extractNameInfo(const NameInfo& p_nameInfo);

		const ShaderRepresentation::Type* _findType(const std::string& p_objectName);
		const ShaderRepresentation::Type* _findType(const TypeInfo& p_typeInfo);

		void _insertVariable(const ShaderRepresentation::Variable& p_variable);

		ShaderRepresentation::ExpressionType _composeExpressionType(const ExpressionTypeInfo& p_expressionType);

		ShaderRepresentation::Variable _composeVariable(const VariableInfo& p_variableInfo);
		ShaderRepresentation::Type _composeType(const BlockInfo& p_block, const std::string& p_suffix = "");

		ShaderRepresentation::SymbolBody _composeSymbolBody(const SymbolBodyInfo& p_symbolBodyInfo);
		std::shared_ptr<ShaderRepresentation::Statement> _composeStatement(const StatementInfo& p_statementInfo);
		std::shared_ptr<ShaderRepresentation::Expression> _composeExpression(const ExpressionInfo& p_expressionInfo);

		std::shared_ptr<ShaderRepresentation::Statement> _composeVariableDeclarationStatement(const VariableDeclarationStatementInfo& p_variableDeclarationStatementInfo);
		std::shared_ptr<ShaderRepresentation::Statement> _composeExpressionStatement(const ExpressionStatementInfo& p_expressionStatementInfo);
		std::shared_ptr<ShaderRepresentation::Statement> _composeAssignmentStatement(const AssignmentStatementInfo& p_assignmentStatementInfo);
		std::shared_ptr<ShaderRepresentation::Statement> _composeReturnStatement(const ReturnStatementInfo& p_returnStatementInfo);
		std::shared_ptr<ShaderRepresentation::Statement> _composeDiscardStatement(const DiscardStatementInfo& p_discardStatementInfo);
		std::shared_ptr<ShaderRepresentation::Statement> _composeIfStatement(const IfStatementInfo& p_ifStatementInfo);
		std::shared_ptr<ShaderRepresentation::Statement> _composeWhileStatement(const WhileStatementInfo& p_whileStatementInfo);
		std::shared_ptr<ShaderRepresentation::Statement> _composeForStatement(const ForStatementInfo& p_forStatementInfo);
		std::shared_ptr<ShaderRepresentation::Statement> _composeRaiseExceptionStatement(const RaiseExceptionStatementInfo& p_raiseExceptionStatementInfo);
		std::shared_ptr<ShaderRepresentation::Statement> _composeCompoundStatement(const CompoundStatementInfo& p_compoundStatementInfo);

		std::shared_ptr<ShaderRepresentation::Expression> _composeLiteralExpression(const LiteralExpressionInfo& p_literalExpressionInfo);
		std::shared_ptr<ShaderRepresentation::Expression> _composeVariableExpression(const VariableExpressionInfo& p_variableExpressionInfo);
		std::shared_ptr<ShaderRepresentation::Expression> _composeBinaryExpression(const BinaryExpressionInfo& p_binaryExpressionInfo);
		std::shared_ptr<ShaderRepresentation::Expression> _composeUnaryExpression(const UnaryExpressionInfo& p_unaryExpressionInfo);
		std::shared_ptr<ShaderRepresentation::Expression> _composeUnaryExpression(const PostfixExpressionInfo& p_postfixExpressionInfo);
		std::shared_ptr<ShaderRepresentation::Expression> _composeFunctionCallExpression(const FunctionCallExpressionInfo& p_functionCallExpressionInfo);
		std::shared_ptr<ShaderRepresentation::Expression> _composeMemberAccessExpression(const MemberAccessExpressionInfo& p_memberAccessExpressionInfo);
		std::shared_ptr<ShaderRepresentation::Expression> _composeArrayAccessExpression(const ArrayAccessExpressionInfo& p_arrayAccessExpressionInfo);

		ShaderRepresentation::Parameter _composeParameter(const ParameterInfo& p_parameterInfo);
		ShaderRepresentation::Function _composeMethodFunction(const ShaderRepresentation::Type* p_originatorType, const FunctionInfo& p_functionInfo);
		ShaderRepresentation::Type::Constructor _composeConstructorFunction(const ShaderRepresentation::Type* p_originatorType, const ConstructorInfo& p_constructorInfo);
		ShaderRepresentation::Function _composeOperatorFunction(const ShaderRepresentation::Type* p_originatorType, const OperatorInfo& p_operatorInfo);

		void _computeMethodAndOperator(ShaderRepresentation::Type* p_originator, const BlockInfo& p_block);

		void _parseStructure(const BlockInfo& p_blockInfo);
		void _parseAttribute(const BlockInfo& p_blockInfo);
		void _parseConstant(const BlockInfo& p_blockInfo);
		void _parseTexture(const TextureInfo& p_textureInfo);
		void _parseFunction(const FunctionInfo& p_functionInfo);
		void _parseNamespace(const NamespaceInfo& p_namespaceInfo);

		void _parsePipelineFlow(const PipelineFlowInfo& p_pipelineFlow);

		ShaderRepresentation::Function _composePipelinePass(const PipelinePassInfo& p_pipelinePass);
		void _parsePipelinePass(const PipelinePassInfo& p_pipelinePass);


		TypeImpl _findTypeImpl(const ShaderRepresentation::Type* p_type);
		TypeImpl _convertType(const ShaderRepresentation::Type* p_type);
		ExpressionTypeImpl _convertExpressionType(const ShaderRepresentation::ExpressionType& p_expressionType);
		ParameterImpl _convertParameter(const ShaderRepresentation::Parameter& p_parameter);
		FunctionBodyImpl _convertFunctionBody(const ShaderRepresentation::SymbolBody& p_symbolBody);
		FunctionImpl _convertConstructor(const TypeImpl& p_originator, const ShaderRepresentation::Type::Constructor& p_constructor);
		FunctionImpl _convertFunction(const TypeImpl& p_originator, const ShaderRepresentation::Function& p_function);
		VariableImpl _convertVariable(const ShaderRepresentation::Variable& p_variable);

		void _composeTypeArray(const std::vector<const ShaderRepresentation::Type*>& typeArray, std::vector<TypeImpl>& p_destination);
		void _composeShaderTypes();
		void _composeShaderPipelineFlows();
		void _composeShaderImpl();

		Product _parse(const Lexer::Output& p_input);

	public:
		static Product parse(const Lexer::Output& p_input);
	};
}
