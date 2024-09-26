#pragma once

#include <fstream>
#include <set>
#include <map>

#include "lumina_tokenizer.hpp"
#include "lumina_exception.hpp"
#include "lumina_utils.hpp"
#include "lumina_metatokenizer.hpp"

namespace Lumina
{
	struct Shader
	{
		std::string inputLayouts;
		std::string outputLayouts;
		std::string constants;
		std::string attributes;
		std::string textures;
		std::string vertexShaderCode;
		std::string fragmentShaderCode;
	};

	struct Compiler
	{
	public:
		using Product = Expected<Shader>;
	private:
		struct Type
		{
			std::string name = "";
			size_t cpuSize;
			size_t gpuSize;
			size_t padding = 0;

			struct Element
			{
				const Type* type;
				std::string name;
				size_t cpuOffset;
				size_t gpuOffset;
				std::vector<size_t> arraySize;
			};

			std::vector<Element> innerElements;

			bool contains(const std::string& p_name)
			{
				for (const auto& element : innerElements)
				{
					if (element.name == p_name)
						return (true);
				}
				return (false);
			}
		
			bool operator<(const Type& other) const
			{
				return name < other.name;
			}
		};

		struct Variable
		{
			const Type* type;
			std::string name;
			std::vector<size_t> arraySizes;
		};

		Product _result;

		size_t nbVertexLayout = 0;
		size_t nbFragmentLayout = 0;
		size_t nbOutputLayout = 0;

		std::vector<std::string> _namespaceNames;

		std::string namespacePrefix(const std::string& p_namespaceSeparator = "_")
		{
			std::string result;

			for (const std::string& name : _namespaceNames)
			{
				result += name + p_namespaceSeparator;
			}

			return (result);
		}

		std::set<Type> _types;
		std::set<Type> _standardTypes;
		std::set<Type> _luminaTypes;
		std::set<Type> _structureTypes;
		static inline const std::string ATTRIBUTE_PREFIX = "Attribute_";
		std::set<Type> _attributeTypes;
		static inline const std::string CONSTANT_PREFIX = "Constant_";
		std::set<Type> _constantTypes;

		std::map<std::string, Variable> _variables;

		static inline const std::string TEXTURE_PREFIX = "Texture_";
		std::vector<const Variable*> _textures;


		void compilePipelineFlow(std::shared_ptr<PipelineFlowMetaToken> p_metaToken)
		{
			if (p_metaToken->inputFlow == "Input")
			{
				if (p_metaToken->outputFlow == "VertexPass")
				{
					const Type* flowType = type(p_metaToken->variableDescriptor.type.value.content);

					if (flowType == nullptr)
					{
						throw TokenBasedError("Type : " + p_metaToken->variableDescriptor.type.value.content + " not found", p_metaToken->variableDescriptor.type.value);
					}

					_result.value.inputLayouts += std::to_string(nbVertexLayout) + " " + p_metaToken->variableDescriptor.type.value.content + "\n";

					_result.value.vertexShaderCode += "layout(location = " + std::to_string(nbVertexLayout) + ") in " + p_metaToken->variableDescriptor.type.value.content + " " + p_metaToken->variableDescriptor.name.content + ";\n";
					nbVertexLayout++;
				}
				else
				{
					throw TokenBasedError("Invalid pipeline flow ouput token. Only \"VertexPass\" is valid with \"Input\" input token.", p_metaToken->outputFlow);
				}
			}
			else if (p_metaToken->inputFlow == "VertexPass")
			{
				if (p_metaToken->outputFlow == "FragmentPass")
				{
					const Type* flowType = type(p_metaToken->variableDescriptor.type.value.content);

					if (flowType == nullptr)
					{
						throw TokenBasedError("Type : " + p_metaToken->variableDescriptor.type.value.content + " not found", p_metaToken->variableDescriptor.type.value);
					}

					_result.value.vertexShaderCode += "layout(location = " + std::to_string(nbFragmentLayout) + ") out " + p_metaToken->variableDescriptor.type.value.content + " " + p_metaToken->variableDescriptor.name.content + ";\n";
					_result.value.fragmentShaderCode += "layout(location = " + std::to_string(nbFragmentLayout) + ") in " + p_metaToken->variableDescriptor.type.value.content + " " + p_metaToken->variableDescriptor.name.content + ";\n";

					nbFragmentLayout++;
				}
				else
				{
					throw TokenBasedError("Invalid pipeline flow ouput token. Only \"FragmentPass\" is valid with \"VertexPass\" input token.", p_metaToken->outputFlow);
				}
			}
			else if (p_metaToken->inputFlow == "FragmentPass")
			{
				if (p_metaToken->outputFlow == "Output")
				{
					const Type* flowType = type(p_metaToken->variableDescriptor.type.value.content);

					if (flowType == nullptr)
					{
						throw TokenBasedError("Type : " + p_metaToken->variableDescriptor.type.value.content + " not found", p_metaToken->variableDescriptor.type.value);
					}

					_result.value.outputLayouts += std::to_string(nbOutputLayout) + " " + p_metaToken->variableDescriptor.type.value.content + "\n";
					_result.value.fragmentShaderCode += "layout(location = " + std::to_string(nbOutputLayout) + ") out " + p_metaToken->variableDescriptor.type.value.content + " " + p_metaToken->variableDescriptor.name.content + ";\n";

					nbOutputLayout++;
				}
				else
				{
					throw TokenBasedError("Invalid pipeline flow ouput token. Only \"Output\" is valid with \"FragmentPass\" input token.", p_metaToken->outputFlow);
				}
			}
			else
			{
				throw TokenBasedError("Invalid pipeline flow input token. Only \"Input\", \"VertexPass\" and \"FragmentPass\" are valid pipeline flow input.", p_metaToken->inputFlow);
			}
		}

