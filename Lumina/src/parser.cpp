#include "parser.hpp"

namespace Lumina
{
	void Parser::composeStandardTypes()
	{
		_insertType({
					.name = "void",
					.attributes = {
				}
			});

		_insertType({
					.name = "bool",
					.attributes = {
				}
			});

		_insertType({
					.name = "Texture",
					.attributes = {
				}
			});
	}

	void Parser::composeScalarTypes()
	{

		_insertType({
					.name = "int",
					.attributes = {
				}
			});


		_insertType({
					.name = "uint",
					.attributes = {
				}
			});

		_insertType({
					.name = "float",
					.attributes = {
				}
			});
	}

	void Parser::composeVector2Types()
	{
		_insertType({
				.name = "Vector2Int",
				.attributes = {
					{
						.type = _findType("int"),
						.name = "x",
						.arraySize = {}
					},
					{
						.type = _findType("int"),
						.name = "y",
						.arraySize = {}
					}
				}
			});

		_insertType({
				.name = "Vector2UInt",
				.attributes = {
					{
						.type = _findType("uint"),
						.name = "x",
						.arraySize = {}
					},
					{
						.type = _findType("uint"),
						.name = "y",
						.arraySize = {}
					}
				}
			});

		_insertType({
				.name = "Vector2",
				.attributes = {
					{
						.type = _findType("float"),
						.name = "x",
						.arraySize = {}
					},
					{
						.type = _findType("float"),
						.name = "y",
						.arraySize = {}
					}
				}
			});
	}

	void Parser::composeVector3Types()
	{
		_insertType({
				.name = "Vector3Int",
				.attributes = {
					{
						.type = _findType("int"),
						.name = "x",
						.arraySize = {}
					},
					{
						.type = _findType("int"),
						.name = "y",
						.arraySize = {}
					},
					{
						.type = _findType("int"),
						.name = "z",
						.arraySize = {}
					}
				}
			});

		_insertType({
				.name = "Vector3UInt",
				.attributes = {
					{
						.type = _findType("uint"),
						.name = "x",
						.arraySize = {}
					},
					{
						.type = _findType("uint"),
						.name = "y",
						.arraySize = {}
					},
					{
						.type = _findType("uint"),
						.name = "z",
						.arraySize = {}
					}
				}
			});

		_insertType({
				.name = "Vector3",
				.attributes = {
					{
						.type = _findType("float"),
						.name = "x",
						.arraySize = {}
					},
					{
						.type = _findType("float"),
						.name = "y",
						.arraySize = {}
					},
					{
						.type = _findType("float"),
						.name = "z",
						.arraySize = {}
					}
				}
			});
	}

	void Parser::composeVector4Types()
	{
		_insertType({
				.name = "Vector4Int",
				.attributes = {
					{
						.type = _findType("int"),
						.name = "x",
						.arraySize = {}
					},
					{
						.type = _findType("int"),
						.name = "y",
						.arraySize = {}
					},
					{
						.type = _findType("int"),
						.name = "z",
						.arraySize = {}
					},
					{
						.type = _findType("int"),
						.name = "w",
						.arraySize = {}
					}
				}
			});

		_insertType({
				.name = "Vector4UInt",
				.attributes = {
					{
						.type = _findType("uint"),
						.name = "x",
						.arraySize = {}
					},
					{
						.type = _findType("uint"),
						.name = "y",
						.arraySize = {}
					},
					{
						.type = _findType("uint"),
						.name = "z",
						.arraySize = {}
					},
					{
						.type = _findType("uint"),
						.name = "w",
						.arraySize = {}
					}
				}
			});

		_insertType({
				.name = "Vector4",
				.attributes = {
					{
						.type = _findType("float"),
						.name = "x",
						.arraySize = {}
					},
					{
						.type = _findType("float"),
						.name = "y",
						.arraySize = {}
					},
					{
						.type = _findType("float"),
						.name = "z",
						.arraySize = {}
					},
					{
						.type = _findType("float"),
						.name = "w",
						.arraySize = {}
					}
				}
			});
	}

	Parser::Parser()
	{
		composeStandardTypes();
		composeScalarTypes();
		composeVector2Types();
		composeVector3Types();
		composeVector4Types();
	}

	Parser::Product Parser::_parse(const Lexer::Output& p_input)
	{
		_product = Product();

		_parseNamespace(p_input.anonymNamespace);

		printParsedData();

		return (_product);
	}

