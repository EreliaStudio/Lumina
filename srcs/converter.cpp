#include "converter.hpp"

#include "ast.hpp"

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

	std::string joinName(const Name &name, const std::string &separator = "::")
	{
		std::string result;
		for (std::size_t i = 0; i < name.parts.size(); ++i)
		{
			if (i > 0)
			{
				result += separator;
			}
			result += safeTokenContent(name.parts[i]);
		}
		return result;
	}

	std::string sanitizeIdentifier(const std::string &name)
	{
		std::string sanitized;
		sanitized.reserve(name.size());
		for (char c : name)
		{
			if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_')
			{
				sanitized.push_back(c);
			}
			else
			{
				sanitized.push_back('_');
			}
		}
		if (sanitized.empty())
		{
			return "_unnamed";
		}
		if ((sanitized[0] >= '0' && sanitized[0] <= '9'))
		{
			sanitized.insert(sanitized.begin(), '_');
		}
		return sanitized;
	}

	std::string mapNumericSuffix(const std::string &base, const std::string &suffix)
	{
		return base + suffix;
	}

std::string convertLuminaType(const std::string &typeName)
{
	static const std::unordered_map<std::string, std::string> mapping = {
	    {"float", "float"},     {"int", "int"},         {"uint", "uint"},       {"bool", "bool"},
		    {"Vector2", "vec2"},    {"Vector3", "vec3"},    {"Vector4", "vec4"},    {"Vector2Int", "ivec2"},
		    {"Vector3Int", "ivec3"},{"Vector4Int", "ivec4"}, {"Vector2UInt", "uvec2"},{"Vector3UInt", "uvec3"},
		    {"Vector4UInt", "uvec4"},{"Color", "vec4"},      {"Matrix2x2", "mat2"},  {"Matrix3x3", "mat3"},
		    {"Matrix4x4", "mat4"}};

	auto it = mapping.find(typeName);
	if (it != mapping.end())
	{
		return it->second;
	}
	return sanitizeIdentifier(typeName);
}

bool isFloatTypeName(const std::string &typeName)
{
	return typeName == "float";
}

bool isFloatVectorTypeName(const std::string &typeName)
{
	return typeName == "Vector2" || typeName == "Vector3" || typeName == "Vector4" || typeName == "Color";
}

bool isColorTypeName(const std::string &typeName)
{
	return typeName == "Color";
}

	std::string binaryOperatorSymbol(BinaryOperator op)
	{
		switch (op)
		{
			case BinaryOperator::Add:
				return "+";
			case BinaryOperator::Subtract:
				return "-";
			case BinaryOperator::Multiply:
				return "*";
			case BinaryOperator::Divide:
				return "/";
			case BinaryOperator::Modulo:
				return "%";
			case BinaryOperator::Less:
				return "<";
			case BinaryOperator::LessEqual:
				return "<=";
			case BinaryOperator::Greater:
				return ">";
			case BinaryOperator::GreaterEqual:
				return ">=";
			case BinaryOperator::Equal:
				return "==";
			case BinaryOperator::NotEqual:
				return "!=";
			case BinaryOperator::LogicalAnd:
				return "&&";
			case BinaryOperator::LogicalOr:
				return "||";
			case BinaryOperator::BitwiseAnd:
				return "&";
			case BinaryOperator::BitwiseOr:
				return "|";
			case BinaryOperator::BitwiseXor:
				return "^";
		}
		return {};
	}

	std::string assignmentOperatorSymbol(AssignmentOperator op)
	{
		switch (op)
		{
			case AssignmentOperator::Assign:
				return "=";
			case AssignmentOperator::AddAssign:
				return "+=";
			case AssignmentOperator::SubtractAssign:
				return "-=";
			case AssignmentOperator::MultiplyAssign:
				return "*=";
			case AssignmentOperator::DivideAssign:
				return "/=";
			case AssignmentOperator::ModuloAssign:
				return "%=";
			case AssignmentOperator::BitwiseAndAssign:
				return "&=";
			case AssignmentOperator::BitwiseOrAssign:
				return "|=";
			case AssignmentOperator::BitwiseXorAssign:
				return "^=";
		}
		return "=";
	}

	void writeIndent(std::ostringstream &oss, int indent)
	{
		for (int i = 0; i < indent; ++i)
		{
			oss << '\t';
		}
	}
}

class ConverterImpl
{
public:
	explicit ConverterImpl(const ConverterInput &input);

