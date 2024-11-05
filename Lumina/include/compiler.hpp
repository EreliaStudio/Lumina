#pragma once

#include "parser.hpp"
#include "shader_code.hpp"

#include <deque>

namespace Lumina
{
	struct Compiler
	{
	public:
		using Product = ShaderCode;
		
	private:
		Product _product;

		std::map<std::string, std::string> _textToSwap;

		Compiler();

		std::string _compileFunction(const FunctionImpl& p_functionImpl);
		std::string _compileTypeImpl(const std::string& p_prefix, const TypeImpl& p_typeImpl);
		std::string _compileUniformBlock(const TypeImpl& p_typeImpl, const std::string& p_blockType);

		void applyPipelineFlow(
			const std::vector<VariableImpl>& p_vertexFlows,
			const std::vector<VariableImpl>& p_fragmentFlows,
			const std::vector<VariableImpl>& p_outputFlows);
		void applyStructures(const std::vector<TypeImpl>& p_structures);
		void applyAttributes(const std::vector<TypeImpl>& p_attributes);
		void applyConstants(const std::vector<TypeImpl>& p_constants);
		void applyFunction(std::string& p_targetString, const FunctionImpl& p_function);
		void applyTexture(const std::vector<VariableImpl>& p_textures);
		void applyPipelinePasses(const PipelinePassImpl& p_vertexPass, const PipelinePassImpl& p_fragmentPass);

		void applyRename();

		Product _compile(const Parser::Output& p_input);

	public:
		static Product compile(const Parser::Output& p_input);
	};
}