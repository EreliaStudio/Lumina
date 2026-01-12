#include "converter.hpp"

#include "ast.hpp"

#include <algorithm>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
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

	constexpr const char kMethodSelfName[] = "_self";

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
	return typeName;
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
			case BinaryOperator::ShiftLeft:
				return "<<";
			case BinaryOperator::ShiftRight:
				return ">>";
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
			case AssignmentOperator::ShiftLeftAssign:
				return "<<=";
			case AssignmentOperator::ShiftRightAssign:
				return ">>=";
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
	struct MethodHelper
	{
		std::string helperName;
		const MethodMember *node = nullptr;
		bool isConst = false;
	};

	struct AggregateInfo
	{
		std::string qualifiedName;
		const AggregateInstruction *node = nullptr;
		AggregateInstruction::Kind kind = AggregateInstruction::Kind::Struct;
		bool isSSBO = false;
		std::string glslTypeName;
		std::string glslInstanceName;
		std::vector<std::string> namespacePath;
		std::unordered_set<std::string> fieldNames;
		std::vector<MethodHelper> methods;
	};

	struct StageUsage
	{
		std::unordered_set<const FunctionInstruction *> functions;
		std::unordered_set<const VariableInstruction *> globals;
		std::unordered_set<std::string> blocks;
		std::unordered_set<std::string> textures;
		std::unordered_set<std::string> methodHelpers;
	};

	void collect(const std::vector<std::unique_ptr<Instruction>> &instructions);
	void collectNamespace(const NamespaceInstruction &ns);
	void collectAggregate(const AggregateInstruction &aggregate);
	void collectVariable(const VariableInstruction &variable);
	void collectFunction(const FunctionInstruction &function);
	void collectStage(const StageFunctionInstruction &stageFunction);

	std::string qualify(const Token &name) const;
	std::string qualify(const Name &name) const;
	std::string remapIdentifier(const Name &name) const;
	std::string remapIdentifier(const std::string &canonical) const;

	StageUsage collectStageUsage(const StageFunctionInstruction *stage) const;
	const MethodHelper *findMethodHelper(const std::string &helperName, const AggregateInfo **aggregate) const;

	void emitCommon(std::ostringstream &oss, const StageUsage &usage) const;
	void emitStructs(std::ostringstream &oss) const;
	void emitBlocks(std::ostringstream &oss, AggregateInstruction::Kind kind, const StageUsage &usage) const;
	void emitBlockMembers(
	    std::ostringstream &oss,
	    const AggregateInstruction &aggregate,
	    int indent,
	    const AggregateInfo *info) const;
	void emitStructMethods(std::ostringstream &oss, const StageUsage &usage) const;
	void emitBlockMethods(std::ostringstream &oss, const std::vector<AggregateInfo> &aggregates, const StageUsage &usage) const;
	void emitGlobalVariables(std::ostringstream &oss, const StageUsage &usage) const;
	void emitTextures(std::ostringstream &oss, const StageUsage &usage) const;
	void emitFunctions(std::ostringstream &oss, const StageUsage &usage) const;

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
	std::string emitArrayLiteral(const ArrayLiteralExpression &literal) const;
	std::string emitIdentifier(const IdentifierExpression &identifier) const;
	std::string emitUnary(const UnaryExpression &unary) const;
	std::string emitBinary(const BinaryExpression &binary) const;
	std::string emitAssignment(const AssignmentExpression &assignment) const;
	std::string emitConditional(const ConditionalExpression &conditional) const;
	std::string emitCall(const CallExpression &call) const;
	std::optional<std::string> emitUserMethodCall(const MemberExpression &member, const CallExpression &call) const;
	std::optional<std::string> emitImplicitSelfCall(const IdentifierExpression &identifier, const CallExpression &call) const;
	std::optional<std::string> emitBuiltinMemberCall(const MemberExpression &member, const CallExpression &call) const;
	std::optional<std::string> emitFloatBuiltinCall(
	    const std::string &method, const std::string &objectExpr, const std::vector<std::string> &arguments) const;
	std::optional<std::string> emitVectorBuiltinCall(const std::string &typeName,
	    const std::string &method, const std::string &objectExpr, const std::vector<std::string> &arguments) const;
	std::string emitMember(const MemberExpression &member) const;
	std::string emitIndex(const IndexExpression &index) const;
	std::string emitPostfix(const PostfixExpression &postfix) const;
	std::optional<std::string> emitSSBOArraySizeAccess(const MemberExpression &member) const;

	std::string typeToGLSL(const TypeName &type) const;
	std::string typeToGLSL(const std::string &typeName) const;
	bool aggregateHasUnsizedArray(const AggregateInstruction &aggregate) const;
	std::string aggregateTypeName(const AggregateInfo &info) const;
	std::optional<std::string> resolveAggregateQualifiedName(const Name &name) const;
	void emitMethodHelper(std::ostringstream &oss, const AggregateInfo &info, const MethodHelper &helper) const;
	void emitFunction(std::ostringstream &oss, const FunctionInstruction &function, const std::string &name) const;
	void emitParameters(std::ostringstream &oss, const std::vector<Parameter> &params) const;
	std::string parameterName(const Token &token) const;
	bool isMethodLocalName(const std::string &name) const;
	const AggregateInfo *findAggregateInfo(const std::string &qualifiedName) const;
	void pushEmissionNamespace(const std::vector<std::string> &ns) const;
	void popEmissionNamespace() const;
	const std::vector<std::string> &currentEmissionNamespace() const;
	bool methodMutatesAggregate(const MethodMember &method, const AggregateInfo &info) const;
	struct MethodAnalysisContext;
	bool statementMutatesAggregate(const Statement &statement, MethodAnalysisContext &ctx, const AggregateInfo &info) const;
	bool expressionMutatesAggregate(const Expression &expression, MethodAnalysisContext &ctx, const AggregateInfo &info) const;
	bool expressionRefersToField(const Expression &expression, MethodAnalysisContext &ctx, const AggregateInfo &info) const;

	const ConverterInput &input;
	const SemanticParseResult &semantic;
	const std::unordered_map<const Expression *, SemanticParseResult::ExpressionInfo> &expressionInfo;

	std::vector<std::string> namespaceStack;
	std::vector<AggregateInfo> structures;
	std::vector<AggregateInfo> attributeBlocks;
	std::vector<AggregateInfo> constantBlocks;
	std::vector<const FunctionInstruction *> functions;
	std::vector<const VariableInstruction *> globalVariables;
	const StageFunctionInstruction *vertexStage = nullptr;
	const StageFunctionInstruction *fragmentStage = nullptr;

	std::unordered_map<std::string, std::string> remappedNames;
	std::unordered_map<std::string, TextureBinding> textureLookup;
	std::unordered_map<const FunctionInstruction *, std::string> functionNames;
	std::unordered_map<const FunctionInstruction *, std::vector<std::string>> functionNamespaces;
	std::unordered_map<const StageFunctionInstruction *, std::vector<std::string>> stageNamespaces;
	std::unordered_map<std::string, const FunctionInstruction *> functionLookup;
	std::unordered_map<std::string, const VariableInstruction *> globalVariableLookup;
	std::unordered_map<std::string, AggregateInstruction::Kind> aggregateKindLookup;
	struct MethodCallInfo
	{
		std::string helperName;
	};
	std::unordered_map<std::string, std::unordered_map<std::string, MethodCallInfo>> methodCallHelpers;
	mutable std::vector<std::string> thisAliasStack;
	mutable const AggregateInfo *currentMethodAggregate = nullptr;
	mutable std::unordered_set<std::string> currentMethodParameters;
	mutable std::vector<std::unordered_set<std::string>> methodLocalNameStack;
	mutable std::vector<std::vector<std::string>> emissionNamespaceStack;
	mutable std::string currentMethodSelfName;
	mutable bool currentMethodUsesSelfParameter = false;
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
			case Instruction::Type::Function:
				collectFunction(static_cast<const FunctionInstruction &>(*instruction));
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
	info.kind = aggregate.kind;
	info.isSSBO = aggregateHasUnsizedArray(aggregate);
	info.namespacePath = namespaceStack;
	const std::string baseName = sanitizeIdentifier(info.qualifiedName);
	info.glslInstanceName = baseName;
	info.glslTypeName = (aggregate.kind == AggregateInstruction::Kind::Struct) ? baseName : (baseName + "_Type");
	for (const std::unique_ptr<StructMember> &member : aggregate.members)
	{
		if (!member)
		{
			continue;
		}

		if (member->kind == StructMember::Kind::Field)
		{
			const auto &field = static_cast<const FieldMember &>(*member);
			for (const VariableDeclarator &declarator : field.declaration.declarators)
			{
				info.fieldNames.insert(safeTokenContent(declarator.name));
			}
			continue;
		}

		if (member->kind != StructMember::Kind::Method)
		{
			continue;
		}

		const auto &method = static_cast<const MethodMember &>(*member);
		if (!method.body)
		{
			continue;
		}

		MethodHelper helper;
		helper.node = &method;
		const bool mutatesAggregate = methodMutatesAggregate(method, info);
		helper.isConst = method.isConst || !mutatesAggregate;
		const std::string sanitizedMethod = sanitizeIdentifier(safeTokenContent(method.name));
		helper.helperName = info.glslTypeName + "__" + sanitizedMethod;
		info.methods.push_back(std::move(helper));
	}

	const std::string sanitized = sanitizeIdentifier(info.qualifiedName);
	remappedNames[info.qualifiedName] = sanitized;
	if (namespaceStack.empty())
	{
		remappedNames[safeTokenContent(aggregate.name)] = sanitized;
	}
	aggregateKindLookup[info.qualifiedName] = aggregate.kind;
	switch (aggregate.kind)
	{
		case AggregateInstruction::Kind::Struct:
			structures.push_back(std::move(info));
			for (const MethodHelper &method : structures.back().methods)
			{
				MethodCallInfo callInfo{method.helperName};
				methodCallHelpers[structures.back().qualifiedName][safeTokenContent(method.node->name)] = std::move(callInfo);
			}
			break;
		case AggregateInstruction::Kind::AttributeBlock:
			attributeBlocks.push_back(std::move(info));
			for (const MethodHelper &method : attributeBlocks.back().methods)
			{
				MethodCallInfo callInfo{method.helperName};
				methodCallHelpers[attributeBlocks.back().qualifiedName][safeTokenContent(method.node->name)] = std::move(callInfo);
			}
			break;
		case AggregateInstruction::Kind::ConstantBlock:
			constantBlocks.push_back(std::move(info));
			for (const MethodHelper &method : constantBlocks.back().methods)
			{
				MethodCallInfo callInfo{method.helperName};
				methodCallHelpers[constantBlocks.back().qualifiedName][safeTokenContent(method.node->name)] = std::move(callInfo);
			}
			break;
	}
}

