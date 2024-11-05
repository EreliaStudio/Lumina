#include "compiler.hpp"

#include <regex>

namespace Lumina
{
	Compiler::Compiler()
	{
		_textToSwap = {
			{"::", "_"},
			{"void", "void"},
			{"bool", "bool"},
			{"int", "int"},
			{"uint", "uint"},
			{"float", "float"},

			{"Vector2", "vec2"},
			{"Vector2Int", "ivec2"},
			{"Vector2UInt", "uvec2"},

			{"Vector3", "vec3"},
			{"Vector3Int", "ivec3"},
			{"Vector3UInt", "uvec3"},

			{"Vector4", "vec4"},
			{"Vector4Int", "ivec4"},
			{"Vector4UInt", "uvec4"},

			{"Color", "vec4"},

			{"Matrix2x2", "mat2"},
			{"Matrix3x3", "mat3"},
			{"Matrix4x4", "mat4"},

			{"Texture", "sampler2D"},
			{"Texture_getPixel", "texture"},

			{"pixelPosition", "gl_Position"},
		};
	}

	std::string Compiler::_compileFunction(const FunctionImpl& p_functionImpl)
	{
		std::string result = "";

		result += p_functionImpl.returnType.type.name;
		for (const auto& dim : p_functionImpl.returnType.arraySizes)
		{
			result += "[" + std::to_string(dim) + "]";
		}
		result += " " + p_functionImpl.name + "(";
		for (size_t i = 0; i < p_functionImpl.parameters.size(); i++)
		{
			if (i != 0)
			{
				result += ", ";
			}

			const auto& param = p_functionImpl.parameters[i];

			result += param.type.name;
			result += " " + param.name;

			for (const auto& dim : param.arraySizes)
			{
				result += "[" + std::to_string(dim) + "]";
			}
		}
		result += ")\n{\n";
		result += p_functionImpl.body.code;
		result += "}";

		return (result);
	}

	std::string Compiler::_compileTypeImpl(const std::string& p_prefix, const TypeImpl& p_typeImpl)
	{
		std::string result;

		result += p_prefix + " " + p_typeImpl.name + "\n{\n";

		for (const auto& attribute : p_typeImpl.attributes)
		{
			result += attribute.type.name;
			result += " " + attribute.name;

			for (const auto& dim : attribute.arraySizes)
			{
				result += "[" + std::to_string(dim) + "]";
			}
			result += ";\n";
		}

		result += "}";

		return (result);
	}

	std::string Compiler::_compileUniformBlockAttribute(const VariableImpl& p_variable, size_t p_tabulationSize)
	{
		std::string result;

		result += std::string(p_tabulationSize * 4, ' ') + p_variable.name + " {";

		if (p_variable.type.attributes.size() != 0)
			result += "\n";

		for (const auto& attribute : p_variable.type.attributes)
		{
			result += _compileUniformBlockAttribute(attribute, p_tabulationSize + 1) + "\n";
		}

		result += "}";

		return (result);
	}

	std::string Compiler::_compileUniformBlock(const TypeImpl& p_typeImpl)
	{
		std::string result;

		result += p_typeImpl.name + " " + p_typeImpl.name.substr(0, p_typeImpl.name.size() - 5) + " {";
		
		if (p_typeImpl.attributes.size() != 0)
			result += "\n";

		for (const auto& attribute : p_typeImpl.attributes)
		{
			result += _compileUniformBlockAttribute(attribute, 1) + "\n";
		}

		result += "}";

		return (result);
	}

