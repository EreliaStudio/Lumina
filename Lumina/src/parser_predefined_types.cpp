#include "parser.hpp"

namespace Lumina
{
	void Parser::composeStandardTypes()
	{
		Type* voidType = _insertType({
					.name = "void",
					.attributes = {
				}
			});

		Type* boolType = _insertType({
					.name = "bool",
					.attributes = {
				}
			});

		boolType->constructors.push_back({
				.parameters = { {.type = boolType, .isReference = false, .name = "value", .arraySize = {} } }
			});

		Type* colorType = _insertType({
					.name = "Color",
					.attributes = {
					{
						.type = _findType("float"),
						.name = "r",
						.arraySize = {}
					},
					{
						.type = _findType("float"),
						.name = "g",
						.arraySize = {}
					},
					{
						.type = _findType("float"),
						.name = "b",
						.arraySize = {}
					},
					{
						.type = _findType("float"),
						.name = "a",
						.arraySize = {}
					}
				}
			});

		colorType->constructors.push_back({
				.parameters = {
					{.type = colorType, .isReference = false, .name = "p_r", .arraySize = {} }
				}
			});
		colorType->constructors.push_back({
				.parameters = {
					{.type = _findType("float"), .isReference = false, .name = "p_r", .arraySize = {} },
					{.type = _findType("float"), .isReference = false, .name = "p_g", .arraySize = {} },
					{.type = _findType("float"), .isReference = false, .name = "p_b", .arraySize = {} },
					{.type = _findType("float"), .isReference = false, .name = "p_a", .arraySize = {} }
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
		Type* intType = _insertType({
					.name = "int",
					.attributes = {
					}
			});

		Type* uintType = _insertType({
					.name = "uint",
					.attributes = {
				}
			});

		Type* floatType = _insertType({
					.name = "float",
					.attributes = {
				}
			});

		intType->constructors.push_back({
				.parameters = { {.type = intType, .isReference = false, .name = "value", .arraySize = {} } }
			});

		intType->constructors.push_back({
				.parameters = { {.type = uintType, .isReference = false, .name = "value", .arraySize = {} } }
			});

		intType->constructors.push_back({
				.parameters = { {.type = floatType, .isReference = false, .name = "value", .arraySize = {} } }
			});



		uintType->constructors.push_back({
				.parameters = { {.type = intType, .isReference = false, .name = "value", .arraySize = {} } }
			});

		uintType->constructors.push_back({
				.parameters = { {.type = uintType, .isReference = false, .name = "value", .arraySize = {} } }
			});

		uintType->constructors.push_back({
				.parameters = { {.type = floatType, .isReference = false, .name = "value", .arraySize = {} } }
			});



		floatType->constructors.push_back({
				.parameters = { {.type = intType, .isReference = false, .name = "value", .arraySize = {} } }
			});

		floatType->constructors.push_back({
				.parameters = { {.type = uintType, .isReference = false, .name = "value", .arraySize = {} } }
			});

		floatType->constructors.push_back({
				.parameters = { {.type = floatType, .isReference = false, .name = "value", .arraySize = {} } }
			});
	}

	void Parser::composeVector2Types()
	{
		Type* ivec2Type = _insertType({
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

		Type* uvec2Type = _insertType({
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

		Type* vec2Type = _insertType({
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

		ivec2Type->constructors.push_back({
				.parameters = { {.type = ivec2Type, .isReference = false, .name = "value", .arraySize = {} } }
			});
		ivec2Type->constructors.push_back({
				.parameters = { {.type = uvec2Type, .isReference = false, .name = "value", .arraySize = {} } }
			});
		ivec2Type->constructors.push_back({
				.parameters = { {.type = vec2Type, .isReference = false, .name = "value", .arraySize = {} } }
			});
		ivec2Type->constructors.push_back({
				.parameters = {
					{.type = _findType("int"), .isReference = false, .name = "p_x", .arraySize = {} },
					{.type = _findType("int"), .isReference = false, .name = "p_y", .arraySize = {} }
				}
			});

		uvec2Type->constructors.push_back({
				.parameters = { {.type = ivec2Type, .isReference = false, .name = "value", .arraySize = {} } }
			});
		uvec2Type->constructors.push_back({
				.parameters = { {.type = uvec2Type, .isReference = false, .name = "value", .arraySize = {} } }
			});
		uvec2Type->constructors.push_back({
				.parameters = { {.type = vec2Type, .isReference = false, .name = "value", .arraySize = {} } }
			});
		uvec2Type->constructors.push_back({
				.parameters = {
					{.type = _findType("uint"), .isReference = false, .name = "p_x", .arraySize = {} },
					{.type = _findType("uint"), .isReference = false, .name = "p_y", .arraySize = {} }
				}
			});

		vec2Type->constructors.push_back({
				.parameters = { {.type = ivec2Type, .isReference = false, .name = "value", .arraySize = {} } }
			});
		vec2Type->constructors.push_back({
				.parameters = { {.type = uvec2Type, .isReference = false, .name = "value", .arraySize = {} } }
			});
		vec2Type->constructors.push_back({
				.parameters = { {.type = vec2Type, .isReference = false, .name = "value", .arraySize = {} } }
			});
		vec2Type->constructors.push_back({
				.parameters = {
					{.type = _findType("float"), .isReference = false, .name = "p_x", .arraySize = {} },
					{.type = _findType("float"), .isReference = false, .name = "p_y", .arraySize = {} }
				}
			});
	}

	void Parser::composeVector3Types()
	{
		Type* ivec3Type = _insertType({
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

		Type* uvec3Type = _insertType({
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

		Type* vec3Type = _insertType({
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



		ivec3Type->constructors.push_back({
				.parameters = { {.type = ivec3Type, .isReference = false, .name = "value", .arraySize = {} } }
			});
		ivec3Type->constructors.push_back({
				.parameters = { {.type = uvec3Type, .isReference = false, .name = "value", .arraySize = {} } }
			});
		ivec3Type->constructors.push_back({
				.parameters = { {.type = vec3Type, .isReference = false, .name = "value", .arraySize = {} } }
			});
		ivec3Type->constructors.push_back({
				.parameters = {
					{.type = _findType("int"), .isReference = false, .name = "p_x", .arraySize = {} },
					{.type = _findType("int"), .isReference = false, .name = "p_y", .arraySize = {} },
					{.type = _findType("int"), .isReference = false, .name = "p_z", .arraySize = {} }
				}
			});
		ivec3Type->constructors.push_back({
				.parameters = {
					{.type = _findType("Vector2Int"), .isReference = false, .name = "p_xy", .arraySize = {} },
					{.type = _findType("int"), .isReference = false, .name = "p_z", .arraySize = {} }
				}
			});

		uvec3Type->constructors.push_back({
				.parameters = { {.type = ivec3Type, .isReference = false, .name = "value", .arraySize = {} } }
			});
		uvec3Type->constructors.push_back({
				.parameters = { {.type = uvec3Type, .isReference = false, .name = "value", .arraySize = {} } }
			});
		uvec3Type->constructors.push_back({
				.parameters = { {.type = vec3Type, .isReference = false, .name = "value", .arraySize = {} } }
			});
		uvec3Type->constructors.push_back({
				.parameters = {
					{.type = _findType("uint"), .isReference = false, .name = "p_x", .arraySize = {} },
					{.type = _findType("uint"), .isReference = false, .name = "p_y", .arraySize = {} },
					{.type = _findType("uint"), .isReference = false, .name = "p_z", .arraySize = {} }
				}
			});
		uvec3Type->constructors.push_back({
				.parameters = {
					{.type = _findType("Vector2UInt"), .isReference = false, .name = "p_xy", .arraySize = {} },
					{.type = _findType("int"), .isReference = false, .name = "p_z", .arraySize = {} }
				}
			});

		vec3Type->constructors.push_back({
				.parameters = { {.type = ivec3Type, .isReference = false, .name = "value", .arraySize = {} } }
			});
		vec3Type->constructors.push_back({
				.parameters = { {.type = uvec3Type, .isReference = false, .name = "value", .arraySize = {} } }
			});
		vec3Type->constructors.push_back({
				.parameters = { {.type = vec3Type, .isReference = false, .name = "value", .arraySize = {} } }
			});
		vec3Type->constructors.push_back({
				.parameters = {
					{.type = _findType("float"), .isReference = false, .name = "p_x", .arraySize = {} },
					{.type = _findType("float"), .isReference = false, .name = "p_y", .arraySize = {} },
					{.type = _findType("float"), .isReference = false, .name = "p_z", .arraySize = {} }
				}
			});
		vec3Type->constructors.push_back({
				.parameters = {
					{.type = _findType("Vector2"), .isReference = false, .name = "p_xy", .arraySize = {} },
					{.type = _findType("float"), .isReference = false, .name = "p_z", .arraySize = {} }
				}
			});
	}

	void Parser::composeVector4Types()
	{
		Type* ivec4Type = _insertType({
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

		Type* uvec4Type = _insertType({
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

		Type* vec4Type = _insertType({
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


		ivec4Type->constructors.push_back({
					.parameters = { {.type = ivec4Type, .isReference = false, .name = "value", .arraySize = {} } }
				});
		ivec4Type->constructors.push_back({
				.parameters = { {.type = uvec4Type, .isReference = false, .name = "value", .arraySize = {} } }
			});
		ivec4Type->constructors.push_back({
				.parameters = { {.type = vec4Type, .isReference = false, .name = "value", .arraySize = {} } }
			});
		ivec4Type->constructors.push_back({
				.parameters = {
					{.type = _findType("int"), .isReference = false, .name = "p_x", .arraySize = {} },
					{.type = _findType("int"), .isReference = false, .name = "p_y", .arraySize = {} },
					{.type = _findType("int"), .isReference = false, .name = "p_z", .arraySize = {} },
					{.type = _findType("int"), .isReference = false, .name = "p_w", .arraySize = {} }
				}
			});
		ivec4Type->constructors.push_back({
				.parameters = {
					{.type = _findType("Vector2Int"), .isReference = false, .name = "p_xy", .arraySize = {} },
					{.type = _findType("int"), .isReference = false, .name = "p_z", .arraySize = {} },
					{.type = _findType("int"), .isReference = false, .name = "p_w", .arraySize = {} }
				}
			});
		ivec4Type->constructors.push_back({
				.parameters = {
					{.type = _findType("Vector3Int"), .isReference = false, .name = "p_xyz", .arraySize = {} },
					{.type = _findType("int"), .isReference = false, .name = "p_w", .arraySize = {} }
				}
			});

		uvec4Type->constructors.push_back({
				.parameters = { {.type = ivec4Type, .isReference = false, .name = "value", .arraySize = {} } }
			});
		uvec4Type->constructors.push_back({
				.parameters = { {.type = uvec4Type, .isReference = false, .name = "value", .arraySize = {} } }
			});
		uvec4Type->constructors.push_back({
				.parameters = { {.type = vec4Type, .isReference = false, .name = "value", .arraySize = {} } }
			});
		uvec4Type->constructors.push_back({
				.parameters = {
					{.type = _findType("uint"), .isReference = false, .name = "p_x", .arraySize = {} },
					{.type = _findType("uint"), .isReference = false, .name = "p_y", .arraySize = {} },
					{.type = _findType("uint"), .isReference = false, .name = "p_z", .arraySize = {} },
					{.type = _findType("uint"), .isReference = false, .name = "p_w", .arraySize = {} }
				}
			});
		uvec4Type->constructors.push_back({
				.parameters = {
					{.type = _findType("Vector2UInt"), .isReference = false, .name = "p_xy", .arraySize = {} },
					{.type = _findType("uint"), .isReference = false, .name = "p_z", .arraySize = {} },
					{.type = _findType("uint"), .isReference = false, .name = "p_w", .arraySize = {} }
				}
			});
		uvec4Type->constructors.push_back({
				.parameters = {
					{.type = _findType("Vector3UInt"), .isReference = false, .name = "p_xyz", .arraySize = {} },
					{.type = _findType("uint"), .isReference = false, .name = "p_w", .arraySize = {} }
				}
			});

		vec4Type->constructors.push_back({
				.parameters = { {.type = ivec4Type, .isReference = false, .name = "value", .arraySize = {} } }
			});
		vec4Type->constructors.push_back({
				.parameters = { {.type = uvec4Type, .isReference = false, .name = "value", .arraySize = {} } }
			});
		vec4Type->constructors.push_back({
				.parameters = { {.type = vec4Type, .isReference = false, .name = "value", .arraySize = {} } }
			});
		vec4Type->constructors.push_back({
				.parameters = {
					{.type = _findType("float"), .isReference = false, .name = "p_x", .arraySize = {} },
					{.type = _findType("float"), .isReference = false, .name = "p_y", .arraySize = {} },
					{.type = _findType("float"), .isReference = false, .name = "p_z", .arraySize = {} },
					{.type = _findType("float"), .isReference = false, .name = "p_w", .arraySize = {} }
				}
			});
		vec4Type->constructors.push_back({
				.parameters = {
					{.type = _findType("Vector2"), .isReference = false, .name = "p_xy", .arraySize = {} },
					{.type = _findType("float"), .isReference = false, .name = "p_z", .arraySize = {} },
					{.type = _findType("float"), .isReference = false, .name = "p_w", .arraySize = {} }
				}
			});
		vec4Type->constructors.push_back({
				.parameters = {
					{.type = _findType("Vector3"), .isReference = false, .name = "p_xyz", .arraySize = {} },
					{.type = _findType("float"), .isReference = false, .name = "p_w", .arraySize = {} }
				}
			});
	}

	Parser::Parser()
	{
		_reservedIdentifiers.insert("main");
		composeScalarTypes();
		composeStandardTypes();
		composeVector2Types();
		composeVector3Types();
		composeVector4Types();
		composeTextureType();
	}
}