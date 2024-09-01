#include "lumina_semantic_checker.hpp"

namespace Lumina
{
	void SemanticChecker::checkPipelineFlowInstruction(const std::filesystem::path& p_file, const std::shared_ptr<PipelineFlowInstruction>& p_instruction)
	{
		std::unordered_map<std::string, Type*>* target;
		std::unordered_map<std::string, Type*>* targetSecond = nullptr;

		if (p_instruction->inputPipeline.content == "Input")
		{
			if (p_instruction->outputPipeline.content == "VertexPass")
			{
				target = &(_vertexPassVariables);
			}
			else
			{
				throw TokenBasedError(p_file, "Invalid pipeline output token for input [Input] [" + p_instruction->outputPipeline.content + "]", p_instruction->outputPipeline);
			}
		}
		else if (p_instruction->inputPipeline.content == "VertexPass")
		{
			if (p_instruction->outputPipeline.content == "FragmentPass")
			{
				target = &(_fragmentPassVariables);
				targetSecond = &(_vertexPassVariables);
			}
			else
			{
				throw TokenBasedError(p_file, "Invalid pipeline output token for input [VertexPass] [" + p_instruction->outputPipeline.content + "]", p_instruction->outputPipeline);
			}
		}
		else
		{
			throw TokenBasedError(p_file, "Invalid pipeline input token [" + p_instruction->inputPipeline.content + "]", p_instruction->inputPipeline);
		}

		Token typeToken = Token::merge(p_instruction->type->tokens, Token::Type::Identifier);

		Type* tmpType = standardType(typeToken.content);
		if (tmpType == nullptr)
		{
			throw TokenBasedError(p_file, "Invalid pipeline flow type [" + typeToken.content + "]", typeToken);
		}

		(*target)[p_instruction->name.content] = tmpType;
		if (targetSecond != nullptr)
			(*targetSecond)[p_instruction->name.content] = tmpType;
	}

	void SemanticChecker::compilePipelineFlowInstruction(const std::shared_ptr<PipelineFlowInstruction>& p_instruction)
	{

		size_t nbElement = 0;
		size_t type = 0;
		std::string glslName = "";

		std::string flowType = p_instruction->type->mergedToken().content;

		if (flowType == "int")
		{
			nbElement = 1;
			type = 0x1404;
			glslName = "int";
		}
		else if (flowType == "float")
		{
			nbElement = 1;
			type = 0x1406;
			glslName = "float";
		}
		else if (flowType == "uint")
		{
			nbElement = 1;
			type = 0x1405;
			glslName = "uint";
		}
		else if (flowType == "bool")
		{
			nbElement = 1;
			type = 0x1400;
			glslName = "bool";
		}
		else if (flowType == "Vector2Int")
		{
			nbElement = 2;
			type = 0x1404;
			glslName = "ivec2";
		}
		else if (flowType == "Vector2")
		{
			nbElement = 2;
			type = 0x1406;
			glslName = "vec2";
		}
		else if (flowType == "Vector2UInt")
		{
			nbElement = 2;
			type = 0x1405;
			glslName = "uvec2";
		}
		else if (flowType == "Vector3Int")
		{
			nbElement = 3;
			type = 0x1404;
			glslName = "ivec3";
		}
		else if (flowType == "Vector3")
		{
			nbElement = 3;
			type = 0x1406;
			glslName = "vec3";
		}
		else if (flowType == "Vector3UInt")
		{
			nbElement = 3;
			type = 0x1405;
			glslName = "uvec3";
		}
		else if (flowType == "Vector4Int")
		{
			nbElement = 4;
			type = 0x1404;
			glslName = "ivec4";
		}
		else if (flowType == "Vector4")
		{
			nbElement = 4;
			type = 0x1406;
			glslName = "vec4";
		}
		else if (flowType == "Vector4UInt")
		{
			nbElement = 4;
			type = 0x1405;
			glslName = "uvec4";
		}
		else if (flowType == "Color")
		{
			nbElement = 4;
			type = 0x1406;
			glslName = "vec4";
		}

		if (p_instruction->inputPipeline.content == "Input")
		{
			_result.sections.layout += std::to_string(_nbVertexPassLayout) + " " + std::to_string(nbElement) + " " + std::to_string(type) + "\n";
			
			_result.sections.vertexShader += "layout(location = " + std::to_string(_nbVertexPassLayout) + ") in " + glslName + " " + p_instruction->name.content + ";\n";

			_nbVertexPassLayout++;
		}
		else
		{
			_result.sections.vertexShader += "layout(location = " + std::to_string(_nbFragmentPassLayout) + ") out " + glslName + " " + p_instruction->name.content + ";\n";
			_result.sections.fragmentShader += "layout(location = " + std::to_string(_nbFragmentPassLayout) + ") in " + glslName + " " + p_instruction->name.content + ";\n";

			_nbFragmentPassLayout++;
		}
	}
}