	void Parser::printParsedData() const
	{
		std::cout << "Available Types:\n";
		for (const auto& type : _availibleTypes)
		{
			std::cout << "	Type: " << type.name << "\n";

			if (!type.attributes.empty())
			{
				std::cout << "		Attributes:\n";
				for (const auto& attr : type.attributes)
				{
					if (attr.type == nullptr)
					{
						std::cout << "			[No type] " << attr.name << "\n";
					}
					else
					{
						std::cout << "			" << attr.type->name << " " << attr.name;
						if (!attr.arraySize.empty())
						{
							std::cout << "[";
							for (size_t i = 0; i < attr.arraySize.size(); ++i)
							{
								if (i > 0) std::cout << ", ";
								std::cout << attr.arraySize[i];
							}
							std::cout << "]";
						}
						std::cout << "\n";
					}
					
				}
			}

			if (!type.methods.empty())
			{
				std::cout << "		Methods:\n";
				for (const auto& methodPair : type.methods)
				{
					for (const auto& method : methodPair.second)
					{
						std::cout << "			" << method.name << "\n";
					}
				}
			}

			if (!type.operators.empty())
			{
				std::cout << "		Operators:\n";
				for (const auto& operatorPair : type.operators)
				{
					for (const auto& op : operatorPair.second)
					{
						std::cout << "			" << op.name << "\n";
					}
				}
			}
		}

		std::cout << "\n	Variables:\n";
		for (const auto& var : _variables)
		{
			if (var.type == nullptr)
			{
				std::cout << "			[No type] " << var.name << "\n";
			}
			else
			{
				std::cout << "			" << var.type->name << " " << var.name;
				if (!var.arraySize.empty())
				{
					std::cout << "[";
					for (size_t i = 0; i < var.arraySize.size(); ++i)
					{
						if (i > 0) std::cout << ", ";
						std::cout << var.arraySize[i];
					}
					std::cout << "]";
				}
				std::cout << "\n";
			}
		}

		std::cout << "\n	Functions:\n";
		for (const auto& funcPair : _availibleFunctions)
		{
			for (const auto& func : funcPair.second)
			{
				std::cout << "		Return Type: " << func.returnType.type->name << "\n";
				std::cout << "		Function: " << func.name << "\n";

				if (!func.parameters.empty())
				{
					std::cout << "		Parameters:\n";
					for (const auto& param : func.parameters)
					{
						if (param.type == nullptr)
						{
							std::cout << "			[No type] " << param.name << "\n";
						}
						else
						{
							std::cout << "			" << param.type->name << " " << param.name;
							if (!param.arraySize.empty())
							{
								std::cout << "[";
								for (size_t i = 0; i < param.arraySize.size(); ++i)
								{
									if (i > 0) std::cout << ", ";
									std::cout << param.arraySize[i];
								}
								std::cout << "]";
							}
							std::cout << "\n";
						}
					}
				}
			}
		}

		std::cout << "\nAttribute Types:\n";
		for (const auto& attrType : _attributesTypes)
		{
			if (attrType == nullptr)
			{
				std::cout << "	Inserted a nullptr type in attribute" << "\n";
			}
			else
			{
				std::cout << "	" << attrType->name << "\n";
			}
		}

		std::cout << "\nConstants Types:\n";
		for (const auto& constType : _constantsTypes)
		{
			if (constType == nullptr)
			{
				std::cout << "Inserted a nullptr type in Constant" << "\n";
			}
			else
			{
				std::cout << "  " << constType->name << "\n";
			}
		}
	}


	std::string Parser::_composeTypeName(const TypeInfo& p_typeInfo)
	{
		std::string result = "";

		for (const auto& nspace : p_typeInfo.nspace)
		{
			result += nspace.content + "_";
		}

		result += p_typeInfo.value.content;

		return (result);
	}

	std::vector<size_t> Parser::_composeSizeArray(const ArraySizeInfo& p_arraySizeInfo)
	{
		std::vector<size_t> result;

		for (const auto& size : p_arraySizeInfo.dims)
		{
			result.push_back(std::stoull(size.content));
		}

		return (result);
	}

	std::string Parser::_composeIdentifierName(const std::string& p_identifierName)
	{
		std::string result = "";
		
		for (const auto& nspace : _nspaces)
		{
			result += nspace + "_";
		}

		result += p_identifierName;

		return (result);
	}

	const Parser::Type* Parser::_insertType(const Type& p_inputType)
	{
		_availibleTypes.insert(p_inputType);
		_reservedIdentifiers.insert(p_inputType.name);
		return (_findType(p_inputType.name));
	}

	const Parser::Type* Parser::_findType(const std::string& p_objectName)
	{
		Parser::Type expectedType = { p_objectName, {} };

		if (_availibleTypes.contains(expectedType) == true)
		{
			return &(*(_availibleTypes.find(expectedType)));
		}

		for (size_t i = 0; i < _nspaces.size(); i++)
		{
			std::string prefix = "";
			for (size_t j = i; j < _nspaces.size(); j++)
			{
				prefix += _nspaces[j] + "_";
			}

			expectedType = {prefix + p_objectName, {}};

			if (_availibleTypes.contains(expectedType) == true)
			{
				return &(*(_availibleTypes.find(expectedType)));
			}
		}

		return (nullptr);
	}

	void Parser::_insertVariable(const Parser::Variable& p_variable)
	{
		_variables.insert(p_variable);
		_reservedIdentifiers.insert(p_variable.name);
	}

	Parser::Variable Parser::_composeVariable(const VariableInfo& p_variableInfo)
	{
		Parser::Variable result;

		result.type = _findType(_composeTypeName(p_variableInfo.type));
		result.name = _composeIdentifierName(p_variableInfo.name.value.content);
		result.arraySize = _composeSizeArray(p_variableInfo.arraySizes);

		return (result);
	}

