#pragma once

#include "lexer.hpp"

#include "shader_impl.hpp"

namespace Lumina
{
	struct Parser
	{
	public:
		using Output = ShaderImpl;
		using Product = Lumina::Expected<Output>;

	private:
		Product _product;

		Parser() = default;

		bool _vertexPassParsed = false;
		bool _fragmentPassParsed = false;

		std::vector<std::string> _currentNamespacePrefix;

		std::string currentPrefix() const;

		VariableImpl parseVariable(const VariableInfo& p_variableInfo);
		PipelineFlowImpl parsePipelineFlow(PipelineFlowImpl::Direction p_direction, const PipelineFlowInfo& p_pipelineInfo);
		void parsePipelineFlows(const std::vector<PipelineFlowInfo>& p_pipelineFlows);
		
		FunctionBodyImpl parseSymbolBody(const SymbolBodyInfo& p_symbolBody);

		FunctionImpl parsePipelinePass(const PipelinePassInfo& p_pipelinePass);
		void parsePipelinePasses(const std::vector<PipelinePassInfo>& p_pipelinePasses);

		FunctionImpl parseMethodFunction(const std::string& p_blockName, const FunctionInfo& p_methodInfo);
		FunctionImpl parseOperatorFunction(const std::string& p_blockName, const OperatorInfo& p_operatorInfo);

		BlockImpl parseBlockInfo(const BlockInfo& p_blockInfo);
		void parseStructureBlockInfos(const std::vector<BlockInfo>& p_structureBlockInfo);
		void parseAttributeBlockInfos(const std::vector<BlockInfo>& p_attributeBlockInfo);
		void parseConstantBlockInfos(const std::vector<BlockInfo>& p_constantsBlockInfo);

		VariableImpl parseTexture(const TextureInfo& p_textureInfo);
		void parseTextures(const std::vector<TextureInfo>& p_textureInfos);

		FunctionImpl parseFunction(const FunctionInfo& p_function);
		void parseFunctions(const std::vector<FunctionInfo>& p_functions);

		void parseNamespace(const NamespaceInfo& p_namespace);

		Product _parse(const Lexer::Output& p_input);

	public:
		static Product parse(const Lexer::Output& p_input);
	};
}