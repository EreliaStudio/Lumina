#include "compiler.hpp"

#include <regex>

namespace Lumina
{
	Compiler::Compiler()
	{
		_textToSwap = {
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

			{"Vector3_xy", "xy"},
			{"Vector3Int_xy", "xy"},
			{"Vector3UInt_xy", "xy"},
			{"Vector4_xy", "xy"},
			{"Vector4_xyz", "xyz"},
			{"Vector4Int_xy", "xy"},
			{"Vector4Int_xyz", "xyz"},
			{"Vector4UInt_xy", "xy"},
			{"Vector4UInt_xyz", "xyz"},
			{"Vector2_length", "length"},
			{"Vector2_normalize", "normalize"},
			{"Vector2_reflect", "reflect"},
			{"Vector2_dot", "dot"},
			{"Vector2_abs", "abs"},
			{"Vector2_floor", "floor"},
			{"Vector2_ceil", "ceil"},
			{"Vector2_mod", "mod"},
			{"Vector2_min", "min"},
			{"Vector2_max", "max"},
			{"Vector2_clamp", "clamp"},
			{"Vector2_step", "step"},
			{"Vector2_smoothstep", "smoothstep"},
			{"Vector2_pow", "pow"},
			{"Vector2_exp", "exp"},
			{"Vector2_log", "log"},
			{"Vector2_exp2", "exp2"},
			{"Vector2_log2", "log2"},
			{"Vector2_sqrt", "sqrt"},
			{"Vector2_inversesqrt", "inversesqrt"},
			{"Vector2_sin", "sin"},
			{"Vector2_cos", "cos"},
			{"Vector2_tan", "tan"},
			{"Vector2_asin", "asin"},
			{"Vector2_acos", "acos"},
			{"Vector2_atan", "atan"},
			{"Vector2_lerp", "mix"},
			{"Vector3_length", "length"},
			{"Vector3_normalize", "normalize"},
			{"Vector3_reflect", "reflect"},
			{"Vector3_dot", "dot"},
			{"Vector3_cross", "cross"},
			{"Vector3_abs", "abs"},
			{"Vector3_floor", "floor"},
			{"Vector3_ceil", "ceil"},
			{"Vector3_mod", "mod"},
			{"Vector3_min", "min"},
			{"Vector3_max", "max"},
			{"Vector3_clamp", "clamp"},
			{"Vector3_step", "step"},
			{"Vector3_smoothstep", "smoothstep"},
			{"Vector3_pow", "pow"},
			{"Vector3_exp", "exp"},
			{"Vector3_log", "log"},
			{"Vector3_exp2", "exp2"},
			{"Vector3_log2", "log2"},
			{"Vector3_sqrt", "sqrt"},
			{"Vector3_inversesqrt", "inversesqrt"},
			{"Vector3_sin", "sin"},
			{"Vector3_cos", "cos"},
			{"Vector3_tan", "tan"},
			{"Vector3_asin", "asin"},
			{"Vector3_acos", "acos"},
			{"Vector3_atan", "atan"},
			{"Vector3_lerp", "mix"},
			{"Vector4_length", "length"},
			{"Vector4_normalize", "normalize"},
			{"Vector4_reflect", "reflect"},
			{"Vector4_dot", "dot"},
			{"Vector4_abs", "abs"},
			{"Vector4_floor", "floor"},
			{"Vector4_ceil", "ceil"},
			{"Vector4_mod", "mod"},
			{"Vector4_min", "min"},
			{"Vector4_max", "max"},
			{"Vector4_clamp", "clamp"},
			{"Vector4_step", "step"},
			{"Vector4_smoothstep", "smoothstep"},
			{"Vector4_pow", "pow"},
			{"Vector4_exp", "exp"},
			{"Vector4_log", "log"},
			{"Vector4_exp2", "exp2"},
			{"Vector4_log2", "log2"},
			{"Vector4_sqrt", "sqrt"},
			{"Vector4_inversesqrt", "inversesqrt"},
			{"Vector4_sin", "sin"},
			{"Vector4_cos", "cos"},
			{"Vector4_tan", "tan"},
			{"Vector4_asin", "asin"},
			{"Vector4_acos", "acos"},
			{"Vector4_atan", "atan"},
			{"Vector4_lerp", "mix"},
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

	Compiler::Product Compiler::_compile(const Parser::Output& p_input)
	{
		_product = Product();

		for (const auto& structure : p_input.structures)
		{
			std::string structureCode = _compileTypeImpl("struct", structure) + "\n\n";

			_product.vertexCodeContent += structureCode;
			_product.fragmentCodeContent += structureCode;
		}

		for (const auto& attribute : p_input.attributes)
		{
			std::string attributeCode = _compileTypeImpl("layout(attributes) uniform ", attribute) + " " + attribute.name.substr(0, attribute.name.size() - 5) + ";\n\n";

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

			std::string functionCode = "uniform sampler2D Texture_" + texture.name + ";\n\n";

			_product.vertexCodeContent += functionCode;
			_product.fragmentCodeContent += functionCode;

		}

		_product.vertexCodeContent += "void main() {\n" + p_input.vertexPipelinePass.body.code + "\n}";

		_product.fragmentCodeContent += "void main() {\n" + p_input.fragmentPipelinePass.body.code + "\n}";

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