void ConverterImpl::collectVariable(const VariableInstruction &variable)
{
	const std::string declaredType = joinName(variable.declaration.type.name);
	if (declaredType == "Texture")
	{
		return;
	}

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
		globalVariableLookup[canonical] = &variable;
	}
}

void ConverterImpl::collectFunction(const FunctionInstruction &function)
{
	const std::string canonical = qualify(function.name);
	const std::string sanitized = sanitizeIdentifier(canonical);
	remappedNames[canonical] = sanitized;
	if (namespaceStack.empty())
	{
		remappedNames[safeTokenContent(function.name)] = sanitized;
	}

	functions.push_back(&function);
	functionNames[&function] = sanitized;
	functionNamespaces[&function] = namespaceStack;
	functionLookup[canonical] = &function;
}

void ConverterImpl::collectStage(const StageFunctionInstruction &stageFunction)
{
	stageNamespaces[&stageFunction] = namespaceStack;
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
	if (canonical == "InstanceID")
	{
		return "gl_InstanceID";
	}
	if (canonical == "TriangleID")
	{
		return "triangleIndex";
	}
	if (auto it = remappedNames.find(canonical); it != remappedNames.end())
	{
		return it->second;
	}
	const std::vector<std::string> &context = currentEmissionNamespace();
	if (!context.empty())
	{
		for (std::size_t depth = context.size(); depth > 0; --depth)
		{
			std::ostringstream oss;
			for (std::size_t i = 0; i < depth; ++i)
			{
				if (i > 0)
				{
					oss << "::";
				}
				oss << context[i];
			}
			if (!canonical.empty())
			{
				oss << "::" << canonical;
			}
			const std::string qualified = oss.str();
			if (auto ctxIt = remappedNames.find(qualified); ctxIt != remappedNames.end())
			{
				return ctxIt->second;
			}
		}
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
	if (canonical == "InstanceID")
	{
		return "gl_InstanceID";
	}
	if (canonical == "TriangleID")
	{
		return "triangleIndex";
	}
	if (auto it = remappedNames.find(canonical); it != remappedNames.end())
	{
		return it->second;
	}
	return canonical;
}

std::optional<std::string> ConverterImpl::resolveAggregateQualifiedName(const Name &name) const
{
	const std::string base = joinName(name);
	if (base.find("::") != std::string::npos || name.parts.size() > 1)
	{
		if (aggregateKindLookup.find(base) != aggregateKindLookup.end())
		{
			return base;
		}
		return std::nullopt;
	}

	const std::vector<std::string> &context = currentEmissionNamespace();
	if (!context.empty())
	{
		for (std::size_t depth = context.size(); depth > 0; --depth)
		{
			std::ostringstream oss;
			for (std::size_t i = 0; i < depth; ++i)
			{
				if (i > 0)
				{
					oss << "::";
				}
				oss << context[i];
			}
			oss << "::" << base;
			const std::string qualified = oss.str();
			if (aggregateKindLookup.find(qualified) != aggregateKindLookup.end())
			{
				return qualified;
			}
		}
	}

	if (aggregateKindLookup.find(base) != aggregateKindLookup.end())
	{
		return base;
	}

	return std::nullopt;
}

ConverterImpl::StageUsage ConverterImpl::collectStageUsage(const StageFunctionInstruction *stage) const
{
	StageUsage usage;
	if (!stage || !stage->body)
	{
		return usage;
	}

	struct UsageCollector
	{
		const ConverterImpl &converter;
		StageUsage &usage;
		std::unordered_set<const FunctionInstruction *> visitedFunctions;
		std::unordered_set<std::string> visitedMethodHelpers;
		std::vector<std::unordered_set<std::string>> localScopes;
		std::vector<std::vector<std::string>> namespaceScopes;
		const AggregateInfo *currentMethodAggregate = nullptr;

		explicit UsageCollector(const ConverterImpl &converter, StageUsage &usage)
		    : converter(converter),
		      usage(usage)
		{
		}

		bool isLocal(const std::string &name) const
		{
			for (auto it = localScopes.rbegin(); it != localScopes.rend(); ++it)
			{
				if (it->find(name) != it->end())
				{
					return true;
				}
			}
			return false;
		}

		void pushScope()
		{
			localScopes.emplace_back();
		}

		void popScope()
		{
			if (!localScopes.empty())
			{
				localScopes.pop_back();
			}
		}

		void addLocal(const std::string &name)
		{
			if (localScopes.empty())
			{
				pushScope();
			}
			localScopes.back().insert(name);
		}

		void pushNamespace(const std::vector<std::string> &ns)
		{
			namespaceScopes.push_back(ns);
		}

		void popNamespace()
		{
			if (!namespaceScopes.empty())
			{
				namespaceScopes.pop_back();
			}
		}

		const std::vector<std::string> &currentNamespace() const
		{
			static const std::vector<std::string> kEmpty;
			if (namespaceScopes.empty())
			{
				return kEmpty;
			}
			return namespaceScopes.back();
		}

		decltype(ConverterImpl::globalVariableLookup)::const_iterator resolveGlobalVariable(const Name &name) const
		{
			const auto &map = converter.globalVariableLookup;
			const std::string base = joinName(name);
			if (base.find("::") != std::string::npos || name.parts.size() > 1)
			{
				return map.find(base);
			}
			const auto &ns = currentNamespace();
			for (std::size_t depth = ns.size(); depth > 0; --depth)
			{
				std::ostringstream oss;
				for (std::size_t i = 0; i < depth; ++i)
				{
					if (i > 0)
					{
						oss << "::";
					}
					oss << ns[i];
				}
				oss << "::" << base;
				auto it = map.find(oss.str());
				if (it != map.end())
				{
					return it;
				}
			}
			return map.find(base);
		}

		decltype(ConverterImpl::textureLookup)::const_iterator resolveTexture(const Name &name) const
		{
			const auto &map = converter.textureLookup;
			const std::string base = joinName(name);
			if (base.find("::") != std::string::npos || name.parts.size() > 1)
			{
				return map.find(base);
			}
			const auto &ns = currentNamespace();
			for (std::size_t depth = ns.size(); depth > 0; --depth)
			{
				std::ostringstream oss;
				for (std::size_t i = 0; i < depth; ++i)
				{
					if (i > 0)
					{
						oss << "::";
					}
					oss << ns[i];
				}
				oss << "::" << base;
				auto it = map.find(oss.str());
				if (it != map.end())
				{
					return it;
				}
			}
			return map.find(base);
		}

		decltype(ConverterImpl::functionLookup)::const_iterator resolveFunction(const Name &name) const
		{
			const auto &map = converter.functionLookup;
			const std::string base = joinName(name);
			if (base.find("::") != std::string::npos || name.parts.size() > 1)
			{
				return map.find(base);
			}
			const auto &ns = currentNamespace();
			for (std::size_t depth = ns.size(); depth > 0; --depth)
			{
				std::ostringstream oss;
				for (std::size_t i = 0; i < depth; ++i)
				{
					if (i > 0)
					{
						oss << "::";
					}
					oss << ns[i];
				}
				oss << "::" << base;
				auto it = map.find(oss.str());
				if (it != map.end())
				{
					return it;
				}
			}
			return map.find(base);
		}

		std::optional<std::string> resolveAggregateKey(const Name &name) const
		{
			const auto &map = converter.aggregateKindLookup;
			const std::string base = joinName(name);
			if (base.find("::") != std::string::npos || name.parts.size() > 1)
			{
				if (map.find(base) != map.end())
				{
					return base;
				}
				return std::nullopt;
			}
			const auto &ns = currentNamespace();
			for (std::size_t depth = ns.size(); depth > 0; --depth)
			{
				std::ostringstream oss;
				for (std::size_t i = 0; i < depth; ++i)
				{
					if (i > 0)
					{
						oss << "::";
					}
					oss << ns[i];
				}
				oss << "::" << base;
				const std::string qualified = oss.str();
				if (map.find(qualified) != map.end())
				{
					return qualified;
				}
			}
			if (map.find(base) != map.end())
			{
				return base;
			}
			return std::nullopt;
		}

		void collectStage(const StageFunctionInstruction *stage)
		{
			if (!stage || !stage->body)
			{
				return;
			}
			auto nsIt = converter.stageNamespaces.find(stage);
			if (nsIt != converter.stageNamespaces.end())
			{
				pushNamespace(nsIt->second);
			}
			else
			{
				pushNamespace({});
			}

			pushScope();
			for (const Parameter &param : stage->parameters)
			{
				addLocal(safeTokenContent(param.name));
			}
			collectStatement(stage->body.get());
			popScope();
			popNamespace();
		}

		void collectFunction(const FunctionInstruction *function)
		{
			if (!function || !function->body)
			{
				return;
			}
			if (!visitedFunctions.insert(function).second)
			{
				return;
			}
			auto nsIt = converter.functionNamespaces.find(function);
			if (nsIt != converter.functionNamespaces.end())
			{
				pushNamespace(nsIt->second);
			}
			else
			{
				pushNamespace({});
			}
			pushScope();
			for (const Parameter &param : function->parameters)
			{
				addLocal(safeTokenContent(param.name));
			}
			collectStatement(function->body.get());
			popScope();
			popNamespace();
		}

		void collectMethod(const MethodHelper &helper, const AggregateInfo &aggregate)
		{
			if (!helper.node || !helper.node->body)
			{
				return;
			}
			if (!visitedMethodHelpers.insert(helper.helperName).second)
			{
				return;
			}
			const AggregateInfo *previousAggregate = currentMethodAggregate;
			currentMethodAggregate = &aggregate;

			pushNamespace(aggregate.namespacePath);
			pushScope();
			addLocal("this");
			for (const std::string &fieldName : aggregate.fieldNames)
			{
				addLocal(fieldName);
			}
			for (const Parameter &param : helper.node->parameters)
			{
				addLocal(safeTokenContent(param.name));
			}
			collectStatement(helper.node->body.get());
			popScope();
			popNamespace();

			currentMethodAggregate = previousAggregate;
		}

		void collectStatement(const Statement *statement)
		{
			if (!statement)
			{
				return;
			}
			switch (statement->kind)
			{
				case Statement::Kind::Block:
				{
					const auto *block = static_cast<const BlockStatement *>(statement);
					for (const auto &stmt : block->statements)
					{
						collectStatement(stmt.get());
					}
					break;
				}
				case Statement::Kind::Expression:
				{
					const auto *expr = static_cast<const ExpressionStatement *>(statement);
					collectExpression(expr->expression.get());
					break;
				}
				case Statement::Kind::Variable:
				{
					const auto *var = static_cast<const VariableStatement *>(statement);
					for (const VariableDeclarator &declarator : var->declaration.declarators)
					{
						addLocal(safeTokenContent(declarator.name));
						if (declarator.arraySize)
						{
							collectExpression(declarator.arraySize.get());
						}
						if (declarator.initializer)
						{
							collectExpression(declarator.initializer.get());
						}
					}
					break;
				}
				case Statement::Kind::If:
				{
					const auto *ifStmt = static_cast<const IfStatement *>(statement);
					collectExpression(ifStmt->condition.get());
					collectStatement(ifStmt->thenBranch.get());
					collectStatement(ifStmt->elseBranch.get());
					break;
				}
				case Statement::Kind::While:
				{
					const auto *whileStmt = static_cast<const WhileStatement *>(statement);
					collectExpression(whileStmt->condition.get());
					collectStatement(whileStmt->body.get());
					break;
				}
				case Statement::Kind::DoWhile:
				{
					const auto *doStmt = static_cast<const DoWhileStatement *>(statement);
					collectStatement(doStmt->body.get());
					collectExpression(doStmt->condition.get());
					break;
				}
				case Statement::Kind::For:
				{
					const auto *forStmt = static_cast<const ForStatement *>(statement);
					collectStatement(forStmt->initializer.get());
					collectExpression(forStmt->condition.get());
					collectExpression(forStmt->increment.get());
					collectStatement(forStmt->body.get());
					break;
				}
				case Statement::Kind::Return:
				{
					const auto *ret = static_cast<const ReturnStatement *>(statement);
					collectExpression(ret->value.get());
					break;
				}
				case Statement::Kind::Break:
				case Statement::Kind::Continue:
				case Statement::Kind::Discard:
					break;
			}
		}

		void collectExpression(const Expression *expression)
		{
			if (!expression)
			{
				return;
			}
			switch (expression->kind)
			{
				case Expression::Kind::Literal:
					break;
				case Expression::Kind::ArrayLiteral:
				{
					const auto *array = static_cast<const ArrayLiteralExpression *>(expression);
					for (const auto &element : array->elements)
					{
						collectExpression(element.get());
					}
					break;
				}
				case Expression::Kind::Identifier:
					handleIdentifier(*static_cast<const IdentifierExpression *>(expression));
					break;
				case Expression::Kind::Unary:
				{
					const auto *unary = static_cast<const UnaryExpression *>(expression);
					collectExpression(unary->operand.get());
					break;
				}
				case Expression::Kind::Binary:
				{
					const auto *binary = static_cast<const BinaryExpression *>(expression);
					collectExpression(binary->left.get());
					collectExpression(binary->right.get());
					break;
				}
				case Expression::Kind::Assignment:
				{
					const auto *assign = static_cast<const AssignmentExpression *>(expression);
					collectExpression(assign->target.get());
					collectExpression(assign->value.get());
					break;
				}
				case Expression::Kind::Conditional:
				{
					const auto *cond = static_cast<const ConditionalExpression *>(expression);
					collectExpression(cond->condition.get());
					collectExpression(cond->thenBranch.get());
					collectExpression(cond->elseBranch.get());
					break;
				}
				case Expression::Kind::Call:
					handleCall(*static_cast<const CallExpression *>(expression));
					break;
				case Expression::Kind::MemberAccess:
				{
					const auto *member = static_cast<const MemberExpression *>(expression);
					collectExpression(member->object.get());
					break;
				}
				case Expression::Kind::IndexAccess:
				{
					const auto *index = static_cast<const IndexExpression *>(expression);
					collectExpression(index->object.get());
					collectExpression(index->index.get());
					break;
				}
				case Expression::Kind::Postfix:
				{
					const auto *postfix = static_cast<const PostfixExpression *>(expression);
					collectExpression(postfix->operand.get());
					break;
				}
			}
		}

		void handleIdentifier(const IdentifierExpression &identifier)
		{
			const std::string name = joinName(identifier.name);
			if (name.empty())
			{
				return;
			}
			if (isLocal(name))
			{
				return;
			}
			if (name == "pixelPosition" || name == "InstanceID" || name == "TriangleID")
			{
				return;
			}

			auto globalIt = resolveGlobalVariable(identifier.name);
			if (globalIt != converter.globalVariableLookup.end())
			{
				usage.globals.insert(globalIt->second);
			}

			if (auto aggregateKey = resolveAggregateKey(identifier.name))
			{
				const auto kindIt = converter.aggregateKindLookup.find(*aggregateKey);
				if (kindIt != converter.aggregateKindLookup.end())
				{
					if (kindIt->second == AggregateInstruction::Kind::ConstantBlock ||
					    kindIt->second == AggregateInstruction::Kind::AttributeBlock)
					{
						usage.blocks.insert(*aggregateKey);
					}
				}
			}

			auto textureIt = resolveTexture(identifier.name);
			if (textureIt != converter.textureLookup.end())
			{
				usage.textures.insert(textureIt->first);
			}
		}

		void markMethodHelper(const std::string &helperName)
		{
			if (!usage.methodHelpers.insert(helperName).second)
			{
				return;
			}
			const AggregateInfo *aggregate = nullptr;
			const MethodHelper *helper = converter.findMethodHelper(helperName, &aggregate);
			if (aggregate && (aggregate->kind == AggregateInstruction::Kind::ConstantBlock ||
			                  aggregate->kind == AggregateInstruction::Kind::AttributeBlock))
			{
				usage.blocks.insert(aggregate->qualifiedName);
			}
			if (aggregate && helper)
			{
				collectMethod(*helper, *aggregate);
			}
		}

		bool handleImplicitMethodCall(const IdentifierExpression &identifier)
		{
			if (!currentMethodAggregate || identifier.name.parts.size() != 1)
			{
				return false;
			}
			const std::string methodName = safeTokenContent(identifier.name.parts.front());
			auto typeIt = converter.methodCallHelpers.find(currentMethodAggregate->qualifiedName);
			if (typeIt == converter.methodCallHelpers.end())
			{
				return false;
			}
			auto helperIt = typeIt->second.find(methodName);
			if (helperIt == typeIt->second.end())
			{
				return false;
			}
			markMethodHelper(helperIt->second.helperName);
			return true;
		}

		void handleMemberCall(const MemberExpression &member)
		{
			auto infoIt = converter.expressionInfo.find(member.object.get());
			if (infoIt == converter.expressionInfo.end())
			{
				return;
			}
			const std::string methodName = safeTokenContent(member.member);
			const std::string objectType = infoIt->second.typeName;
			auto typeIt = converter.methodCallHelpers.find(objectType);
			if (typeIt == converter.methodCallHelpers.end())
			{
				return;
			}
			auto helperIt = typeIt->second.find(methodName);
			if (helperIt == typeIt->second.end())
			{
				return;
			}
			markMethodHelper(helperIt->second.helperName);
		}

		void handleCall(const CallExpression &call)
		{
			if (const auto *member = dynamic_cast<const MemberExpression *>(call.callee.get()))
			{
				collectExpression(member->object.get());
				handleMemberCall(*member);
				for (const auto &arg : call.arguments)
				{
					collectExpression(arg.get());
				}
				return;
			}

			if (const auto *identifier = dynamic_cast<const IdentifierExpression *>(call.callee.get()))
			{
				if (!handleImplicitMethodCall(*identifier))
				{
					auto functionIt = resolveFunction(identifier->name);
					if (functionIt != converter.functionLookup.end())
					{
						if (usage.functions.insert(functionIt->second).second)
						{
							collectFunction(functionIt->second);
						}
					}
					else
					{
						const std::string name = joinName(identifier->name);
						if (convertLuminaType(name) != name)
						{
							// Type constructor; no function dependency.
						}
					}
				}
				for (const auto &arg : call.arguments)
				{
					collectExpression(arg.get());
				}
				return;
			}

			collectExpression(call.callee.get());
			for (const auto &arg : call.arguments)
			{
				collectExpression(arg.get());
			}
		}
	};

	UsageCollector collector(*this, usage);
	collector.collectStage(stage);
	return usage;
}

void ConverterImpl::emitCommon(std::ostringstream &oss, const StageUsage &usage) const
{
	emitStructs(oss);
	emitStructMethods(oss, usage);
	emitBlocks(oss, AggregateInstruction::Kind::ConstantBlock, usage);
	emitBlockMethods(oss, constantBlocks, usage);
	emitBlocks(oss, AggregateInstruction::Kind::AttributeBlock, usage);
	emitBlockMethods(oss, attributeBlocks, usage);
	emitGlobalVariables(oss, usage);
	emitFunctions(oss, usage);
	emitTextures(oss, usage);
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
		emitBlockMembers(oss, *aggregate, 1, nullptr);
		oss << "};\n\n";
	}
}

