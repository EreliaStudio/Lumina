#include "compiler.hpp"

#include <regex>
#include <unordered_map>

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

		_flatTypes = {
			"int",
			"uint",

			"Vector2Int",
			"Vector2UInt",

			"Vector3Int",
			"Vector3UInt",

			"Vector4Int",
			"Vector4UInt",
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
			result += std::string(4, ' ') + attribute.type.name;
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

	struct DataRepresentation
	{
		struct Attribute
		{
			std::string type;
			std::string name;

			size_t cpuOffset;
			size_t gpuOffset;

			size_t elementCount;
			size_t padding;
		};

		size_t cpuSize;
		size_t gpuSize;
		std::vector<Attribute> attributes;
	};

	static std::unordered_map<std::string, DataRepresentation> _typeDataRepresentation = {
		{"bool", {
			.cpuSize = 4,
			.gpuSize = 4,
			.attributes = {}
		}},
		{"int", {
			.cpuSize = 4,
			.gpuSize = 4,
			.attributes = {}
		}},
		{"uint", {
			.cpuSize = 4,
			.gpuSize = 4,
			.attributes = {}
		}},
		{"float", {
			.cpuSize = 4,
			.gpuSize = 4,
			.attributes = {}
		}},

		{"Matrix2x2", {
			.cpuSize = 16,
			.gpuSize = 16,
			.attributes = {}
		}},

		{"Matrix3x3", {
			.cpuSize = 36,
			.gpuSize = 36,
			.attributes = {}
		}},

		{"Matrix4x4", {
			.cpuSize = 64,
			.gpuSize = 64,
			.attributes = {}
		}},
	};

	size_t getAlignment(size_t size)
	{
		if (size == 4)
			return 4;
		else if (size == 8)
			return 8;
		else
			return 16;
	}

	DataRepresentation buildDataRepresentation(const TypeImpl& p_typeImpl)
	{
		auto it = _typeDataRepresentation.find(p_typeImpl.name);
		if (it != _typeDataRepresentation.end())
		{
			return it->second;
		}

		DataRepresentation dataRep = {
			.cpuSize = 0,
			.gpuSize = 0,
			.attributes = {}
		};

		size_t cpuOffset = 0;
		size_t gpuOffset = 0;

		for (const auto& attribute : p_typeImpl.attributes)
		{
			DataRepresentation attrDataRep = buildDataRepresentation(attribute.type);

			size_t elementCount = 1;
			for (const auto& dim : attribute.arraySizes)
			{
				elementCount *= dim;
			}

			size_t alignment = getAlignment(attrDataRep.gpuSize);
			size_t padding = (alignment - (attrDataRep.gpuSize % 16)) % 16;

			gpuOffset = ((gpuOffset + alignment - 1) / alignment) * alignment;

			DataRepresentation::Attribute attr = {
				.type = attribute.type.name,
				.name = attribute.name,
				.cpuOffset = cpuOffset,
				.gpuOffset = gpuOffset,
				.elementCount = elementCount,
				.padding = padding
			};

			dataRep.attributes.push_back(attr);

			dataRep.cpuSize += elementCount * attrDataRep.cpuSize;
			dataRep.gpuSize += attrDataRep.gpuSize;
			dataRep.gpuSize += (elementCount - 1) * (attrDataRep.gpuSize + attr.padding);

			cpuOffset += elementCount * attrDataRep.cpuSize;
			gpuOffset += attrDataRep.gpuSize;
			gpuOffset += (elementCount - 1) * (attrDataRep.gpuSize + attr.padding);
		}

		_typeDataRepresentation[p_typeImpl.name] = dataRep;

		return dataRep;
	}

	void appendAttributes(const DataRepresentation& dataRep, size_t indentLevel, std::string& result)
	{
		std::string indent(indentLevel * 4, ' ');
		for (const auto& attribute : dataRep.attributes)
		{
			DataRepresentation attrDataRep = _typeDataRepresentation[attribute.type];

			result += indent + attribute.name + " " +
				std::to_string(attribute.cpuOffset) + " " +
				std::to_string(attrDataRep.cpuSize) + " " +
				std::to_string(attribute.gpuOffset) + " " +
				std::to_string(attrDataRep.gpuSize) + " " +
				std::to_string(attribute.elementCount) + " " +
				std::to_string(attribute.padding);

			if (attrDataRep.attributes.size() != 0)
			{
				result += " {\n";
				appendAttributes(attrDataRep, indentLevel + 1, result);
				result += indent + "}\n";
			}
			else
			{
				result += " {}\n";
			}
		}
	}

	std::string Compiler::_compileUniformBlock(const TypeImpl& p_typeImpl)
	{
		DataRepresentation dataRep = buildDataRepresentation(p_typeImpl);

		std::string result;

		result = std::regex_replace(p_typeImpl.name, std::regex("::"), "_") + " " + p_typeImpl.name.substr(0, p_typeImpl.name.size() - 5) + " " +
			std::to_string(dataRep.cpuSize) + " " + std::to_string(dataRep.gpuSize) + " {\n";

		appendAttributes(dataRep, 1, result);

		result += "}\n";

		return result;
	}
	
	void Compiler::applyPipelinePass(const PipelinePassImpl& p_pass, std::string& p_target,
		const std::vector<TypeImpl>& p_structures,
		const std::vector<TypeImpl>& p_attributes,
		const std::vector<TypeImpl>& p_constants)
	{
		for (const auto& type : p_pass.body.usedTypes)
		{
			std::string typeCode = "";

			if (std::find(p_structures.begin(), p_structures.end(), type) != p_structures.end())
			{
				typeCode = _compileTypeImpl("struct", type) + "\n\n";
			}
			else if (std::find(p_attributes.begin(), p_attributes.end(), type) != p_attributes.end())
			{
				typeCode = _compileTypeImpl("layout(attributes) uniform", type) + " " + type.name.substr(0, type.name.size() - 5) + ";\n\n";

				if (_insertedAttributes.contains(type.name) == false)
				{
					_insertedAttributes.insert(type.name);
					_product.attributeContent += _compileUniformBlock(type);
				}
			}
			else if (std::find(p_constants.begin(), p_constants.end(), type) != p_constants.end())
			{
				typeCode = _compileTypeImpl("layout(constants) uniform", type) + " " + type.name.substr(0, type.name.size() - 5) + ";\n\n";

				if (_insertedConstants.contains(type.name) == false)
				{
					_insertedConstants.insert(type.name);
					_product.constantContent += _compileUniformBlock(type);
				}
			}

			p_target += typeCode;
		}
	}

	Compiler::Product Compiler::_compile(const Parser::Output& p_input)
	{
		_product = Product();

		applyPipelineFlow(p_input.vertexPipelineFlows, p_input.fragmentPipelineFlows, p_input.outputPipelineFlows);

		applyPipelinePass(p_input.vertexPipelinePass, _product.vertexCodeContent, p_input.structures, p_input.attributes, p_input.constants);
		applyPipelinePass(p_input.fragmentPipelinePass, _product.fragmentCodeContent, p_input.structures, p_input.attributes, p_input.constants);

		applyTexture(p_input.textures);

		for (const auto& function : p_input.vertexPipelinePass.body.calledFunctions)
		{
			_product.vertexCodeContent += _compileFunction(function) + "\n\n";
		}

		for (const auto& function : p_input.fragmentPipelinePass.body.calledFunctions)
		{
			_product.fragmentCodeContent += _compileFunction(function) + "\n\n";
		}

		_product.vertexCodeContent += "void main()\n{\n" + p_input.vertexPipelinePass.body.code + "    out_instanceID = instanceID;\n}";
		_product.fragmentCodeContent += "void main()\n{\n" + p_input.fragmentPipelinePass.body.code + "}";

		std::regex word_regex(R"(layout\s*\(\s*location\s*=\s*0\s*\)\s*out\sflat\s+int\s+instanceID\s*;)");

		_product.vertexCodeContent = std::regex_replace(_product.vertexCodeContent, word_regex, "layout(location = 0) out flat int out_instanceID;");

		word_regex = std::regex("\\binstanceID\\b");

		_product.vertexCodeContent = std::regex_replace(_product.vertexCodeContent, word_regex, "gl_InstanceID");

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
			_product.layoutContent += std::to_string(i) + " " + p_vertexFlows[i].type.name + " " + p_vertexFlows[i].name + "\n";
			_product.vertexCodeContent += "layout (location = " + std::to_string(i) + ") in " + std::string(_flatTypes.contains(p_vertexFlows[i].type.name) == true ? "flat " : "") + p_vertexFlows[i].type.name + " " + p_vertexFlows[i].name + ";\n";
		}

		for (size_t i = 0; i < p_fragmentFlows.size(); i++)
		{
			_product.vertexCodeContent += "layout (location = " + std::to_string(i) + ") out " + std::string(_flatTypes.contains(p_fragmentFlows[i].type.name) == true ? "flat " : "") + p_fragmentFlows[i].type.name + " " + p_fragmentFlows[i].name + ";\n";
			_product.fragmentCodeContent += "layout (location = " + std::to_string(i) + ") in " + std::string(_flatTypes.contains(p_fragmentFlows[i].type.name) == true ? "flat " : "") + p_fragmentFlows[i].type.name + " " + p_fragmentFlows[i].name + ";\n";
		}

		for (size_t i = 0; i < p_outputFlows.size(); i++)
		{
			_product.frameBufferContent += std::to_string(i) + " " + p_outputFlows[i].type.name + " " + p_outputFlows[i].name + "\n";
			_product.fragmentCodeContent += "layout (location = " + std::to_string(i) + ") out " + std::string(_flatTypes.contains(p_outputFlows[i].type.name) == true ? "flat " : "") + p_outputFlows[i].type.name + " " + p_outputFlows[i].name + ";\n";
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