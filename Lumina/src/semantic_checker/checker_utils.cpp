#include "lumina_semantic_checker.hpp"

#include <regex>

namespace Lumina
{
	void _applyConversion(std::string& p_stringToConvert)
	{
		p_stringToConvert = std::regex_replace(p_stringToConvert, std::regex(R"(\Vector2\b)"), "vec2");
		p_stringToConvert = std::regex_replace(p_stringToConvert, std::regex(R"(\Vector2Int\b)"), "ivec2");
		p_stringToConvert = std::regex_replace(p_stringToConvert, std::regex(R"(\Vector2UInt\b)"), "uvec2");

		p_stringToConvert = std::regex_replace(p_stringToConvert, std::regex(R"(\Vector3\b)"), "vec3");
		p_stringToConvert = std::regex_replace(p_stringToConvert, std::regex(R"(\Vector3Int\b)"), "ivec3");
		p_stringToConvert = std::regex_replace(p_stringToConvert, std::regex(R"(\Vector3UInt\b)"), "uvec3");

		p_stringToConvert = std::regex_replace(p_stringToConvert, std::regex(R"(\Vector4\b)"), "vec4");
		p_stringToConvert = std::regex_replace(p_stringToConvert, std::regex(R"(\Vector4Int\b)"), "ivec4");
		p_stringToConvert = std::regex_replace(p_stringToConvert, std::regex(R"(\Vector4UInt\b)"), "uvec4");

		p_stringToConvert = std::regex_replace(p_stringToConvert, std::regex(R"(\Matrix4x4\b)"), "mat4");
		p_stringToConvert = std::regex_replace(p_stringToConvert, std::regex(R"(\Matrix3x3\b)"), "mat3");
		p_stringToConvert = std::regex_replace(p_stringToConvert, std::regex(R"(\Matrix2x2\b)"), "mat2");
	}

	void _applytextureRenaming(std::string& p_stringToConvert)
	{
		std::regex samplerRegex(R"(sampler2D\s+(\w+))");

		std::smatch match;

		if (std::regex_search(p_stringToConvert, match, samplerRegex))
		{
			std::string textureName = match[1].str();

			std::regex textureNameRegex(R"(\b)" + textureName + R"(\b)");

			p_stringToConvert = std::regex_replace(p_stringToConvert, textureNameRegex, "luminaTexture_" + textureName);
		}

		p_stringToConvert = std::regex_replace(p_stringToConvert, std::regex(R"(\bgetPixel\b)"), "texture");
	}

	void _appendVersionString(std::string& p_stringToConvert)
	{
		p_stringToConvert = "#version 430\n\n" + p_stringToConvert;
	}

	void _convertPixelPositionToGL_Position(std::string& p_stringToConvert)
	{
		p_stringToConvert = std::regex_replace(p_stringToConvert, std::regex(R"(\pixelPosition\b)"), "gl_Position");
	}

	void SemanticChecker::Result::ShaderSection::convertLuminaToGLSL()
	{
		_applyConversion(layout);
		_applyConversion(constant);
		_applyConversion(attribute);
		_applyConversion(texture);
		_applyConversion(vertexShader);
		_applyConversion(fragmentShader);

		_applytextureRenaming(vertexShader);
		_applytextureRenaming(fragmentShader);

		_appendVersionString(vertexShader);
		_convertPixelPositionToGL_Position(vertexShader);
		_appendVersionString(fragmentShader);
	}

	size_t SemanticChecker::alignOffset(size_t p_currentOffset, size_t p_currentSize, size_t p_alignment)
	{
		size_t bytesLeft = p_currentOffset % p_alignment;

		if (bytesLeft + p_currentSize <= 16)
			return (p_currentOffset);

		if (p_currentOffset % p_alignment != 0)
		{
			p_currentOffset = ((p_currentOffset + p_alignment - 1) / p_alignment) * p_alignment;
		}
		return p_currentOffset;
	}

	void SemanticChecker::insertUniformDefinition(std::string& p_attributeContent, size_t p_tabulation, Type* p_typeToInsert)
	{
		for (const auto& nestedAttribute : p_typeToInsert->attributes)
		{
			p_attributeContent += std::string(p_tabulation, ' ') + nestedAttribute.name
				+ " " + std::to_string(nestedAttribute.cpu.offset)
				+ " " + std::to_string(nestedAttribute.cpu.size)
				+ " " + std::to_string(nestedAttribute.gpu.offset)
				+ " " + std::to_string(nestedAttribute.gpu.size);

			SemanticChecker::Type* attributeType = nestedAttribute.type;
			if (attributeType->attributes.empty() == false)
			{
				p_attributeContent += " {\n";
				insertUniformDefinition(p_attributeContent, p_tabulation + 4, attributeType);
				p_attributeContent += std::string(p_tabulation, ' ') + "}\n";
			}
			else
			{
				p_attributeContent += " {}\n";
			}

		}
	}