	Parser::Type Parser::_composeType(const BlockInfo& p_block, const std::string& p_suffix)
	{
		std::string typeName = _composeIdentifierName(p_block.name.value.content + p_suffix);

		Parser::Type result = { typeName, {} };

		for (const auto& attributeInfo : p_block.attributes)
		{
			Parser::Variable newAttribute = _composeVariable(attributeInfo);

			result.attributes.insert(newAttribute);
		}

		return (result);
	}

	Parser::Function Parser::_composeMethodFunction(const Parser::Type* p_originator, const FunctionInfo& p_functionInfo)
	{
		Parser::Function result;

		result.name = _composeIdentifierName(p_originator->name + "_" + p_functionInfo.name.value.content);

		return (result);
	}

	Parser::Function Parser::_composeOperatorFunction(const Parser::Type* p_originator, const OperatorInfo& p_operatorInfo)
	{
		Parser::Function result;

		std::map<std::string, std::string> operatorNameMap = {
			{"+", "Plus"},
			{"+=", "PlusEqual"},
			{"-", "Minus"},
			{"-=", "MinusEqual"},
			{"*", "Mult"},
			{"*=", "MultEqual"},
			{"/", "Div"},
			{"/=", "DivEqual"},
			{"%", "Modulo"},
			{"%=", "ModuloEqual"},
			{"&&", "And"},
			{"||", "Or"},
			{"==", "Equal"},
			{"!=", "Diff"},
			{"<", "Lower"},
			{">", "Greater"},
			{"<=", "LEqual"},
			{">=", "GEqual"},
		};

		result.name = _composeIdentifierName(p_originator->name + "_Operator" + operatorNameMap[p_operatorInfo.opeType.content]);

		return (result);
	}

	void Parser::_computeMethodAndOperator(const Parser::Type* p_originator, const BlockInfo& p_block)
	{
		for (const auto& methodArray : p_block.methodInfos)
		{
			for (const auto& method : methodArray.second)
			{
				Parser::Function newMethods = _composeMethodFunction(p_originator, method);

				_availibleFunctions[newMethods.name].push_back(newMethods);
			}
		}

		for (const auto& operatorArray : p_block.operatorInfos)
		{
			for (const auto& ope : operatorArray.second)
			{
				Parser::Function newOperator = _composeOperatorFunction(p_originator, ope);

				_availibleFunctions[newOperator.name].push_back(newOperator);
			}
		}
	}

	void Parser::_parseStructure(const BlockInfo& p_block)
	{
		const Type* insertedType = _insertType(_composeType(p_block));

		_computeMethodAndOperator(insertedType, p_block);
	}

	void Parser::_parseAttribute(const BlockInfo& p_block)
	{
		const Type* insertedType = _insertType(_composeType(p_block, "_Attributes"));

		_attributesTypes.insert(insertedType);

		_computeMethodAndOperator(insertedType, p_block);

		_insertVariable({
				.type = insertedType,
				.name = _composeIdentifierName(p_block.name.value.content),
				.arraySize = {}
			});

		_computeMethodAndOperator(insertedType, p_block);
	}

	void Parser::_parseConstant(const BlockInfo& p_block)
	{
		const Type* insertedType = _insertType(_composeType(p_block, "_Constants"));

		_constantsTypes.insert(insertedType);

		_computeMethodAndOperator(insertedType, p_block);

		_insertVariable({
				.type = insertedType,
				.name = _composeIdentifierName(p_block.name.value.content),
				.arraySize = {}
			});

		_computeMethodAndOperator(insertedType, p_block);
	}

	void Parser::_parseTexture(const TextureInfo& p_texture)
	{
		Parser::Variable newTexture = {
				.type = _findType("Texture"),
				.name = _composeIdentifierName(p_texture.name.value.content),
				.arraySize = _composeSizeArray(p_texture.arraySizes)
		};

		_variables.insert(newTexture);
		_reservedIdentifiers.insert(newTexture.name);
	}

	void Parser::_parseFunction(const FunctionInfo& p_function)
	{

	}

	void Parser::_parseNamespace(const NamespaceInfo& p_namespace)
	{
		for (const auto& block : p_namespace.structureBlocks)
		{
			_parseStructure(block);
		}

		for (const auto& block : p_namespace.attributeBlocks)
		{
			_parseAttribute(block);
		}

		for (const auto& block : p_namespace.constantBlocks)
		{
			_parseConstant(block);
		}

		for (const auto& texture : p_namespace.textureInfos)
		{
			_parseTexture(texture);
		}

		for (auto it : p_namespace.functionInfos)
		{
			for (const auto& function : it.second)
			{
				_parseFunction(function);
			}
		}

		for (const auto& nspace : p_namespace.nestedNamespaces)
		{
			_nspaces.push_back(nspace.name.value.content);

			_parseNamespace(nspace);

			_nspaces.pop_back();
		}
	}

	Parser::Product Parser::parse(const Lexer::Output& p_input)
	{
		return (Parser()._parse(p_input));
	}
}