	ShaderSources run();

private:
	struct AggregateInfo
	{
		std::string qualifiedName;
		const AggregateInstruction *node = nullptr;
		bool isSSBO = false;
	};

	void collect(const std::vector<std::unique_ptr<Instruction>> &instructions);
	void collectNamespace(const NamespaceInstruction &ns);
	void collectAggregate(const AggregateInstruction &aggregate);
	void collectVariable(const VariableInstruction &variable);
	void collectStage(const StageFunctionInstruction &stageFunction);

	std::string qualify(const Token &name) const;
	std::string qualify(const Name &name) const;
	std::string remapIdentifier(const Name &name) const;
	std::string remapIdentifier(const std::string &canonical) const;

	void emitCommon(std::ostringstream &oss) const;
	void emitStructs(std::ostringstream &oss) const;
	void emitBlocks(std::ostringstream &oss, AggregateInstruction::Kind kind) const;
	void emitBlockMembers(std::ostringstream &oss, const AggregateInstruction &aggregate, int indent) const;
	void emitGlobalVariables(std::ostringstream &oss) const;
	void emitTextures(std::ostringstream &oss) const;

	void emitInterface(std::ostringstream &oss, const std::vector<StageIO> &entries, const char *qualifier) const;
	void emitStage(std::ostringstream &oss, const StageFunctionInstruction *stage, Stage stageKind) const;
	void emitBlockStatement(std::ostringstream &oss, const BlockStatement &block, int indent) const;
	void emitStatement(std::ostringstream &oss, const Statement &statement, int indent) const;
	void emitVariableStatement(std::ostringstream &oss, const VariableStatement &statement, int indent) const;
	void emitIfStatement(std::ostringstream &oss, const IfStatement &statement, int indent) const;
	void emitWhileStatement(std::ostringstream &oss, const WhileStatement &statement, int indent) const;
	void emitDoWhileStatement(std::ostringstream &oss, const DoWhileStatement &statement, int indent) const;
	void emitForStatement(std::ostringstream &oss, const ForStatement &statement, int indent) const;
	void emitReturnStatement(std::ostringstream &oss, const ReturnStatement &statement, int indent) const;

	std::string emitExpression(const Expression &expression) const;
	std::string emitLiteral(const LiteralExpression &literal) const;
	std::string emitIdentifier(const IdentifierExpression &identifier) const;
	std::string emitUnary(const UnaryExpression &unary) const;
	std::string emitBinary(const BinaryExpression &binary) const;
	std::string emitAssignment(const AssignmentExpression &assignment) const;
	std::string emitConditional(const ConditionalExpression &conditional) const;
	std::string emitCall(const CallExpression &call) const;
	std::optional<std::string> emitBuiltinMemberCall(const MemberExpression &member, const CallExpression &call) const;
	std::optional<std::string> emitFloatBuiltinCall(
	    const std::string &method, const std::string &objectExpr, const std::vector<std::string> &arguments) const;
	std::optional<std::string> emitVectorBuiltinCall(const std::string &typeName,
	    const std::string &method, const std::string &objectExpr, const std::vector<std::string> &arguments) const;
	std::string emitMember(const MemberExpression &member) const;
	std::string emitIndex(const IndexExpression &index) const;
	std::string emitPostfix(const PostfixExpression &postfix) const;

	std::string typeToGLSL(const TypeName &type) const;
	std::string typeToGLSL(const std::string &typeName) const;
	bool aggregateHasUnsizedArray(const AggregateInstruction &aggregate) const;

	const ConverterInput &input;
	const SemanticParseResult &semantic;
	const std::unordered_map<const Expression *, SemanticParseResult::ExpressionInfo> &expressionInfo;

	std::vector<std::string> namespaceStack;
	std::vector<AggregateInfo> structures;
	std::vector<AggregateInfo> attributeBlocks;
	std::vector<AggregateInfo> constantBlocks;
	std::vector<const VariableInstruction *> globalVariables;
	const StageFunctionInstruction *vertexStage = nullptr;
	const StageFunctionInstruction *fragmentStage = nullptr;

	std::unordered_map<std::string, std::string> remappedNames;
	std::unordered_map<std::string, TextureBinding> textureLookup;
};

ConverterImpl::ConverterImpl(const ConverterInput &input)
    : input(input),
      semantic(input.semantic),
      expressionInfo(input.semantic.expressionInfo)
{
	for (const TextureBinding &binding : input.textures)
	{
		textureLookup[binding.luminaName] = binding;
		remappedNames[binding.luminaName] = binding.glslName;
	}
	collect(semantic.instructions);
}