	std::string SemanticChecker::createNamespacePrefix() const
	{
		std::string result = "";

		for (size_t i = 0; i < _currentNamespace.size(); i++)
		{
			result += _currentNamespace[i].content + "::";
		}
		return (result);
	}

	void SemanticChecker::throwException(const std::filesystem::path& p_filePath, const std::string& p_errorMessage, const Token& p_errorToken)
	{
		if (_currentNamespace.size() == 0)
		{
			throw TokenBasedError(p_filePath, p_errorMessage, p_errorToken);
		}
		else
		{
			std::string namespacePrefix = createNamespacePrefix();

			throw TokenBasedError(p_filePath, p_errorMessage + " in namespace [" + namespacePrefix.substr(0, namespacePrefix.size() - 2) + "]", p_errorToken);
		}
	}

	void SemanticChecker::addType(const SemanticChecker::Type& p_type)
	{
		_types.push_back(p_type);
	}

	SemanticChecker::Type* SemanticChecker::type(const std::string& p_typeName)
	{
		std::vector<std::string> namespacePrefixes;
		std::string currentPrefix = "";

		for (const auto& ns : _currentNamespace)
		{
			if (!currentPrefix.empty())
			{
				currentPrefix += "::";
			}
			currentPrefix += ns.content;
			namespacePrefixes.push_back(currentPrefix);
		}

		namespacePrefixes.push_back("");

		for (const auto& prefix : namespacePrefixes)
		{
			std::string fullTypeName = prefix.empty() ? p_typeName : prefix + "::" + p_typeName;
			
			if (fullTypeName.substr(0, 2) == "::")
				fullTypeName = fullTypeName.substr(2, fullTypeName.size() - 2);

			auto it = std::find_if(_types.begin(), _types.end(), [&fullTypeName](const Type& type)
				{
					return type.name == fullTypeName;
				});

			if (it != _types.end())
			{
				return &(*it);
			}
		}

		return nullptr;
	}

	SemanticChecker::Type* SemanticChecker::type(const std::vector<Token>& p_tokens)
	{
		std::string typeName = "";
		for (const auto& token : p_tokens)
		{
			typeName += token.content;
		}
		return (type(typeName));
	}

	SemanticChecker::Type* SemanticChecker::standardType(const std::string& p_standardTypeName)
	{
		SemanticChecker::Type* result = type(p_standardTypeName);

		if (result == nullptr || _standardTypes.contains(result) == false)
			return (nullptr);
		return (result);
	}

	SemanticChecker::Type* SemanticChecker::structure(const std::string& p_structureName)
	{
		SemanticChecker::Type* result = type(p_structureName);

		if (result == nullptr || (_structures.contains(result) == false && _standardTypes.contains(result) == false))
			return (nullptr);
		return (result);
	}

	SemanticChecker::Type* SemanticChecker::attribute(const std::string& p_attributeName)
	{
		SemanticChecker::Type* result = type(p_attributeName);

		if (result == nullptr || _attributes.contains(result) == false)
			return (nullptr);
		return (result);
	}

	SemanticChecker::Type* SemanticChecker::constant(const std::string& p_constantName)
	{
		SemanticChecker::Type* result = type(p_constantName);

		if (result == nullptr || _constants.contains(result) == false)
			return (nullptr);
		return (result);
	}


	std::vector<SemanticChecker::Symbol>* SemanticChecker::symbolArray(const std::string& p_symbolName)
	{
		std::vector<std::string> namespacePrefixes;
		std::string currentPrefix = "";

		for (const auto& ns : _currentNamespace)
		{
			if (!currentPrefix.empty())
			{
				currentPrefix += "::";
			}
			currentPrefix += ns.content;
			namespacePrefixes.push_back(currentPrefix);
		}

		namespacePrefixes.push_back("");

		for (const auto& prefix : namespacePrefixes)
		{
			std::string fullSymbolName = prefix.empty() ? p_symbolName : prefix + "::" + p_symbolName;

			if (fullSymbolName.substr(0, 2) == "::")
				fullSymbolName = fullSymbolName.substr(2, fullSymbolName.size() - 2);

			if (_symbols.contains(fullSymbolName) == true)
			{
				return (&(_symbols[fullSymbolName]));
			}
		}
		
		return (nullptr);
	}
	
	void SemanticChecker::addSymbol(const SemanticChecker::Symbol& p_symbol)
	{
		std::vector<SemanticChecker::Symbol>& symbolArray = _symbols[p_symbol.name];

		symbolArray.push_back(p_symbol);
	}

	void SemanticChecker::addStandardType(const SemanticChecker::Type& p_standardType)
	{
		addType(p_standardType);
		_standardTypes.insert(type(p_standardType.name));
		for (const auto& constructor : p_standardType.constructors)
		{
			Symbol newSymbol;

			newSymbol.name = p_standardType.name;
			newSymbol.returnType = type(p_standardType.name);

			for (const auto& parameter : constructor)
				newSymbol.parameters.push_back({ "", parameter});

			addSymbol(newSymbol);
		}
	}
	
