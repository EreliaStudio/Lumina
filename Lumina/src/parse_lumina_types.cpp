#include "parser.hpp"

#include "tokenizer.hpp"
#include "lexer.hpp"

namespace Lumina
{
	const std::map<std::string, std::string> Parser::_operatorNames = {
			{"+", "Plus"},
			{"-", "Minus"},
			{"*", "Multiply"},
			{"/", "Divide"},
			{"%", "Modulo"},

			{"=", "Assign"},
			{"+=", "AddAssign"},
			{"-=", "SubtractAssign"},
			{"*=", "MultiplyAssign"},
			{"/=", "DivideAssign"},
			{"%=", "ModuloAssign"},

			{"==", "Equal"},
			{"!=", "NEqual"},
			{"<", "Less"},
			{">", "Greater"},
			{"<=", "LEqual"},
			{">=", "GEqual"},

			{"&&", "And"},
			{"||", "Or"},

			{"++", "Increment"},
			{"--", "Decrement"},
	};

	Parser::Parser()
	{
		_availibleTypes = {
			{ "void", {} },
			{ "bool", {} },
			{ "int", {} },
			{ "uint", {} },
			{ "float", {} },
			{ "Matrix2x2", {} },
			{ "Matrix3x3", {} },
			{ "Matrix4x4", {} },
			{ "Vector2", {
					{{ "float", {}}, "x", {} },
					{{ "float", {}}, "y", {} }
				}
			},
			{ "Vector2Int", {
					{{ "int", {}}, "x", {} },
					{{ "int", {}}, "y", {} }
				}
			},
			{ "Vector2UInt", {
					{{ "uint", {}}, "x", {} },
					{{ "uint", {}}, "y", {} }
				}
			},
			{ "Vector3", {
					{{ "float", {}}, "x", {} },
					{{ "float", {}}, "y", {} },
					{{ "float", {}}, "z", {} }
				}
			},
			{ "Vector3Int", {
					{{ "int", {}}, "x", {} },
					{{ "int", {}}, "y", {} },
					{{ "int", {}}, "z", {} }
				}
			},
			{ "Vector3UInt", {
					{{ "uint", {}}, "x", {} },
					{{ "uint", {}}, "y", {} },
					{{ "uint", {}}, "z", {} }
				}
			},
			{ "Vector4", {
					{{ "float", {}}, "x", {} },
					{{ "float", {}}, "y", {} },
					{{ "float", {}}, "z", {} },
					{{ "float", {}}, "w", {} }
				}
			},
			{ "Vector4Int", {
					{{ "int", {}}, "x", {} },
					{{ "int", {}}, "y", {} },
					{{ "int", {}}, "z", {} },
					{{ "int", {}}, "w", {} }
				}
			},
			{ "Vector4UInt", {
					{{ "uint", {}}, "x", {} },
					{{ "uint", {}}, "y", {} },
					{{ "uint", {}}, "z", {} },
					{{ "uint", {}}, "w", {} }
				}
			},
			{ "Color", {}},
			{ "Texture", {} }
		};

		_convertionTable = {
			{
				_getType("int"),
				{_getType("int"), _getType("uint"), _getType("float")}
			},
			{
				_getType("float"),
				{_getType("float"), _getType("int"), _getType("uint")}
			},
			{
				_getType("uint"),
				{_getType("uint"), _getType("int"), _getType("float")}
			},
			{
				_getType("Vector2"),
				{_getType("Vector2"), _getType("Vector2Int"), _getType("Vector2UInt")}
			},
			{
				_getType("Vector2Int"),
				{_getType("Vector2Int"), _getType("Vector2"), _getType("Vector2UInt")}
			},
			{
				_getType("Vector2UInt"),
				{_getType("Vector2UInt"), _getType("Vector2Int"), _getType("Vector2")}
			},
			{
				_getType("Vector3"),
				{_getType("Vector3"), _getType("Vector3Int"), _getType("Vector3UInt")}
			},
			{
				_getType("Vector3Int"),
				{_getType("Vector3Int"), _getType("Vector3"), _getType("Vector3UInt")}
			},
			{
				_getType("Vector3UInt"),
				{_getType("Vector3UInt"), _getType("Vector3Int"), _getType("Vector3")}
			},
			{
				_getType("Vector4"),
				{_getType("Vector4"), _getType("Vector4Int"), _getType("Vector4UInt")}
			},
			{
				_getType("Vector4Int"),
				{_getType("Vector4Int"), _getType("Vector4"), _getType("Vector4UInt")}
			},
			{
				_getType("Vector4UInt"),
				{_getType("Vector4UInt"), _getType("Vector4Int"), _getType("Vector4")}
			}
		};

		std::map<std::string, std::vector<std::vector<std::string>>> constructorDescriptors = {
			{
				"bool",
				{
					{},
					{"int"},
					{"bool"},
				}
			},
			{
				"int",
				{
					{},
					{"int"},
					{"uint"},
					{"float"},
				}
			},
			{
				"uint",
				{
					{},
					{"int"},
					{"uint"},
					{"float"},
				}
			},
			{
				"float",
				{
					{},
					{"int"},
					{"uint"},
					{"float"},
				}
			},
			{
				"Vector2",
				{
					{},
					{"float", "float"}
				}
			},
			{
				"Vector2Int",
				{
					{},
					{"int", "int"}
				}
			},
			{
				"Vector2UInt",
				{
					{},
					{"uint", "uint"}
				}
			},
			{
				"Vector3",
				{
					{},
					{"float", "float", "float"},
					{"Vector2", "float"}
				}
			},
			{
				"Vector3Int",
				{
					{},
					{"int", "int", "int"},
					{"Vector2Int", "int"}
				}
			},
			{
				"Vector3UInt",
				{
					{},
					{"uint", "uint", "uint"},
					{"Vector2UInt", "uint"}
				}
			},
			{
				"Vector4",
				{
					{},
					{"float", "float", "float", "float"},
					{"Vector2", "float", "float"},
					{"Vector3", "float"}
				}
			},
			{
				"Vector4Int",
				{
					{},
					{"int", "int", "int", "int"},
					{"Vector2Int", "int", "int"},
					{"Vector3Int", "int"}
				}
			},
			{
				"Vector4UInt",
				{
					{},
					{"uint", "uint", "uint", "uint"},
					{"Vector2UInt", "uint", "uint"},
					{"Vector3UInt", "uint"}
				}
			}
		};

		for (const auto& [key, constructorArray] : constructorDescriptors)
		{
			for (const auto& parameterList : constructorArray)
			{
				FunctionImpl toAdd = {
					.isPrototype = false,
					.returnType = {_getType(key), {}},
					.name = key,
					.parameters = {
					},
					.body = {}
				};

				for (const auto& parameter : parameterList)
				{
					toAdd.parameters.push_back({
							.type = _getType(parameter),
							.isReference = false,
							.name = "",
							.arraySizes = {}
						});
				}

				_availibleFunctions.insert(toAdd);
			}
		}

		FunctionImpl getPixelFunction = {
				.isPrototype = false,
				.returnType = {_getType("Color"), {}},
				.name = "Texture_getPixel",
				.parameters = {
					{
						.type = _getType("Texture"),
						.isReference = false,
						.name = "this",
						.arraySizes = {}
					},
					{
						.type = _getType("Vector2"),
						.isReference = false,
						.name = "UVs",
						.arraySizes = {}
					}
				},
				.body = {
					.code = "return (texture(this, UVs));\n"
				}
		};

		_availibleFunctions.insert(getPixelFunction);
		_product.value.functions.push_back(getPixelFunction);
		
		std::vector<std::tuple<std::string, std::string, std::string, std::string>> operatorToAdd = {
			{"Matrix2x2", "*", "Vector2", "Vector2"},
			{"Matrix3x3", "*", "Vector3", "Vector3"},
			{"Matrix4x4", "*", "Vector4", "Vector4"}
		};

		std::vector<std::tuple<std::string, std::string>> unaryOperatorsToAdd = {
			{"int", "++"},
			{"int", "--"},
			{"int", "+"},
			{"int", "-"},
			{"uint", "++"},
			{"uint", "--"},
			{"float", "+"},
			{"float", "-"},
		};

		struct Descriptor
		{
			std::string name;
			std::vector<std::string> targets;
		};

		using Operation = std::tuple<std::vector<Descriptor>, std::vector<std::string>, std::vector<std::string>>;

		std::vector<Operation> operations = {
			{
				{ {"void", {"void"}} },
				{},
				{}
			},
			{
				{ {"bool", {"bool"}} },
				{"=", "==", "!="},
				{}
			},
			{
				{
					{"float",{
					 "float", "uint", "int"
					}},
					{"Vector2", {
					 "Vector2", "Vector2UInt", "Vector2Int"
					}},
					{"Vector3", {
					 "Vector3", "Vector3UInt", "Vector3Int"
					}},
					{"Vector4", {
					 "Vector4", "Vector4UInt", "Vector4Int"
					}}
				},
				{"=", "==", "!=", "+", "-", "*", "/", "+=", "-=", "*=", "/="},
				{"-", "+"}
			},
			{
				{
					{"uint",{
					 "float", "uint", "int"
					}},
					{"Vector2UInt", {
					 "Vector2", "Vector2UInt", "Vector2Int"
					}},
					{"Vector3UInt", {
					 "Vector3", "Vector3UInt", "Vector3Int"
					}},
					{"Vector4UInt", {
					 "Vector4", "Vector4UInt", "Vector4Int"
					}}
				},
				{"=", "==", "!=", "+", "-", "*", "/", "%", "+=", "-=", "*=", "/=", "%="},
				{}
			},
			{
				{
					{"int",{
					 "float", "uint", "int"
					}},
					{"Vector2Int", {
					 "Vector2", "Vector2UInt", "Vector2Int"
					}},
					{"Vector3Int", {
					 "Vector3", "Vector3UInt", "Vector3Int"
					}},
					{"Vector4Int", {
					 "Vector4", "Vector4UInt", "Vector4Int"
					}}
				},
				{"=", "==", "!=", "+", "-", "*", "/", "%", "+=", "-=", "*=", "/=", "%="},
				{"-", "+"}
			},
			{
				{
					{"Matrix2x2", {
					 "Matrix2x2"
					}},
					{"Matrix3x3", {
					 "Matrix3x3"
					}},
					{"Matrix4x4", {
					 "Matrix4x4"
					}}
				},
				{"=", "+", "*", "+=", "*="},
				{}
			}
		};

		for (const auto& operation : operations)
		{
			const auto& descriptors = std::get<0>(operation);
			const auto& operators = std::get<1>(operation);

			for (const auto& descriptor : descriptors)
			{
				const std::string& lhsType = descriptor.name;
				const auto& targets = descriptor.targets;

				for (const auto& targetType : targets)
				{
					for (const auto& op : operators)
					{
						std::string returnType;

						if (op == "==" || op == "!=" || op == "<" || op == ">" || op == "<=" || op == ">=")
						{
							returnType = "bool";
						}
						else if (op == "=" || op == "+=" || op == "-=" || op == "*=" || op == "/=" || op == "%=")
						{
							returnType = lhsType;
						}
						else if (op == "+" || op == "-" || op == "*" || op == "/" || op == "%")
						{
							if (lhsType == targetType)
							{
								returnType = lhsType;
							}
							else if ((lhsType == "float" && (targetType == "int" || targetType == "uint")) ||
								((lhsType == "int" || lhsType == "uint") && targetType == "float"))
							{
								returnType = "float";
							}
							else
							{
								returnType = lhsType;
							}
						}
						else
						{
							returnType = lhsType;
						}

						operatorToAdd.push_back(std::make_tuple(lhsType, op, targetType, returnType));
					}
				}
			}
		}

		
		for (const auto& tuple : operatorToAdd)
		{
			FunctionImpl toAdd = {
				.isPrototype = false,
				.returnType = {_getType(std::get<3>(tuple)), {}},
				.name = std::get<0>(tuple) + "_Operator" + _operatorNames.at(std::get<1>(tuple)),
				.parameters = {
					{
						.type = _getType(std::get<0>(tuple)),
						.isReference = false,
						.name = "lhs",
						.arraySizes = {}
					},
					{
						.type = _getType(std::get<2>(tuple)),
						.isReference = false,
						.name = "rhs",
						.arraySizes = {}
					}
				},
				.body = {
					.code = ""
				}
			};

			_availibleFunctions.insert(toAdd);
			//_product.value.functions.push_back(toAdd);
		}

		for (const auto& tuple : unaryOperatorsToAdd)
		{
			FunctionImpl toAdd = {
				.isPrototype = false,
				.returnType = {_getType(std::get<0>(tuple)), {}},
				.name = std::get<0>(tuple) + "_Operator" + _operatorNames.at(std::get<1>(tuple)),
				.parameters = {
					{
						.type = _getType(std::get<0>(tuple)),
						.isReference = true, // Usually true for unary operators that modify the operand
						.name = "value",
						.arraySizes = {}
					}
				},
				.body = {
					.code = ""
				}
			};

			_availibleFunctions.insert(toAdd);
		}

		_vertexVariables.insert({ _getType("Vector4"),  "pixelPosition", {} });
		_fragmentVariables.insert({ _getType("Color"),  "pixelColor", {} });

		struct MethodDescriptor {
			std::string methodName;
			std::string glslFunction;
			std::string returnType;
			std::vector<std::string> parameterTypes; // Parameter types excluding 'this'
		};

		// Map of type name to list of methods
		std::map<std::string, std::vector<MethodDescriptor>> methodsPerType = {
			{ "Vector2", {
				{ "length", "length", "float", {} },
				{ "normalize", "normalize", "Vector2", {} },
				{ "dot", "dot", "float", { "Vector2" } },
				{ "abs", "abs", "Vector2", {} },
				{ "floor", "floor", "Vector2", {} },
				{ "ceil", "ceil", "Vector2", {} },
				{ "mod", "mod", "Vector2", { "float" } },
				{ "min", "min", "Vector2", { "Vector2" } },
				{ "max", "max", "Vector2", { "Vector2" } },
				{ "clamp", "clamp", "Vector2", { "Vector2", "Vector2" } },
				{ "step", "step", "Vector2", { "Vector2" } },
				{ "smoothstep", "smoothstep", "Vector2", { "Vector2", "Vector2" } },
				{ "pow", "pow", "Vector2", { "Vector2" } },
				{ "exp", "exp", "Vector2", {} },
				{ "log", "log", "Vector2", {} },
				{ "exp2", "exp2", "Vector2", {} },
				{ "log2", "log2", "Vector2", {} },
				{ "sqrt", "sqrt", "Vector2", {} },
				{ "inversesqrt", "inversesqrt", "Vector2", {} },
				{ "sin", "sin", "Vector2", {} },
				{ "cos", "cos", "Vector2", {} },
				{ "tan", "tan", "Vector2", {} },
				{ "asin", "asin", "Vector2", {} },
				{ "acos", "acos", "Vector2", {} },
				{ "atan", "atan", "Vector2", {} },
				{ "lerp", "mix", "Vector2", { "Vector2", "float" } }
			}},
			{ "Vector3", {
				{ "length", "length", "float", {} },
				{ "normalize", "normalize", "Vector3", {} },
				{ "dot", "dot", "float", { "Vector3" } },
				{ "cross", "cross", "Vector3", { "Vector3" } },
				{ "abs", "abs", "Vector3", {} },
				{ "floor", "floor", "Vector3", {} },
				{ "ceil", "ceil", "Vector3", {} },
				{ "mod", "mod", "Vector3", { "float" } },
				{ "min", "min", "Vector3", { "Vector3" } },
				{ "max", "max", "Vector3", { "Vector3" } },
				{ "clamp", "clamp", "Vector3", { "Vector3", "Vector3" } },
				{ "step", "step", "Vector3", { "Vector3" } },
				{ "smoothstep", "smoothstep", "Vector3", { "Vector3", "Vector3" } },
				{ "pow", "pow", "Vector3", { "Vector3" } },
				{ "exp", "exp", "Vector3", {} },
				{ "log", "log", "Vector3", {} },
				{ "exp2", "exp2", "Vector3", {} },
				{ "log2", "log2", "Vector3", {} },
				{ "sqrt", "sqrt", "Vector3", {} },
				{ "inversesqrt", "inversesqrt", "Vector3", {} },
				{ "sin", "sin", "Vector3", {} },
				{ "cos", "cos", "Vector3", {} },
				{ "tan", "tan", "Vector3", {} },
				{ "asin", "asin", "Vector3", {} },
				{ "acos", "acos", "Vector3", {} },
				{ "atan", "atan", "Vector3", {} },
				{ "lerp", "mix", "Vector3", { "Vector3", "float" } }
			}},
			{ "Vector4", {
				{ "length", "length", "float", {} },
				{ "normalize", "normalize", "Vector4", {} },
				{ "dot", "dot", "float", { "Vector4" } },
				{ "abs", "abs", "Vector4", {} },
				{ "floor", "floor", "Vector4", {} },
				{ "ceil", "ceil", "Vector4", {} },
				{ "mod", "mod", "Vector4", { "float" } },
				{ "min", "min", "Vector4", { "Vector4" } },
				{ "max", "max", "Vector4", { "Vector4" } },
				{ "clamp", "clamp", "Vector4", { "Vector4", "Vector4" } },
				{ "step", "step", "Vector4", { "Vector4" } },
				{ "smoothstep", "smoothstep", "Vector4", { "Vector4", "Vector4" } },
				{ "pow", "pow", "Vector4", { "Vector4" } },
				{ "exp", "exp", "Vector4", {} },
				{ "log", "log", "Vector4", {} },
				{ "exp2", "exp2", "Vector4", {} },
				{ "log2", "log2", "Vector4", {} },
				{ "sqrt", "sqrt", "Vector4", {} },
				{ "inversesqrt", "inversesqrt", "Vector4", {} },
				{ "sin", "sin", "Vector4", {} },
				{ "cos", "cos", "Vector4", {} },
				{ "tan", "tan", "Vector4", {} },
				{ "asin", "asin", "Vector4", {} },
				{ "acos", "acos", "Vector4", {} },
				{ "atan", "atan", "Vector4", {} },
				{ "lerp", "mix", "Vector4", { "Vector4", "float" } }
			}}
		};

		for (const auto& typeMethodsPair : methodsPerType) {
			const std::string& typeName = typeMethodsPair.first;
			const std::vector<MethodDescriptor>& methods = typeMethodsPair.second;

			for (const auto& method : methods) {
				// Determine return type
				ExpressionTypeImpl returnType = { _getType(method.returnType), {} };

				// Create the method function
				FunctionImpl methodFunction;
				methodFunction.isPrototype = false;
				methodFunction.returnType = returnType;
				methodFunction.name = typeName + "_" + method.methodName;

				// Parameters: 'this' and any additional parameters
				methodFunction.parameters.push_back({
					.type = _getType(typeName),
					.isReference = false,
					.name = "this",
					.arraySizes = {}
					});

				// Add additional parameters
				for (const auto& paramTypeName : method.parameterTypes) {
					ExpressionTypeImpl paramType = { _getType(paramTypeName), {} };
					methodFunction.parameters.push_back({
						.type = paramType.type,
						.isReference = false,
						.name = "param" + std::to_string(methodFunction.parameters.size()),
						.arraySizes = {}
						});
				}

				// Build the method body
				std::string args = "this";
				for (size_t i = 1; i < methodFunction.parameters.size(); ++i) {
					args += ", " + methodFunction.parameters[i].name;
				}
				methodFunction.body.code = "return " + method.glslFunction + "(" + args + ");";

				// Insert the method into available functions
				_availibleFunctions.insert(methodFunction);

				// Optionally, add the method to the product's functions
				_product.value.functions.push_back(methodFunction);
			}
		}

		struct FunctionDescriptor {
			std::string functionName;                      // Name of the function in your language
			std::string glslFunction;                      // Corresponding GLSL function
			ExpressionTypeImpl returnType;                 // Return type of the function
			std::vector<ExpressionTypeImpl> parameterTypes; // Parameter types
		};

		// Create a map of functions per type
		std::map<std::string, std::vector<FunctionDescriptor>> functionsPerType = {
			// Functions for float type
			{ "float", {
				// Trigonometric Functions
				{ "sin", "sin", { _getType("float"), {} }, { { _getType("float"), {} } } },
				{ "cos", "cos", { _getType("float"), {} }, { { _getType("float"), {} } } },
				{ "tan", "tan", { _getType("float"), {} }, { { _getType("float"), {} } } },
				{ "asin", "asin", { _getType("float"), {} }, { { _getType("float"), {} } } },
				{ "acos", "acos", { _getType("float"), {} }, { { _getType("float"), {} } } },
				{ "atan", "atan", { _getType("float"), {} }, { { _getType("float"), {} } } },
				// Mathematical Functions
				{ "min", "min", { _getType("float"), {} }, { { _getType("float"), {} }, { _getType("float"), {} } } },
				{ "max", "max", { _getType("float"), {} }, { { _getType("float"), {} }, { _getType("float"), {} } } },
				{ "clamp", "clamp", { _getType("float"), {} }, { { _getType("float"), {} }, { _getType("float"), {} }, { _getType("float"), {} } } },
				{ "lerp", "mix", { _getType("float"), {} }, { { _getType("float"), {} }, { _getType("float"), {} }, { _getType("float"), {} } } },
				// Exponential Functions
				{ "pow", "pow", { _getType("float"), {} }, { { _getType("float"), {} }, { _getType("float"), {} } } },
				{ "exp", "exp", { _getType("float"), {} }, { { _getType("float"), {} } } },
				{ "log", "log", { _getType("float"), {} }, { { _getType("float"), {} } } },
				{ "exp2", "exp2", { _getType("float"), {} }, { { _getType("float"), {} } } },
				{ "log2", "log2", { _getType("float"), {} }, { { _getType("float"), {} } } },
				{ "sqrt", "sqrt", { _getType("float"), {} }, { { _getType("float"), {} } } },
				{ "inversesqrt", "inversesqrt", { _getType("float"), {} }, { { _getType("float"), {} } } },
				// Other Functions
				{ "abs", "abs", { _getType("float"), {} }, { { _getType("float"), {} } } },
				{ "mod", "mod", { _getType("float"), {} }, { { _getType("float"), {} }, { _getType("float"), {} } } },
				{ "floor", "floor", { _getType("float"), {} }, { { _getType("float"), {} } } },
				{ "ceil", "ceil", { _getType("float"), {} }, { { _getType("float"), {} } } },
				{ "fract", "fract", { _getType("float"), {} }, { { _getType("float"), {} } } },
				{ "sign", "sign", { _getType("float"), {} }, { { _getType("float"), {} } } }
			}},
			// Functions for int type
			{ "int", {
				// Mathematical Functions
				{ "min", "min", { _getType("int"), {} }, { { _getType("int"), {} }, { _getType("int"), {} } } },
				{ "max", "max", { _getType("int"), {} }, { { _getType("int"), {} }, { _getType("int"), {} } } },
				{ "clamp", "clamp", { _getType("int"), {} }, { { _getType("int"), {} }, { _getType("int"), {} }, { _getType("int"), {} } } },
				{ "abs", "abs", { _getType("int"), {} }, { { _getType("int"), {} } } },
				{ "mod", "mod", { _getType("int"), {} }, { { _getType("int"), {} }, { _getType("int"), {} } } },
				// Other Functions
				{ "sign", "sign", { _getType("int"), {} }, { { _getType("int"), {} } } }
				// Note: Trigonometric functions are not applicable to int
				// 'lerp' can be defined if desired
			}},
			// Functions for uint type
			{ "uint", {
				// Mathematical Functions
				{ "min", "min", { _getType("uint"), {} }, { { _getType("uint"), {} }, { _getType("uint"), {} } } },
				{ "max", "max", { _getType("uint"), {} }, { { _getType("uint"), {} }, { _getType("uint"), {} } } },
				{ "clamp", "clamp", { _getType("uint"), {} }, { { _getType("uint"), {} }, { _getType("uint"), {} }, { _getType("uint"), {} } } },
				{ "abs", "abs", { _getType("uint"), {} }, { { _getType("uint"), {} } } },
				{ "mod", "mod", { _getType("uint"), {} }, { { _getType("uint"), {} }, { _getType("uint"), {} } } }
				// Note: 'sign' may not be meaningful for uint
				// Trigonometric functions are not applicable to uint
			}},
		};

		// Function to add functions from the map to _availibleFunctions
		for (const auto& [typeName, functions] : functionsPerType) {
			for (const auto& func : functions) {
				FunctionImpl functionImpl;
				functionImpl.isPrototype = false;
				functionImpl.returnType = func.returnType;
				functionImpl.name = func.functionName;
				functionImpl.parameters.clear();

				// Add parameters
				for (size_t i = 0; i < func.parameterTypes.size(); ++i) {
					functionImpl.parameters.push_back({
						.type = func.parameterTypes[i].type,
						.isReference = false,
						.name = "param" + std::to_string(i),
						.arraySizes = {}
						});
				}

				// Build the function body
				std::string args;
				for (size_t i = 0; i < functionImpl.parameters.size(); ++i) {
					if (i > 0) args += ", ";
					args += functionImpl.parameters[i].name;
				}
				functionImpl.body.code = "";

				// Insert the function into available functions
				_availibleFunctions.insert(functionImpl);
			}
		}
	}
}
