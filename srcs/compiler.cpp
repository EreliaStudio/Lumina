#include "compiler.hpp"

#include "converter.hpp"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <iostream>
#include <limits>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace
{
	std::string safeTokenContent(const Token &token)
	{
		return token.content.empty() ? "<anonymous>" : token.content;
	}

	std::string formatName(const Name &name)
	{
		if (name.parts.empty())
		{
			return {};
		}

		std::string text;
		for (std::size_t i = 0; i < name.parts.size(); ++i)
		{
			if (i > 0)
			{
				text += "::";
			}
			text += safeTokenContent(name.parts[i]);
		}
		return text;
	}

	std::string formatTypeName(const TypeName &type)
	{
		std::string text;
		if (type.isConst)
		{
			text += "const ";
		}
		text += formatName(type.name);
		return text;
	}

	using LayoutEntry = StageIO;
	using FramebufferEntry = StageIO;
	using TextureEntry = TextureBinding;

struct BlockMember
{
	std::string name;
	std::string kind = "Element";
	int offset = 0;
	int size = 0;
	int elementSize = 0;
	int elementCount = 0;
	std::vector<BlockMember> members;
};

enum class MemoryLayout
{
	Std140,
	Std430
};

struct TypeLayoutInfo
{
	int size = 0;
	int alignment = 1;
	std::vector<BlockMember> members;
};

struct FieldLayoutInfo
{
	BlockMember member;
	int alignment = 1;
	int size = 0;
};

int roundUp(int value, int alignment)
{
	if (alignment <= 0)
	{
		return value;
	}
	const int remainder = value % alignment;
	return remainder == 0 ? value : value + alignment - remainder;
}

bool isScalarType(const std::string &typeName)
{
	return typeName == "bool" || typeName == "int" || typeName == "uint" || typeName == "float";
}

bool isColorType(const std::string &typeName)
{
	return typeName == "Color";
}

bool tryParseVector(const std::string &typeName, int &components)
{
	if (typeName.rfind("Vector", 0) != 0 || typeName.size() < 7)
	{
		return false;
	}
	if (!std::isdigit(static_cast<unsigned char>(typeName[6])))
	{
		return false;
	}
	components = typeName[6] - '0';
	return components >= 2 && components <= 4;
}

bool tryParseMatrix(const std::string &typeName, int &columns, int &rows)
{
	if (typeName.rfind("Matrix", 0) != 0)
	{
		return false;
	}
	const std::size_t xPos = typeName.find('x', 6);
	if (xPos == std::string::npos || xPos + 1 >= typeName.size())
	{
		return false;
	}
	columns = std::stoi(typeName.substr(6, xPos - 6));
	rows = std::stoi(typeName.substr(xPos + 1));
	return columns > 0 && rows > 0;
}

	struct DynamicArrayLayout
	{
		std::string name;
		int offset = 0;
		int elementStride = 0;
		int elementPadding = 0;
		std::vector<BlockMember> members;
	};

	struct BlockDefinition
	{
		std::string name;
		std::string type;
		int size = 0;
		std::vector<BlockMember> members;
		std::optional<DynamicArrayLayout> dynamicArray;
	};

std::string jsonEscape(const std::string &value)
	{
		std::string escaped;
		escaped.reserve(value.size() + 8);
		for (char c : value)
		{
			switch (c)
			{
				case '\\':
					escaped += "\\\\";
					break;
				case '"':
					escaped += "\\\"";
					break;
				case '\b':
					escaped += "\\b";
					break;
				case '\f':
					escaped += "\\f";
					break;
				case '\n':
					escaped += "\\n";
					break;
				case '\r':
					escaped += "\\r";
					break;
				case '\t':
					escaped += "\\t";
					break;
				default:
				{
					const unsigned char uc = static_cast<unsigned char>(c);
					if (uc < 0x20)
					{
						const char *digits = "0123456789ABCDEF";
						escaped += "\\u00";
						escaped.push_back(digits[uc >> 4]);
						escaped.push_back(digits[uc & 0x0F]);
					}
					else
					{
						escaped.push_back(c);
					}
					break;
				}
			}
		}
		return escaped;
	}

	std::optional<int> evaluateIntegralExpression(const Expression &expression)
	{
		switch (expression.kind)
		{
			case Expression::Kind::Literal:
			{
				const auto &literal = static_cast<const LiteralExpression &>(expression);
				try
				{
					long long value = std::stoll(literal.literal.content, nullptr, 0);
					if (value > std::numeric_limits<int>::max() || value < std::numeric_limits<int>::min())
					{
						return std::nullopt;
					}
					return static_cast<int>(value);
				}
				catch (...)
				{
					return std::nullopt;
				}
			}
			case Expression::Kind::Unary:
			{
				const auto &unary = static_cast<const UnaryExpression &>(expression);
				if (!unary.operand)
				{
					return std::nullopt;
				}
				auto operand = evaluateIntegralExpression(*unary.operand);
				if (!operand)
				{
					return std::nullopt;
				}
				switch (unary.op)
				{
					case UnaryOperator::Positive:
						return *operand;
					case UnaryOperator::Negate:
						return -*operand;
					case UnaryOperator::BitwiseNot:
						return ~*operand;
					default:
						return std::nullopt;
				}
			}
			case Expression::Kind::Binary:
			{
				const auto &binary = static_cast<const BinaryExpression &>(expression);
				if (!binary.left || !binary.right)
				{
					return std::nullopt;
				}
				auto left = evaluateIntegralExpression(*binary.left);
				auto right = evaluateIntegralExpression(*binary.right);
				if (!left || !right)
				{
					return std::nullopt;
				}
				switch (binary.op)
				{
					case BinaryOperator::Add:
						return *left + *right;
					case BinaryOperator::Subtract:
						return *left - *right;
					case BinaryOperator::Multiply:
						return *left * *right;
					case BinaryOperator::Divide:
						return (*right == 0) ? std::nullopt : std::optional<int>(*left / *right);
					case BinaryOperator::Modulo:
						return (*right == 0) ? std::nullopt : std::optional<int>(*left % *right);
					case BinaryOperator::BitwiseAnd:
						return *left & *right;
					case BinaryOperator::BitwiseOr:
						return *left | *right;
					case BinaryOperator::BitwiseXor:
						return *left ^ *right;
					case BinaryOperator::ShiftLeft:
						return *left << *right;
					case BinaryOperator::ShiftRight:
						return *left >> *right;
					default:
						return std::nullopt;
				}
			}
			case Expression::Kind::ArrayLiteral:
				return std::nullopt;
			default:
				return std::nullopt;
		}
	}

	std::optional<int> evaluateArrayLength(const VariableDeclarator &declarator)
	{
		if (!declarator.hasArraySuffix || !declarator.hasArraySize || !declarator.arraySize)
		{
			return std::nullopt;
		}
		return evaluateIntegralExpression(*declarator.arraySize);
}

void writeIndent(std::ostringstream &oss, int indent)
	{
		for (int i = 0; i < indent; ++i)
		{
			oss.put(' ');
		}
	}

	void writeJsonString(std::ostringstream &oss, const std::string &text)
	{
		oss << '"' << jsonEscape(text) << '"';
	}

	template <typename Collection, typename Writer>
	void writeJsonArray(std::ostringstream &oss, int indent, const Collection &items, Writer writer)
	{
		oss << "[";
		if (items.empty())
		{
			oss << "]";
			return;
		}

		oss << "\n";
		for (std::size_t i = 0; i < items.size(); ++i)
		{
			writer(items[i], indent + 2);
			if (i + 1 < items.size())
			{
				oss << ",\n";
			}
			else
			{
				oss << "\n";
			}
		}
		writeIndent(oss, indent);
		oss << "]";
	}

	void writeBlockMembers(std::ostringstream &oss, const std::vector<BlockMember> &members, int indent);

	void writeBlockMember(std::ostringstream &oss, const BlockMember &member, int indent)
	{
		writeIndent(oss, indent);
		oss << "{\n";

		writeIndent(oss, indent + 2);
		writeJsonString(oss, "name");
		oss << ": ";
		writeJsonString(oss, member.name);
		oss << ",\n";

		writeIndent(oss, indent + 2);
		writeJsonString(oss, "offset");
		oss << ": " << member.offset << ",\n";

		writeIndent(oss, indent + 2);
		writeJsonString(oss, "type");
		oss << ": ";
		writeJsonString(oss, member.kind);
		oss << ",\n";

		writeIndent(oss, indent + 2);
		writeJsonString(oss, "size");
		bool hasArrayInfo = (member.kind == "Array");
		bool hasNested = !member.members.empty();
		oss << ": " << member.size;
		oss << (hasArrayInfo || hasNested ? ",\n" : "\n");

		if (hasArrayInfo)
		{
			writeIndent(oss, indent + 2);
			writeJsonString(oss, "elementSize");
			oss << ": " << member.elementSize << ",\n";

			writeIndent(oss, indent + 2);
			writeJsonString(oss, "nbElements");
			oss << ": " << member.elementCount;
			oss << (hasNested ? ",\n" : "\n");
		}

		if (hasNested)
		{
			writeIndent(oss, indent + 2);
			writeJsonString(oss, "members");
			oss << ": ";
			writeBlockMembers(oss, member.members, indent + 2);
			oss << "\n";
		}

		writeIndent(oss, indent);
		oss << "}";
	}

	void writeBlockMembers(std::ostringstream &oss, const std::vector<BlockMember> &members, int indent)
	{
		writeJsonArray(
		    oss,
		    indent,
		    members,
		    [&](const BlockMember &child, int childIndent)
		    {
			    writeBlockMember(oss, child, childIndent);
		    });
	}

	void writeDynamicArray(std::ostringstream &oss, const DynamicArrayLayout &layout, int indent)
	{
		writeIndent(oss, indent);
		oss << "{\n";

		writeIndent(oss, indent + 2);
		writeJsonString(oss, "name");
		oss << ": ";
		writeJsonString(oss, layout.name);
		oss << ",\n";

		writeIndent(oss, indent + 2);
		writeJsonString(oss, "offset");
		oss << ": " << layout.offset << ",\n";

		writeIndent(oss, indent + 2);
		writeJsonString(oss, "elementStride");
		oss << ": " << layout.elementStride << ",\n";

		writeIndent(oss, indent + 2);
		writeJsonString(oss, "elementPadding");
		const bool hasMembers = !layout.members.empty();
		oss << ": " << layout.elementPadding;
		oss << (hasMembers ? ",\n" : "\n");

		if (hasMembers)
		{
			writeIndent(oss, indent + 2);
			writeJsonString(oss, "members");
			oss << ": ";
			writeBlockMembers(oss, layout.members, indent + 2);
			oss << "\n";
		}

		writeIndent(oss, indent);
		oss << "}";
	}

	struct CompilerContext
	{
		std::vector<LayoutEntry> layouts;
		std::vector<StageIO> varyings;
		std::vector<FramebufferEntry> framebuffers;
		std::vector<TextureEntry> textures;
		std::vector<BlockDefinition> constants;
		std::vector<BlockDefinition> attributes;
		std::string vertexSource;
		std::string fragmentSource;
		bool hasVertexStage = false;
		bool hasFragmentStage = false;

		std::unordered_map<std::string, const AggregateInstruction *> structLookup;
		std::vector<std::string> namespaceStack;

		int nextLayoutLocation = 0;
		int nextVaryingLocation = 0;
		int nextFramebufferLocation = 0;
		int nextTextureLocation = 0;

		void collectStructs(const std::vector<std::unique_ptr<Instruction>> &instructions)
		{
			for (const std::unique_ptr<Instruction> &instruction : instructions)
			{
				if (!instruction)
				{
					continue;
				}

				switch (instruction->type)
				{
					case Instruction::Type::Aggregate:
					{
						const auto &aggregate = static_cast<const AggregateInstruction &>(*instruction);
						if (aggregate.kind == AggregateInstruction::Kind::Struct)
						{
							const std::string qualified = qualify(aggregate.name);
							structLookup.emplace(qualified, &aggregate);
						}
						break;
					}
					case Instruction::Type::Namespace:
					{
						const auto &ns = static_cast<const NamespaceInstruction &>(*instruction);
						pushNamespace(ns.name);
						collectStructs(ns.instructions);
						popNamespace();
						break;
					}
					default:
						break;
				}
			}
		}

		void process(const std::vector<std::unique_ptr<Instruction>> &instructions)
		{
			for (const std::unique_ptr<Instruction> &instruction : instructions)
			{
				if (!instruction)
				{
					continue;
				}

				switch (instruction->type)
				{
					case Instruction::Type::Pipeline:
						handlePipeline(static_cast<const PipelineInstruction &>(*instruction));
						break;
					case Instruction::Type::Variable:
						handleVariable(static_cast<const VariableInstruction &>(*instruction));
						break;
					case Instruction::Type::Aggregate:
						handleAggregate(static_cast<const AggregateInstruction &>(*instruction));
						break;
					case Instruction::Type::StageFunction:
						handleStageFunction(static_cast<const StageFunctionInstruction &>(*instruction));
						break;
					case Instruction::Type::Namespace:
					{
						const auto &ns = static_cast<const NamespaceInstruction &>(*instruction);
						pushNamespace(ns.name);
						process(ns.instructions);
						popNamespace();
						break;
					}
					default:
						break;
				}
			}
		}

		void handlePipeline(const PipelineInstruction &pipeline)
		{
			const std::string variableName = safeTokenContent(pipeline.variable);
			const std::string typeName = formatTypeName(pipeline.payloadType);

			if (pipeline.source == Stage::Input && pipeline.destination == Stage::VertexPass)
			{
				LayoutEntry entry;
				entry.location = nextLayoutLocation++;
				entry.type = typeName;
				entry.name = variableName;
				layouts.push_back(std::move(entry));
			}
			else if (pipeline.source == Stage::FragmentPass && pipeline.destination == Stage::Output)
			{
				FramebufferEntry entry;
				entry.location = nextFramebufferLocation++;
				entry.type = typeName;
				entry.name = variableName;
				framebuffers.push_back(std::move(entry));
			}
			else if (pipeline.source == Stage::VertexPass && pipeline.destination == Stage::FragmentPass)
			{
				StageIO entry;
				entry.location = nextVaryingLocation++;
				entry.type = typeName;
				entry.name = variableName;
				varyings.push_back(std::move(entry));
			}
		}

		void handleVariable(const VariableInstruction &variable)
		{
			const std::string declaredType = formatName(variable.declaration.type.name);
			if (declaredType != "Texture")
			{
				return;
			}

			for (const VariableDeclarator &declarator : variable.declaration.declarators)
			{
				TextureEntry entry;
				entry.luminaName = safeTokenContent(declarator.name);
				entry.location = nextTextureLocation;
				entry.glslName = "_tx" + std::to_string(nextTextureLocation);
				++nextTextureLocation;
				entry.type = "sampler2D";
				entry.scope = declarator.textureBindingScope;
				textures.push_back(std::move(entry));
			}
		}

		void handleAggregate(const AggregateInstruction &aggregate)
		{
			switch (aggregate.kind)
			{
				case AggregateInstruction::Kind::ConstantBlock:
					constants.push_back(makeBlockDefinition(aggregate));
					break;
				case AggregateInstruction::Kind::AttributeBlock:
					attributes.push_back(makeBlockDefinition(aggregate));
					break;
				default:
					break;
			}
		}

		void handleStageFunction(const StageFunctionInstruction &stageFunction)
		{
			const std::string label = safeTokenContent(stageFunction.stageToken);
			const std::string placeholder = "// TODO: emit GLSL for " + label + "\n";
			switch (stageFunction.stage)
			{
				case Stage::VertexPass:
					hasVertexStage = true;
					vertexSource = placeholder;
					break;
				case Stage::FragmentPass:
					hasFragmentStage = true;
					fragmentSource = placeholder;
					break;
				default:
					break;
			}
		}

		BlockDefinition makeBlockDefinition(const AggregateInstruction &aggregate)
		{
			BlockDefinition block;
			block.name = qualify(aggregate.name);
			block.type = aggregateHasUnsizedArray(aggregate) ? "SSBO" : "UBO";
			block.size = 0;

			std::vector<std::string> recursion;
			recursion.push_back(block.name);
			block.members = buildMembers(aggregate, recursion, block);
			recursion.pop_back();

			return block;
		}

		void assignDynamicArray(
		    BlockDefinition &block,
		    const TypeName &elementType,
		    const VariableDeclarator &declarator,
		    std::vector<std::string> &recursion,
		    MemoryLayout layout,
		    int &currentOffset,
		    int &maxAlign)
		{
			if (block.dynamicArray.has_value())
			{
				throw std::runtime_error(
				    "Block '" + block.name + "' contains more than one unsized array (previous '" +
				    block.dynamicArray->name + "', new '" + safeTokenContent(declarator.name) + "')");
			}

			TypeLayoutInfo elementLayout = layoutType(elementType, layout, recursion);
			block.type = "SSBO";

			int arrayAlignment = elementLayout.alignment;
			if (layout == MemoryLayout::Std140)
			{
				arrayAlignment = roundUp(arrayAlignment, 16);
			}

			const int alignedOffset = roundUp(currentOffset, arrayAlignment);

			DynamicArrayLayout dynamicLayout;
			dynamicLayout.name = safeTokenContent(declarator.name);
			dynamicLayout.offset = alignedOffset;
			dynamicLayout.elementPadding = 0;
			if (layout == MemoryLayout::Std140)
			{
				dynamicLayout.elementStride = roundUp(elementLayout.size, 16);
			}
			else
			{
				dynamicLayout.elementStride = roundUp(elementLayout.size, elementLayout.alignment);
			}
			dynamicLayout.members = std::move(elementLayout.members);
			block.dynamicArray = std::move(dynamicLayout);

			currentOffset = alignedOffset;
			maxAlign = std::max(maxAlign, arrayAlignment);
		}

		std::vector<BlockMember> buildMembers(
		    const AggregateInstruction &aggregate, std::vector<std::string> &recursion, BlockDefinition &block)
		{
			const MemoryLayout layout = (block.type == "SSBO") ? MemoryLayout::Std430 : MemoryLayout::Std140;
			std::vector<BlockMember> members;
			int currentOffset = 0;
			int maxAlign = 1;
			bool hasDynamicArray = false;

			for (const std::unique_ptr<StructMember> &member : aggregate.members)
			{
				if (!member || member->kind != StructMember::Kind::Field)
				{
					continue;
				}

				const auto &field = static_cast<const FieldMember &>(*member);
				for (const VariableDeclarator &declarator : field.declaration.declarators)
				{
					const bool hasArray = declarator.hasArraySuffix;
					const bool hasSize = declarator.hasArraySize;
					if (hasArray && !hasSize)
					{
						assignDynamicArray(block, field.declaration.type, declarator, recursion, layout, currentOffset, maxAlign);
						hasDynamicArray = true;
						break;
					}

					FieldLayoutInfo info = layoutField(field.declaration.type, declarator, layout, recursion);
					const int alignedOffset = roundUp(currentOffset, info.alignment);
					info.member.offset = alignedOffset;
					info.member.size = info.size;
					currentOffset = alignedOffset + info.size;
					maxAlign = std::max(maxAlign, info.alignment);
					members.push_back(std::move(info.member));
				}

				if (hasDynamicArray)
				{
					break;
				}
			}

			int blockAlignment = maxAlign;
			if (layout == MemoryLayout::Std140)
			{
				blockAlignment = roundUp(blockAlignment, 16);
			}

			if (block.dynamicArray.has_value())
			{
				block.size = roundUp(block.dynamicArray->offset, blockAlignment);
			}
			else
			{
				block.size = roundUp(currentOffset, blockAlignment);
			}

			return members;
		}

		std::string qualify(const Token &name) const
		{
			std::string qualified;
			for (std::size_t i = 0; i < namespaceStack.size(); ++i)
			{
				if (i > 0)
				{
					qualified += "::";
				}
				qualified += namespaceStack[i];
			}

			if (!qualified.empty())
			{
				qualified += "::";
			}
			qualified += safeTokenContent(name);
			return qualified;
		}

		void pushNamespace(const Token &name)
		{
			namespaceStack.push_back(safeTokenContent(name));
		}

		void popNamespace()
		{
			if (!namespaceStack.empty())
			{
				namespaceStack.pop_back();
			}
		}

	private:
		FieldLayoutInfo layoutField(
		    const TypeName &type,
		    const VariableDeclarator &declarator,
		    MemoryLayout layout,
		    std::vector<std::string> &recursion) const;

		TypeLayoutInfo layoutType(const TypeName &type, MemoryLayout layout, std::vector<std::string> &recursion) const;

		TypeLayoutInfo layoutAggregateType(
		    const AggregateInstruction &aggregate,
		    MemoryLayout layout,
		    std::vector<std::string> &recursion) const;

		bool aggregateHasUnsizedArray(const AggregateInstruction &aggregate) const;
	};

	FieldLayoutInfo CompilerContext::layoutField(
	    const TypeName &type,
	    const VariableDeclarator &declarator,
	    MemoryLayout layout,
	    std::vector<std::string> &recursion) const
	{
		FieldLayoutInfo result;
		result.member.name = safeTokenContent(declarator.name);
		result.member.kind = "Element";

		TypeLayoutInfo typeLayout = layoutType(type, layout, recursion);
		result.member.members = typeLayout.members;
		result.size = typeLayout.size;
		result.alignment = typeLayout.alignment;

		result.member.elementSize = 0;
		result.member.elementCount = 0;

		if (declarator.hasArraySuffix)
		{
			result.member.kind = "Array";
			int arrayAlignment = typeLayout.alignment;
			int stride = typeLayout.size;

			if (layout == MemoryLayout::Std140)
			{
				arrayAlignment = roundUp(arrayAlignment, 16);
				stride = roundUp(stride, 16);
			}
			else
			{
				stride = roundUp(stride, typeLayout.alignment);
			}

			result.member.elementSize = stride;
			const std::optional<int> count = evaluateArrayLength(declarator);
			result.member.elementCount = count.value_or(0);
			result.alignment = arrayAlignment;
			result.size = stride * result.member.elementCount;
		}

		result.member.size = result.size;
		return result;
	}

	TypeLayoutInfo CompilerContext::layoutType(
	    const TypeName &type, MemoryLayout layout, std::vector<std::string> &recursion) const
	{
		TypeLayoutInfo info;
		const std::string typeName = formatName(type.name);
		if (typeName.empty())
		{
			info.size = 0;
			info.alignment = 4;
			return info;
		}

		if (isScalarType(typeName))
		{
			info.size = 4;
			info.alignment = 4;
			return info;
		}

		if (isColorType(typeName))
		{
			info.size = 16;
			info.alignment = 16;
			return info;
		}

		int components = 0;
		if (tryParseVector(typeName, components))
		{
			info.size = components * 4;
			info.alignment = (components == 2) ? 8 : 16;
			return info;
		}

		int columns = 0;
		int rows = 0;
		if (tryParseMatrix(typeName, columns, rows))
		{
			int columnAlignment = (rows == 2) ? 8 : 16;
			if (layout == MemoryLayout::Std140)
			{
				columnAlignment = roundUp(columnAlignment, 16);
			}
			const int stride = roundUp(rows * 4, (layout == MemoryLayout::Std140) ? 16 : columnAlignment);
			info.size = stride * columns;
			info.alignment = columnAlignment;
			return info;
		}

		const auto structIt = structLookup.find(typeName);
		if (structIt == structLookup.end())
		{
			info.size = 0;
			info.alignment = 16;
			return info;
		}

		if (std::find(recursion.begin(), recursion.end(), typeName) != recursion.end())
		{
			info.size = 0;
			info.alignment = 16;
			return info;
		}

		recursion.push_back(typeName);
		info = layoutAggregateType(*structIt->second, layout, recursion);
		recursion.pop_back();
		return info;
	}

TypeLayoutInfo CompilerContext::layoutAggregateType(
    const AggregateInstruction &aggregate,
    MemoryLayout layout,
    std::vector<std::string> &recursion) const
{
		TypeLayoutInfo info;
		int currentOffset = 0;
		int maxAlign = 1;

		for (const std::unique_ptr<StructMember> &member : aggregate.members)
		{
			if (!member || member->kind != StructMember::Kind::Field)
			{
				continue;
			}

			const auto &field = static_cast<const FieldMember &>(*member);
			for (const VariableDeclarator &declarator : field.declaration.declarators)
			{
				FieldLayoutInfo fieldLayout = layoutField(field.declaration.type, declarator, layout, recursion);
				const int alignedOffset = roundUp(currentOffset, fieldLayout.alignment);
				fieldLayout.member.offset = alignedOffset;
				fieldLayout.member.size = fieldLayout.size;
				currentOffset = alignedOffset + fieldLayout.size;
				maxAlign = std::max(maxAlign, fieldLayout.alignment);
				info.members.push_back(std::move(fieldLayout.member));
			}
		}

		int structAlignment = maxAlign;
		if (layout == MemoryLayout::Std140)
		{
			structAlignment = roundUp(structAlignment, 16);
		}

	info.size = roundUp(currentOffset, structAlignment);
	info.alignment = structAlignment;
	return info;
}

bool CompilerContext::aggregateHasUnsizedArray(const AggregateInstruction &aggregate) const
{
	for (const std::unique_ptr<StructMember> &member : aggregate.members)
	{
		if (!member || member->kind != StructMember::Kind::Field)
		{
			continue;
		}

		const auto &field = static_cast<const FieldMember &>(*member);
		for (const VariableDeclarator &declarator : field.declaration.declarators)
		{
			if (declarator.hasArraySuffix && !declarator.hasArraySize)
			{
				return true;
			}
		}
	}

	return false;
}

std::string emitJson(const CompilerContext &context)
{
	std::ostringstream oss;
	oss << "{\n";

		writeIndent(oss, 2);
		writeJsonString(oss, "shader");
		oss << ": {\n";

		writeIndent(oss, 4);
		writeJsonString(oss, "sources");
		oss << ": {\n";

		writeIndent(oss, 6);
		writeJsonString(oss, "vertex");
		oss << ": ";
		if (context.hasVertexStage)
		{
			writeJsonString(oss, context.vertexSource);
		}
		else
		{
			writeJsonString(oss, "");
		}
		oss << ",\n";

		writeIndent(oss, 6);
		writeJsonString(oss, "fragment");
		oss << ": ";
		if (context.hasFragmentStage)
		{
			writeJsonString(oss, context.fragmentSource);
		}
		else
		{
			writeJsonString(oss, "");
		}
		oss << "\n";

		writeIndent(oss, 4);
		oss << "}\n";

		writeIndent(oss, 2);
		oss << "},\n";

		writeIndent(oss, 2);
		writeJsonString(oss, "layouts");
		oss << ": ";
		writeJsonArray(
		    oss,
		    2,
		    context.layouts,
		    [&](const LayoutEntry &entry, int entryIndent)
		    {
			    writeIndent(oss, entryIndent);
			    oss << "{\n";

			    writeIndent(oss, entryIndent + 2);
			    writeJsonString(oss, "location");
			    oss << ": " << entry.location << ",\n";

			    writeIndent(oss, entryIndent + 2);
			    writeJsonString(oss, "type");
			    oss << ": ";
			    writeJsonString(oss, entry.type);
			    oss << ",\n";

			    writeIndent(oss, entryIndent + 2);
			    writeJsonString(oss, "name");
			    oss << ": ";
			    writeJsonString(oss, entry.name);
			    oss << "\n";

			    writeIndent(oss, entryIndent);
			    oss << "}";
		    });
		oss << ",\n";

		writeIndent(oss, 2);
		writeJsonString(oss, "framebuffers");
		oss << ": ";
		writeJsonArray(
		    oss,
		    2,
		    context.framebuffers,
		    [&](const FramebufferEntry &entry, int entryIndent)
		    {
			    writeIndent(oss, entryIndent);
			    oss << "{\n";

			    writeIndent(oss, entryIndent + 2);
			    writeJsonString(oss, "location");
			    oss << ": " << entry.location << ",\n";

			    writeIndent(oss, entryIndent + 2);
			    writeJsonString(oss, "type");
			    oss << ": ";
			    writeJsonString(oss, entry.type);
			    oss << ",\n";

			    writeIndent(oss, entryIndent + 2);
			    writeJsonString(oss, "name");
			    oss << ": ";
			    writeJsonString(oss, entry.name);
			    oss << "\n";

			    writeIndent(oss, entryIndent);
			    oss << "}";
		    });
		oss << ",\n";

		writeIndent(oss, 2);
		writeJsonString(oss, "textures");
		oss << ": ";
		writeJsonArray(
		    oss,
		    2,
		    context.textures,
		    [&](const TextureEntry &entry, int entryIndent)
		    {
			    writeIndent(oss, entryIndent);
			    oss << "{\n";

			    writeIndent(oss, entryIndent + 2);
			    writeJsonString(oss, "location");
			    oss << ": " << entry.location << ",\n";

			    writeIndent(oss, entryIndent + 2);
			    writeJsonString(oss, "luminaName");
			    oss << ": ";
			    writeJsonString(oss, entry.luminaName);
			    oss << ",\n";

			    writeIndent(oss, entryIndent + 2);
			    writeJsonString(oss, "type");
			    oss << ": ";
			    writeJsonString(oss, entry.type);
			    oss << ",\n";

			    writeIndent(oss, entryIndent + 2);
			    writeJsonString(oss, "scope");
			    oss << ": ";
			    const char *scope = (entry.scope == TextureBindingScope::Attribute) ? "attribute" : "constant";
			    writeJsonString(oss, scope);
			    oss << "\n";

			    writeIndent(oss, entryIndent);
			    oss << "}";
		    });
		oss << ",\n";

		writeIndent(oss, 2);
		writeJsonString(oss, "constants");
		oss << ": ";
		writeJsonArray(
		    oss,
		    2,
		    context.constants,
		    [&](const BlockDefinition &block, int blockIndent)
		    {
			    writeIndent(oss, blockIndent);
			    oss << "{\n";

			    writeIndent(oss, blockIndent + 2);
			    writeJsonString(oss, "name");
			    oss << ": ";
			    writeJsonString(oss, block.name);
			    oss << ",\n";

			    writeIndent(oss, blockIndent + 2);
			    writeJsonString(oss, "type");
			    oss << ": ";
			    writeJsonString(oss, block.type);
			    oss << ",\n";

			    writeIndent(oss, blockIndent + 2);
			    writeJsonString(oss, "size");
			    oss << ": " << block.size << ",\n";

			    writeIndent(oss, blockIndent + 2);
			    writeJsonString(oss, "members");
			    oss << ": ";
			    writeBlockMembers(oss, block.members, blockIndent + 2);

			    if (block.dynamicArray.has_value())
			    {
				    oss << ",\n";
				    writeIndent(oss, blockIndent + 2);
				    writeJsonString(oss, "dynamicArrayLayout");
				    oss << ": ";
				    writeDynamicArray(oss, *block.dynamicArray, blockIndent + 2);
				    oss << "\n";
			    }
			    else
			    {
				    oss << "\n";
			    }

			    writeIndent(oss, blockIndent);
			    oss << "}";
		    });
		oss << ",\n";

		writeIndent(oss, 2);
		writeJsonString(oss, "attributes");
		oss << ": ";
		writeJsonArray(
		    oss,
		    2,
		    context.attributes,
		    [&](const BlockDefinition &block, int blockIndent)
		    {
			    writeIndent(oss, blockIndent);
			    oss << "{\n";

			    writeIndent(oss, blockIndent + 2);
			    writeJsonString(oss, "name");
			    oss << ": ";
			    writeJsonString(oss, block.name);
			    oss << ",\n";

			    writeIndent(oss, blockIndent + 2);
			    writeJsonString(oss, "type");
			    oss << ": ";
			    writeJsonString(oss, block.type);
			    oss << ",\n";

			    writeIndent(oss, blockIndent + 2);
			    writeJsonString(oss, "size");
			    oss << ": " << block.size << ",\n";

			    writeIndent(oss, blockIndent + 2);
			    writeJsonString(oss, "members");
			    oss << ": ";
			    writeBlockMembers(oss, block.members, blockIndent + 2);

			    if (block.dynamicArray.has_value())
			    {
				    oss << ",\n";
				    writeIndent(oss, blockIndent + 2);
				    writeJsonString(oss, "dynamicArrayLayout");
				    oss << ": ";
				    writeDynamicArray(oss, *block.dynamicArray, blockIndent + 2);
				    oss << "\n";
			    }
			    else
			    {
				    oss << "\n";
			    }

			    writeIndent(oss, blockIndent);
			    oss << "}";
		    });
		oss << "\n";

		oss << "}\n";
		return oss.str();
	}
}