		Type composeType(std::shared_ptr<BlockMetaToken> p_metaToken)
		{
			if (type(p_metaToken->name.content) != nullptr)
			{
				throw TokenBasedError("Type [" + p_metaToken->name.content + "] already defined.", p_metaToken->name);
			}

			Type result = Type();

			size_t cpuOffset = 0;
			size_t gpuOffset = 0;
			result.name = p_metaToken->name.content;
			for (const auto& element : p_metaToken->elements)
			{
				if (result.contains(element.name.content) == true)
				{
					throw TokenBasedError("Attribute [" + element.name.content + "] already defined in [" + result.name + "] structure.", element.name);
				}

				Type::Element newElement;

				newElement.name = element.name.content;
				newElement.type = type(element.type.value.content);
				if (newElement.type == type("Texture"))
				{
					throw TokenBasedError("Texture can't be placed inside block.", element.name);
				}
				newElement.arraySize = element.arraySizes;

				size_t totalSize = 1;
				for (const auto& size : newElement.arraySize)
					totalSize *= size;

				size_t padding = 0;
				if (newElement.type->gpuSize == 12)
					padding = 4;
				else if (newElement.type->gpuSize >= 16)
					padding = (16 - (newElement.type->gpuSize % 16)) % 16;

				if ((gpuOffset % 16) != 0)
				{
					size_t bytesLeft = 16 - (gpuOffset % 16);
					if (bytesLeft < newElement.type->gpuSize)
					{
						gpuOffset += bytesLeft;
					}
				}

				newElement.gpuOffset = gpuOffset;
				gpuOffset += (newElement.type->gpuSize + padding) * totalSize;

				newElement.cpuOffset = cpuOffset;
				cpuOffset += newElement.type->cpuSize * totalSize;


				if (newElement.type == nullptr)
				{
					throw TokenBasedError("Type [" + element.type.value.content + "] not found.", p_metaToken->name);
				}

				result.innerElements.push_back(newElement);
			}

			result.gpuSize = gpuOffset;
			result.cpuSize = cpuOffset;

			return (result);
		}

		std::string composeBlockString(const std::string& p_prefix, std::shared_ptr<BlockMetaToken> p_metaToken)
		{
			std::string result = "";

			result += namespacePrefix() + p_prefix + p_metaToken->name.content + " {\n";
			for (const auto& element : p_metaToken->elements)
			{
				result += "    " + element.type.value.content + " " + element.name.content;
				for (const auto& size : element.arraySizes)
				{
					result += "[" + std::to_string(size) + "]";
				}
				result += ";\n";
			}
			result += "}\n";

			return (result);
		}