	Compiler::Product Compiler::_compile(const Parser::Output& p_input)
	{
		_product = Product();

		applyPipelineFlow(p_input.vertexPipelineFlows, p_input.fragmentPipelineFlows, p_input.outputPipelineFlows);

		for (const auto& type : p_input.vertexPipelinePass.body.usedTypes)
		{
			std::string typeCode = "";

			if (std::find(p_input.structures.begin(), p_input.structures.end(), type) != p_input.structures.end())
			{
				typeCode = _compileTypeImpl("struct", type) + "\n\n";
			}
			else if (std::find(p_input.attributes.begin(), p_input.attributes.end(), type) != p_input.attributes.end())
			{
				typeCode = _compileTypeImpl("layout(attributes) uniform ", type) + " " + type.name.substr(0, type.name.size() - 5) + ";\n\n";

				_product.attributeContent += _compileUniformBlock(type);
			}
			else if (std::find(p_input.constants.begin(), p_input.constants.end(), type) != p_input.constants.end())
			{
				typeCode = _compileTypeImpl("layout(constants) uniform ", type) + " " + type.name.substr(0, type.name.size() - 5) + ";\n\n";

				_product.constantContent += _compileUniformBlock(type);
			}

			_product.vertexCodeContent += typeCode;
		}

		for (const auto& type : p_input.fragmentPipelinePass.body.usedTypes)
		{
			std::string typeCode = "";

			if (std::find(p_input.structures.begin(), p_input.structures.end(), type) != p_input.structures.end())
			{
				typeCode = _compileTypeImpl("struct", type) + "\n\n";
			}
			else if (std::find(p_input.attributes.begin(), p_input.attributes.end(), type) != p_input.attributes.end())
			{
				typeCode = _compileTypeImpl("layout(attributes) uniform ", type) + " " + type.name.substr(0, type.name.size() - 5) + ";\n\n";

				_product.attributeContent += _compileUniformBlock(type);
			}
			else if (std::find(p_input.constants.begin(), p_input.constants.end(), type) != p_input.constants.end())
			{
				typeCode = _compileTypeImpl("layout(constants) uniform ", type) + " " + type.name.substr(0, type.name.size() - 5) + ";\n\n";

				_product.constantContent += _compileUniformBlock(type);
			}

			_product.fragmentCodeContent += typeCode;
		}

		applyTexture(p_input.textures);

		for (const auto& function : p_input.vertexPipelinePass.body.calledFunctions)
		{
			_product.vertexCodeContent += _compileFunction(function) + "\n\n";
		}

		for (const auto& function : p_input.fragmentPipelinePass.body.calledFunctions)
		{
			_product.fragmentCodeContent += _compileFunction(function) + "\n\n";
		}

		_product.vertexCodeContent += "void main()\n{\n" + p_input.vertexPipelinePass.body.code + "\n}";
		_product.fragmentCodeContent += "void main()\n{\n" + p_input.fragmentPipelinePass.body.code + "\n}";

		applyRename();

		return (_product);
	}
	
	void Compiler::applyPipelineFlow(
		const std::vector<VariableImpl>& p_vertexFlows,
		const std::vector<VariableImpl>& p_fragmentFlows,
		const std::vector<VariableImpl>& p_outputFlows)
	{
		for (size_t i = 0; i < p_vertexFlows.size(); i++)
		{
			_product.layoutContent += "in " + p_vertexFlows[i].type.name + " " + p_vertexFlows[i].name + "\n";
			_product.vertexCodeContent += "layout (location = " + std::to_string(i) + ") in " + p_vertexFlows[i].type.name + " " + p_vertexFlows[i].name + ";\n";
		}

		for (size_t i = 0; i < p_fragmentFlows.size(); i++)
		{
			_product.vertexCodeContent += "layout (location = " + std::to_string(i) + ") out " + p_fragmentFlows[i].type.name + " " + p_fragmentFlows[i].name + ";\n";
			_product.fragmentCodeContent += "layout (location = " + std::to_string(i) + ") in " + p_fragmentFlows[i].type.name + " " + p_fragmentFlows[i].name + ";\n";
		}

		for (size_t i = 0; i < p_outputFlows.size(); i++)
		{
			_product.layoutContent += "out " + p_outputFlows[i].type.name + " " + p_outputFlows[i].name + "\n";
			_product.fragmentCodeContent += "layout (location = " + std::to_string(i) + ") out " + p_outputFlows[i].type.name + " " + p_outputFlows[i].name + ";\n";
		}

		_product.vertexCodeContent += "\n";
		_product.fragmentCodeContent += "\n";
	}

	void Compiler::applyFunction(std::string& p_targetString, const FunctionImpl& p_function)
	{
		for (const auto& function : p_function.body.calledFunctions)
		{
			applyFunction(p_targetString, function);
		}

		p_targetString += _compileFunction(p_function) + "\n\n";
	}

	void Compiler::applyTexture(const std::vector<VariableImpl>& p_textures)
	{
		for (const auto& texture : p_textures)
		{
			_product.textureContent += texture.name + " Texture_" + texture.name + "\n";

			std::string functionCode = "uniform sampler2D " + texture.name + ";\n\n";

			_product.vertexCodeContent += functionCode;
			_product.fragmentCodeContent += functionCode;

			_textToSwap[texture.name] = "Texture_" + texture.name;
		}
	}
	
	void Compiler::applyRename()
	{
		for (const auto& [key, value] : _textToSwap)
		{
			std::regex word_regex("\\b" + key + "\\b");

			_product.vertexCodeContent = std::regex_replace(_product.vertexCodeContent, word_regex, value);
			_product.fragmentCodeContent = std::regex_replace(_product.fragmentCodeContent, word_regex, value);
		}
	}

	Compiler::Product Compiler::compile(const Parser::Output& p_input)
	{
		return (Compiler()._compile(p_input));
	}
}