	void SemanticChecker::addStructure(const SemanticChecker::Type& p_structure)
	{
		addType(p_structure);
		_structures.insert(type(p_structure.name));
	}

	void SemanticChecker::addAttribute(const SemanticChecker::Type& p_attribute)
	{
		addType(p_attribute);
		_attributes.insert(type(p_attribute.name));

		_vertexPassVariables[p_attribute.name] = type(p_attribute.name);
		_fragmentPassVariables[p_attribute.name] = type(p_attribute.name);
	}

	void SemanticChecker::addConstant(const SemanticChecker::Type& p_constant)
	{
		addType(p_constant);
		_constants.insert(type(p_constant.name));

		_vertexPassVariables[p_constant.name] = type(p_constant.name);
		_fragmentPassVariables[p_constant.name] = type(p_constant.name);
	}
	void SemanticChecker::setupTypes()
	{
		addStandardType({
			.name = "void"
			});

		addStandardType({
			.name = "Texture"
			});

		addStandardType({
			.name = "bool",
			.cpuSize = sizeof(bool),
			.gpuSize = 1,
			.operators = { "&&", "||" },
			.comparaisonOperators = { "==", "!=" }
			});

		addStandardType({
			.name = "int",
			.cpuSize = sizeof(int),
			.gpuSize = 4,
			.operators = { "+", "-", "*", "/", "%", "+=", "-=", "*=", "/=", "%=" },
			.comparaisonOperators = { "==", "!=", "<", ">", "<=", ">=" }
			});

		addStandardType({
			.name = "float",
			.cpuSize = sizeof(float),
			.gpuSize = 4,
			.operators = { "+", "-", "*", "/", "+=", "-=", "*=", "/=" },
			.comparaisonOperators = { "==", "!=", "<", ">", "<=", ">=" }
			});

		addStandardType({
			.name = "uint",
			.cpuSize = sizeof(unsigned int),
			.gpuSize = 4,
			.operators = { "+", "-", "*", "/", "%", "+=", "-=", "*=", "/=", "%=" },
			.comparaisonOperators = { "==", "!=", "<", ">", "<=", ">=" }
			});

		addStandardType({
			.name = "Vector2",
			.cpuSize = sizeof(float) * 2,
			.gpuSize = 8,
			.attributes = {
				{
					.type = type("float"),
					.name = "x",
					.nbElement = 1,
					.cpu = {.offset = 0, .size = sizeof(float) },
					.gpu = {.offset = 0, .size = 4 }
				},
				{
					.type = type("float"),
					.name = "y",
					.nbElement = 1,
					.cpu = {.offset = sizeof(float), .size = sizeof(float) },
					.gpu = {.offset = 4, .size = 4 }
				}
			},
			.operators = { "+", "-", "*", "/", "+=", "-=", "*=", "/=" },
			.comparaisonOperators = { "==", "!=" },
			.constructors = {
				{type("float"), type("float")}
			}
			});

		addStandardType({
			.name = "Vector2Int",
			.cpuSize = sizeof(int) * 2,
			.gpuSize = 8,
			.attributes = {
				{
					.type = type("int"),
					.name = "x",
					.nbElement = 1,
					.cpu = {.offset = 0, .size = sizeof(int) },
					.gpu = {.offset = 0, .size = 4 }
				},
				{
					.type = type("int"),
					.name = "y",
					.nbElement = 1,
					.cpu = {.offset = sizeof(int), .size = sizeof(int) },
					.gpu = {.offset = 4, .size = 4 }
				}
			},
			.operators = { "+", "-", "*", "/", "+=", "-=", "*=", "/=" },
			.comparaisonOperators = { "==", "!=" },
			.constructors = {
				{type("int"), type("int")}
			}
			});

		addStandardType({
			.name = "Vector2UInt",
			.cpuSize = sizeof(unsigned int) * 2,
			.gpuSize = 8,
			.attributes = {
				{
					.type = type("uint"),
					.name = "x",
					.nbElement = 1,
					.cpu = {.offset = 0, .size = sizeof(unsigned int) },
					.gpu = {.offset = 0, .size = 4 }
				},
				{
					.type = type("uint"),
					.name = "y",
					.nbElement = 1,
					.cpu = {.offset = sizeof(unsigned int), .size = sizeof(unsigned int) },
					.gpu = {.offset = 4, .size = 4 }
				}
			},
			.operators = { "+", "-", "*", "/", "+=", "-=", "*=", "/=" },
			.comparaisonOperators = { "==", "!=" },
			.constructors = {
				{type("uint"), type("uint")}
			}
			});

		addStandardType({
			.name = "Vector3",
			.cpuSize = sizeof(float) * 3,
			.gpuSize = 12,
			.attributes = {
				{
					.type = type("float"),
					.name = "x",
					.nbElement = 1,
					.cpu = {.offset = 0, .size = sizeof(float) },
					.gpu = {.offset = 0, .size = 4 }
				},
				{
					.type = type("float"),
					.name = "y",
					.nbElement = 1,
					.cpu = {.offset = sizeof(float), .size = sizeof(float) },
					.gpu = {.offset = 4, .size = 4 }
				},
				{
					.type = type("float"),
					.name = "z",
					.nbElement = 1,
					.cpu = {.offset = sizeof(float) * 2, .size = sizeof(float) },
					.gpu = {.offset = 8, .size = 4 }
				},
				{
					.type = type("float"),
					.name = "xy",
					.nbElement = 2,
					.cpu = {.offset = 0, .size = sizeof(float) * 2 },
					.gpu = {.offset = 0, .size = 8 }
				}
			},
			.operators = { "+", "-", "*", "/", "+=", "-=", "*=", "/=" },
			.comparaisonOperators = { "==", "!=" },
			.constructors = {
				{type("float"), type("float"), type("float")},
				{type("Vector2"), type("float")},
				{type("float"), type("Vector2")}
			}
			});

		addStandardType({
			.name = "Vector3Int",
			.cpuSize = sizeof(int) * 3,
			.gpuSize = 12,
			.attributes = {
				{
					.type = type("int"),
					.name = "x",
					.nbElement = 1,
					.cpu = {.offset = 0, .size = sizeof(int) },
					.gpu = {.offset = 0, .size = 4 }
				},
				{
					.type = type("int"),
					.name = "y",
					.nbElement = 1,
					.cpu = {.offset = sizeof(int), .size = sizeof(int) },
					.gpu = {.offset = 4, .size = 4 }
				},
				{
					.type = type("int"),
					.name = "z",
					.nbElement = 1,
					.cpu = {.offset = sizeof(int) * 2, .size = sizeof(int) },
					.gpu = {.offset = 8, .size = 4 }
				},
				{
					.type = type("int"),
					.name = "xy",
					.nbElement = 2,
					.cpu = {.offset = 0, .size = sizeof(int) * 2 },
					.gpu = {.offset = 0, .size = 8 }
				}
			},
			.operators = { "+", "-", "*", "/", "+=", "-=", "*=", "/=" },
			.comparaisonOperators = { "==", "!=" },
			.constructors = {
				{type("int"), type("int"), type("int")},
				{type("Vector2Int"), type("int")},
				{type("int"), type("Vector2Int")}
			}
			});

		addStandardType({
			.name = "Vector3UInt",
			.cpuSize = sizeof(unsigned int) * 3,
			.gpuSize = 12,
			.attributes = {
				{
					.type = type("uint"),
					.name = "x",
					.nbElement = 1,
					.cpu = {.offset = 0, .size = sizeof(unsigned int) },
					.gpu = {.offset = 0, .size = 4 }
				},
				{
					.type = type("uint"),
					.name = "y",
					.nbElement = 1,
					.cpu = {.offset = sizeof(unsigned int), .size = sizeof(unsigned int) },
					.gpu = {.offset = 4, .size = 4 }
				},
				{
					.type = type("uint"),
					.name = "z",
					.nbElement = 1,
					.cpu = {.offset = sizeof(unsigned int) * 2, .size = sizeof(unsigned int) },
					.gpu = {.offset = 8, .size = 4 }
				},
				{
					.type = type("uint"),
					.name = "xy",
					.nbElement = 2,
					.cpu = {.offset = 0, .size = sizeof(unsigned int) * 2 },
					.gpu = {.offset = 0, .size = 8 }
				}
			},
			.operators = { "+", "-", "*", "/", "+=", "-=", "*=", "/=" },
			.comparaisonOperators = { "==", "!=" },
			.constructors = {
				{type("uint"), type("uint"), type("uint")},
				{type("Vector2UInt"), type("uint")},
				{type("uint"), type("Vector2UInt")}
			}
			});

		addStandardType({
			.name = "Vector4",
			.cpuSize = sizeof(float) * 4,
			.gpuSize = 16,
			.attributes = {
				{
					.type = type("float"),
					.name = "x",
					.nbElement = 1,
					.cpu = {.offset = 0, .size = sizeof(float) },
					.gpu = {.offset = 0, .size = 4 }
				},
				{
					.type = type("float"),
					.name = "y",
					.nbElement = 1,
					.cpu = {.offset = sizeof(float), .size = sizeof(float) },
					.gpu = {.offset = 4, .size = 4 }
				},
				{
					.type = type("float"),
					.name = "z",
					.nbElement = 1,
					.cpu = {.offset = sizeof(float) * 2, .size = sizeof(float) },
					.gpu = {.offset = 8, .size = 4 }
				},
				{
					.type = type("float"),
					.name = "w",
					.nbElement = 1,
					.cpu = {.offset = sizeof(float) * 3, .size = sizeof(float) },
					.gpu = {.offset = 12, .size = 4 }
				},
				{
					.type = type("float"),
					.name = "xy",
					.nbElement = 2,
					.cpu = {.offset = 0, .size = sizeof(float) * 2 },
					.gpu = {.offset = 0, .size = 8 }
				},
				{
					.type = type("float"),
					.name = "xyz",
					.nbElement = 3,
					.cpu = {.offset = 0, .size = sizeof(float) * 3 },
					.gpu = {.offset = 0, .size = 12 }
				}
			},
			.operators = { "+", "-", "*", "/", "+=", "-=", "*=", "/=" },
			.comparaisonOperators = { "==", "!=" },
			.constructors = {
				{type("float"), type("float"), type("float"), type("float")},
				{type("Vector3"), type("float")},
				{type("float"), type("Vector3")},
				{type("Vector2"), type("Vector2")},
				{type("float"), type("Vector2"), type("float")},
				{type("Vector2"), type("float"), type("float")},
				{type("float"), type("float"), type("Vector2")}
			}
			});

		addStandardType({
			.name = "Vector4Int",
			.cpuSize = sizeof(int) * 4,
			.gpuSize = 16,
			.attributes = {
				{
					.type = type("int"),
					.name = "x",
					.nbElement = 1,
					.cpu = {.offset = 0, .size = sizeof(int) },
					.gpu = {.offset = 0, .size = 4 }
				},
				{
					.type = type("int"),
					.name = "y",
					.nbElement = 1,
					.cpu = {.offset = sizeof(int), .size = sizeof(int) },
					.gpu = {.offset = 4, .size = 4 }
				},
				{
					.type = type("int"),
					.name = "z",
					.nbElement = 1,
					.cpu = {.offset = sizeof(int) * 2, .size = sizeof(int) },
					.gpu = {.offset = 8, .size = 4 }
				},
				{
					.type = type("int"),
					.name = "w",
					.nbElement = 1,
					.cpu = {.offset = sizeof(int) * 3, .size = sizeof(int) },
					.gpu = {.offset = 12, .size = 4 }
				},
				{
					.type = type("int"),
					.name = "xy",
					.nbElement = 2,
					.cpu = {.offset = 0, .size = sizeof(int) * 2 },
					.gpu = {.offset = 0, .size = 8 }
				},
				{
					.type = type("int"),
					.name = "xyz",
					.nbElement = 3,
					.cpu = {.offset = 0, .size = sizeof(int) * 3 },
					.gpu = {.offset = 0, .size = 12 }
				}
			},
			.operators = { "+", "-", "*", "/", "+=", "-=", "*=", "/=" },
			.comparaisonOperators = { "==", "!=" },
			.constructors = {
				{type("int"), type("int"), type("int"), type("int")},
				{type("Vector3Int"), type("int")},
				{type("int"), type("Vector3Int")},
				{type("Vector2Int"), type("Vector2Int")},
				{type("int"), type("Vector2Int"), type("int")},
				{type("Vector2Int"), type("int"), type("int")},
				{type("int"), type("int"), type("Vector2Int")}
			}
			});

		addStandardType({
			.name = "Vector4UInt",
			.cpuSize = sizeof(unsigned int) * 4,
			.gpuSize = 16,
			.attributes = {
				{
					.type = type("uint"),
					.name = "x",
					.nbElement = 1,
					.cpu = {.offset = 0, .size = sizeof(unsigned int) },
					.gpu = {.offset = 0, .size = 4 }
				},
				{
					.type = type("uint"),
					.name = "y",
					.nbElement = 1,
					.cpu = {.offset = sizeof(unsigned int), .size = sizeof(unsigned int) },
					.gpu = {.offset = 4, .size = 4 }
				},
				{
					.type = type("uint"),
					.name = "z",
					.nbElement = 1,
					.cpu = {.offset = sizeof(unsigned int) * 2, .size = sizeof(unsigned int) },
					.gpu = {.offset = 8, .size = 4 }
				},
				{
					.type = type("uint"),
					.name = "w",
					.nbElement = 1,
					.cpu = {.offset = sizeof(unsigned int) * 3, .size = sizeof(unsigned int) },
					.gpu = {.offset = 12, .size = 4 }
				},
				{
					.type = type("uint"),
					.name = "xy",
					.nbElement = 2,
					.cpu = {.offset = 0, .size = sizeof(unsigned int) * 2 },
					.gpu = {.offset = 0, .size = 8 }
				},
				{
					.type = type("uint"),
					.name = "xyz",
					.nbElement = 3,
					.cpu = {.offset = 0, .size = sizeof(unsigned int) * 3 },
					.gpu = {.offset = 0, .size = 12 }
				}
			},
			.operators = { "+", "-", "*", "/", "+=", "-=", "*=", "/=" },
			.comparaisonOperators = { "==", "!=" },
			.constructors = {
				{type("uint"), type("uint"), type("uint"), type("uint")},
				{type("Vector3UInt"), type("uint")},
				{type("uint"), type("Vector3UInt")},
				{type("Vector2UInt"), type("Vector2UInt")},
				{type("uint"), type("Vector2UInt"), type("uint")},
				{type("Vector2UInt"), type("uint"), type("uint")},
				{type("uint"), type("uint"), type("Vector2UInt")}
			}
			});

		addStandardType({
			.name = "Color",
			.cpuSize = sizeof(float) * 4,
			.gpuSize = 16,
			.attributes = {
				{
					.type = type("float"),
					.name = "r",
					.nbElement = 1,
					.cpu = {.offset = 0, .size = sizeof(float) },
					.gpu = {.offset = 0, .size = 4 }
				},
				{
					.type = type("float"),
					.name = "g",
					.nbElement = 1,
					.cpu = {.offset = sizeof(float), .size = sizeof(float) },
					.gpu = {.offset = 4, .size = 4 }
				},
				{
					.type = type("float"),
					.name = "b",
					.nbElement = 1,
					.cpu = {.offset = sizeof(float) * 2, .size = sizeof(float) },
					.gpu = {.offset = 8, .size = 4 }
				},
				{
					.type = type("float"),
					.name = "a",
					.nbElement = 1,
					.cpu = {.offset = sizeof(float) * 3, .size = sizeof(float) },
					.gpu = {.offset = 12, .size = 4 }
				},
				{
					.type = type("float"),
					.name = "rgb",
					.nbElement = 3,
					.cpu = {.offset = 0, .size = sizeof(float) * 3},
					.gpu = {.offset = 0, .size = 12 }
				},
				{
					.type = type("float"),
					.name = "rgba",
					.nbElement = 4,
					.cpu = {.offset = 0, .size = sizeof(float) * 4},
					.gpu = {.offset = 0, .size = 16 }
				}
			},
			.operators = { "+", "-", "*", "/", "+=", "-=", "*=", "/=" },
			.comparaisonOperators = { "==", "!=" },
			.constructors = {
				{type("float"), type("float"), type("float"), type("float")},
				{type("Vector3"), type("float")},
				{type("float"), type("Vector3")},
				{type("Vector2"), type("Vector2")},
				{type("float"), type("Vector2"), type("float")},
				{type("Vector2"), type("float"), type("float")},
				{type("float"), type("float"), type("Vector2")}
			}
			});


		// Accepted type conversions (unchanged)
		type("int")->acceptedConversions = { type("float"), type("uint") };
		type("float")->acceptedConversions = { type("int"), type("uint") };
		type("uint")->acceptedConversions = { type("int"), type("float") };
		type("bool")->acceptedConversions = {};

		type("Vector2")->acceptedConversions = { type("Vector2Int"), type("Vector2UInt") };
		type("Vector2Int")->acceptedConversions = { type("Vector2"), type("Vector2UInt") };
		type("Vector2UInt")->acceptedConversions = { type("Vector2"), type("Vector2Int") };

		type("Vector3")->acceptedConversions = { type("Vector3Int"), type("Vector3UInt") };
		type("Vector3Int")->acceptedConversions = { type("Vector3"), type("Vector3UInt") };
		type("Vector3UInt")->acceptedConversions = { type("Vector3"), type("Vector3Int") };

		type("Color")->acceptedConversions = { type("Vector4") };
		type("Vector4")->acceptedConversions = { type("Vector4Int"), type("Vector4UInt"), type("Color") };
		type("Vector4Int")->acceptedConversions = { type("Vector4"), type("Vector4UInt") };
		type("Vector4UInt")->acceptedConversions = { type("Vector4"), type("Vector4Int") };
	}