Compiler::Compiler(bool enableDebugOutput) : debugEnabled(enableDebugOutput) {}

std::string Compiler::operator()(const SemanticParseResult &result) const
{
	CompilerContext context;
	StageIO triangleIndex;
	triangleIndex.location = 0;
	triangleIndex.type = "uint";
	triangleIndex.name = "triangleIndex";
	triangleIndex.flat = true;
	context.varyings.push_back(std::move(triangleIndex));
	context.nextVaryingLocation = 1;
	context.collectStructs(result.instructions);
	context.namespaceStack.clear();
	context.process(result.instructions);
	// Reassign locations to keep them sequential starting at zero.
	for (std::size_t i = 0; i < context.framebuffers.size(); ++i)
	{
		context.framebuffers[i].location = static_cast<int>(i);
	}
	context.nextFramebufferLocation = static_cast<int>(context.framebuffers.size());

	ConverterInput converterInput{
	    .semantic = result,
	    .vertexInputs = context.layouts,
	    .stageVaryings = context.varyings,
	    .fragmentOutputs = context.framebuffers,
	    .textures = context.textures,
	};

	Converter converter;
	ShaderSources sources = converter(converterInput);

	if (debugEnabled)
	{
		if (!sources.vertex.empty())
		{
			std::cout << "\n=== Vertex Shader ===\n" << sources.vertex << "\n";
		}
		else
		{
			std::cout << "\n=== Vertex Shader ===\n<none>\n";
		}

		if (!sources.fragment.empty())
		{
			std::cout << "\n=== Fragment Shader ===\n" << sources.fragment << "\n";
		}
		else
		{
			std::cout << "\n=== Fragment Shader ===\n<none>\n";
		}
	}

	context.hasVertexStage = !sources.vertex.empty();
	context.vertexSource = std::move(sources.vertex);
	context.hasFragmentStage = !sources.fragment.empty();
	context.fragmentSource = std::move(sources.fragment);

	return emitJson(context);
}