		void compileStructure(std::shared_ptr<StructureMetaToken> p_metaToken)
		{
			Type structType = composeType(p_metaToken);
			std::string structCode = "struct " + composeBlockString("", p_metaToken);

			_result.value.vertexShaderCode += structCode;
			_result.value.fragmentShaderCode += structCode;

			addAttributeType(structType);
		}

		void insertElement(std::string& p_stringToFill, const Type::Element& p_elementToInsert, size_t p_nbSpace)
		{
			p_stringToFill += std::string(p_nbSpace, ' ') + p_elementToInsert.name + " " + std::to_string(p_elementToInsert.cpuOffset) + " " + std::to_string(p_elementToInsert.type->cpuSize) + " " + std::to_string(p_elementToInsert.gpuOffset) + " " + std::to_string(p_elementToInsert.type->gpuSize);
			if (p_elementToInsert.type->innerElements.size() == 0)
			{
				p_stringToFill += " {}";
			}
			else
			{
				p_stringToFill += " {\n";
				for (const auto& innerElement : p_elementToInsert.type->innerElements)
				{
					insertElement(p_stringToFill, innerElement, p_nbSpace + 4);
				}
				p_stringToFill += std::string(p_nbSpace, ' ') + "}";
			}
			if (p_elementToInsert.arraySize.size() != 0)
			{
				size_t bufferSize = 1;
				for (const auto& size : p_elementToInsert.arraySize)
					bufferSize *= size;
				size_t padding = 0;
				if (p_elementToInsert.type->gpuSize == 12)
					padding = 4;
				else if (p_elementToInsert.type->gpuSize >= 16)
					padding = (16 - (p_elementToInsert.type->gpuSize % 16)) % 16;
				p_stringToFill += " ";
				for (size_t i = 0; i < p_elementToInsert.arraySize.size(); i++)
				{
					if (i != 0)
						p_stringToFill += "x";
					p_stringToFill += std::to_string(p_elementToInsert.arraySize[i]);
				}
				p_stringToFill += " " + std::to_string(bufferSize) + " " + std::to_string(padding);
			}
			p_stringToFill += "\n";
		}

		void compileAttribute(std::shared_ptr<AttributeMetaToken> p_metaToken)
		{
			Type attributeType = composeType(p_metaToken);

			_result.value.attributes += namespacePrefix() + ATTRIBUTE_PREFIX + attributeType.name + " " + attributeType.name + " " + std::to_string(attributeType.cpuSize) + " " + std::to_string(attributeType.gpuSize) + " {\n";
			for (const auto& element : attributeType.innerElements)
			{
				insertElement(_result.value.attributes, element, 4);
			}
			_result.value.attributes += "}\n";

			std::string attributeCode = "layout(attribute) uniform " + composeBlockString(ATTRIBUTE_PREFIX, p_metaToken);

			_result.value.vertexShaderCode += attributeCode;
			_result.value.fragmentShaderCode += attributeCode;

			addAttributeType(attributeType);

			_variables[namespacePrefix("::") + attributeType.name] = {
				type(attributeType.name),
				p_metaToken->name.content,
				{}
			};
		}

		void compileConstant(std::shared_ptr<ConstantMetaToken> p_metaToken)
		{
			Type constantType = composeType(p_metaToken);

			_result.value.constants += namespacePrefix() + CONSTANT_PREFIX + constantType.name + " " + constantType.name + " " + std::to_string(constantType.cpuSize) + " " + std::to_string(constantType.gpuSize) + " {\n";
			for (const auto& element : constantType.innerElements)
			{
				insertElement(_result.value.constants, element, 4);
			}
			_result.value.constants += "}\n";

			std::string constantCode = "layout(constant) uniform " + composeBlockString(CONSTANT_PREFIX, p_metaToken);

			_result.value.vertexShaderCode += constantCode;
			_result.value.fragmentShaderCode += constantCode;

			addConstantType(constantType);

			_variables[namespacePrefix("::") + constantType.name] = {
				type(constantType.name),
				p_metaToken->name.content,
				{}
			};
		}

