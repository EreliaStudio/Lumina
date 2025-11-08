#pragma once

#include "semantic_parser.hpp"

#include <string>
#include <vector>

struct StageIO
{
	int location = 0;
	std::string type;
	std::string name;
};

struct TextureBinding
{
	std::string luminaName;
	std::string glslName;
	std::string type;
};

struct ConverterInput
{
	const SemanticParseResult &semantic;
	std::vector<StageIO> vertexInputs;
	std::vector<StageIO> stageVaryings;
	std::vector<StageIO> fragmentOutputs;
	std::vector<TextureBinding> textures;
	bool hasPixelColor = false;
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
