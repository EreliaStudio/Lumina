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

		boolType->acceptedConvertions = { boolType };

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

		colorType->acceptedConvertions = { colorType };

		colorType->constructors = {
			composeConstructor(colorType, "Color(float p_r, float p_g, float p_b, float p_a){r = p_r; g = p_g; b = p_b; a = p_a;}")	,
			composeConstructor(colorType, "Color(Color p_other){r = p_other.r; g = p_other.g; b = p_other.b; a = p_other.a;}")
		};
	}

	void Parser::composeTextureType()
	{
		Type* textureType = _insertType({
					.name = "Texture",
					.attributes = {
				}
			});

		textureType->methods["getPixel"] = {
			composeMethod(textureType, R"(Color getPixel(Vector2 p_UV){return (texture(self, p_UV));})")
		};
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

		intType->acceptedConvertions = { intType, uintType, floatType };
		uintType->acceptedConvertions = { intType, uintType, floatType };
		floatType->acceptedConvertions = { intType, uintType, floatType };
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

		ivec2Type->acceptedConvertions = { ivec2Type, uvec2Type, vec2Type };
		uvec2Type->acceptedConvertions = { ivec2Type, uvec2Type, vec2Type };
		vec2Type->acceptedConvertions = { ivec2Type, uvec2Type, vec2Type };

		ivec2Type->constructors = {
				composeConstructor(ivec2Type, R"(Vector2Int(int p_x, int p_y){x = p_x; y = p_y;})")
		};

		uvec2Type->constructors = {
				composeConstructor(uvec2Type, R"(Vector2UInt(uint p_x, uint p_y){x = p_x; y = p_y;})")
		};

		vec2Type->constructors = {
				composeConstructor(vec2Type, R"(Vector2(float p_x, float p_y){x = p_x; y = p_y;})")
		};
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


		ivec3Type->acceptedConvertions = { ivec3Type, uvec3Type, vec3Type };
		uvec3Type->acceptedConvertions = { ivec3Type, uvec3Type, vec3Type };
		vec3Type->acceptedConvertions = { ivec3Type, uvec3Type, vec3Type };

		ivec3Type->constructors = {
				composeConstructor(ivec3Type, R"(Vector3Int(int p_x, int p_y, int p_z){x = p_x; y = p_y; z = p_z;})"),
				composeConstructor(ivec3Type, R"(Vector3Int(Vector2Int p_input, int p_z){x = p_input.x; y = p_input.y; z = p_z;})")
		};

		uvec3Type->constructors = {
				composeConstructor(uvec3Type, R"(Vector3UInt(uint p_x, uint p_y, uint p_z){x = p_x; y = p_y; z = p_z;})"),
				composeConstructor(uvec3Type, R"(Vector3UInt(Vector2UInt p_input, uint p_z){x = p_input.x; y = p_input.y; z = p_z;})")
		};

		vec3Type->constructors = {
				composeConstructor(vec3Type, R"(Vector3(float p_x, float p_y, float p_z){x = p_x; y = p_y; z = p_z;})"),
				composeConstructor(vec3Type, R"(Vector3(Vector2 p_input, float p_z){x = p_input.x; y = p_input.y; z = p_z;})")
		};
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


		ivec4Type->acceptedConvertions = { ivec4Type, uvec4Type, vec4Type };
		uvec4Type->acceptedConvertions = { ivec4Type, uvec4Type, vec4Type };
		vec4Type->acceptedConvertions = { ivec4Type, uvec4Type, vec4Type };

		ivec4Type->constructors = {
				composeConstructor(ivec4Type, R"(Vector4Int(int p_x, int p_y, int p_z, int p_w){x = p_x; y = p_y; z = p_z; w = p_w;})"),
				composeConstructor(ivec4Type, R"(Vector4Int(Vector2Int p_input, int p_z, int p_w){x = p_input.x; y = p_input.y; z = p_z; w = p_w;})"),
				composeConstructor(ivec4Type, R"(Vector4Int(Vector3Int p_input, int p_w){x = p_input.x; y = p_input.y; z = p_input.z; w = p_w;})")
		};

		uvec4Type->constructors = {
				composeConstructor(uvec4Type, R"(Vector4UInt(uint p_x, uint p_y, uint p_z, uint p_w){x = p_x; y = p_y; z = p_z; w = p_w;})"),
				composeConstructor(uvec4Type, R"(Vector4UInt(Vector2UInt p_input, uint p_z, uint p_w){x = p_input.x; y = p_input.y; z = p_z; w = p_w;})"),
				composeConstructor(uvec4Type, R"(Vector4UInt(Vector3UInt p_input, uint p_w){x = p_input.x; y = p_input.y; z = p_input.z; w = p_w;})")
		};

		vec4Type->constructors = {
				composeConstructor(vec4Type, R"(Vector4(float p_x, float p_y, float p_z, float p_w){x = p_x; y = p_y; z = p_z; w = p_w;})"),
				composeConstructor(vec4Type, R"(Vector4(Vector2 p_input, float p_z, float p_w){x = p_input.x; y = p_input.y; z = p_z; w = p_w;})"),
				composeConstructor(vec4Type, R"(Vector4(Vector3 p_input, float p_w){x = p_input.x; y = p_input.y; z = p_input.z; w = p_w;})")
		};
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