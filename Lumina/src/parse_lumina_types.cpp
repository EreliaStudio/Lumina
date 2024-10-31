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

		std::vector<Lumina::Token> predefinedTokens = Lumina::Tokenizer::tokenize("predefined_header/lumina_header.lum");

		Lexer::Product lexerProduct = Lexer::lex(predefinedTokens);

		if (lexerProduct.errors.size() != 0)
		{
			for (const auto& error : lexerProduct.errors)
			{
				_product.errors.push_back(error);
			}
			return;
		}

		_parse(lexerProduct.value);

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

		_vertexVariables.insert({ _getType("Vector4"),  "pixelPosition", {} });
		_fragmentVariables.insert({ _getType("Color"),  "pixelColor", {} });
	}
}