		void compileTexture(std::shared_ptr<TextureMetaToken> p_metaToken)
		{
			_result.value.fragmentShaderCode += "uniform sampler2D " + TEXTURE_PREFIX + namespacePrefix() + p_metaToken->name.content + ";\n";

			_result.value.textures += namespacePrefix("::") + p_metaToken->name.content + " " + TEXTURE_PREFIX + namespacePrefix() + p_metaToken->name.content + " " + std::to_string(_textures.size()) + "\n";

			_variables[namespacePrefix("::") + p_metaToken->name.content] = {
				type("Texture"),
				p_metaToken->name.content,
				{}
			};

			_textures.push_back(&(_variables[namespacePrefix("::") + p_metaToken->name.content]));
		}

		void compileFunction(std::shared_ptr<FunctionMetaToken> p_metaToken)
		{

		}

		void compilePipelineBody(std::shared_ptr<PipelineBodyMetaToken> p_metaToken)
		{

		}

		void compileNamespace(std::shared_ptr<NamespaceMetaToken> p_metaToken)
		{
			_namespaceNames.push_back(p_metaToken->name.content);

			for (const auto& innerMetaToken : p_metaToken->innerMetaTokens)
			{
				try
				{
					switch (innerMetaToken->type)
					{
					case MetaToken::Type::Constant:
					{
						compileConstant(static_pointer_cast<ConstantMetaToken>(innerMetaToken));
						break;
					}
					case MetaToken::Type::Attribute:
					{
						compileAttribute(static_pointer_cast<AttributeMetaToken>(innerMetaToken));
						break;
					}
					case MetaToken::Type::Structure:
					{
						compileStructure(static_pointer_cast<StructureMetaToken>(innerMetaToken));
						break;
					}
					case MetaToken::Type::Texture:
					{
						compileTexture(static_pointer_cast<TextureMetaToken>(innerMetaToken));
						break;
					}
					case MetaToken::Type::Function:
					{
						compileFunction(static_pointer_cast<FunctionMetaToken>(innerMetaToken));
						break;
					}
					case MetaToken::Type::Namespace:
					{
						compileNamespace(static_pointer_cast<NamespaceMetaToken>(innerMetaToken));
						break;
					}
					default:
					{
						throw TokenBasedError("Unknow meta token type", Token());
						break;
					}
					}
				}
				catch (TokenBasedError& e)
				{
					_result.errors.push_back(e);
				}
			}

			_namespaceNames.pop_back();
		}

		Product _compile(const std::vector<std::shared_ptr<MetaToken>>& p_metaTokens)
		{
			_result = Product();

			_result.value.outputLayouts += "0 Vector4\n";
			_result.value.fragmentShaderCode += "layout(location = 0) out vec4 pixelColor;\n";
			nbOutputLayout++;

			for (const auto& metaToken : p_metaTokens)
			{
				try
				{
				switch (metaToken->type)
				{
					case MetaToken::Type::PipelineFlow:
					{
						compilePipelineFlow(static_pointer_cast<PipelineFlowMetaToken>(metaToken));
						break;
					}
					case MetaToken::Type::PipelineBody:
					{
						compilePipelineBody(static_pointer_cast<PipelineBodyMetaToken>(metaToken));
						break;
					}
					case MetaToken::Type::Constant:
					{
						compileConstant(static_pointer_cast<ConstantMetaToken>(metaToken));
						break;
					}
					case MetaToken::Type::Attribute:
					{
						compileAttribute(static_pointer_cast<AttributeMetaToken>(metaToken));
						break;
					}
					case MetaToken::Type::Structure:
					{
						compileStructure(static_pointer_cast<StructureMetaToken>(metaToken));
						break;
					}
					case MetaToken::Type::Texture:
					{
						compileTexture(static_pointer_cast<TextureMetaToken>(metaToken));
						break;
					}
					case MetaToken::Type::Function:
					{
						compileFunction(static_pointer_cast<FunctionMetaToken>(metaToken));
						break;
					}
					case MetaToken::Type::Namespace:
					{
						compileNamespace(static_pointer_cast<NamespaceMetaToken>(metaToken));
						break;
					}
					default:
					{
						throw TokenBasedError("Unknow meta token type", Token());
						break;
					}
				}
				}
				catch (TokenBasedError& e)
				{
					_result.errors.push_back(e);
				}
			}

			return (_result);
		}