void ConverterImpl::collect(const std::vector<std::unique_ptr<Instruction>> &instructions)
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
				collectAggregate(static_cast<const AggregateInstruction &>(*instruction));
				break;
			case Instruction::Type::Variable:
				collectVariable(static_cast<const VariableInstruction &>(*instruction));
				break;
			case Instruction::Type::Namespace:
				collectNamespace(static_cast<const NamespaceInstruction &>(*instruction));
				break;
			case Instruction::Type::StageFunction:
				collectStage(static_cast<const StageFunctionInstruction &>(*instruction));
				break;
			default:
				break;
		}
	}
}

void ConverterImpl::collectNamespace(const NamespaceInstruction &ns)
{
	namespaceStack.push_back(sanitizeIdentifier(safeTokenContent(ns.name)));
	collect(ns.instructions);
	namespaceStack.pop_back();
}

void ConverterImpl::collectAggregate(const AggregateInstruction &aggregate)
{
	AggregateInfo info;
	info.qualifiedName = qualify(aggregate.name);
	info.node = &aggregate;
	info.isSSBO = aggregateHasUnsizedArray(aggregate);
	const std::string sanitized = sanitizeIdentifier(info.qualifiedName);
	remappedNames[info.qualifiedName] = sanitized;
	if (namespaceStack.empty())
	{
		remappedNames[safeTokenContent(aggregate.name)] = sanitized;
	}
	switch (aggregate.kind)
	{
		case AggregateInstruction::Kind::Struct:
			structures.push_back(info);
			break;
		case AggregateInstruction::Kind::AttributeBlock:
			attributeBlocks.push_back(info);
			break;
		case AggregateInstruction::Kind::ConstantBlock:
			constantBlocks.push_back(info);
			break;
	}
}

void ConverterImpl::collectVariable(const VariableInstruction &variable)
{
	globalVariables.push_back(&variable);
	for (const VariableDeclarator &declarator : variable.declaration.declarators)
	{
		const std::string canonical = qualify(declarator.name);
		const std::string sanitized = sanitizeIdentifier(canonical);
		if (namespaceStack.empty())
		{
			remappedNames[safeTokenContent(declarator.name)] = sanitized;
		}
		remappedNames[canonical] = sanitized;
	}
}

void ConverterImpl::collectStage(const StageFunctionInstruction &stageFunction)
{
	switch (stageFunction.stage)
	{
		case Stage::VertexPass:
			vertexStage = &stageFunction;
			break;
		case Stage::FragmentPass:
			fragmentStage = &stageFunction;
			break;
		default:
			break;
	}
}

std::string ConverterImpl::qualify(const Token &name) const
{
	if (namespaceStack.empty())
	{
		return safeTokenContent(name);
	}
	std::ostringstream oss;
	for (std::size_t i = 0; i < namespaceStack.size(); ++i)
	{
		if (i > 0)
		{
			oss << "::";
		}
		oss << namespaceStack[i];
	}
	if (!namespaceStack.empty())
	{
		oss << "::";
	}
	oss << safeTokenContent(name);
	return oss.str();
}

std::string ConverterImpl::qualify(const Name &name) const
{
	std::string joined = joinName(name);
	if (joined.empty())
	{
		return joined;
	}
	if (joined.find("::") != std::string::npos || namespaceStack.empty())
	{
		return joined;
	}
	std::ostringstream oss;
	for (std::size_t i = 0; i < namespaceStack.size(); ++i)
	{
		if (i > 0)
		{
			oss << "::";
		}
		oss << namespaceStack[i];
	}
	if (!namespaceStack.empty())
	{
		oss << "::";
	}
	oss << joined;
	return oss.str();
}

std::string ConverterImpl::remapIdentifier(const Name &name) const
{
	const std::string canonical = joinName(name);
	if (canonical == "pixelPosition")
	{
		return "gl_Position";
	}
	if (auto it = remappedNames.find(canonical); it != remappedNames.end())
	{
		return it->second;
	}
	if (name.parts.size() == 1)
	{
		const std::string simple = safeTokenContent(name.parts.front());
		if (auto simpleIt = remappedNames.find(simple); simpleIt != remappedNames.end())
		{
			return simpleIt->second;
		}
		return simple;
	}

	std::string flattened;
	for (std::size_t i = 0; i < name.parts.size(); ++i)
	{
		if (i > 0)
		{
			flattened += "_";
		}
		flattened += sanitizeIdentifier(safeTokenContent(name.parts[i]));
	}
	return flattened;
}

