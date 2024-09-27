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

		static inline const std::string ATTRIBUTE_PREFIX = "Attribute_";
		static inline const std::string CONSTANT_PREFIX = "Constant_";
		static inline const std::string TEXTURE_PREFIX = "Texture_";

		struct Type;

		struct Variable
		{
			const Type* type;
			std::string name;
			std::vector<size_t> arraySizes;

			bool isSame(const Variable& p_other) const
			{
				if (type != p_other.type)
					return (false);
				if (arraySizes.size() != p_other.arraySizes.size())
					return(false);

				for (size_t i = 0; i < arraySizes.size(); i++)
				{
					if (arraySizes[i] != p_other.arraySizes[i])
						return (false);
				}

				return (true);
			}

			bool operator<(const Variable& p_other) const
			{
				if (type != p_other.type)
					return type < p_other.type;
				if (name != p_other.name)
					return name < p_other.name;
				return arraySizes < p_other.arraySizes;
			}
		};
		struct Type
		{
			std::string name = "";
			size_t cpuSize;
			size_t gpuSize;
			size_t padding = 0;

			struct Element
			{
				Variable variable;
				size_t cpuOffset;
				size_t gpuOffset;
			};

			std::vector<Element> innerElements;

			bool contains(const std::string& p_name)
			{
				for (const auto& element : innerElements)
				{
					if (element.variable.name == p_name)
						return (true);
				}
				return (false);
			}
		
			bool operator<(const Type& other) const
			{
				return name < other.name;
			}
		};


		struct Function
		{
			struct Return
			{
				const Type* type;
				std::vector<size_t> arraySizes;

				bool operator == (const Return& p_other)
				{
					if (type != p_other.type)
						return (false);
					if (arraySizes.size() != p_other.arraySizes.size())
						return(false);

					for (size_t i = 0; i < arraySizes.size(); i++)
					{
						if (arraySizes[i] != p_other.arraySizes[i])
							return (false);
					}

					return (true);
				}

				bool operator != (const Return& p_other)
				{
					return (!(operator==(p_other)));
				}
			};

			Return returnType;
			std::string name;
			std::vector<Variable> parameters;
		};

		Product _result;

		size_t nbVertexLayout = 0;
		size_t nbFragmentLayout = 0;
		size_t nbOutputLayout = 0;
		size_t nbTexture = 0;

		std::vector<std::string> _namespaceNames;

		std::string namespacePrefix()
		{
			std::string result;

			for (const std::string& name : _namespaceNames)
			{
				result += name + "::";
			}

			return (result);
		}

		std::set<Type> _types;
		std::set<Type> _standardTypes;

		std::map<std::string, std::vector<Function>> _functions;

		std::set<Variable> _vertexVariables;
		std::set<Variable> _fragmentVariables;

		Variable composeVariable(const VariableDescriptor& p_descriptor)
		{
			Variable result;

			result.type = type(p_descriptor.type.value);
			result.name = p_descriptor.name.content;
			result.arraySizes = p_descriptor.arraySizes;

			if (result.type == nullptr)
			{
				throw TokenBasedError("Type : " + p_descriptor.type.value.content + " not found", p_descriptor.type.value);
			}

			return (result);
		}

		void compilePipelineFlow(std::shared_ptr<PipelineFlowMetaToken> p_metaToken)
		{
			Variable newVariable = composeVariable(p_metaToken->variableDescriptor);

			if (p_metaToken->inputFlow == "Input")
			{
				if (p_metaToken->outputFlow == "VertexPass")
				{
					_result.value.inputLayouts += std::to_string(nbVertexLayout) + " " + newVariable.type->name + "\n";

					_result.value.vertexShaderCode += "layout(location = " + std::to_string(nbVertexLayout) + ") in " + newVariable.type->name + " " + newVariable.name + ";\n\n";
					
					_vertexVariables.insert(newVariable);

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
					_result.value.vertexShaderCode += "layout(location = " + std::to_string(nbFragmentLayout) + ") out " + newVariable.type->name + " " + newVariable.name + ";\n\n";
					_result.value.fragmentShaderCode += "layout(location = " + std::to_string(nbFragmentLayout) + ") in " + newVariable.type->name + " " + newVariable.name + ";\n\n";

					_vertexVariables.insert(newVariable);
					_fragmentVariables.insert(newVariable);

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
					_result.value.outputLayouts += std::to_string(nbOutputLayout) + " " + newVariable.type->name + "\n\n";
					_result.value.fragmentShaderCode += "layout(location = " + std::to_string(nbOutputLayout) + ") out " + newVariable.type->name + " " + newVariable.name + ";\n\n";

					_fragmentVariables.insert(newVariable);

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
			Type result;

			result.name = namespacePrefix() + p_metaToken->name.content;

			if (p_metaToken->type == MetaToken::Type::Attribute ||
				p_metaToken->type == MetaToken::Type::Constant)
			{
				result.name += "Type";
			}

			if (_type(result.name) != nullptr)
			{
				throw TokenBasedError("Type [" + result.name + "] already defined.", p_metaToken->name);
			}

			size_t cpuOffset = 0;
			size_t gpuOffset = 0;
			for (const auto& element : p_metaToken->elements)
			{
				if (result.contains(element.name.content) == true)
				{
					throw TokenBasedError("Attribute [" + element.name.content + "] already defined in [" + result.name + "] structure.", element.name);
				}

				Type::Element newElement;

				newElement.variable.name = element.name.content;
				newElement.variable.type = type(element.type.value);
				if (newElement.variable.type == _type("Texture"))
				{
					throw TokenBasedError("Texture can't be placed inside block.", element.name);
				}
				newElement.variable.arraySizes = element.arraySizes;

				size_t totalSize = 1;
				for (const auto& size : newElement.variable.arraySizes)
					totalSize *= size;

				size_t padding = 0;
				if (newElement.variable.type->gpuSize == 12)
					padding = 4;
				else if (newElement.variable.type->gpuSize >= 16)
					padding = (16 - (newElement.variable.type->gpuSize % 16)) % 16;

				if ((gpuOffset % 16) != 0)
				{
					size_t bytesLeft = 16 - (gpuOffset % 16);
					if (bytesLeft < newElement.variable.type->gpuSize)
					{
						gpuOffset += bytesLeft;
					}
				}

				newElement.gpuOffset = gpuOffset;
				gpuOffset += (newElement.variable.type->gpuSize + padding) * totalSize;

				newElement.cpuOffset = cpuOffset;
				cpuOffset += newElement.variable.type->cpuSize * totalSize;


				if (newElement.variable.type == nullptr)
				{
					throw TokenBasedError("Type [" + element.type.value.content + "] not found.", p_metaToken->name);
				}

				result.innerElements.push_back(newElement);
			}

			result.gpuSize = gpuOffset;
			result.cpuSize = cpuOffset;

			return (result);
		}

		Variable composeVariable(std::shared_ptr<BlockMetaToken> p_metaToken)
		{
			Type tmpType = composeType(p_metaToken);
			std::string name = namespacePrefix() + p_metaToken->name.content;

			addType(tmpType);

			Variable result;

			result.type = _type(tmpType.name);
			result.name = name;

			return (result);
		}

		enum class BlockType
		{
			Constant,
			Attribute,
			Structure
		};

		std::string composeBlockCode(const BlockType& p_blockType, const Variable& p_variable)
		{
			std::string result = "";

			switch (p_blockType)
			{
			case BlockType::Constant:
			{
				result += "layout (constant) uniform ";
				break;
			}
			case BlockType::Attribute:
			{
				result += "layout (attribute) uniform ";
				break;
			}
			case BlockType::Structure:
			{
				result += "struct ";
				break;
			}
			}

			result += namespacePrefix() + p_variable.type->name;

			result += " {\n";
			for (const auto& element : p_variable.type->innerElements)
			{
				result += "    " + element.variable.type->name + " " + element.variable.name;
				for (const auto& size : element.variable.arraySizes)
				{
					result += "[" + std::to_string(size) + "]";
				}
				result += ";\n";
			}
			result += "}";

			if (p_blockType != BlockType::Structure)
			{
				result += " " + namespacePrefix() + p_variable.name;
			}

			result += ";\n\n";

			return (result);
		}

		void compileStructure(std::shared_ptr<StructureMetaToken> p_metaToken)
		{
			Type structType = composeType(p_metaToken);

			std::string structCode = composeBlockCode(BlockType::Structure, { &structType, "", {} });

			_result.value.vertexShaderCode += structCode;
			_result.value.fragmentShaderCode += structCode;

			addType(structType);
		}

		void insertElement(std::string& p_stringToFill, const Type::Element& p_elementToInsert, size_t p_nbSpace)
		{
			p_stringToFill += std::string(p_nbSpace, ' ') + p_elementToInsert.variable.name + " " + std::to_string(p_elementToInsert.cpuOffset) + " " + std::to_string(p_elementToInsert.variable.type->cpuSize) + " " + std::to_string(p_elementToInsert.gpuOffset) + " " + std::to_string(p_elementToInsert.variable.type->gpuSize);
			if (p_elementToInsert.variable.type->innerElements.size() == 0)
			{
				p_stringToFill += " {}";
			}
			else
			{
				p_stringToFill += " {\n";
				for (const auto& innerElement : p_elementToInsert.variable.type->innerElements)
				{
					insertElement(p_stringToFill, innerElement, p_nbSpace + 4);
				}
				p_stringToFill += std::string(p_nbSpace, ' ') + "}";
			}
			if (p_elementToInsert.variable.arraySizes.size() != 0)
			{
				size_t bufferSize = 1;
				for (const auto& size : p_elementToInsert.variable.arraySizes)
					bufferSize *= size;
				size_t padding = 0;
				if (p_elementToInsert.variable.type->gpuSize == 12)
					padding = 4;
				else if (p_elementToInsert.variable.type->gpuSize >= 16)
					padding = (16 - (p_elementToInsert.variable.type->gpuSize % 16)) % 16;
				p_stringToFill += " ";
				for (size_t i = 0; i < p_elementToInsert.variable.arraySizes.size(); i++)
				{
					if (i != 0)
						p_stringToFill += "x";
					p_stringToFill += std::to_string(p_elementToInsert.variable.arraySizes[i]);
				}
				p_stringToFill += " " + std::to_string(bufferSize) + " " + std::to_string(padding);
			}
			p_stringToFill += "\n";
		}

		std::string composeDataDescriptor(const Variable& p_variable)
		{
			std::string result = p_variable.type->name + " " + p_variable.name + " " + std::to_string(p_variable.type->cpuSize) + " " + std::to_string(p_variable.type->gpuSize) + " {\n";
			for (const auto& element : p_variable.type->innerElements)
			{
				insertElement(_result.value.attributes, element, 4);
			}
			_result.value.attributes += "}\n";

			return (result);
		}

		void compileAttribute(std::shared_ptr<AttributeMetaToken> p_metaToken)
		{
			Variable attributeVariable = composeVariable(p_metaToken);

			_result.value.attributes += composeDataDescriptor(attributeVariable);

			std::string attributeCode = composeBlockCode(BlockType::Attribute, attributeVariable);

			_result.value.vertexShaderCode += attributeCode;
			_result.value.fragmentShaderCode += attributeCode;

			_vertexVariables.insert(attributeVariable);
			_fragmentVariables.insert(attributeVariable);
		}

		void compileConstant(std::shared_ptr<ConstantMetaToken> p_metaToken)
		{
			Variable constantVariable = composeVariable(p_metaToken);

			_result.value.constants += composeDataDescriptor(constantVariable);

			std::string constantCode = composeBlockCode(BlockType::Attribute, constantVariable);

			_result.value.vertexShaderCode += constantCode;
			_result.value.fragmentShaderCode += constantCode;

			_vertexVariables.insert(constantVariable);
			_fragmentVariables.insert(constantVariable);
		}

		void compileTexture(std::shared_ptr<TextureMetaToken> p_metaToken)
		{
			Variable newTextureVariable = { _type("Texture"), namespacePrefix() + p_metaToken->name.content, {} };

			_result.value.fragmentShaderCode += "uniform sampler2D " + newTextureVariable.name + ";\n";

			_result.value.textures += newTextureVariable.name + " " + newTextureVariable.name + " " + std::to_string(nbTexture) + "\n";

			_fragmentVariables.insert(newTextureVariable);

			nbTexture++;
		}

		std::string parseNumberElement(const std::shared_ptr<Expression::NumberElement>& element)
		{
			std::string result = "";

			result += element->value.content;

			return result;
		}

		std::string parseBooleanElement(const std::shared_ptr<Expression::BooleanElement>& element)
		{
			std::string result = "";

			result += element->value.content;

			return result;
		}

		std::string parseVariableDesignationElement(const std::shared_ptr<Expression::VariableDesignationElement>& element)
		{
			std::string result = "";

			if (element->signOperator.type != Lumina::Token::Type::Unknow)
			{
				result += element->signOperator.content;
			}

			for (const auto& ns : element->namespaceChain)
			{
				result += ns.content + "::";
			}

			result += element->name.content;

			for (const auto& accessor : element->accessors)
			{
				if (accessor->type == Instruction::Type::SymbolBody)
				{
					auto castedAccessor = std::static_pointer_cast<Expression::VariableDesignationElement::AccessorElement>(accessor);
					result += "." + castedAccessor->name.content;
				}
			}
			return result;
		}

		std::string parseOperatorElement(const std::shared_ptr<Expression::OperatorElement>& element)
		{
			std::string result = "";
			result += element->operatorToken.content;
			return result;
		}

		std::string parseComparatorOperatorElement(const std::shared_ptr<Expression::ComparatorOperatorElement>& element)
		{
			std::string result = "";
			result += element->operatorToken.content;
			return result;
		}

		std::string parseConditionOperatorElement(const std::shared_ptr<Expression::ConditionOperatorElement>& element)
		{
			std::string result = "";
			result += element->operatorToken.content;
			return result;
		}

		std::string parseIncrementorElement(const std::shared_ptr<Expression::IncrementorElement>& element)
		{
			std::string result = "";
			result += element->operatorToken.content;
			return result;
		}

		std::string parseSymbolCallElement(const std::shared_ptr<Expression::SymbolCallElement>& element)
		{
			std::string result = "";

			for (const auto& ns : element->namespaceChain)
			{
				result += ns.content + "::";
			}

			result += element->functionName.content + "(";

			for (size_t i = 0; i < element->parameters.size(); ++i)
			{
				result += parseExpression(element->parameters[i]);
				if (i != element->parameters.size() - 1)
				{
					result += ", ";
				}
			}

			result += ")";
			return result;
		}

		std::string parseExpression(const std::shared_ptr<Expression> p_expression)
		{
			std::string result = "";

			for (const auto& element : p_expression->elements)
			{
				try
				{
					switch (element->elementType)
					{
					case Expression::Element::Type::Number:
						result += parseNumberElement(std::static_pointer_cast<Expression::NumberElement>(element));
						break;
					case Expression::Element::Type::Boolean:
						result += parseBooleanElement(std::static_pointer_cast<Expression::BooleanElement>(element));
						break;
					case Expression::Element::Type::VariableDesignation:
						result += parseVariableDesignationElement(std::static_pointer_cast<Expression::VariableDesignationElement>(element));
						break;
					case Expression::Element::Type::Operator:
						result += parseOperatorElement(std::static_pointer_cast<Expression::OperatorElement>(element));
						break;
					case Expression::Element::Type::ComparaisonOperator:
						result += parseComparatorOperatorElement(std::static_pointer_cast<Expression::ComparatorOperatorElement>(element));
						break;
					case Expression::Element::Type::ConditionOperator:
						result += parseConditionOperatorElement(std::static_pointer_cast<Expression::ConditionOperatorElement>(element));
						break;
					case Expression::Element::Type::Incrementor:
						result += parseIncrementorElement(std::static_pointer_cast<Expression::IncrementorElement>(element));
						break;
					case Expression::Element::Type::SymbolCall:
						result += parseSymbolCallElement(std::static_pointer_cast<Expression::SymbolCallElement>(element));
						break;
					default:
						throw TokenBasedError("Unknown element type", Token());
					}
				}
				catch (TokenBasedError& e)
				{
					_result.errors.push_back(e);
				}
			}

			return result;
		}


		std::string parseVariableDeclaration(const std::shared_ptr<Instruction>& instruction)
		{
			std::string result = "";

			return (result);
		}

		std::string parseVariableAssignation(const std::shared_ptr<Instruction>& instruction)
		{
			std::string result = "";

			return (result);
		}

		std::string parseSymbolCall(const std::shared_ptr<Instruction>& instruction)
		{
			std::string result = "";

			return (result);
		}

		std::string parseIfStatement(const std::shared_ptr<Instruction>& instruction)
		{
			std::string result = "";

			return (result);
		}

		std::string parseWhileStatement(const std::shared_ptr<Instruction>& instruction)
		{
			std::string result = "";

			return (result);
		}

		std::string parseForStatement(const std::shared_ptr<Instruction>& instruction)
		{
			std::string result = "";

			return (result);
		}

		std::string parseReturnStatement(const std::shared_ptr<Instruction>& instruction)
		{
			std::string result = "";

			return (result);
		}

		std::string parseDiscardStatement(const std::shared_ptr<Instruction>& instruction)
		{
			std::string result = "";

			return (result);
		}

		std::string compileSymbolBody(SymbolBody p_metaToken)
		{
			std::string result;

			for (const auto& instruction : p_metaToken.instructions)
			{
				try
				{
					switch (instruction->type)
					{
					case Instruction::Type::VariableDeclaration:
					{
						result += parseVariableDeclaration(instruction);
						break;
					}

					case Instruction::Type::VariableAssignation:
					{
						result += parseVariableAssignation(instruction);
						break;
					}
					case Instruction::Type::SymbolCall:
					{
						result += parseSymbolCall(instruction);
						break;
					}
					case Instruction::Type::IfStatement:
					{
						result += parseIfStatement(instruction);
						break;
					}
					case Instruction::Type::WhileStatement:
					{
						result += parseWhileStatement(instruction);
						break;
					}
					case Instruction::Type::ForStatement:
					{
						result += parseForStatement(instruction);
						break;
					}
					case Instruction::Type::ReturnStatement:
					{
						result += parseReturnStatement(instruction);
						break;
					}
					case Instruction::Type::DiscardStatement:
					{
						result += parseDiscardStatement(instruction);
						break;
					}
					default:
						throw TokenBasedError("Unknown instruction type", Token());
					}
				}
				catch (TokenBasedError& e)
				{
					_result.errors.push_back(e);
				}
			}

			return (result);
		}


		void compileFunction(std::shared_ptr<FunctionMetaToken> p_metaToken)
		{
			Function newFunction;

			newFunction.returnType = { type(p_metaToken->returnType.type.value), p_metaToken->returnType.arraySizes };
			newFunction.name = namespacePrefix() + p_metaToken->name.content;
	
			for (const auto& parameter : p_metaToken->parameters)
			{
				Variable newParameter;

				newParameter.type = type(parameter.type.value);
				newParameter.name = parameter.name.content;
				newParameter.arraySizes = parameter.arraySizes;

				newFunction.parameters.push_back(newParameter);
			}

			if (_functions.contains(newFunction.name) == true)
			{
				const Function& tmpFunction = _functions[newFunction.name].front();

				if (tmpFunction.returnType != newFunction.returnType)
				{
					throw TokenBasedError("Function [" + p_metaToken->name.content + "] already defined with a different return type.", p_metaToken->name);
				}

				for (const auto& function : _functions[newFunction.name])
				{
					if (function.parameters.size() == newFunction.parameters.size())
					{
						bool different = false;

						for (size_t i = 0; different == false && i < function.parameters.size(); i++)
						{
							if (function.parameters[i].isSame(newFunction.parameters[i]) == false)
							{
								different = true;
							}
						}

						if (different == false)
						{
							throw TokenBasedError("Function [" + p_metaToken->name.content + "] already defined with a the same parameters types.", p_metaToken->name);
						}
					}
				}
			}

			_functions[newFunction.name].push_back(newFunction);

			std::string functionCode = "";

			functionCode += newFunction.returnType.type->name + " " + newFunction.name + "(";
			for (size_t i = 0; i < newFunction.parameters.size(); i++)
			{
				if (i != 0)
					functionCode += ", ";
				functionCode += newFunction.parameters[i].type->name + " " + newFunction.parameters[i].name;
				for (const auto& size : newFunction.parameters[i].arraySizes)
				{
					functionCode += "[" + std::to_string(size) + "]";
				}
			}
			functionCode += "){\n";
			functionCode += compileSymbolBody(p_metaToken->body);
			functionCode += "};\n";

			_result.value.vertexShaderCode += functionCode;
			_result.value.fragmentShaderCode += functionCode;

		}

		void compilePipelineBody(std::shared_ptr<PipelineBodyMetaToken> p_metaToken)
		{
			std::string functionCode = "";

			functionCode += "void main(){\n";
			functionCode += compileSymbolBody(p_metaToken->body);
			functionCode += "};\n";

			if (p_metaToken->target == "VertexPass")
			{
				_result.value.vertexShaderCode += functionCode;
			}
			else if (p_metaToken->target == "FragmentPass")
			{
				_result.value.fragmentShaderCode += functionCode;
			}
			else
			{
				throw TokenBasedError("Invalid pipeline pass definition.", p_metaToken->target);
			}
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

			_result.value.outputLayouts += "0 Vector4\n\n";
			_result.value.fragmentShaderCode += "layout(location = 0) out vec4 pixelColor;\n\n";
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
			const Type* floatTypePtr = _type("float");
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
						.variable = {
							.type = floatTypePtr,
							.name = "x",
							.arraySizes = {}
						},
						.cpuOffset = 0,
						.gpuOffset = 0
					}//,
					//{
					//	.variable = {
					//		.type = floatTypePtr,
					//		.name = "y",
					//		.arraySizes = {}
					//	},
					//	.cpuOffset = 4,
					//	.gpuOffset = 4
					//}
				}
			});

			addStandardType({
				.name = "Vector3",
				.cpuSize = 12,
				.gpuSize = 12,
				.padding = 16,
				.innerElements = {
					{
						.variable = {
							.type = floatTypePtr,
							.name = "x",
							.arraySizes = {}
						},
						.cpuOffset = 0,
						.gpuOffset = 0
					},
					{
						.variable = {
							.type = floatTypePtr,
							.name = "y",
							.arraySizes = {}
						},
						.cpuOffset = 4,
						.gpuOffset = 4
					},
					{
						.variable = {
							.type = floatTypePtr,
							.name = "z",
							.arraySizes = {}
						},
						.cpuOffset = 8,
						.gpuOffset = 8
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
						.variable = {
							.type = floatTypePtr,
							.name = "x",
							.arraySizes = {}
						},
						.cpuOffset = 0,
						.gpuOffset = 0
					},
					{
						.variable = {
							.type = floatTypePtr,
							.name = "y",
							.arraySizes = {}
						},
						.cpuOffset = 4,
						.gpuOffset = 4
					},
					{
						.variable = {
							.type = floatTypePtr,
							.name = "z",
							.arraySizes = {}
						},
						.cpuOffset = 8,
						.gpuOffset = 8
					},
					{
						.variable = {
							.type = floatTypePtr,
							.name = "w",
							.arraySizes = {}
						},
						.cpuOffset = 12,
						.gpuOffset = 12
					}
				}
				});
		}

		void createIntVectorTypes()
		{
			const Type* intTypePtr = _type("int");
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
						.variable = {
							.type = intTypePtr,
							.name = "x",
							.arraySizes = {}
						},
						.cpuOffset = 0,
						.gpuOffset = 0
					},
					{
						.variable = {
							.type = intTypePtr,
							.name = "y",
							.arraySizes = {}
						},
						.cpuOffset = 4,
						.gpuOffset = 4
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
						.variable = {
							.type = intTypePtr,
							.name = "x",
							.arraySizes = {}
						},
						.cpuOffset = 0,
						.gpuOffset = 0
					},
					{
						.variable = {
							.type = intTypePtr,
							.name = "y",
							.arraySizes = {}
						},
						.cpuOffset = 4,
						.gpuOffset = 4
					},
					{
						.variable = {
							.type = intTypePtr,
							.name = "z",
							.arraySizes = {}
						},
						.cpuOffset = 8,
						.gpuOffset = 8
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
						.variable = {
							.type = intTypePtr,
							.name = "x",
							.arraySizes = {}
						},
						.cpuOffset = 0,
						.gpuOffset = 0
					},
					{
						.variable = {
							.type = intTypePtr,
							.name = "y",
							.arraySizes = {}
						},
						.cpuOffset = 4,
						.gpuOffset = 4
					},
					{
						.variable = {
							.type = intTypePtr,
							.name = "z",
							.arraySizes = {}
						},
						.cpuOffset = 8,
						.gpuOffset = 8
					},
					{
						.variable = {
							.type = intTypePtr,
							.name = "w",
							.arraySizes = {}
						},
						.cpuOffset = 12,
						.gpuOffset = 12
					}
				}
				});
		}

		void createUIntVectorTypes()
		{
			const Type* uintTypePtr = _type("uint");
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
						.variable = {
							.type = uintTypePtr,
							.name = "x",
							.arraySizes = {}
						},
						.cpuOffset = 0,
						.gpuOffset = 0
					},
					{
						.variable = {
							.type = uintTypePtr,
							.name = "y",
							.arraySizes = {}
						},
						.cpuOffset = 4,
						.gpuOffset = 4
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
						.variable = {
							.type = uintTypePtr,
							.name = "x",
							.arraySizes = {}
						},
						.cpuOffset = 0,
						.gpuOffset = 0
					},
					{
						.variable = {
							.type = uintTypePtr,
							.name = "y",
							.arraySizes = {}
						},
						.cpuOffset = 4,
						.gpuOffset = 4
					},
					{
						.variable = {
							.type = uintTypePtr,
							.name = "z",
							.arraySizes = {}
						},
						.cpuOffset = 8,
						.gpuOffset = 8
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
						.variable = {
							.type = uintTypePtr,
							.name = "x",
							.arraySizes = {}
						},
						.cpuOffset = 0,
						.gpuOffset = 0
					},
					{
						.variable = {
							.type = uintTypePtr,
							.name = "y",
							.arraySizes = {}
						},
						.cpuOffset = 4,
						.gpuOffset = 4
					},
					{
						.variable = {
							.type = uintTypePtr,
							.name = "z",
							.arraySizes = {}
						},
						.cpuOffset = 8,
						.gpuOffset = 8
					},
					{
						.variable = {
							.type = uintTypePtr,
							.name = "w",
							.arraySizes = {}
						},
						.cpuOffset = 12,
						.gpuOffset = 12
					}
				}
				});
		}

		void createMatrixTypes()
		{
			const Type* floatTypePtr = _type("float");
			if (!floatTypePtr)
			{
				throw std::runtime_error("Type 'float' not found");
			}

			addType({
				.name = "Matrix2x2",
				.cpuSize = 16,
				.gpuSize = 16,
				.padding = 16,
				.innerElements = {}
				});

			addType({
				.name = "Matrix3x3",
				.cpuSize = 36,
				.gpuSize = 36,
				.padding = 16,
				.innerElements = {}
				});

			addType({
				.name = "Matrix4x4",
				.cpuSize = 64,
				.gpuSize = 64,
				.padding = 16,
				.innerElements = {}
				});
		}

		void createLuminaTypes()
		{
			addType({
				.name = "Texture",
				.cpuSize = 0,
				.gpuSize = 0,
				.padding = 0,
				.innerElements = {}
				});

			addType({
				.name = "void",
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

		void addType(const Type& p_type)
		{
			if (_types.contains(p_type) == true)
			{
				throw std::runtime_error("Type [" + p_type.name + "] already defined");
			}
			_types.insert(p_type);
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

		const Type* _type(const std::string& p_typeName) const
		{
			auto it = _types.find(Type{ p_typeName, {}, {} });
			if (it != _types.end())
			{
				return &(*it);
			}
			return nullptr;
		}
		const Type* type(const Lumina::Token& p_typeToken) const
		{
			const Type* result = _type(p_typeToken.content);

			if (result == nullptr)
			{
				throw TokenBasedError("Type [" + p_typeToken.content + "] not found", p_typeToken);
			}

			return (result);
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
