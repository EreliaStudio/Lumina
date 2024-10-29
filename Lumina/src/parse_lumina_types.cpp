#include "parser.hpp"

#include "tokenizer.hpp"
#include "lexer.hpp"

namespace Lumina
{
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

		_availibleFunctions.push_back({
				.isPrototype = false,
				.returnType = {_getType("Color"), {}},
				.name = "Texture_getPixel",
				.parameters = {
					{
						.type = _getType("Texture"),
						.isReference = false,
						.name = "this",
						.arraySize = {}
					},
					{
						.type = _getType("Vector2"),
						.isReference = false,
						.name = "UVs",
						.arraySize = {}
					}
				},
				.body = {
					.code = "return (texture(this, UVs));"
				}
			});

		_availibleFunctions.push_back({
				.isPrototype = false,
				.returnType = {_getType("Vector4"), {}},
				.name = "Matrix4x4_OperatorMultiplyVector4",
				.parameters = {
					{
						.type = _getType("Matrix4x4"),
						.isReference = false,
						.name = "matrix",
						.arraySize = {}
					},
					{
						.type = _getType("Vector4"),
						.isReference = false,
						.name = "vector",
						.arraySize = {}
					}
				},
				.body = {
					.code = ""
				}
			});

		_availibleFunctions.push_back({
				.isPrototype = false,
				.returnType = {_getType("Matrix4x4"), {}},
				.name = "Matrix4x4_OperatorMultiplyMatrix4x4",
				.parameters = {
					{
						.type = _getType("Matrix4x4"),
						.isReference = false,
						.name = "matrix",
						.arraySize = {}
					},
					{
						.type = _getType("Matrix4x4"),
						.isReference = false,
						.name = "other",
						.arraySize = {}
					}
				},
				.body = {
					.code = ""
				}
			});

		_availibleFunctions.push_back({
				.isPrototype = false,
				.returnType = {_getType("Matrix4x4"), {}},
				.name = "Matrix4x4_OperatorPlusMatrix4x4",
				.parameters = {
					{
						.type = _getType("Matrix4x4"),
						.isReference = false,
						.name = "matrix",
						.arraySize = {}
					},
					{
						.type = _getType("Matrix4x4"),
						.isReference = false,
						.name = "other",
						.arraySize = {}
					}
				},
				.body = {
					.code = ""
				}
			});
	}
}