std::string ConverterImpl::remapIdentifier(const std::string &canonical) const
{
	if (canonical == "pixelPosition")
	{
		return "gl_Position";
	}
	if (auto it = remappedNames.find(canonical); it != remappedNames.end())
	{
		return it->second;
	}
	return canonical;
}

void ConverterImpl::emitCommon(std::ostringstream &oss) const
{
	emitStructs(oss);
	emitBlocks(oss, AggregateInstruction::Kind::ConstantBlock);
	emitBlocks(oss, AggregateInstruction::Kind::AttributeBlock);
	emitGlobalVariables(oss);
	emitTextures(oss);
}

void ConverterImpl::emitStructs(std::ostringstream &oss) const
{
	for (const AggregateInfo &info : structures)
	{
		const auto *aggregate = info.node;
		if (!aggregate)
		{
			continue;
		}
		oss << "struct " << sanitizeIdentifier(info.qualifiedName) << "\n{\n";
		emitBlockMembers(oss, *aggregate, 1);
		oss << "};\n\n";
	}
}

void ConverterImpl::emitBlocks(std::ostringstream &oss, AggregateInstruction::Kind kind) const
{
	const std::vector<AggregateInfo> &blocks = (kind == AggregateInstruction::Kind::ConstantBlock) ? constantBlocks :
	                                                                                                 attributeBlocks;
	const char *bindingKeyword = (kind == AggregateInstruction::Kind::AttributeBlock) ? "ATTRIBUTE_BINDING" :
	                                                                       "CONSTANT_BINDING";

	for (const AggregateInfo &info : blocks)
	{
		const AggregateInstruction *aggregate = info.node;
		if (!aggregate)
		{
			continue;
		}
		const bool ssbo = info.isSSBO;
		const std::string blockName = sanitizeIdentifier(info.qualifiedName);
		const std::string blockTypeName = blockName + "_Type";

		oss << "layout(binding = " << bindingKeyword << ", " << (ssbo ? "std430" : "std140") << ") "
		    << (ssbo ? "buffer" : "uniform") << " " << blockTypeName << "\n{\n";
		emitBlockMembers(oss, *aggregate, 1);
		oss << "} " << blockName << ";\n\n";
	}
}

void ConverterImpl::emitBlockMembers(std::ostringstream &oss, const AggregateInstruction &aggregate, int indent) const
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
			writeIndent(oss, indent);
			oss << typeToGLSL(field.declaration.type) << " " << sanitizeIdentifier(safeTokenContent(declarator.name));
			if (declarator.hasArraySuffix && declarator.arraySize)
			{
				oss << "["
				    << emitExpression(*declarator.arraySize)
				    << "]";
			}
			else if (declarator.hasArraySuffix)
			{
				oss << "[]";
			}
			oss << ";\n";
		}
	}
}

void ConverterImpl::emitGlobalVariables(std::ostringstream &oss) const
{
	for (const VariableInstruction *variable : globalVariables)
	{
		if (!variable)
		{
			continue;
		}
		const std::string type = joinName(variable->declaration.type.name);
		if (type == "Texture")
		{
			continue;
		}
		for (const VariableDeclarator &declarator : variable->declaration.declarators)
		{
			const std::string name = remapIdentifier(qualify(declarator.name));
			oss << (variable->declaration.type.isConst ? "const " : "");
			oss << typeToGLSL(variable->declaration.type) << " " << name;
			if (declarator.initializer)
			{
				oss << " = " << emitExpression(*declarator.initializer);
			}
			oss << ";\n";
		}
	}
	if (!globalVariables.empty())
	{
		oss << "\n";
	}
}

void ConverterImpl::emitTextures(std::ostringstream &oss) const
{
	for (const auto &[_, binding] : textureLookup)
	{
		oss << "uniform " << binding.type << " " << binding.glslName << ";\n";
	}
	if (!textureLookup.empty())
	{
		oss << "\n";
	}
}

void ConverterImpl::emitInterface(std::ostringstream &oss, const std::vector<StageIO> &entries, const char *qualifier) const
{
	for (const StageIO &entry : entries)
	{
		oss << "layout(location = " << entry.location << ") " << qualifier << " " << typeToGLSL(entry.type) << " "
		    << entry.name << ";\n";
	}
	if (!entries.empty())
	{
		oss << "\n";
	}
}