void ConverterImpl::emitBlocks(std::ostringstream &oss, AggregateInstruction::Kind kind, const StageUsage &usage) const
{
	const std::vector<AggregateInfo> &blocks = (kind == AggregateInstruction::Kind::ConstantBlock) ? constantBlocks :
	                                                                                                 attributeBlocks;
	const char *bindingKeyword = (kind == AggregateInstruction::Kind::AttributeBlock) ? "ATTRIBUTE_BINDING" :
	                                                                       "CONSTANT_BINDING";

	for (const AggregateInfo &info : blocks)
	{
		if (usage.blocks.find(info.qualifiedName) == usage.blocks.end())
		{
			continue;
		}
		const AggregateInstruction *aggregate = info.node;
		if (!aggregate)
		{
			continue;
		}
		const bool ssbo = info.isSSBO;
		const std::string &blockName = info.glslInstanceName;
		const std::string &blockTypeName = info.glslTypeName;

		oss << "layout(binding = " << bindingKeyword << ", std430) "
		    << (ssbo ? "buffer" : "uniform") << " " << blockTypeName << "\n{\n";
		emitBlockMembers(oss, *aggregate, 1, &info);
		oss << "} " << blockName << ";\n\n";
	}
}

void ConverterImpl::emitBlockMembers(
    std::ostringstream &oss,
    const AggregateInstruction &aggregate,
    int indent,
    const AggregateInfo *info) const
{
	const bool addSize = info && info->isSSBO;
	const std::string blockName = info ? info->glslInstanceName : std::string{};

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
			if (addSize && declarator.hasArraySuffix && !declarator.hasArraySize)
			{
				const std::string arrayName = sanitizeIdentifier(safeTokenContent(declarator.name));
				oss << "uint spk_" << blockName << "_" << arrayName << "_size;\n";
				writeIndent(oss, indent);
			}
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

void ConverterImpl::emitStructMethods(std::ostringstream &oss, const StageUsage &usage) const
{
	bool emitted = false;
	for (const AggregateInfo &info : structures)
	{
		for (const MethodHelper &helper : info.methods)
		{
			if (usage.methodHelpers.find(helper.helperName) == usage.methodHelpers.end())
			{
				continue;
			}
			emitMethodHelper(oss, info, helper);
			emitted = true;
		}
	}
	if (emitted)
	{
		oss << "\n";
	}
}

void ConverterImpl::emitBlockMethods(
    std::ostringstream &oss, const std::vector<AggregateInfo> &aggregates, const StageUsage &usage) const
{
	bool emitted = false;
	for (const AggregateInfo &info : aggregates)
	{
		for (const MethodHelper &helper : info.methods)
		{
			if (usage.methodHelpers.find(helper.helperName) == usage.methodHelpers.end())
			{
				continue;
			}
			emitMethodHelper(oss, info, helper);
			emitted = true;
		}
	}
	if (emitted)
	{
		oss << "\n";
	}
}

void ConverterImpl::emitGlobalVariables(std::ostringstream &oss, const StageUsage &usage) const
{
	for (const VariableInstruction *variable : globalVariables)
	{
		if (!variable)
		{
			continue;
		}
		if (usage.globals.find(variable) == usage.globals.end())
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
	if (!globalVariables.empty() && !usage.globals.empty())
	{
		oss << "\n";
	}
}

void ConverterImpl::emitTextures(std::ostringstream &oss, const StageUsage &usage) const
{
	if (input.textures.empty())
	{
		return;
	}

	std::vector<TextureBinding> bindings = input.textures;
	std::sort(bindings.begin(), bindings.end(), [](const TextureBinding &a, const TextureBinding &b) {
		return a.location < b.location;
	});

	for (const TextureBinding &binding : bindings)
	{
		if (usage.textures.find(binding.luminaName) == usage.textures.end())
		{
			continue;
		}
		oss << "layout(binding = " << binding.location << ") uniform " << binding.type << " " << binding.glslName
		    << ";\n";
	}
	if (!usage.textures.empty())
	{
		oss << "\n";
	}
}

void ConverterImpl::emitFunctions(std::ostringstream &oss, const StageUsage &usage) const
{
	bool emitted = false;
	for (const FunctionInstruction *function : functions)
	{
		if (!function || !function->body)
		{
			continue;
		}
		if (usage.functions.find(function) == usage.functions.end())
		{
			continue;
		}
		const auto it = functionNames.find(function);
		if (it == functionNames.end())
		{
			continue;
		}
		emitFunction(oss, *function, it->second);
		emitted = true;
	}
	if (emitted)
	{
		oss << "\n";
	}
}

void ConverterImpl::emitFunction(std::ostringstream &oss, const FunctionInstruction &function, const std::string &name) const
{
	oss << typeToGLSL(function.returnType) << " " << name << "(";
	emitParameters(oss, function.parameters);
	oss << ")\n";
	if (function.body)
	{
		auto nsIt = functionNamespaces.find(&function);
		if (nsIt != functionNamespaces.end())
		{
			pushEmissionNamespace(nsIt->second);
		}
		oss << "{\n";
		emitBlockStatement(oss, *function.body, 1);
		oss << "}\n";
		if (nsIt != functionNamespaces.end())
		{
			popEmissionNamespace();
		}
	}
	else
	{
		oss << "{ }\n";
	}
	oss << "\n";
}

void ConverterImpl::emitParameters(std::ostringstream &oss, const std::vector<Parameter> &params) const
{
	for (std::size_t i = 0; i < params.size(); ++i)
	{
		if (i > 0)
		{
			oss << ", ";
		}
		const Parameter &param = params[i];
		if (param.isReference)
		{
			oss << "inout ";
		}
		else if (param.type.isConst)
		{
			oss << "const ";
		}
		oss << typeToGLSL(param.type) << " " << parameterName(param.name);
	}
}

std::string ConverterImpl::parameterName(const Token &token) const
{
	return sanitizeIdentifier(safeTokenContent(token));
}

bool ConverterImpl::isMethodLocalName(const std::string &name) const
{
	for (auto it = methodLocalNameStack.rbegin(); it != methodLocalNameStack.rend(); ++it)
	{
		if (it->find(name) != it->end())
		{
			return true;
		}
	}
	return false;
}

const ConverterImpl::AggregateInfo *ConverterImpl::findAggregateInfo(const std::string &qualifiedName) const
{
	const auto finder = [&qualifiedName](const std::vector<AggregateInfo> &collection) -> const AggregateInfo * {
		for (const AggregateInfo &info : collection)
		{
			if (info.qualifiedName == qualifiedName)
			{
				return &info;
			}
		}
		return nullptr;
	};

	if (const AggregateInfo *info = finder(structures))
	{
		return info;
	}
	if (const AggregateInfo *info = finder(attributeBlocks))
	{
		return info;
	}
	if (const AggregateInfo *info = finder(constantBlocks))
	{
		return info;
	}
	return nullptr;
}

const ConverterImpl::MethodHelper *ConverterImpl::findMethodHelper(
    const std::string &helperName, const AggregateInfo **aggregate) const
{
	const auto finder = [&helperName, aggregate](const std::vector<AggregateInfo> &collection) -> const MethodHelper * {
		for (const AggregateInfo &info : collection)
		{
			for (const MethodHelper &helper : info.methods)
			{
				if (helper.helperName == helperName)
				{
					if (aggregate)
					{
						*aggregate = &info;
					}
					return &helper;
				}
			}
		}
		return nullptr;
	};

	if (const MethodHelper *helper = finder(structures))
	{
		return helper;
	}
	if (const MethodHelper *helper = finder(attributeBlocks))
	{
		return helper;
	}
	if (const MethodHelper *helper = finder(constantBlocks))
	{
		return helper;
	}
	if (aggregate)
	{
		*aggregate = nullptr;
	}
	return nullptr;
}

void ConverterImpl::pushEmissionNamespace(const std::vector<std::string> &ns) const
{
	emissionNamespaceStack.push_back(ns);
}

void ConverterImpl::popEmissionNamespace() const
{
	if (!emissionNamespaceStack.empty())
	{
		emissionNamespaceStack.pop_back();
	}
}

const std::vector<std::string> &ConverterImpl::currentEmissionNamespace() const
{
	static const std::vector<std::string> kEmptyNamespace;
	if (emissionNamespaceStack.empty())
	{
		return kEmptyNamespace;
	}
	return emissionNamespaceStack.back();
}

struct ConverterImpl::MethodAnalysisContext
{
	MethodAnalysisContext()
	{
		scopes.emplace_back();
	}

	void pushScope()
	{
		scopes.emplace_back();
	}

	void popScope()
	{
		if (!scopes.empty())
		{
			scopes.pop_back();
		}
	}

	void addName(const std::string &rawName)
	{
		if (scopes.empty())
		{
			scopes.emplace_back();
		}
		scopes.back().insert(sanitizeIdentifier(rawName));
	}

	bool isShadowed(const std::string &rawName) const
	{
		const std::string sanitized = sanitizeIdentifier(rawName);
		for (auto it = scopes.rbegin(); it != scopes.rend(); ++it)
		{
			if (it->find(sanitized) != it->end())
			{
				return true;
			}
		}
		return false;
	}

	std::vector<std::unordered_set<std::string>> scopes;
};

bool ConverterImpl::methodMutatesAggregate(const MethodMember &method, const AggregateInfo &info) const
{
	if (!method.body)
	{
		return false;
	}

	MethodAnalysisContext ctx;
	for (const Parameter &param : method.parameters)
	{
		ctx.addName(safeTokenContent(param.name));
	}
	return statementMutatesAggregate(*method.body, ctx, info);
}

bool ConverterImpl::statementMutatesAggregate(
    const Statement &statement, MethodAnalysisContext &ctx, const AggregateInfo &info) const
{
	switch (statement.kind)
	{
		case Statement::Kind::Block:
		{
			const auto &block = static_cast<const BlockStatement &>(statement);
			ctx.pushScope();
			for (const std::unique_ptr<Statement> &stmt : block.statements)
			{
				if (stmt && statementMutatesAggregate(*stmt, ctx, info))
				{
					ctx.popScope();
					return true;
				}
			}
			ctx.popScope();
			return false;
		}
		case Statement::Kind::Expression:
		{
			const auto &expr = static_cast<const ExpressionStatement &>(statement);
			return expr.expression && expressionMutatesAggregate(*expr.expression, ctx, info);
		}
		case Statement::Kind::Variable:
		{
			const auto &var = static_cast<const VariableStatement &>(statement);
			for (const VariableDeclarator &decl : var.declaration.declarators)
			{
				if (decl.initializer && expressionMutatesAggregate(*decl.initializer, ctx, info))
				{
					return true;
				}
				ctx.addName(safeTokenContent(decl.name));
			}
			return false;
		}
		case Statement::Kind::If:
		{
			const auto &ifStmt = static_cast<const IfStatement &>(statement);
			if (ifStmt.condition && expressionMutatesAggregate(*ifStmt.condition, ctx, info))
			{
				return true;
			}
			if (ifStmt.thenBranch && statementMutatesAggregate(*ifStmt.thenBranch, ctx, info))
			{
				return true;
			}
			if (ifStmt.elseBranch && statementMutatesAggregate(*ifStmt.elseBranch, ctx, info))
			{
				return true;
			}
			return false;
		}
		case Statement::Kind::While:
		{
			const auto &whileStmt = static_cast<const WhileStatement &>(statement);
			if (whileStmt.condition && expressionMutatesAggregate(*whileStmt.condition, ctx, info))
			{
				return true;
			}
			return whileStmt.body && statementMutatesAggregate(*whileStmt.body, ctx, info);
		}
		case Statement::Kind::DoWhile:
		{
			const auto &doStmt = static_cast<const DoWhileStatement &>(statement);
			if (doStmt.body && statementMutatesAggregate(*doStmt.body, ctx, info))
			{
				return true;
			}
			return doStmt.condition && expressionMutatesAggregate(*doStmt.condition, ctx, info);
		}
		case Statement::Kind::For:
		{
			const auto &forStmt = static_cast<const ForStatement &>(statement);
			ctx.pushScope();
			if (forStmt.initializer && statementMutatesAggregate(*forStmt.initializer, ctx, info))
			{
				ctx.popScope();
				return true;
			}
			if (forStmt.condition && expressionMutatesAggregate(*forStmt.condition, ctx, info))
			{
				ctx.popScope();
				return true;
			}
			if (forStmt.increment && expressionMutatesAggregate(*forStmt.increment, ctx, info))
			{
				ctx.popScope();
				return true;
			}
			const bool bodyResult = forStmt.body && statementMutatesAggregate(*forStmt.body, ctx, info);
			ctx.popScope();
			return bodyResult;
		}
		case Statement::Kind::Return:
		{
			const auto &ret = static_cast<const ReturnStatement &>(statement);
			return ret.value && expressionMutatesAggregate(*ret.value, ctx, info);
		}
		case Statement::Kind::Break:
		case Statement::Kind::Continue:
		case Statement::Kind::Discard:
			return false;
	}
	return false;
}

bool ConverterImpl::expressionMutatesAggregate(
    const Expression &expression, MethodAnalysisContext &ctx, const AggregateInfo &info) const
{
	switch (expression.kind)
	{
		case Expression::Kind::Literal:
			return false;
		case Expression::Kind::ArrayLiteral:
		{
			const auto &literal = static_cast<const ArrayLiteralExpression &>(expression);
			for (const std::unique_ptr<Expression> &element : literal.elements)
			{
				if (element && expressionMutatesAggregate(*element, ctx, info))
				{
					return true;
				}
			}
			return false;
		}
		case Expression::Kind::Identifier:
			return false;
		case Expression::Kind::Unary:
		{
			const auto &unary = static_cast<const UnaryExpression &>(expression);
			if (unary.op == UnaryOperator::PreIncrement || unary.op == UnaryOperator::PreDecrement)
			{
				if (unary.operand && expressionRefersToField(*unary.operand, ctx, info))
				{
					return true;
				}
			}
			return unary.operand && expressionMutatesAggregate(*unary.operand, ctx, info);
		}
		case Expression::Kind::Binary:
		{
			const auto &binary = static_cast<const BinaryExpression &>(expression);
			return (binary.left && expressionMutatesAggregate(*binary.left, ctx, info)) ||
			       (binary.right && expressionMutatesAggregate(*binary.right, ctx, info));
		}
		case Expression::Kind::Assignment:
		{
			const auto &assign = static_cast<const AssignmentExpression &>(expression);
			if (assign.target && expressionRefersToField(*assign.target, ctx, info))
			{
				return true;
			}
			return assign.value && expressionMutatesAggregate(*assign.value, ctx, info);
		}
		case Expression::Kind::Conditional:
		{
			const auto &conditional = static_cast<const ConditionalExpression &>(expression);
			return (conditional.condition && expressionMutatesAggregate(*conditional.condition, ctx, info)) ||
			       (conditional.thenBranch && expressionMutatesAggregate(*conditional.thenBranch, ctx, info)) ||
			       (conditional.elseBranch && expressionMutatesAggregate(*conditional.elseBranch, ctx, info));
		}
		case Expression::Kind::Call:
		{
			const auto &call = static_cast<const CallExpression &>(expression);
			if (call.callee && expressionMutatesAggregate(*call.callee, ctx, info))
			{
				return true;
			}
			for (const std::unique_ptr<Expression> &arg : call.arguments)
			{
				if (arg && expressionMutatesAggregate(*arg, ctx, info))
				{
					return true;
				}
			}
			return false;
		}
		case Expression::Kind::MemberAccess:
		{
			const auto &member = static_cast<const MemberExpression &>(expression);
			return member.object && expressionMutatesAggregate(*member.object, ctx, info);
		}
		case Expression::Kind::IndexAccess:
		{
			const auto &index = static_cast<const IndexExpression &>(expression);
			return (index.object && expressionMutatesAggregate(*index.object, ctx, info)) ||
			       (index.index && expressionMutatesAggregate(*index.index, ctx, info));
		}
		case Expression::Kind::Postfix:
		{
			const auto &postfix = static_cast<const PostfixExpression &>(expression);
			if (postfix.operand && expressionRefersToField(*postfix.operand, ctx, info))
			{
				return true;
			}
			return postfix.operand && expressionMutatesAggregate(*postfix.operand, ctx, info);
		}
	}
	return false;
}

bool ConverterImpl::expressionRefersToField(
    const Expression &expression, MethodAnalysisContext &ctx, const AggregateInfo &info) const
{
	switch (expression.kind)
	{
		case Expression::Kind::Identifier:
		{
			const auto &identifier = static_cast<const IdentifierExpression &>(expression);
			if (identifier.name.parts.size() != 1)
			{
				return false;
			}
			const std::string simple = safeTokenContent(identifier.name.parts.front());
			if (ctx.isShadowed(simple))
			{
				return false;
			}
			return info.fieldNames.find(sanitizeIdentifier(simple)) != info.fieldNames.end();
		}
		case Expression::Kind::MemberAccess:
		{
			const auto &member = static_cast<const MemberExpression &>(expression);
			if (!member.object)
			{
				return false;
			}
			const auto *objectIdentifier = dynamic_cast<const IdentifierExpression *>(member.object.get());
			if (!objectIdentifier || objectIdentifier->name.parts.size() != 1)
			{
				return false;
			}
			const std::string objectName = safeTokenContent(objectIdentifier->name.parts.front());
			if (objectName != "this")
			{
				return false;
			}
			return info.fieldNames.find(sanitizeIdentifier(safeTokenContent(member.member))) != info.fieldNames.end();
		}
		default:
			return false;
	}
}

void ConverterImpl::emitInterface(std::ostringstream &oss, const std::vector<StageIO> &entries, const char *qualifier) const
{
	for (const StageIO &entry : entries)
	{
		oss << "layout(location = " << entry.location << ") ";
		if (entry.flat)
		{
			oss << "flat ";
		}
		oss << qualifier << " " << typeToGLSL(entry.type) << " " << entry.name << ";\n";
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

	const auto nsIt = stageNamespaces.find(stage);
	if (nsIt != stageNamespaces.end())
	{
		pushEmissionNamespace(nsIt->second);
	}

	oss << "void main()\n{\n";
	if (stageKind == Stage::VertexPass)
	{
		writeIndent(oss, 1);
		oss << "triangleIndex = uint(gl_VertexID / 3);\n";
	}
	emitBlockStatement(oss, *stage->body, 1);
	oss << "}\n";

	if (nsIt != stageNamespaces.end())
	{
		popEmissionNamespace();
	}
}

void ConverterImpl::emitBlockStatement(std::ostringstream &oss, const BlockStatement &block, int indent) const
{
	if (currentMethodAggregate)
	{
		methodLocalNameStack.emplace_back();
	}
	for (const std::unique_ptr<Statement> &statement : block.statements)
	{
		if (statement)
		{
			emitStatement(oss, *statement, indent);
		}
	}
	if (currentMethodAggregate)
	{
		methodLocalNameStack.pop_back();
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
		const std::string originalName = safeTokenContent(declarator.name);
		const std::string varName = sanitizeIdentifier(originalName);
		writeIndent(oss, indent);
		oss << type << " " << varName;
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

		if (currentMethodAggregate && !methodLocalNameStack.empty())
		{
			methodLocalNameStack.back().insert(originalName);
		}
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
		case Expression::Kind::ArrayLiteral:
			return emitArrayLiteral(static_cast<const ArrayLiteralExpression &>(expression));
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

std::string ConverterImpl::emitArrayLiteral(const ArrayLiteralExpression &literal) const
{
	std::ostringstream oss;
	std::string typeName;
	std::optional<std::size_t> arraySize;
	auto infoIt = expressionInfo.find(&literal);
	if (infoIt != expressionInfo.end())
	{
		typeName = infoIt->second.typeName;
		if (infoIt->second.hasArraySize && infoIt->second.arraySize)
		{
			arraySize = infoIt->second.arraySize;
		}
	}

	if (typeName.empty())
	{
		oss << "{";
		for (std::size_t i = 0; i < literal.elements.size(); ++i)
		{
			if (i > 0)
			{
				oss << ", ";
			}
			if (literal.elements[i])
			{
				oss << emitExpression(*literal.elements[i]);
			}
		}
		oss << "}";
		return oss.str();
	}

	oss << typeToGLSL(typeName);
	if (arraySize)
	{
		oss << "[" << *arraySize << "]";
	}
	else
	{
		oss << "[]";
	}
	oss << "(";
	for (std::size_t i = 0; i < literal.elements.size(); ++i)
	{
		if (i > 0)
		{
			oss << ", ";
		}
		if (literal.elements[i])
		{
			oss << emitExpression(*literal.elements[i]);
		}
	}
	oss << ")";
	return oss.str();
}

std::string ConverterImpl::emitIdentifier(const IdentifierExpression &identifier) const
{
	if (identifier.name.parts.size() == 1)
	{
		const std::string simple = identifier.name.parts.front().content;
		if (!thisAliasStack.empty() && simple == "this")
		{
			return thisAliasStack.back();
		}

		if (currentMethodAggregate && !currentMethodSelfName.empty())
		{
			if (simple == currentMethodSelfName)
			{
				return currentMethodSelfName;
			}
			const std::string sanitizedField = sanitizeIdentifier(simple);
			if (currentMethodParameters.find(simple) == currentMethodParameters.end() &&
			    currentMethodAggregate->fieldNames.find(sanitizedField) != currentMethodAggregate->fieldNames.end() &&
			    !isMethodLocalName(simple))
			{
				return currentMethodSelfName + "." + sanitizedField;
			}
		}
	}
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
	std::string leftExpr = emitExpression(*binary.left);
	std::string rightExpr = emitExpression(*binary.right);

	return "(" + leftExpr + " " + binaryOperatorSymbol(binary.op) + " " + rightExpr + ")";
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

		if (std::optional<std::string> userCall = emitUserMethodCall(*member, call))
		{
			return *userCall;
		}
	}

	if (const auto *identifier = dynamic_cast<const IdentifierExpression *>(call.callee.get()))
	{
		if (std::optional<std::string> implicit = emitImplicitSelfCall(*identifier, call))
		{
			return *implicit;
		}

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
	if (std::optional<std::string> sizeAccess = emitSSBOArraySizeAccess(member))
	{
		return *sizeAccess;
	}
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

std::optional<std::string> ConverterImpl::emitSSBOArraySizeAccess(const MemberExpression &member) const
{
	if (safeTokenContent(member.member) != "size")
	{
		return std::nullopt;
	}
	if (!member.object)
	{
		return std::nullopt;
	}

	auto infoIt = expressionInfo.find(member.object.get());
	if (infoIt == expressionInfo.end())
	{
		return std::nullopt;
	}
	if (!infoIt->second.isArray || infoIt->second.hasArraySize)
	{
		return std::nullopt;
	}

	std::string blockName;
	std::string arrayName;

	if (const auto *arrayIdentifier = dynamic_cast<const IdentifierExpression *>(member.object.get()))
	{
		if (!currentMethodAggregate || !currentMethodAggregate->isSSBO ||
		    (currentMethodAggregate->kind != AggregateInstruction::Kind::ConstantBlock &&
		        currentMethodAggregate->kind != AggregateInstruction::Kind::AttributeBlock))
		{
			return std::nullopt;
		}
		if (arrayIdentifier->name.parts.size() != 1)
		{
			return std::nullopt;
		}
		if (currentMethodSelfName.empty())
		{
			return std::nullopt;
		}
		const std::string simple = safeTokenContent(arrayIdentifier->name.parts.front());
		const std::string sanitizedField = sanitizeIdentifier(simple);
		if (currentMethodAggregate->fieldNames.find(sanitizedField) == currentMethodAggregate->fieldNames.end())
		{
			return std::nullopt;
		}
		blockName = currentMethodSelfName;
		arrayName = sanitizedField;
	}
	else if (const auto *arrayMember = dynamic_cast<const MemberExpression *>(member.object.get()))
	{
		if (!arrayMember->object)
		{
			return std::nullopt;
		}
		const auto *rootIdentifier = dynamic_cast<const IdentifierExpression *>(arrayMember->object.get());
		if (!rootIdentifier)
		{
			return std::nullopt;
		}
		auto aggregateKey = resolveAggregateQualifiedName(rootIdentifier->name);
		if (!aggregateKey)
		{
			return std::nullopt;
		}
		const AggregateInfo *aggregate = findAggregateInfo(*aggregateKey);
		if (!aggregate || !aggregate->isSSBO ||
		    (aggregate->kind != AggregateInstruction::Kind::ConstantBlock &&
		        aggregate->kind != AggregateInstruction::Kind::AttributeBlock))
		{
			return std::nullopt;
		}
		blockName = remapIdentifier(rootIdentifier->name);
		arrayName = sanitizeIdentifier(safeTokenContent(arrayMember->member));
	}

	if (blockName.empty() || arrayName.empty())
	{
		return std::nullopt;
	}

	const std::string sizeName = "spk_" + blockName + "_" + arrayName + "_size";
	return blockName + "." + sizeName;
}

std::string ConverterImpl::typeToGLSL(const TypeName &type) const
{
	return sanitizeIdentifier(convertLuminaType(joinName(type.name)));
}

std::string ConverterImpl::typeToGLSL(const std::string &typeName) const
{
	return sanitizeIdentifier(convertLuminaType(typeName));
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

std::string ConverterImpl::aggregateTypeName(const AggregateInfo &info) const
{
	return info.glslTypeName;
}

void ConverterImpl::emitMethodHelper(std::ostringstream &oss, const AggregateInfo &info, const MethodHelper &helper) const
{
	if (!helper.node || !helper.node->body)
	{
		return;
	}

	oss << typeToGLSL(helper.node->returnType) << " " << helper.helperName << "(";
	bool first = true;
	const bool isStructAggregate = info.kind == AggregateInstruction::Kind::Struct;
	const bool needsSelfParameter = isStructAggregate;
	const std::string aggregateType = aggregateTypeName(info);
	if (needsSelfParameter)
	{
		if (!helper.isConst)
		{
			oss << "inout ";
		}
		else
		{
			oss << "const ";
		}
		oss << aggregateType << " " << kMethodSelfName;
		first = false;
	}
	for (const Parameter &param : helper.node->parameters)
	{
		if (!first)
		{
			oss << ", ";
		}
		if (param.isReference)
		{
			oss << "inout ";
		}
		else if (param.type.isConst)
		{
			oss << "const ";
		}
		oss << typeToGLSL(param.type) << " " << parameterName(param.name);
		first = false;
	}
	oss << ")\n{\n";
	currentMethodAggregate = &info;
	currentMethodParameters.clear();
	methodLocalNameStack.clear();
	currentMethodSelfName = needsSelfParameter ? std::string(kMethodSelfName) : info.glslInstanceName;
	currentMethodUsesSelfParameter = needsSelfParameter;
	for (const Parameter &param : helper.node->parameters)
	{
		currentMethodParameters.insert(safeTokenContent(param.name));
	}
	thisAliasStack.push_back(currentMethodSelfName);
	pushEmissionNamespace(info.namespacePath);
	emitBlockStatement(oss, *helper.node->body, 1);
	popEmissionNamespace();
	thisAliasStack.pop_back();
	methodLocalNameStack.clear();
	currentMethodParameters.clear();
	currentMethodAggregate = nullptr;
	currentMethodSelfName.clear();
	currentMethodUsesSelfParameter = false;
	oss << "}\n\n";
}

std::optional<std::string> ConverterImpl::emitUserMethodCall(const MemberExpression &member, const CallExpression &call) const
{
	auto infoIt = expressionInfo.find(member.object.get());
	if (infoIt == expressionInfo.end())
	{
		return std::nullopt;
	}

	const std::string objectType = infoIt->second.typeName;
	const std::string methodName = safeTokenContent(member.member);
	auto typeIt = methodCallHelpers.find(objectType);
	if (typeIt == methodCallHelpers.end())
	{
		return std::nullopt;
	}
	auto helperIt = typeIt->second.find(methodName);
	if (helperIt == typeIt->second.end())
	{
		return std::nullopt;
	}

	const AggregateInfo *aggregateInfo = findAggregateInfo(objectType);
	const bool needsSelfArgument =
	    !aggregateInfo || aggregateInfo->kind == AggregateInstruction::Kind::Struct;

	std::ostringstream oss;
	oss << helperIt->second.helperName << "(";
	bool first = true;
	if (needsSelfArgument)
	{
		oss << emitExpression(*member.object);
		first = false;
	}
	for (const std::unique_ptr<Expression> &argument : call.arguments)
	{
		if (!first)
		{
			oss << ", ";
		}
		oss << emitExpression(*argument);
		first = false;
	}
	oss << ")";
	return oss.str();
}

std::optional<std::string> ConverterImpl::emitImplicitSelfCall(const IdentifierExpression &identifier, const CallExpression &call) const
{
	if (!currentMethodAggregate || identifier.name.parts.size() != 1)
	{
		return std::nullopt;
	}

	const std::string methodName = identifier.name.parts.front().content;
	auto typeIt = methodCallHelpers.find(currentMethodAggregate->qualifiedName);
	if (typeIt == methodCallHelpers.end())
	{
		return std::nullopt;
	}
	auto helperIt = typeIt->second.find(methodName);
	if (helperIt == typeIt->second.end())
	{
		return std::nullopt;
	}

	std::ostringstream oss;
	oss << helperIt->second.helperName << "(";
	bool first = true;
	if (currentMethodUsesSelfParameter && !currentMethodSelfName.empty())
	{
		oss << currentMethodSelfName;
		first = false;
	}
	for (const std::unique_ptr<Expression> &argument : call.arguments)
	{
		if (!first)
		{
			oss << ", ";
		}
		oss << emitExpression(*argument);
		first = false;
	}
	oss << ")";
	return oss.str();
}

ShaderSources ConverterImpl::run()
{
	ShaderSources sources;
	const StageUsage vertexUsage = collectStageUsage(vertexStage);
	const StageUsage fragmentUsage = collectStageUsage(fragmentStage);

	{
		std::ostringstream vertex;
		vertex << "#version 450 core\n"
		       << "#extension GL_NV_uniform_buffer_std430_layout : enable\n\n";
		emitInterface(vertex, input.vertexInputs, "in");
		emitInterface(vertex, input.stageVaryings, "out");
		emitCommon(vertex, vertexUsage);
		emitStage(vertex, vertexStage, Stage::VertexPass);
		sources.vertex = vertex.str();
	}

	{
		std::ostringstream fragment;
		fragment << "#version 450 core\n"
		         << "#extension GL_NV_uniform_buffer_std430_layout : enable\n\n";
		emitInterface(fragment, input.stageVaryings, "in");
		emitInterface(fragment, input.fragmentOutputs, "out");
		emitCommon(fragment, fragmentUsage);
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
