#pragma once

#include "semantic_parser.hpp"

#include <string>
#include <vector>

struct StageIO
{
	int location = 0;
	std::string type;
	std::string name;
	bool flat = false;
};

struct TextureBinding
{
	int location = 0;
	std::string luminaName;
	std::string glslName;
	std::string type;
	TextureBindingScope scope = TextureBindingScope::Constant;
};

struct ConverterInput
{
	const SemanticParseResult &semantic;
	std::vector<StageIO> vertexInputs;
	std::vector<StageIO> stageVaryings;
	std::vector<StageIO> fragmentOutputs;
	std::vector<TextureBinding> textures;
};

struct ShaderSources
{
	std::string vertex;
	std::string fragment;
};

struct Converter
{
	ShaderSources operator()(const ConverterInput &input) const;
};