void ConverterImpl::emitStage(std::ostringstream &oss, const StageFunctionInstruction *stage, Stage stageKind) const
{
	if (!stage || !stage->body)
	{
		oss << "void main()\n{\n}\n";
		return;
	}

	oss << "void main()\n{\n";
	emitBlockStatement(oss, *stage->body, 1);
	oss << "}\n";
}

void ConverterImpl::emitBlockStatement(std::ostringstream &oss, const BlockStatement &block, int indent) const
{
	for (const std::unique_ptr<Statement> &statement : block.statements)
	{
		if (statement)
		{
			emitStatement(oss, *statement, indent);
		}
	}
}

void ConverterImpl::emitStatement(std::ostringstream &oss, const Statement &statement, int indent) const
{
	switch (statement.kind)
	{
		case Statement::Kind::Block:
		{
			writeIndent(oss, indent);
			oss << "{\n";
			emitBlockStatement(oss, static_cast<const BlockStatement &>(statement), indent + 1);
			writeIndent(oss, indent);
			oss << "}\n";
			break;
		}
		case Statement::Kind::Expression:
		{
			const auto &expr = static_cast<const ExpressionStatement &>(statement);
			if (expr.expression)
			{
				writeIndent(oss, indent);
				oss << emitExpression(*expr.expression) << ";\n";
			}
			break;
		}
		case Statement::Kind::Variable:
			emitVariableStatement(oss, static_cast<const VariableStatement &>(statement), indent);
			break;
		case Statement::Kind::If:
			emitIfStatement(oss, static_cast<const IfStatement &>(statement), indent);
			break;
		case Statement::Kind::While:
			emitWhileStatement(oss, static_cast<const WhileStatement &>(statement), indent);
			break;
		case Statement::Kind::DoWhile:
			emitDoWhileStatement(oss, static_cast<const DoWhileStatement &>(statement), indent);
			break;
		case Statement::Kind::For:
			emitForStatement(oss, static_cast<const ForStatement &>(statement), indent);
			break;
		case Statement::Kind::Return:
			emitReturnStatement(oss, static_cast<const ReturnStatement &>(statement), indent);
			break;
		case Statement::Kind::Break:
			writeIndent(oss, indent);
			oss << "break;\n";
			break;
		case Statement::Kind::Continue:
			writeIndent(oss, indent);
			oss << "continue;\n";
			break;
		case Statement::Kind::Discard:
			writeIndent(oss, indent);
			oss << "discard;\n";
			break;
		default:
			break;
	}
}

void ConverterImpl::emitVariableStatement(std::ostringstream &oss, const VariableStatement &statement, int indent) const
{
	const std::string type = typeToGLSL(statement.declaration.type);
	for (const VariableDeclarator &declarator : statement.declaration.declarators)
	{
		writeIndent(oss, indent);
		oss << type << " " << safeTokenContent(declarator.name);
		if (declarator.hasArraySuffix && declarator.arraySize)
		{
			oss << "[" << emitExpression(*declarator.arraySize) << "]";
		}
		else if (declarator.hasArraySuffix)
		{
			oss << "[]";
		}
		if (declarator.initializer)
		{
			oss << " = " << emitExpression(*declarator.initializer);
		}
		oss << ";\n";
	}
}

void ConverterImpl::emitIfStatement(std::ostringstream &oss, const IfStatement &statement, int indent) const
{
	writeIndent(oss, indent);
	oss << "if (" << emitExpression(*statement.condition) << ")\n";
	emitStatement(oss, *statement.thenBranch, indent + 1);
	if (statement.elseBranch)
	{
		writeIndent(oss, indent);
		oss << "else\n";
		emitStatement(oss, *statement.elseBranch, indent + 1);
	}
}

void ConverterImpl::emitWhileStatement(std::ostringstream &oss, const WhileStatement &statement, int indent) const
{
	writeIndent(oss, indent);
	oss << "while (" << emitExpression(*statement.condition) << ")\n";
	emitStatement(oss, *statement.body, indent + 1);
}

void ConverterImpl::emitDoWhileStatement(std::ostringstream &oss, const DoWhileStatement &statement, int indent) const
{
	writeIndent(oss, indent);
	oss << "do\n";
	emitStatement(oss, *statement.body, indent + 1);
	writeIndent(oss, indent);
	oss << "while (" << emitExpression(*statement.condition) << ");\n";
}

