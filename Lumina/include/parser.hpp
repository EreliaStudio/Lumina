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
		Parser();

		Product _product;

		std::vector<std::string> _nspaces;
		std::set<TypeImpl> _availibleTypes;

		TypeImpl _getType(const std::string& p_relativeName);
		TypeImpl _getType(const TypeInfo& p_typeName);

		std::string _composeName(const NameInfo& p_nameInfo);
		std::vector<size_t> _composeArraySizes(const ArraySizeInfo& p_arraySize);

		VariableImpl _composeVariable(const VariableInfo& p_variableInfo);
		VariableImpl _composePipelineFlowVariable(const PipelineFlowInfo& p_pipelineFlowInfo);

		ExpressionTypeImpl _composeExpressionTypeImpl(const ExpressionTypeInfo& p_expressionTypeInfo);

		ParameterImpl _composeParameter(const ParameterInfo& p_parameterInfo);
		SymbolBodyImpl _composeSymbolBody(const SymbolBodyInfo& p_symbolBodyInfo);

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
