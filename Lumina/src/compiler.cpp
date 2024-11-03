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
		result += ") {\n";
		result += p_functionImpl.body.code;
		result += "}";

		return (result);
	}

	std::string Compiler::_compileTypeImpl(const std::string& p_prefix, const TypeImpl& p_typeImpl)
	{
		std::string result;

		result += p_prefix + " " + p_typeImpl.name + " {\n";

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

	std::string Compiler::_compileUniformBlock(const TypeImpl& p_typeImpl)
	{
		std::string result;



		return (result);
	}

	Compiler::Product Compiler::_compile(const Parser::Output& p_input)
	{
		_product = Product();

		for (size_t i = 0; i < p_input.vertexPipelineFlows.size(); i++)
		{
			_product.layoutContent += "in " + p_input.vertexPipelineFlows[i].type.name + " " + p_input.vertexPipelineFlows[i].name + "\n";
			_product.vertexCodeContent += "layout (location = " + std::to_string(i) + ") in " + p_input.vertexPipelineFlows[i].type.name + " " + p_input.vertexPipelineFlows[i].name + ";\n";
		}

		for (size_t i = 0; i < p_input.fragmentPipelineFlows.size(); i++)
		{
			_product.vertexCodeContent += "layout (location = " + std::to_string(i) + ") out " + p_input.fragmentPipelineFlows[i].type.name + " " + p_input.fragmentPipelineFlows[i].name + ";\n";
			_product.fragmentCodeContent += "layout (location = " + std::to_string(i) + ") in " + p_input.fragmentPipelineFlows[i].type.name + " " + p_input.fragmentPipelineFlows[i].name + ";\n";
		}

		for (size_t i = 0; i < p_input.outputPipelineFlows.size(); i++)
		{
			_product.layoutContent += "out " + p_input.outputPipelineFlows[i].type.name + " " + p_input.outputPipelineFlows[i].name + "\n";
			_product.fragmentCodeContent += "layout (location = " + std::to_string(i) + ") out " + p_input.outputPipelineFlows[i].type.name + " " + p_input.outputPipelineFlows[i].name + ";\n";
		}

		_product.vertexCodeContent += "\n";
		_product.fragmentCodeContent += "\n";

		for (const auto& structure : p_input.structures)
		{
			std::string structureCode = _compileTypeImpl("struct", structure) + "\n\n";

			_product.vertexCodeContent += structureCode;
			_product.fragmentCodeContent += structureCode;
		}

		for (const auto& attribute : p_input.attributes)
		{
			std::string attributeCode = _compileTypeImpl("layout(attributes) uniform ", attribute) + " " + attribute.name.substr(0, attribute.name.size() - 5) + ";\n\n";

			_product.attributeContent += _compileUniformBlock(attribute);

			_product.vertexCodeContent += attributeCode;
			_product.fragmentCodeContent += attributeCode;
		}

		for (const auto& constant : p_input.constants)
		{
			std::string constantCode = _compileTypeImpl("layout(constants) uniform ", constant) + " " + constant.name.substr(0, constant.name.size() - 5) + ";\n\n";

			_product.vertexCodeContent += constantCode;
			_product.fragmentCodeContent += constantCode;
		}

		for (const auto& function : p_input.functions)
		{
			std::string functionCode = _compileFunction(function) + "\n\n";

			_product.vertexCodeContent += functionCode;
			_product.fragmentCodeContent += functionCode;
		}

		for (const auto& texture : p_input.textures)
		{
			_textureNames.push_back(texture.name);

			std::string functionCode = "uniform sampler2D " + texture.name + ";\n\n";

			_product.vertexCodeContent += functionCode;
			_product.fragmentCodeContent += functionCode;
		}

		_product.vertexCodeContent += "void main() {\n" + p_input.vertexPipelinePass.body.code + "\n}";

		_product.fragmentCodeContent += "void main() {\n" + p_input.fragmentPipelinePass.body.code + "\n}";

		for (const auto& textureName : _textureNames)
		{
			std::regex word_regex("\\b" + textureName + "\\b");

			_product.textureContent += textureName + " Texture_" + textureName + "\n";

			_product.vertexCodeContent = std::regex_replace(_product.vertexCodeContent, word_regex, "Texture_" + textureName);
			_product.fragmentCodeContent = std::regex_replace(_product.fragmentCodeContent, word_regex, "Texture_" + textureName);
		}

		for (const auto& [key, value] : _textToSwap)
		{
			std::regex word_regex("\\b" + key + "\\b");

			_product.vertexCodeContent = std::regex_replace(_product.vertexCodeContent, word_regex, value);
			_product.fragmentCodeContent = std::regex_replace(_product.fragmentCodeContent, word_regex, value);
		}

		return (_product);
	}

	Compiler::Product Compiler::compile(const Parser::Output& p_input)
	{
		return (Compiler()._compile(p_input));
	}
}