void ConverterImpl::emitForStatement(std::ostringstream &oss, const ForStatement &statement, int indent) const
{
	writeIndent(oss, indent);
	oss << "for (";
	if (statement.initializer)
	{
		if (statement.initializer->kind == Statement::Kind::Variable)
		{
			const auto &var = static_cast<const VariableStatement &>(*statement.initializer);
			if (!var.declaration.declarators.empty())
			{
				const VariableDeclarator &decl = var.declaration.declarators.front();
				oss << typeToGLSL(var.declaration.type) << " " << safeTokenContent(decl.name);
				if (decl.initializer)
				{
					oss << " = " << emitExpression(*decl.initializer);
				}
			}
		}
		else if (statement.initializer->kind == Statement::Kind::Expression)
		{
			const auto &expr = static_cast<const ExpressionStatement &>(*statement.initializer);
			if (expr.expression)
			{
				oss << emitExpression(*expr.expression);
			}
		}
	}
	oss << "; ";
	if (statement.condition)
	{
		oss << emitExpression(*statement.condition);
	}
	oss << "; ";
	if (statement.increment)
	{
		oss << emitExpression(*statement.increment);
	}
	oss << ")\n";
	emitStatement(oss, *statement.body, indent + 1);
}

void ConverterImpl::emitReturnStatement(std::ostringstream &oss, const ReturnStatement &statement, int indent) const
{
	writeIndent(oss, indent);
	oss << "return";
	if (statement.value)
	{
		oss << " " << emitExpression(*statement.value);
	}
	oss << ";\n";
}

std::string ConverterImpl::emitExpression(const Expression &expression) const
{
	switch (expression.kind)
	{
		case Expression::Kind::Literal:
			return emitLiteral(static_cast<const LiteralExpression &>(expression));
		case Expression::Kind::Identifier:
			return emitIdentifier(static_cast<const IdentifierExpression &>(expression));
		case Expression::Kind::Unary:
			return emitUnary(static_cast<const UnaryExpression &>(expression));
		case Expression::Kind::Binary:
			return emitBinary(static_cast<const BinaryExpression &>(expression));
		case Expression::Kind::Assignment:
			return emitAssignment(static_cast<const AssignmentExpression &>(expression));
		case Expression::Kind::Conditional:
			return emitConditional(static_cast<const ConditionalExpression &>(expression));
		case Expression::Kind::Call:
			return emitCall(static_cast<const CallExpression &>(expression));
		case Expression::Kind::MemberAccess:
			return emitMember(static_cast<const MemberExpression &>(expression));
		case Expression::Kind::IndexAccess:
			return emitIndex(static_cast<const IndexExpression &>(expression));
		case Expression::Kind::Postfix:
			return emitPostfix(static_cast<const PostfixExpression &>(expression));
	}
	return {};
}

std::string ConverterImpl::emitLiteral(const LiteralExpression &literal) const
{
	return literal.literal.content;
}

std::string ConverterImpl::emitIdentifier(const IdentifierExpression &identifier) const
{
	return remapIdentifier(identifier.name);
}

std::string ConverterImpl::emitUnary(const UnaryExpression &unary) const
{
	std::string op;
	switch (unary.op)
	{
		case UnaryOperator::Positive:
			op = "+";
			break;
		case UnaryOperator::Negate:
			op = "-";
			break;
		case UnaryOperator::LogicalNot:
			op = "!";
			break;
		case UnaryOperator::BitwiseNot:
			op = "~";
			break;
		case UnaryOperator::PreIncrement:
			op = "++";
			break;
		case UnaryOperator::PreDecrement:
			op = "--";
			break;
	}
	return op + emitExpression(*unary.operand);
}

std::string ConverterImpl::emitBinary(const BinaryExpression &binary) const
{
	return "(" + emitExpression(*binary.left) + " " + binaryOperatorSymbol(binary.op) + " " + emitExpression(*binary.right) +
	       ")";
}

std::string ConverterImpl::emitAssignment(const AssignmentExpression &assignment) const
{
	return emitExpression(*assignment.target) + " " + assignmentOperatorSymbol(assignment.op) + " " +
	       emitExpression(*assignment.value);
}

std::string ConverterImpl::emitConditional(const ConditionalExpression &conditional) const
{
	return "(" + emitExpression(*conditional.condition) + " ? " + emitExpression(*conditional.thenBranch) + " : " +
	       emitExpression(*conditional.elseBranch) + ")";
}