		void createScalarTypes()
		{
			addStandardType({
				.name = "float",
				.cpuSize = 4,
				.gpuSize = 4,
				.padding = 4,
				.innerElements = {}
				});

			addStandardType({
				.name = "int",
				.cpuSize = 4,
				.gpuSize = 4,
				.padding = 4,
				.innerElements = {}
				});

			addStandardType({
				.name = "uint",
				.cpuSize = 4,
				.gpuSize = 4,
				.padding = 4,
				.innerElements = {}
				});

			addStandardType({
				.name = "bool",
				.cpuSize = 1,
				.gpuSize = 1,
				.padding = 1,
				.innerElements = {}
				});
		}

		void createFloatVectorTypes()
		{
			const Type* floatTypePtr = type("float");
			if (!floatTypePtr)
			{
				throw std::runtime_error("Type 'float' not found");
			}

			addStandardType({
				.name = "Vector2",
				.cpuSize = 8,
				.gpuSize = 8,
				.padding = 8,
				.innerElements = {
					{
						.type = floatTypePtr,
						.name = "x",
						.cpuOffset = 0,
						.gpuOffset = 0,
						.arraySize = {}
					},
					{
						.type = floatTypePtr,
						.name = "y",
						.cpuOffset = 4,
						.gpuOffset = 4,
						.arraySize = {}
					}
				}
				});

			addStandardType({
				.name = "Vector3",
				.cpuSize = 12,
				.gpuSize = 12,
				.padding = 16,
				.innerElements = {
					{
						.type = floatTypePtr,
						.name = "x",
						.cpuOffset = 0,
						.gpuOffset = 0,
						.arraySize = {}
					},
					{
						.type = floatTypePtr,
						.name = "y",
						.cpuOffset = 4,
						.gpuOffset = 4,
						.arraySize = {}
					},
					{
						.type = floatTypePtr,
						.name = "z",
						.cpuOffset = 8,
						.gpuOffset = 8,
						.arraySize = {}
					}
				}
				});

			addStandardType({
				.name = "Vector4",
				.cpuSize = 16,
				.gpuSize = 16,
				.padding = 16,
				.innerElements = {
					{
						.type = floatTypePtr,
						.name = "x",
						.cpuOffset = 0,
						.gpuOffset = 0,
						.arraySize = {}
					},
					{
						.type = floatTypePtr,
						.name = "y",
						.cpuOffset = 4,
						.gpuOffset = 4,
						.arraySize = {}
					},
					{
						.type = floatTypePtr,
						.name = "z",
						.cpuOffset = 8,
						.gpuOffset = 8,
						.arraySize = {}
					},
					{
						.type = floatTypePtr,
						.name = "w",
						.cpuOffset = 12,
						.gpuOffset = 12,
						.arraySize = {}
					}
				}
				});
		}

		void createIntVectorTypes()
		{
			const Type* intTypePtr = type("int");
			if (!intTypePtr)
			{
				throw std::runtime_error("Type 'int' not found");
			}

			addStandardType({
				.name = "Vector2Int",
				.cpuSize = 8,
				.gpuSize = 8,
				.padding = 8,
				.innerElements = {
					{
						.type = intTypePtr,
						.name = "x",
						.cpuOffset = 0,
						.gpuOffset = 0,
						.arraySize = {}
					},
					{
						.type = intTypePtr,
						.name = "y",
						.cpuOffset = 4,
						.gpuOffset = 4,
						.arraySize = {}
					}
				}
				});

			addStandardType({
				.name = "Vector3Int",
				.cpuSize = 12,
				.gpuSize = 12,
				.padding = 16,
				.innerElements = {
					{
						.type = intTypePtr,
						.name = "x",
						.cpuOffset = 0,
						.gpuOffset = 0,
						.arraySize = {}
					},
					{
						.type = intTypePtr,
						.name = "y",
						.cpuOffset = 4,
						.gpuOffset = 4,
						.arraySize = {}
					},
					{
						.type = intTypePtr,
						.name = "z",
						.cpuOffset = 8,
						.gpuOffset = 8,
						.arraySize = {}
					}
				}
				});

			addStandardType({
				.name = "Vector4Int",
				.cpuSize = 16,
				.gpuSize = 16,
				.padding = 16,
				.innerElements = {
					{
						.type = intTypePtr,
						.name = "x",
						.cpuOffset = 0,
						.gpuOffset = 0,
						.arraySize = {}
					},
					{
						.type = intTypePtr,
						.name = "y",
						.cpuOffset = 4,
						.gpuOffset = 4,
						.arraySize = {}
					},
					{
						.type = intTypePtr,
						.name = "z",
						.cpuOffset = 8,
						.gpuOffset = 8,
						.arraySize = {}
					},
					{
						.type = intTypePtr,
						.name = "w",
						.cpuOffset = 12,
						.gpuOffset = 12,
						.arraySize = {}
					}
				}
				});
		}