	void SemanticChecker::setupStructures()
	{
		addStructure({
			.name = "Matrix2x2",
			.cpuSize = sizeof(float) * 4,  // 2x2 matrix
			.gpuSize = 16,
			.attributes = {},  // No attributes for matrices as a whole entity
			.acceptedConversions = { type("Vector2"), type("Vector2Int"), type("Vector2UInt") },
			.operators = { "*" },
			});

		addStructure({
			.name = "Matrix3x3",
			.cpuSize = sizeof(float) * 9,  // 3x3 matrix
			.gpuSize = 36,
			.attributes = {},  // No attributes for matrices as a whole entity
			.acceptedConversions = { type("Vector3"), type("Vector3Int"), type("Vector3UInt") },
			.operators = { "*" },
			});

		addStructure({
			.name = "Matrix4x4",
			.cpuSize = sizeof(float) * 16,  // 4x4 matrix
			.gpuSize = 64,
			.attributes = {},  // No attributes for matrices as a whole entity
			.acceptedConversions = { type("Vector4"), type("Vector4Int"), type("Vector4UInt") },
			.operators = { "*" },
			});

		type("Vector2")->acceptedConversions.insert(type("Matrix2x2"));
		type("Vector3")->acceptedConversions.insert(type("Matrix3x3"));
		type("Vector4")->acceptedConversions.insert(type("Matrix4x4"));
	}