std::string ConverterImpl::emitCall(const CallExpression &call) const
{
	if (const auto *member = dynamic_cast<const MemberExpression *>(call.callee.get()))
	{
		const std::string method = safeTokenContent(member->member);
		auto infoIt = expressionInfo.find(member->object.get());
		const std::string objectType = (infoIt != expressionInfo.end()) ? infoIt->second.typeName : std::string{};
		if (objectType == "Texture" && method == "getPixel" && !call.arguments.empty())
		{
			return "texture(" + emitExpression(*member->object) + ", " + emitExpression(*call.arguments.front()) + ")";
		}

		if (std::optional<std::string> builtin = emitBuiltinMemberCall(*member, call))
		{
			return *builtin;
		}
	}

	if (const auto *identifier = dynamic_cast<const IdentifierExpression *>(call.callee.get()))
	{
		const std::string name = joinName(identifier->name);
		std::string callee = convertLuminaType(name);
		if (callee == name)
		{
			callee = remapIdentifier(identifier->name);
		}
		std::ostringstream oss;
		oss << callee << "(";
		for (std::size_t i = 0; i < call.arguments.size(); ++i)
		{
			if (i > 0)
			{
				oss << ", ";
			}
			oss << emitExpression(*call.arguments[i]);
		}
		oss << ")";
		return oss.str();
	}

	if (const auto *member = dynamic_cast<const MemberExpression *>(call.callee.get()))
	{
		std::ostringstream oss;
		oss << emitExpression(*member->object) << "." << safeTokenContent(member->member) << "(";
		for (std::size_t i = 0; i < call.arguments.size(); ++i)
		{
			if (i > 0)
			{
				oss << ", ";
			}
			oss << emitExpression(*call.arguments[i]);
		}
		oss << ")";
		return oss.str();
	}

	return {};
}

std::optional<std::string> ConverterImpl::emitBuiltinMemberCall(
    const MemberExpression &member, const CallExpression &call) const
{
	const std::string method = safeTokenContent(member.member);
	auto infoIt = expressionInfo.find(member.object.get());
	if (infoIt == expressionInfo.end())
	{
		return std::nullopt;
	}

	const std::string objectType = infoIt->second.typeName;
	const std::string objectExpr = emitExpression(*member.object);
	std::vector<std::string> arguments;
	arguments.reserve(call.arguments.size());
	for (const std::unique_ptr<Expression> &argument : call.arguments)
	{
		if (argument)
		{
			arguments.push_back(emitExpression(*argument));
		}
		else
		{
			arguments.emplace_back();
		}
	}

	if (isFloatTypeName(objectType))
	{
		return emitFloatBuiltinCall(method, objectExpr, arguments);
	}

	if (isFloatVectorTypeName(objectType))
	{
		return emitVectorBuiltinCall(objectType, method, objectExpr, arguments);
	}

	return std::nullopt;
}

std::optional<std::string> ConverterImpl::emitFloatBuiltinCall(
    const std::string &method, const std::string &objectExpr, const std::vector<std::string> &arguments) const
{
	const auto hasArgs = [&](std::size_t expected) { return arguments.size() == expected; };

	auto wrap = [&](const std::string &name) { return name + "(" + objectExpr + ")"; };

	if (method == "abs" || method == "sign" || method == "floor" || method == "ceil" || method == "fract" ||
	    method == "exp" || method == "log" || method == "exp2" || method == "log2" || method == "sqrt" ||
	    method == "inversesqrt" || method == "sin" || method == "cos" || method == "tan" || method == "asin" ||
	    method == "acos" || method == "atan")
	{
		if (hasArgs(0))
		{
			return wrap(method);
		}
		return std::nullopt;
	}

	if ((method == "mod" || method == "min" || method == "max" || method == "pow") && hasArgs(1))
	{
		return method + "(" + objectExpr + ", " + arguments[0] + ")";
	}

	if (method == "clamp" && hasArgs(2))
	{
		return "clamp(" + objectExpr + ", " + arguments[0] + ", " + arguments[1] + ")";
	}

	if (method == "mix" && hasArgs(2))
	{
		return "mix(" + objectExpr + ", " + arguments[0] + ", " + arguments[1] + ")";
	}

	if (method == "step" && hasArgs(1))
	{
		return "step(" + arguments[0] + ", " + objectExpr + ")";
	}

	if (method == "smoothstep" && hasArgs(2))
	{
		return "smoothstep(" + arguments[0] + ", " + arguments[1] + ", " + objectExpr + ")";
	}

	return std::nullopt;
}