		void createUIntVectorTypes()
		{
			const Type* uintTypePtr = type("uint");
			if (!uintTypePtr)
			{
				throw std::runtime_error("Type 'uint' not found");
			}

			addStandardType({
				.name = "Vector2UInt",
				.cpuSize = 8,
				.gpuSize = 8,
				.padding = 8,
				.innerElements = {
					{
						.type = uintTypePtr,
						.name = "x",
						.cpuOffset = 0,
						.gpuOffset = 0,
						.arraySize = {}
					},
					{
						.type = uintTypePtr,
						.name = "y",
						.cpuOffset = 4,
						.gpuOffset = 4,
						.arraySize = {}
					}
				}
				});

			addStandardType({
				.name = "Vector3UInt",
				.cpuSize = 12,
				.gpuSize = 12,
				.padding = 16,
				.innerElements = {
					{
						.type = uintTypePtr,
						.name = "x",
						.cpuOffset = 0,
						.gpuOffset = 0,
						.arraySize = {}
					},
					{
						.type = uintTypePtr,
						.name = "y",
						.cpuOffset = 4,
						.gpuOffset = 4,
						.arraySize = {}
					},
					{
						.type = uintTypePtr,
						.name = "z",
						.cpuOffset = 8,
						.gpuOffset = 8,
						.arraySize = {}
					}
				}
				});

			addStandardType({
				.name = "Vector4UInt",
				.cpuSize = 16,
				.gpuSize = 16,
				.padding = 16,
				.innerElements = {
					{
						.type = uintTypePtr,
						.name = "x",
						.cpuOffset = 0,
						.gpuOffset = 0,
						.arraySize = {}
					},
					{
						.type = uintTypePtr,
						.name = "y",
						.cpuOffset = 4,
						.gpuOffset = 4,
						.arraySize = {}
					},
					{
						.type = uintTypePtr,
						.name = "z",
						.cpuOffset = 8,
						.gpuOffset = 8,
						.arraySize = {}
					},
					{
						.type = uintTypePtr,
						.name = "w",
						.cpuOffset = 12,
						.gpuOffset = 12,
						.arraySize = {}
					}
				}
				});
		}

		void createMatrixTypes()
		{
			const Type* floatTypePtr = type("float");
			if (!floatTypePtr)
			{
				throw std::runtime_error("Type 'float' not found");
			}

			addStandardType({
				.name = "Matrix2x2",
				.cpuSize = 16,
				.gpuSize = 16,
				.padding = 16,
				.innerElements = {}
				});

			addStandardType({
				.name = "Matrix3x3",
				.cpuSize = 36,
				.gpuSize = 36,
				.padding = 16,
				.innerElements = {}
				});

			addStandardType({
				.name = "Matrix4x4",
				.cpuSize = 64,
				.gpuSize = 64,
				.padding = 16,
				.innerElements = {}
				});
		}

		void createLuminaTypes()
		{
			addLuminaType({
				.name = "Texture",
				.cpuSize = 0,
				.gpuSize = 0,
				.padding = 0,
				.innerElements = {}
				});
		}

		Compiler()
		{
			createScalarTypes();

			createFloatVectorTypes();
			createIntVectorTypes();
			createUIntVectorTypes();

			createMatrixTypes();

			createLuminaTypes();
		}

		void addStandardType(const Type& p_type)
		{
			if (_types.contains(p_type) == true)
			{
				throw std::runtime_error("Type [" + p_type.name + "] already defined");
			}
			_types.insert(p_type);
			_standardTypes.insert(p_type);
		}