	void SemanticChecker::setupSymbols()
	{
		_symbols["mix"].push_back(
			{
				type("Vector2"),
				"mix",
				{
					{"x", type("Vector2")},
					{"y", type("Vector2")},
					{"a", type("float")}
				}
			});
		_symbols["mix"].push_back(
			{
				type("Vector3"),
				"mix",
				{
					{"x", type("Vector3")},
					{"y", type("Vector3")},
					{"a", type("float")}
				}
			});
		_symbols["mix"].push_back(
			{
				type("Vector4"),
				"mix",
				{
					{"x", type("Vector4")},
					{"y", type("Vector4")},
					{"a", type("float")}
				}
			});

		_symbols["normalize"].push_back(
			{
				type("Vector2"),
				"normalize",
				{
					{"v", type("Vector2")}
				}
			});

		_symbols["normalize"].push_back(
			{
				type("Vector3"),
				"normalize",
				{
					{"v", type("Vector3")}
				}
			});

		_symbols["normalize"].push_back(
			{
				type("Vector4"),
				"normalize",
				{
					{"v", type("Vector4")}
				}
			});

		_symbols["dot"].push_back(
			{
				type("float"),
				"dot",
				{
					{"x", type("Vector2")},
					{"y", type("Vector2")}
				}
			});

		_symbols["dot"].push_back(
			{
				type("float"),
				"dot",
				{
					{"x", type("Vector3")},
					{"y", type("Vector3")}
				}
			});

		_symbols["dot"].push_back(
			{
				type("float"),
				"dot",
				{
					{"x", type("Vector4")},
					{"y", type("Vector4")}
				}
			});

		_symbols["cross"].push_back(
			{
				type("Vector3"),
				"cross",
				{
					{"x", type("Vector3")},
					{"y", type("Vector3")}
				}
			});

		_symbols["length"].push_back(
			{
				type("float"),
				"length",
				{
					{"v", type("Vector2")}
				}
			});

		_symbols["length"].push_back(
			{
				type("float"),
				"length",
				{
					{"v", type("Vector3")}
				}
			});

		_symbols["length"].push_back(
			{
				type("float"),
				"length",
				{
					{"v", type("Vector4")}
				}
			});

		_symbols["transpose"].push_back(
			{
				type("Matrix2x2"),
				"transpose",
				{
					{"m", type("Matrix2x2")}
				}
			});

		_symbols["transpose"].push_back(
			{
				type("Matrix3x3"),
				"transpose",
				{
					{"m", type("Matrix3x3")}
				}
			});

		_symbols["transpose"].push_back(
			{
				type("Matrix4x4"),
				"transpose",
				{
					{"m", type("Matrix4x4")}
				}
			});

		_symbols["determinant"].push_back(
			{
				type("float"),
				"determinant",
				{
					{"m", type("Matrix2x2")}
				}
			});

		_symbols["determinant"].push_back(
			{
				type("float"),
				"determinant",
				{
					{"m", type("Matrix3x3")}
				}
			});

		_symbols["determinant"].push_back(
			{
				type("float"),
				"determinant",
				{
					{"m", type("Matrix4x4")}
				}
			});

		_symbols["reflect"].push_back(
			{
				type("Vector2"),
				"reflect",
				{
					{"I", type("Vector2")},
					{"N", type("Vector2")}
				}
			});

		_symbols["reflect"].push_back(
			{
				type("Vector3"),
				"reflect",
				{
					{"I", type("Vector3")},
					{"N", type("Vector3")}
				}
			});

		_symbols["reflect"].push_back(
			{
				type("Vector4"),
				"reflect",
				{
					{"I", type("Vector4")},
					{"N", type("Vector4")}
				}
			});

		_symbols["refract"].push_back(
			{
				type("Vector2"),
				"refract",
				{
					{"I", type("Vector2")},
					{"N", type("Vector2")},
					{"eta", type("float")}
				}
			});

		_symbols["refract"].push_back(
			{
				type("Vector3"),
				"refract",
				{
					{"I", type("Vector3")},
					{"N", type("Vector3")},
					{"eta", type("float")}
				}
			});

		_symbols["refract"].push_back(
			{
				type("Vector4"),
				"refract",
				{
					{"I", type("Vector4")},
					{"N", type("Vector4")},
					{"eta", type("float")}
				}
			});

		_symbols["clamp"].push_back(
			{
				type("float"),
				"clamp",
				{
					{"x", type("float")},
					{"minVal", type("float")},
					{"maxVal", type("float")}
				}
			});

		_symbols["clamp"].push_back(
			{
				type("Vector2"),
				"clamp",
				{
					{"x", type("Vector2")},
					{"minVal", type("Vector2")},
					{"maxVal", type("Vector2")}
				}
			});

		_symbols["clamp"].push_back(
			{
				type("Vector3"),
				"clamp",
				{
					{"x", type("Vector3")},
					{"minVal", type("Vector3")},
					{"maxVal", type("Vector3")}
				}
			});

		_symbols["clamp"].push_back(
			{
				type("Vector4"),
				"clamp",
				{
					{"x", type("Vector4")},
					{"minVal", type("Vector4")},
					{"maxVal", type("Vector4")}
				}
			});

		_symbols["max"].push_back(
			{
				type("float"),
				"max",
				{
					{"x", type("float")},
					{"y", type("float")}
				}
			});

		_symbols["max"].push_back(
			{
				type("Vector2"),
				"max",
				{
					{"x", type("Vector2")},
					{"y", type("Vector2")}
				}
			});

		_symbols["max"].push_back(
			{
				type("Vector3"),
				"max",
				{
					{"x", type("Vector3")},
					{"y", type("Vector3")}
				}
			});

		_symbols["max"].push_back(
			{
				type("Vector4"),
				"max",
				{
					{"x", type("Vector4")},
					{"y", type("Vector4")}
				}
			});

		_symbols["min"].push_back(
			{
				type("float"),
				"min",
				{
					{"x", type("float")},
					{"y", type("float")}
				}
			});

		_symbols["min"].push_back(
			{
				type("Vector2"),
				"min",
				{
					{"x", type("Vector2")},
					{"y", type("Vector2")}
				}
			});

		_symbols["min"].push_back(
			{
				type("Vector3"),
				"min",
				{
					{"x", type("Vector3")},
					{"y", type("Vector3")}
				}
			});

		_symbols["min"].push_back(
			{
				type("Vector4"),
				"min",
				{
					{"x", type("Vector4")},
					{"y", type("Vector4")}
				}
			});

		_symbols["smoothstep"].push_back(
			{
				type("float"),
				"smoothstep",
				{
					{"edge0", type("float")},
					{"edge1", type("float")},
					{"x", type("float")}
				}
			});

		_symbols["smoothstep"].push_back(
			{
				type("Vector2"),
				"smoothstep",
				{
					{"edge0", type("Vector2")},
					{"edge1", type("Vector2")},
					{"x", type("float")}
				}
			});

		_symbols["smoothstep"].push_back(
			{
				type("Vector3"),
				"smoothstep",
				{
					{"edge0", type("Vector3")},
					{"edge1", type("Vector3")},
					{"x", type("float")}
				}
			});

		_symbols["smoothstep"].push_back(
			{
				type("Vector4"),
				"smoothstep",
				{
					{"edge0", type("Vector4")},
					{"edge1", type("Vector4")},
					{"x", type("float")}
				}
			});

		_symbols["inverse"].push_back(
			{
				type("Matrix2x2"),
				"inverse",
				{
					{"m", type("Matrix2x2")}
				}
			});

		_symbols["inverse"].push_back(
			{
				type("Matrix3x3"),
				"inverse",
				{
					{"m", type("Matrix3x3")}
				}
			});

		_symbols["inverse"].push_back(
			{
				type("Matrix4x4"),
				"inverse",
				{
					{"m", type("Matrix4x4")}
				}
			});

		_symbols["getPixel"].push_back(
			{
				type("Vector4"),
				"getPixel",
				{
					{"texture", type("Texture")},
					{"uv", type("Vector2")}
				}
			});
	}
	
	std::ostream& operator<<(std::ostream& os, const SemanticChecker::Type& type)
	{
		os << "Type: " << type.name << "\n";
		os << "Attributes:\n";
		for (const auto& attr : type.attributes)
		{
			os << "  - " << attr.name << ": " << (attr.type ? attr.type->name : "null") << (attr.nbElement == 0 ? "" : "[" + std::to_string(attr.nbElement) + "]") << "\n";
		}
		os << "Accepted Conversions:\n";
		for (const auto& conv : type.acceptedConversions)
		{
			os << "  - " << (conv ? conv->name : "null") << "\n";
		}
		os << "Accept Operations:\n";
		if (type.operators.size() == 0)
			os << " - None";
		for (const auto& conv : type.operators)
		{
			os << "  - " << conv << "\n";
		}
		os << "Constructors:\n";
		for (const auto& constructor : type.constructors)
		{
			os << "  " << type.name << "(";
			for (size_t i = 0; i < constructor.size(); ++i)
			{
				os << (constructor[i] ? constructor[i]->name : "null");
				if (i < constructor.size() - 1)
				{
					os << ", ";
				}
			}
			os << ")\n";
		}
		return os;
	}
}