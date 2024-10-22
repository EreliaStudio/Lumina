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
					.name = "Color",
					.attributes = {
				}
			});
	}

	void Parser::composeTextureType()
	{
		Type* textureType = _insertType({
					.name = "Texture",
					.attributes = {
				}
			});

		textureType->methods.insert({
				"getPixel", {
					{
					.returnType = {
							.type = _findType("Color"),
							.arraySize = {}
						},
					.name = {"getPixel"},
					.parameters = {
							{
								.type = _findType("Vector2"),
								.name = "UV",
								.arraySize = {}
							}
						}
					}
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
		composeTextureType();
	}
}