		void addLuminaType(const Type& p_type)
		{
			if (_types.contains(p_type) == true)
			{
				throw std::runtime_error("Type [" + p_type.name + "] already defined");
			}
			_types.insert(p_type);
			_luminaTypes.insert(p_type);
		}

		void addConstantType(const Type& p_type)
		{
			if (_types.contains(p_type) == true)
			{
				throw std::runtime_error("Type [" + p_type.name + "] already defined");
			}
			_types.insert(p_type);
			_constantTypes.insert(p_type);
		}

		void addAttributeType(const Type& p_type)
		{
			if (_types.contains(p_type) == true)
			{
				throw std::runtime_error("Type [" + p_type.name + "] already defined");
			}
			_types.insert(p_type);
			_attributeTypes.insert(p_type);
		}

		void addStructureType(const Type& p_type)
		{
			if (_types.contains(p_type) == true)
			{
				throw std::runtime_error("Type [" + p_type.name + "] already defined");
			}
			_types.insert(p_type);
			_structureTypes.insert(p_type);
		}

		const Type* type(const std::string& p_typeName) const
		{
			auto it = _types.find(Type{ p_typeName, {}, {} });
			if (it != _types.end())
			{
				return &(*it);
			}
			return nullptr;
		}

	public:
		static Expected<Shader> compile(const std::vector<std::shared_ptr<MetaToken>>& p_metaTokens)
		{
			return (Compiler()._compile(p_metaTokens));
		}
	};
}

int main(int argc, char** argv)
{
	if (argc != 3)
	{
		std::cout << "Usage : " << argv[0] << " [path to your lumina shader code] [path to your compiled shader file]" << std::endl;
		return (0);
	}

	std::vector<Lumina::Token> tokens = Lumina::Tokenizer::tokenize(argv[1]);

	Lumina::MetaTokenizer::Product metaTokens = Lumina::MetaTokenizer::analyse(tokens);

	if (metaTokens.errors.size() != 0)
	{
		for (const auto& error : metaTokens.errors)
		{
			std::cout << error.what() << std::endl;
		}
		return (-1);
	}

	Lumina::Compiler::Product shader = Lumina::Compiler::compile(metaTokens.value);

	if (shader.errors.size() != 0)
	{
		for (const auto& error : shader.errors)
		{
			std::cout << error.what() << std::endl;
		}
		return (-1);
	}

	std::fstream compiledShader(argv[2], std::ios_base::out);

	const std::string INPUT_LAYOUTS_DELIMITER = "## INPUT LAYOUTS DEFINITION ##";
	const std::string OUTPUT_LAYOUTS_DELIMITER = "## OUTPUT LAYOUTS DEFINITION ##";
	const std::string CONSTANTS_DELIMITER = "## CONSTANTS DEFINITION ##";
	const std::string ATTRIBUTES_DELIMITER = "## ATTRIBUTES DEFINITION ##";
	const std::string TEXTURES_DELIMITER = "## TEXTURES DEFINITION ##";
	const std::string VERTEX_DELIMITER = "## VERTEX SHADER CODE ##";
	const std::string FRAGMENT_DELIMITER = "## FRAGMENT SHADER CODE ##";

	compiledShader << INPUT_LAYOUTS_DELIMITER << std::endl;
	compiledShader << shader.value.inputLayouts << std::endl;
	compiledShader << OUTPUT_LAYOUTS_DELIMITER << std::endl;
	compiledShader << shader.value.outputLayouts << std::endl;
	compiledShader << CONSTANTS_DELIMITER << std::endl;
	compiledShader << shader.value.constants << std::endl;
	compiledShader << ATTRIBUTES_DELIMITER << std::endl;
	compiledShader << shader.value.attributes << std::endl;
	compiledShader << TEXTURES_DELIMITER << std::endl;
	compiledShader << shader.value.textures << std::endl;
	compiledShader << VERTEX_DELIMITER << std::endl;
	compiledShader << shader.value.vertexShaderCode << std::endl;
	compiledShader << FRAGMENT_DELIMITER << std::endl;
	compiledShader << shader.value.fragmentShaderCode << std::endl;

	compiledShader.close();

	std::cout << Lumina::readFileAsString(argv[2]);


	return (0);
}