std::optional<std::string> ConverterImpl::emitVectorBuiltinCall(
    const std::string &typeName,
    const std::string &method,
    const std::string &objectExpr,
    const std::vector<std::string> &arguments) const
{
	const auto hasArgs = [&](std::size_t expected) { return arguments.size() == expected; };

	if (method == "dot" && hasArgs(1))
	{
		return "dot(" + objectExpr + ", " + arguments[0] + ")";
	}
	if (method == "length" && hasArgs(0))
	{
		return "length(" + objectExpr + ")";
	}
	if (method == "distance" && hasArgs(1))
	{
		return "distance(" + objectExpr + ", " + arguments[0] + ")";
	}
	if (method == "normalize" && hasArgs(0))
	{
		return "normalize(" + objectExpr + ")";
	}
	if (method == "cross" && typeName == "Vector3" && hasArgs(1))
	{
		return "cross(" + objectExpr + ", " + arguments[0] + ")";
	}
	if (method == "reflect" && hasArgs(1))
	{
		return "reflect(" + objectExpr + ", " + arguments[0] + ")";
	}
	if ((method == "abs" || method == "floor" || method == "ceil" || method == "fract" || method == "exp" ||
	        method == "log" || method == "exp2" || method == "log2" || method == "sqrt" || method == "inversesqrt" ||
	        method == "sin" || method == "cos" || method == "tan" || method == "asin" || method == "acos" ||
	        method == "atan") &&
	    hasArgs(0))
	{
		return method + "(" + objectExpr + ")";
	}
	if ((method == "mod" || method == "min" || method == "max" || method == "pow") && hasArgs(1))
	{
		return method + "(" + objectExpr + ", " + arguments[0] + ")";
	}
	if (method == "clamp" && hasArgs(2))
	{
		return "clamp(" + objectExpr + ", " + arguments[0] + ", " + arguments[1] + ")";
	}
	if (method == "lerp" && hasArgs(2))
	{
		return "mix(" + objectExpr + ", " + arguments[0] + ", " + arguments[1] + ")";
	}
	if (method == "step" && hasArgs(1))
	{
		return "step(" + arguments[0] + ", " + objectExpr + ")";
	}
	if (method == "smoothstep" && hasArgs(2))
	{
		return "smoothstep(" + arguments[0] + ", " + arguments[1] + ", " + objectExpr + ")";
	}
	if (method == "saturate" && isColorTypeName(typeName) && hasArgs(0))
	{
		return "clamp(" + objectExpr + ", 0.0, 1.0)";
	}

	return std::nullopt;
}

std::string ConverterImpl::emitMember(const MemberExpression &member) const
{
	return emitExpression(*member.object) + "." + safeTokenContent(member.member);
}

std::string ConverterImpl::emitIndex(const IndexExpression &index) const
{
	return emitExpression(*index.object) + "[" + emitExpression(*index.index) + "]";
}

std::string ConverterImpl::emitPostfix(const PostfixExpression &postfix) const
{
	const std::string op = (postfix.op == PostfixOperator::Increment) ? "++" : "--";
	return emitExpression(*postfix.operand) + op;
}

std::string ConverterImpl::typeToGLSL(const TypeName &type) const
{
	std::string glsl = convertLuminaType(joinName(type.name));
	if (type.isConst)
	{
		glsl = "const " + glsl;
	}
	return glsl;
}

std::string ConverterImpl::typeToGLSL(const std::string &typeName) const
{
	return convertLuminaType(typeName);
}

bool ConverterImpl::aggregateHasUnsizedArray(const AggregateInstruction &aggregate) const
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

ShaderSources ConverterImpl::run()
{
	ShaderSources sources;

	{
		std::ostringstream vertex;
		vertex << "#version 450 core\n\n";
		emitInterface(vertex, input.vertexInputs, "in");
		emitInterface(vertex, input.stageVaryings, "out");
		emitCommon(vertex);
		emitStage(vertex, vertexStage, Stage::VertexPass);
		sources.vertex = vertex.str();
	}

	{
		std::ostringstream fragment;
		fragment << "#version 450 core\n\n";
		emitInterface(fragment, input.stageVaryings, "in");
		emitInterface(fragment, input.fragmentOutputs, "out");
		emitCommon(fragment);
		emitStage(fragment, fragmentStage, Stage::FragmentPass);
		sources.fragment = fragment.str();
	}

	return sources;
}

ShaderSources Converter::operator()(const ConverterInput &input) const
{
	ConverterImpl impl(input);
	return impl.run();
}
