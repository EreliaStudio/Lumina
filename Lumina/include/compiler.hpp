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
		std::string _compileUniformBlockAttribute(const VariableImpl& p_variable, size_t p_tabulationSize);
		std::string _compileUniformBlock(const TypeImpl& p_typeImpl);

		void applyPipelineFlow(
			const std::vector<VariableImpl>& p_vertexFlows,
			const std::vector<VariableImpl>& p_fragmentFlows,
			const std::vector<VariableImpl>& p_outputFlows);
		void applyFunction(std::string& p_targetString, const FunctionImpl& p_function);
		void applyTexture(const std::vector<VariableImpl>& p_textures);

		void applyRename();

		Product _compile(const Parser::Output& p_input);

	public:
		static Product compile(const Parser::Output& p_input);
	};
}