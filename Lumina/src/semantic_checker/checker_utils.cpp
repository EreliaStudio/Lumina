#include "lumina_semantic_checker.hpp"

namespace Lumina
{
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
		Type voidType;
		voidType.name = "void";
		addStandardType(voidType);

		// bool type
		Type boolType;
		boolType.name = "bool";
		boolType.operators = { "&&", "||" };
		boolType.comparaisonOperators = { "==", "!=" };
		addStandardType(boolType);

		// int type
		Type intType;
		intType.name = "int";
		intType.operators = { "+", "-", "*", "/", "%", "+=", "-=", "*=", "/=", "%=" };
		intType.comparaisonOperators = { "==", "!=", "<", ">", "<=", ">=" };
		addStandardType(intType);

		// float type
		Type floatType;
		floatType.name = "float";
		floatType.operators = { "+", "-", "*", "/", "+=", "-=", "*=", "/=" };
		floatType.comparaisonOperators = { "==", "!=", "<", ">", "<=", ">=" };
		addStandardType(floatType);

		// uint type
		Type uintType;
		uintType.name = "uint";
		uintType.operators = { "+", "-", "*", "/", "%", "+=", "-=", "*=", "/=", "%=" };
		uintType.comparaisonOperators = { "==", "!=", "<", ">", "<=", ">=" };
		addStandardType(uintType);

		// Vector2 type
		Type vector2Type;
		vector2Type.name = "Vector2";
		vector2Type.attributes.push_back({ type("float"), "x", 1 });
		vector2Type.attributes.push_back({ type("float"), "y", 1 });
		vector2Type.operators = { "+", "-", "*", "/", "+=", "-=", "*=", "/=" }; // Assuming element-wise operations
		vector2Type.comparaisonOperators = { "==", "!=" }; // Assuming vector comparison
		vector2Type.constructors = {
			{type("float"), type("float")}
		};
		addStandardType(vector2Type);

		// Vector2Int type
		Type vector2IntType;
		vector2IntType.name = "Vector2Int";
		vector2IntType.attributes.push_back({ type("int"), "x", 1 });
		vector2IntType.attributes.push_back({ type("int"), "y", 1 });
		vector2IntType.operators = { "+", "-", "*", "/", "+=", "-=", "*=", "/=" }; // Assuming element-wise operations
		vector2IntType.comparaisonOperators = { "==", "!=" }; // Assuming vector comparison
		vector2IntType.constructors = {
			{type("int"), type("int")}
		};
		addStandardType(vector2IntType);

		// Vector2UInt type
		Type vector2UIntType;
		vector2UIntType.name = "Vector2UInt";
		vector2UIntType.attributes.push_back({ type("uint"), "x", 1 });
		vector2UIntType.attributes.push_back({ type("uint"), "y", 1 });
		vector2UIntType.operators = { "+", "-", "*", "/", "+=", "-=", "*=", "/=" }; // Assuming element-wise operations
		vector2UIntType.comparaisonOperators = { "==", "!=" }; // Assuming vector comparison
		vector2UIntType.constructors = {
			{type("uint"), type("uint")}
		};
		addStandardType(vector2UIntType);

		// Vector3 type
		Type vector3Type;
		vector3Type.name = "Vector3";
		vector3Type.attributes.push_back({ type("float"), "x", 1 });
		vector3Type.attributes.push_back({ type("float"), "y", 1 });
		vector3Type.attributes.push_back({ type("float"), "z", 1 });
		vector3Type.attributes.push_back({ type("Vector2"), "xy", 1 });
		vector3Type.attributes.push_back({ type("Vector2"), "yz", 1 });
		vector3Type.operators = { "+", "-", "*", "/", "+=", "-=", "*=", "/=" }; // Assuming element-wise operations
		vector3Type.comparaisonOperators = { "==", "!=" }; // Assuming vector comparison
		vector3Type.constructors = {
			{type("float"), type("float"), type("float")},
			{type("Vector2"), type("float")},
			{type("float"), type("Vector2")}
		};
		addStandardType(vector3Type);

