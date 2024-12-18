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
		std::map<std::string, std::string> _textureToSwap;

		std::set<std::string> _flatTypes;

		std::set<std::string> _insertedAttributes;
		std::set<std::string> _insertedConstants;

		Compiler();

		std::string _compileFunction(const FunctionImpl& p_functionImpl);
		std::string _compileTypeImpl(const std::string& p_prefix, const TypeImpl& p_typeImpl);
		std::string _compileUniformBlockAttribute(const VariableImpl& p_variable, size_t p_tabulationSize,
			size_t& cpuOffset, size_t& cpuOwnerSize, size_t& gpuOffset, size_t& gpuOwnerSize);
		std::string _compileUniformBlock(const TypeImpl& p_typeImpl);

		void applyPipelineFlow(
			const std::vector<VariableImpl>& p_vertexFlows,
			const std::vector<VariableImpl>& p_fragmentFlows,
			const std::vector<VariableImpl>& p_outputFlows);
		void applyFunction(std::string& p_targetString, const FunctionImpl& p_function);
		void applyTexture(const std::vector<VariableImpl>& p_textures);

		void applyPipelinePass(
			const PipelinePassImpl& p_pass,
			std::string& p_target,
			const std::vector<TypeImpl>& p_structures,
			const std::vector<TypeImpl>& p_attributes,
			const std::vector<TypeImpl>& p_constants);

		void applyRename();

		Product _compile(const Parser::Output& p_input);

	public:
		static Product compile(const Parser::Output& p_input);
	};
}