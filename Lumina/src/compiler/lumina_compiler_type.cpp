#include "lumina_compiler.hpp"

namespace Lumina
{
	void Compiler::createScalarTypes()
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

	void Compiler::createFloatVectorTypes()
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
				},
				{
					.variable = {
						.type = floatTypePtr,
						.name = "y",
						.arraySizes = {}
					},
					.cpuOffset = 4,
					.gpuOffset = 4
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

	void Compiler::createIntVectorTypes()
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

	void Compiler::createUIntVectorTypes()
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

	void Compiler::createMatrixTypes()
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

	void Compiler::createLuminaTypes()
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

	void Compiler::addType(const Compiler::Type& p_type)
	{
		if (_types.contains(p_type) == true)
		{
			throw std::runtime_error("Type [" + p_type.name + "] already defined");
		}
		_types.insert(p_type);
	}

	void Compiler::addStandardType(const Compiler::Type& p_type)
	{
		if (_types.contains(p_type) == true)
		{
			throw std::runtime_error("Type [" + p_type.name + "] already defined");
		}
		_types.insert(p_type);
		_standardTypes.insert(p_type);
	}

	const Compiler::Type* Compiler::_type(const std::string& p_typeName) const
	{
		auto it = _types.find(Type{ p_typeName, {}, {} });
		if (it != _types.end())
		{
			return &(*it);
		}
		return nullptr;
	}
	const Compiler::Type* Compiler::type(const Lumina::Token& p_typeToken) const
	{
		const Type* result = _type(p_typeToken.content);

		if (result == nullptr)
		{
			throw TokenBasedError("Type [" + p_typeToken.content + "] not found", p_typeToken);
		}

		return (result);
	}
}