		// Vector3Int type
		Type vector3IntType;
		vector3IntType.name = "Vector3Int";
		vector3IntType.attributes.push_back({ type("int"), "x", 1 });
		vector3IntType.attributes.push_back({ type("int"), "y", 1 });
		vector3IntType.attributes.push_back({ type("int"), "z", 1 });
		vector3IntType.attributes.push_back({ type("Vector2Int"), "xy", 1 });
		vector3IntType.attributes.push_back({ type("Vector2Int"), "yz", 1 });
		vector3IntType.operators = { "+", "-", "*", "/", "+=", "-=", "*=", "/=" }; // Assuming element-wise operations
		vector3IntType.comparaisonOperators = { "==", "!=" }; // Assuming vector comparison
		vector3IntType.constructors = {
			{type("int"), type("int"), type("int")},
			{type("Vector2Int"), type("int")},
			{type("int"), type("Vector2Int")}
		};
		addStandardType(vector3IntType);

		// Vector3UInt type
		Type vector3UIntType;
		vector3UIntType.name = "Vector3UInt";
		vector3UIntType.attributes.push_back({ type("uint"), "x", 1 });
		vector3UIntType.attributes.push_back({ type("uint"), "y", 1 });
		vector3UIntType.attributes.push_back({ type("uint"), "z", 1 });
		vector3UIntType.attributes.push_back({ type("Vector2UInt"), "xy", 1 });
		vector3UIntType.attributes.push_back({ type("Vector2UInt"), "yz", 1 });
		vector3UIntType.operators = { "+", "-", "*", "/", "+=", "-=", "*=", "/=" }; // Assuming element-wise operations
		vector3UIntType.comparaisonOperators = { "==", "!=" }; // Assuming vector comparison
		vector3UIntType.constructors = {
			{type("uint"), type("uint"), type("uint")},
			{type("Vector2UInt"), type("uint")},
			{type("uint"), type("Vector2UInt")}
		};
		addStandardType(vector3UIntType);

		// Vector4 type
		Type vector4Type;
		vector4Type.name = "Vector4";
		vector4Type.attributes.push_back({ type("float"), "x", 1 });
		vector4Type.attributes.push_back({ type("float"), "y", 1 });
		vector4Type.attributes.push_back({ type("float"), "z", 1 });
		vector4Type.attributes.push_back({ type("float"), "w", 1 });
		vector4Type.attributes.push_back({ type("Vector2"), "xy", 1 });
		vector4Type.attributes.push_back({ type("Vector3"), "xyz", 1 });
		vector4Type.attributes.push_back({ type("float"), "r", 1 });
		vector4Type.attributes.push_back({ type("float"), "g", 1 });
		vector4Type.attributes.push_back({ type("float"), "b", 1 });
		vector4Type.attributes.push_back({ type("float"), "a", 1 });
		vector4Type.attributes.push_back({ type("Vector3"), "rgb", 1 });
		vector4Type.attributes.push_back({ type("Vector3"), "rgba", 1 });
		vector4Type.operators = { "+", "-", "*", "/", "+=", "-=", "*=", "/=" }; // Assuming element-wise operations
		vector4Type.comparaisonOperators = { "==", "!=" }; // Assuming vector comparison
		vector4Type.constructors = {
			{type("float"), type("float"), type("float"), type("float")},
			{type("Vector3"), type("float")},
			{type("float"), type("Vector3")},
			{type("Vector2"), type("Vector2")},
			{type("float"), type("Vector2"), type("float")},
			{type("Vector2"), type("float"), type("float")},
			{type("float"), type("float"), type("Vector2")}
		};
		addStandardType(vector4Type);

		// Vector4Int type
		Type vector4IntType;
		vector4IntType.name = "Vector4Int";
		vector4IntType.attributes.push_back({ type("int"), "x", 1 });
		vector4IntType.attributes.push_back({ type("int"), "y", 1 });
		vector4IntType.attributes.push_back({ type("int"), "z", 1 });
		vector4IntType.attributes.push_back({ type("int"), "w", 1 });
		vector4IntType.attributes.push_back({ type("Vector2Int"), "xy", 1 });
		vector4IntType.attributes.push_back({ type("Vector3Int"), "xyz", 1 });
		vector4IntType.attributes.push_back({ type("int"), "r", 1 });
		vector4IntType.attributes.push_back({ type("int"), "g", 1 });
		vector4IntType.attributes.push_back({ type("int"), "b", 1 });
		vector4IntType.attributes.push_back({ type("int"), "a", 1 });
		vector4IntType.attributes.push_back({ type("Vector3Int"), "rgb", 1 });
		vector4IntType.attributes.push_back({ type("Vector3Int"), "rgba", 1 });
		vector4IntType.operators = { "+", "-", "*", "/", "+=", "-=", "*=", "/=" }; // Assuming element-wise operations
		vector4IntType.comparaisonOperators = { "==", "!=" }; // Assuming vector comparison
		vector4IntType.constructors = {
			{type("int"), type("int"), type("int"), type("int")},
			{type("Vector3Int"), type("int")},
			{type("int"), type("Vector3Int")},
			{type("Vector2Int"), type("Vector2Int")},
			{type("int"), type("Vector2Int"), type("int")},
			{type("Vector2Int"), type("int"), type("int")},
			{type("int"), type("int"), type("Vector2Int")}
		};
		addStandardType(vector4IntType);

		// Vector4UInt type
		Type vector4UIntType;
		vector4UIntType.name = "Vector4UInt";
		vector4UIntType.attributes.push_back({ type("uint"), "x", 1 });
		vector4UIntType.attributes.push_back({ type("uint"), "y", 1 });
		vector4UIntType.attributes.push_back({ type("uint"), "z", 1 });
		vector4UIntType.attributes.push_back({ type("uint"), "w", 1 });
		vector4UIntType.attributes.push_back({ type("Vector2UInt"), "xy", 1 });
		vector4UIntType.attributes.push_back({ type("Vector3UInt"), "xyz", 1 });
		vector4UIntType.attributes.push_back({ type("uint"), "r", 1 });
		vector4UIntType.attributes.push_back({ type("uint"), "g", 1 });
		vector4UIntType.attributes.push_back({ type("uint"), "b", 1 });
		vector4UIntType.attributes.push_back({ type("uint"), "a", 1 });
		vector4UIntType.attributes.push_back({ type("Vector3UInt"), "rgb", 1 });
		vector4UIntType.attributes.push_back({ type("Vector3UInt"), "rgba", 1 });
		vector4UIntType.operators = { "+", "-", "*", "/", "+=", "-=", "*=", "/=" }; // Assuming element-wise operations
		vector4UIntType.comparaisonOperators = { "==", "!=" }; // Assuming vector comparison
		vector4UIntType.constructors = {
			{type("uint"), type("uint"), type("uint"), type("uint")},
			{type("Vector3UInt"), type("uint")},
			{type("uint"), type("Vector3UInt")},
			{type("Vector2UInt"), type("Vector2UInt")},
			{type("uint"), type("Vector2UInt"), type("uint")},
			{type("Vector2UInt"), type("uint"), type("uint")},
			{type("uint"), type("uint"), type("Vector2UInt")}
		};
		addStandardType(vector4UIntType);

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

		type("Vector4")->acceptedConversions = { type("Vector4Int"), type("Vector4UInt") };
		type("Vector4Int")->acceptedConversions = { type("Vector4"), type("Vector4UInt") };
		type("Vector4UInt")->acceptedConversions = { type("Vector4"), type("Vector4Int") };
	}

	void SemanticChecker::setupStructures()
	{
		// Matrix2x2 type
		Type matrix2x2;
		matrix2x2.name = "Matrix2x2";
		matrix2x2.operators = { "*" };
		matrix2x2.acceptedConversions = { type("Vector2"), type("Vector2Int"), type("Vector2UInt") };
		addStructure(matrix2x2);

		// Matrix3x3 type
		Type matrix3x3;
		matrix3x3.name = "Matrix3x3";
		matrix3x3.operators = { "*" };
		matrix3x3.acceptedConversions = { type("Vector3"), type("Vector3Int"), type("Vector3UInt") };
		addStructure(matrix3x3);

		// Matrix4x4 type
		Type matrix4x4;
		matrix4x4.name = "Matrix4x4";
		matrix4x4.operators = { "*" };
		matrix4x4.acceptedConversions = { type("Vector4"), type("Vector4Int"), type("Vector4UInt") };
		addStructure(matrix4x4);

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
	}
	
	std::ostream& operator<<(std::ostream& os, const SemanticChecker::Type& type)
	{
		os << "Type: " << type.name << "\n";
		os << "Attributes:\n";
		for (const auto& attr : type.attributes)
		{
			os << "  - " << attr.name << ": " << (attr.type ? attr.type->name : "null") << (attr.size == 0 ? "" : "[" + std::to_string(attr.size) + "]") << "\n";
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