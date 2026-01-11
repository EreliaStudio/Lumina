#include "semantic_parser.hpp"

#include <array>
#include <cctype>
#include <cstddef>
#include <filesystem>
#include <initializer_list>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace
{
        std::string joinName(const Name &name)
        {
                std::string result;
                for (std::size_t i = 0; i < name.parts.size(); ++i)
                {
                        if (i > 0)
                        {
                                result += "::";
                        }
                        result += name.parts[i].content;
                }
                return result;
        }

        std::string stageToString(Stage stage)
        {
                switch (stage)
                {
                        case Stage::Input:
                                return "Input";
                        case Stage::VertexPass:
                                return "VertexPass";
                        case Stage::FragmentPass:
                                return "FragmentPass";
                        case Stage::Output:
                                return "Output";
                }
                return "Unknown";
        }

        std::size_t stageIndex(Stage stage)
        {
                switch (stage)
                {
                        case Stage::Input:
                                return 0;
                        case Stage::VertexPass:
                                return 1;
                        case Stage::FragmentPass:
                                return 2;
                        case Stage::Output:
                                return 3;
                }
                return 0;
        }

        Token makeSyntheticStageToken(Stage stage)
        {
                Token token;
                token.origin = std::filesystem::path("<semantic>");
                token.content = stageToString(stage);
                token.start = token.end = Token::Location{0, 0};
                return token;
        }

        Token makeSyntheticToken(const std::string &content)
        {
                Token token;
                token.origin = std::filesystem::path("<builtin>");
                token.content = content;
                token.start = token.end = Token::Location{0, 0};
                return token;
        }

        struct TypeInfo
        {
                std::string name;
                bool isConst = false;
                bool isReference = false;
                bool isArray = false;
                bool hasArraySize = false;
                std::optional<std::size_t> arraySize;

                bool valid() const { return !name.empty(); }
        };

        struct TypedValue
        {
                TypeInfo type;
                bool isLValue = false;
        };

        struct Symbol
        {
                Token token;
                TypeInfo type;
                bool isAssignable = false;
        };

        struct FunctionSignature
        {
                Token nameToken;
                TypeInfo returnType;
                bool returnsReference = false;
                bool isMethod = false;
                bool isConstMethod = false;
                std::vector<TypeInfo> parameters;
                std::string displayName;
        };

        struct AggregateField
        {
                Token nameToken;
                TypeInfo type;
        };

        struct AggregateInfo
        {
                Token nameToken;
                std::string qualifiedName;
                std::unordered_map<std::string, AggregateField> fields;
                std::unordered_map<std::string, std::vector<FunctionSignature>> methods;
                std::unordered_map<std::string, std::vector<FunctionSignature>> operators;
                std::vector<FunctionSignature> constructors;
                bool hasUserDefaultConstructor = false;
                bool hasExplicitConstructor = false;
                bool allowUnsizedArrays = false;
        };

        std::string typeToString(const TypeInfo &type)
        {
                std::ostringstream out;
                if (type.isConst)
                {
                        out << "const ";
                }
                out << type.name;
                if (type.isArray)
                {
                        out << '[';
                        if (type.arraySize)
                        {
                                out << *type.arraySize;
                        }
                        out << ']';
                }
                if (type.isReference)
                {
                        out << '&';
                }
                return out.str();
        }

	bool isIntLikeTypeName(const std::string &name);
	bool isUIntLikeTypeName(const std::string &name);
	int vectorDimension(const std::string &name);

        bool typeEquals(const TypeInfo &lhs, const TypeInfo &rhs)
        {
                return lhs.name == rhs.name && lhs.isConst == rhs.isConst && lhs.isReference == rhs.isReference &&
                       lhs.isArray == rhs.isArray && lhs.hasArraySize == rhs.hasArraySize && lhs.arraySize == rhs.arraySize;
        }

bool typeAssignable(TypeInfo dest, TypeInfo src)
{
	dest.isConst = false;
	src.isConst = false;
	if (typeEquals(dest, src))
	{
		return true;
	}
	if (dest.isReference != src.isReference || dest.isArray != src.isArray || dest.hasArraySize != src.hasArraySize ||
	    dest.arraySize != src.arraySize)
	{
		return false;
	}
	const bool destIntLike = isIntLikeTypeName(dest.name);
	const bool destUIntLike = isUIntLikeTypeName(dest.name);
	const bool srcIntLike = isIntLikeTypeName(src.name);
	const bool srcUIntLike = isUIntLikeTypeName(src.name);
	if ((destIntLike && srcUIntLike) || (destUIntLike && srcIntLike))
	{
		return vectorDimension(dest.name) == vectorDimension(src.name);
	}
	return false;
}

        TypeInfo stripReference(TypeInfo type)
        {
                type.isReference = false;
                return type;
        }

        const Token &expressionToken(const Expression &expression, const Token &fallback)
        {
			switch (expression.kind)
			{
				case Expression::Kind::Literal:
					return static_cast<const LiteralExpression &>(expression).literal;
				case Expression::Kind::ArrayLiteral:
					return static_cast<const ArrayLiteralExpression &>(expression).leftBrace;
				case Expression::Kind::Identifier:
				{
					const auto &identifier = static_cast<const IdentifierExpression &>(expression);
					if (!identifier.name.parts.empty())
                                {
                                        return identifier.name.parts.front();
                                }
                                break;
                        }
                        case Expression::Kind::Unary:
                        {
                                const auto &unary = static_cast<const UnaryExpression &>(expression);
                                if (unary.operand)
                                {
                                        return expressionToken(*unary.operand, fallback);
                                }
                                break;
                        }
                        case Expression::Kind::Binary:
                        {
                                const auto &binary = static_cast<const BinaryExpression &>(expression);
                                if (binary.left)
                                {
                                        return expressionToken(*binary.left, fallback);
                                }
                                if (binary.right)
                                {
                                        return expressionToken(*binary.right, fallback);
                                }
                                break;
                        }
                        case Expression::Kind::Assignment:
                        {
                                const auto &assign = static_cast<const AssignmentExpression &>(expression);
                                if (assign.target)
                                {
                                        return expressionToken(*assign.target, fallback);
                                }
                                break;
                        }
                        case Expression::Kind::Conditional:
                        {
                                const auto &conditional = static_cast<const ConditionalExpression &>(expression);
                                if (conditional.condition)
                                {
                                        return expressionToken(*conditional.condition, fallback);
                                }
                                break;
                        }
                        case Expression::Kind::Call:
                        {
                                const auto &call = static_cast<const CallExpression &>(expression);
                                if (call.callee)
                                {
                                        return expressionToken(*call.callee, fallback);
                                }
                                break;
                        }
                        case Expression::Kind::MemberAccess:
                                return static_cast<const MemberExpression &>(expression).member;
                        case Expression::Kind::IndexAccess:
                        {
                                const auto &index = static_cast<const IndexExpression &>(expression);
                                if (index.object)
                                {
                                        return expressionToken(*index.object, fallback);
                                }
                                break;
                        }
                        case Expression::Kind::Postfix:
                        {
                                const auto &postfix = static_cast<const PostfixExpression &>(expression);
                                if (postfix.operand)
                                {
                                        return expressionToken(*postfix.operand, fallback);
                                }
                                break;
                        }
                }
                return fallback;
        }

        const Token &tokenOrFallback(const Token &token, const Token &fallback)
        {
                if (!token.content.empty())
                {
                        return token;
                }
                return fallback;
        }

int componentIndex(char component)
        {
                switch (component)
                {
                        case 'x':
                        case 'r':
                                return 0;
                        case 'y':
                        case 'g':
                                return 1;
                        case 'z':
                        case 'b':
                                return 2;
                        case 'w':
                        case 'a':
                                return 3;
                        default:
                                break;
                }
                return -1;
        }

        struct SwizzleDescriptor
        {
                std::string scalarType;
                int dimension = 0;
                std::string vectorPrefix;
                std::string vectorSuffix;
                std::unordered_map<int, std::string> customResultTypes;
        };

        const std::unordered_map<std::string, SwizzleDescriptor> builtinSwizzleTypes = {
            {"Vector2", {"float", 2, "Vector", "", {}}},
            {"Vector3", {"float", 3, "Vector", "", {}}},
            {"Vector4", {"float", 4, "Vector", "", {}}},
            {"Vector2Int", {"int", 2, "Vector", "Int", {}}},
            {"Vector3Int", {"int", 3, "Vector", "Int", {}}},
            {"Vector4Int", {"int", 4, "Vector", "Int", {}}},
            {"Vector2UInt", {"uint", 2, "Vector", "UInt", {}}},
            {"Vector3UInt", {"uint", 3, "Vector", "UInt", {}}},
            {"Vector4UInt", {"uint", 4, "Vector", "UInt", {}}},
            {"Color", {"float", 4, "Vector", "", {{4, "Color"}}}},
        };

        std::optional<TypeInfo> resolveBuiltinFieldType(const std::string &typeName, const std::string &fieldName)
        {
                auto descriptorIt = builtinSwizzleTypes.find(typeName);
                if (descriptorIt == builtinSwizzleTypes.end() || fieldName.empty() || fieldName.size() > 4)
                {
                        return std::nullopt;
                }

                const SwizzleDescriptor &descriptor = descriptorIt->second;
                for (char component : fieldName)
                {
                        int index = componentIndex(component);
                        if (index < 0 || index >= descriptor.dimension)
                        {
                                return std::nullopt;
                        }
                }

                TypeInfo type;
                if (fieldName.size() == 1)
                {
                        type.name = descriptor.scalarType;
                        return type;
                }

                auto customResult = descriptor.customResultTypes.find(static_cast<int>(fieldName.size()));
                if (customResult != descriptor.customResultTypes.end())
                {
                        type.name = customResult->second;
                        return type;
                }

                type.name = descriptor.vectorPrefix + std::to_string(fieldName.size()) + descriptor.vectorSuffix;
                return type;
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
		return {};
	}

	bool isScalarTypeName(const std::string &name)
	{
		return name == "float" || name == "int" || name == "uint";
	}

	int vectorDimension(const std::string &name)
	{
		if (name == "Color")
		{
			return 4;
		}
		if (name.rfind("Vector", 0) != 0 || name.size() < 7)
		{
			return 0;
		}
		const char digit = name[6];
		if (!std::isdigit(static_cast<unsigned char>(digit)))
		{
			return 0;
		}
		return digit - '0';
	}

	bool parseMatrixTypeName(const std::string &name, int &columns, int &rows)
	{
		if (name.rfind("Matrix", 0) != 0)
		{
			return false;
		}
		std::size_t xPos = name.find('x', 6);
		if (xPos == std::string::npos || xPos + 1 >= name.size())
		{
			return false;
		}
		columns = std::stoi(name.substr(6, xPos - 6));
		rows = std::stoi(name.substr(xPos + 1));
		return columns > 0 && rows > 0;
	}

	bool isFloatTypeName(const std::string &name)
	{
		return name == "float";
	}

	bool isFloatVectorTypeName(const std::string &name)
	{
		return name == "Vector2" || name == "Vector3" || name == "Vector4";
	}

	bool isColorTypeName(const std::string &name)
	{
		return name == "Color";
	}

	bool isFloatVectorOrColorTypeName(const std::string &name)
	{
		return isFloatVectorTypeName(name) || isColorTypeName(name);
	}

	bool isIntVectorTypeName(const std::string &name)
	{
		return name == "Vector2Int" || name == "Vector3Int" || name == "Vector4Int";
	}

	bool isUIntVectorTypeName(const std::string &name)
	{
		return name == "Vector2UInt" || name == "Vector3UInt" || name == "Vector4UInt";
	}

	bool isFloatLikeTypeName(const std::string &name)
	{
		return isFloatTypeName(name) || isFloatVectorOrColorTypeName(name);
	}

	bool isIntLikeTypeName(const std::string &name)
	{
		return name == "int" || isIntVectorTypeName(name);
	}

	bool isUIntLikeTypeName(const std::string &name)
	{
		return name == "uint" || isUIntVectorTypeName(name);
	}

	bool isMatrixTypeName(const std::string &name)
	{
		int cols = 0;
		int rows = 0;
		return parseMatrixTypeName(name, cols, rows);
	}

	bool isArithmeticTypeName(const std::string &name)
	{
		return isScalarTypeName(name) || vectorDimension(name) != 0 || isMatrixTypeName(name) || name == "Color";
	}

	std::optional<TypeInfo> resolveBuiltinBinaryType(const TypeInfo &left, const TypeInfo &right, BinaryOperator op)
	{
		auto makeResult = [](const TypeInfo &prototype)
		{
			TypeInfo result = prototype;
			result.isReference = false;
			result.isConst = false;
			return result;
		};

		const bool leftScalar = isScalarTypeName(left.name);
		const bool rightScalar = isScalarTypeName(right.name);
		const int leftVectorDim = vectorDimension(left.name);
		const int rightVectorDim = vectorDimension(right.name);
		int leftCols = 0;
		int leftRows = 0;
		const bool leftMatrix = parseMatrixTypeName(left.name, leftCols, leftRows);
		int rightCols = 0;
		int rightRows = 0;
		const bool rightMatrix = parseMatrixTypeName(right.name, rightCols, rightRows);

		switch (op)
		{
			case BinaryOperator::Add:
			case BinaryOperator::Subtract:
				if (leftVectorDim > 0 && leftVectorDim == rightVectorDim)
				{
					return makeResult(left);
				}
				if (leftMatrix && rightMatrix && leftCols == rightCols && leftRows == rightRows)
				{
					return makeResult(left);
				}
				if (leftScalar && rightScalar)
				{
					return makeResult(left);
				}
				return std::nullopt;
			case BinaryOperator::Multiply:
				if (leftScalar && (rightVectorDim > 0 || rightMatrix || rightScalar))
				{
					return makeResult(right);
				}

				if (rightScalar && (leftVectorDim > 0 || leftMatrix || leftScalar))
				{
					return makeResult(left);
				}

				if (leftVectorDim > 0 && rightVectorDim > 0 && leftVectorDim == rightVectorDim)
				{
					return makeResult(left);
				}

				if (leftMatrix && rightVectorDim > 0 && leftCols == rightVectorDim)
				{
					return makeResult(right);
				}

				if (rightMatrix && leftVectorDim > 0 && rightRows == leftVectorDim)
				{
					return makeResult(left);
				}

				if (leftMatrix && rightMatrix && leftCols == rightRows)
				{
					return makeResult(left);
				}

				return std::nullopt;
		case BinaryOperator::Divide:
			if (leftVectorDim > 0 && rightScalar)
			{
				return makeResult(left);
			}
			if (leftScalar && rightScalar)
			{
				return makeResult(left);
			}
			if (leftScalar && rightVectorDim > 0)
			{
				return makeResult(right);
			}
			return std::nullopt;
		case BinaryOperator::Modulo:
		{
			const bool leftInt = left.name == "int";
			const bool rightInt = right.name == "int";
			const bool leftUInt = left.name == "uint";
			const bool rightUInt = right.name == "uint";
			if (leftScalar && rightScalar && (leftInt || leftUInt) && (rightInt || rightUInt))
			{
				if (leftUInt || rightUInt)
				{
					TypeInfo result;
					result.name = "uint";
					return makeResult(result);
				}
				return makeResult(left);
			}
			return std::nullopt;
		}
			case BinaryOperator::Less:
			case BinaryOperator::LessEqual:
			case BinaryOperator::Greater:
			case BinaryOperator::GreaterEqual:
			case BinaryOperator::Equal:
			case BinaryOperator::NotEqual:
				if (leftScalar && rightScalar)
				{
					return makeResult(left);
				}
				return std::nullopt;
			default:
				return std::nullopt;
		}
	}

	std::optional<TypeInfo> resolveElementWiseBinaryType(const TypeInfo &left, const TypeInfo &right)
	{
		const int leftVectorDim = vectorDimension(left.name);
		const int rightVectorDim = vectorDimension(right.name);

		if (leftVectorDim > 0 && rightVectorDim > 0 && leftVectorDim == rightVectorDim)
		{
			return left;
		}

		if (isScalarTypeName(left.name) && isScalarTypeName(right.name))
		{
			return left;
		}

		return std::nullopt;
	}


        bool baseTypeEquals(const TypeInfo &lhs, const TypeInfo &rhs)
        {
                return lhs.name == rhs.name && lhs.isArray == rhs.isArray && lhs.hasArraySize == rhs.hasArraySize &&
                       lhs.arraySize == rhs.arraySize &&
                       lhs.isConst == rhs.isConst;
        }

        bool isVoidType(const TypeInfo &type)
        {
                return type.name == "void" && !type.isReference && !type.isArray;
        }

        struct Analyzer
        {
                struct StageState
                {
                        bool defined = false;
                        Token token;
                };

                struct State
                {
                        std::unordered_map<std::string, Token> types;
                        std::unordered_map<std::string, AggregateInfo> aggregates;
                        std::unordered_map<std::string, std::vector<FunctionSignature>> functions;
                        std::unordered_map<std::string, Symbol> globals;
                        std::unordered_map<std::string, Symbol> pipelineVariables;
                        std::array<std::unordered_map<std::string, Symbol>, 4> stageBuiltins;
                        std::array<std::unordered_map<std::string, Symbol>, 4> stagePipeline;
			std::array<std::unordered_set<std::string>, 4> stageRequiredBuiltins;
                        StageState vertex;
                        StageState fragment;
                        std::vector<std::string> namespaceStack;
                };

                Analyzer()
                {
                        resetStageBuiltins();
                }

                std::unordered_set<std::string> builtinTypes = {
                    "void",  "bool",      "int",      "uint",      "float",    "Color",     "Texture",  "Vector2",
                    "Vector2Int",        "Vector2UInt",          "Vector3",   "Vector3Int", "Vector3UInt", "Vector4",
                    "Vector4Int",        "Vector4UInt",          "Matrix2x2", "Matrix3x3",  "Matrix4x4"};

                std::unordered_set<std::string> numericTypes = {
                    "int",  "uint", "float", "Vector2", "Vector2Int", "Vector2UInt", "Vector3", "Vector3Int",
                    "Vector3UInt", "Vector4", "Vector4Int", "Vector4UInt"};

                std::unordered_set<std::string> pipelineAllowedTypes = {"bool",        "int",      "uint",      "float",
                    "Color",       "Vector2",   "Vector2Int", "Vector2UInt", "Vector3",   "Vector3Int", "Vector3UInt",
                    "Vector4",     "Vector4Int", "Vector4UInt", "Matrix2x2",   "Matrix3x3",  "Matrix4x4"};

		State state;
		std::unordered_map<const Expression *, SemanticParseResult::ExpressionInfo> expressionInfo;

		SemanticParseResult operator()(std::vector<std::unique_ptr<Instruction>> instructions)
		{
			state = State{};
			expressionInfo.clear();
			resetStageBuiltins();
			registerBuiltinAggregates();

			SemanticParseResult result;
			result.instructions = std::move(instructions);

			collectTypes(result.instructions);
			collectDeclarations(result.instructions);

			state.namespaceStack.clear();

			for (const std::unique_ptr<Instruction> &instruction : result.instructions)
			{
				if (instruction)
				{
					analyzeInstruction(*instruction);
				}
			}

			finalize();
			result.expressionInfo = std::move(expressionInfo);
			return result;
		}

		private:
				void recordExpression(const Expression &expression, const TypedValue &value)
				{
						if (!value.type.valid())
						{
								return;
						}

						SemanticParseResult::ExpressionInfo info;
						TypeInfo base = stripReference(value.type);
						info.typeName = base.name;
						info.isConst = base.isConst;
						info.isReference = value.type.isReference;
						info.isArray = base.isArray;
						info.hasArraySize = base.hasArraySize;
						info.arraySize = base.arraySize;
						info.isLValue = value.isLValue;
						expressionInfo[&expression] = std::move(info);
				}

				const Token &textureBindingToken(const VariableDeclarator &declarator) const
				{
						if (!declarator.textureBindingToken.content.empty())
						{
								return declarator.textureBindingToken;
						}
						return declarator.name;
				}

		void resetStageBuiltins()
                {
                        for (auto &builtins : state.stageBuiltins)
                        {
                                builtins.clear();
                        }
			for (auto &required : state.stageRequiredBuiltins)
			{
				required.clear();
			}

                        Symbol pixelPosition;
                        pixelPosition.type = TypeInfo{"Vector4"};
                        pixelPosition.token = makeSyntheticStageToken(Stage::VertexPass);
                        state.stageBuiltins[stageIndex(Stage::VertexPass)]["pixelPosition"] = pixelPosition;
			state.stageRequiredBuiltins[stageIndex(Stage::VertexPass)].insert("pixelPosition");

			Symbol instanceId;
			instanceId.type = TypeInfo{"uint"};
			instanceId.token = makeSyntheticToken("InstanceID");
			state.stageBuiltins[stageIndex(Stage::VertexPass)]["InstanceID"] = instanceId;
			state.stageBuiltins[stageIndex(Stage::FragmentPass)]["InstanceID"] = instanceId;

			Symbol triangleId;
			triangleId.type = TypeInfo{"uint"};
			triangleId.token = makeSyntheticToken("TriangleID");
			state.stageBuiltins[stageIndex(Stage::VertexPass)]["TriangleID"] = triangleId;
			state.stageBuiltins[stageIndex(Stage::FragmentPass)]["TriangleID"] = triangleId;

		}

                void registerBuiltinAggregates()
                {
                        AggregateInfo textureInfo;
                        textureInfo.nameToken = makeSyntheticToken("Texture");
                        textureInfo.qualifiedName = "Texture";

                        FunctionSignature getPixel;
                        getPixel.nameToken = makeSyntheticToken("getPixel");
                        getPixel.returnType = TypeInfo{"Color"};
                        getPixel.displayName = "Texture::getPixel";
                        getPixel.isMethod = true;
                        getPixel.isConstMethod = true;

                        TypeInfo uvParam;
                        uvParam.name = "Vector2";
                        getPixel.parameters.push_back(uvParam);

                        textureInfo.methods["getPixel"].push_back(getPixel);

                        state.aggregates[textureInfo.qualifiedName] = std::move(textureInfo);
                }

                void collectTypes(const std::vector<std::unique_ptr<Instruction>> &instructions)
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
                                                registerAggregateType(static_cast<const AggregateInstruction &>(*instruction));
                                                break;
                                        case Instruction::Type::Namespace:
                                        {
                                                const auto &ns = static_cast<const NamespaceInstruction &>(*instruction);
                                                pushNamespace(ns.name);
                                                collectTypes(ns.instructions);
                                                popNamespace();
                                                break;
                                        }
                                        default:
                                                break;
                                }
                        }
                }

                void collectDeclarations(const std::vector<std::unique_ptr<Instruction>> &instructions)
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
                                                registerAggregateMembers(static_cast<const AggregateInstruction &>(*instruction));
                                                break;
                                        case Instruction::Type::Variable:
                                                registerVariable(static_cast<const VariableInstruction &>(*instruction));
                                                break;
                                        case Instruction::Type::Function:
                                                registerFunction(static_cast<const FunctionInstruction &>(*instruction));
                                                break;
                                        case Instruction::Type::StageFunction:
                                                registerStageFunction(static_cast<const StageFunctionInstruction &>(*instruction));
                                                break;
                                        case Instruction::Type::Pipeline:
                                                registerPipeline(static_cast<const PipelineInstruction &>(*instruction));
                                                break;
                                        case Instruction::Type::Namespace:
                                        {
                                                const auto &ns = static_cast<const NamespaceInstruction &>(*instruction);
                                                pushNamespace(ns.name);
                                                collectDeclarations(ns.instructions);
                                                popNamespace();
                                                break;
                                        }
                                }
                        }
                }

                void pushNamespace(const Token &name)
                {
                        state.namespaceStack.push_back(name.content);
                }

                void popNamespace()
                {
                        if (!state.namespaceStack.empty())
                        {
                                state.namespaceStack.pop_back();
                        }
                }

                std::string currentNamespace() const
                {
                        if (state.namespaceStack.empty())
                        {
                                return {};
                        }
                        std::string result;
                        for (std::size_t i = 0; i < state.namespaceStack.size(); ++i)
                        {
                                if (i > 0)
                                {
                                        result += "::";
                                }
                                result += state.namespaceStack[i];
                        }
                        return result;
                }

                std::vector<std::string> namespaceCandidates(const std::string &name) const
                {
                        std::vector<std::string> candidates;
                        const std::vector<std::string> &stack = state.namespaceStack;
                        for (std::size_t count = stack.size(); count > 0; --count)
                        {
                                std::string prefix;
                                for (std::size_t i = 0; i < count; ++i)
                                {
                                        if (i > 0)
                                        {
                                                prefix += "::";
                                        }
                                        prefix += stack[i];
                                }
                                candidates.push_back(prefix + "::" + name);
                        }
                        candidates.push_back(name);
                        return candidates;
                }

				std::vector<std::string> resolveQualifiedCandidates(const Name &name) const
				{
						if (name.parts.empty())
						{
								return {};
						}

						if (name.parts.size() > 1)
						{
								const std::string joined = joinName(name);
								const std::string current = currentNamespace();
								if (!current.empty() && joined.rfind(current + "::", 0) == 0)
								{
										return {joined};
								}
								return namespaceCandidates(joined);
						}

						return namespaceCandidates(name.parts.front().content);
				}


                void registerAggregateType(const AggregateInstruction &aggregate)
                {
                        const std::string qualified = qualify(aggregate.name);
                        if (qualified.find("::") == std::string::npos && builtinTypes.find(qualified) != builtinTypes.end())
                        {
                                emitError("Cannot redefine builtin type '" + qualified + "'", aggregate.name);
                                return;
                        }

                        const auto [it, inserted] = state.types.emplace(qualified, aggregate.name);
                        if (!inserted)
                        {
                                emitError("Type '" + qualified + "' already defined", aggregate.name);
                        }
                }

                void registerAggregateMembers(const AggregateInstruction &aggregate)
                {
                        const std::string qualified = qualify(aggregate.name);
                        AggregateInfo &info = state.aggregates[qualified];
                        info.nameToken = aggregate.name;
                        info.qualifiedName = qualified;
                        info.allowUnsizedArrays =
                            (aggregate.kind == AggregateInstruction::Kind::AttributeBlock ||
                                aggregate.kind == AggregateInstruction::Kind::ConstantBlock);

                        for (const std::unique_ptr<StructMember> &member : aggregate.members)
                        {
                                if (!member)
                                {
                                        continue;
                                }

                                switch (member->kind)
                                {
                                        case StructMember::Kind::Field:
                                                registerField(qualified, static_cast<const FieldMember &>(*member));
                                                break;
                                        case StructMember::Kind::Method:
                                                registerMethod(qualified, static_cast<const MethodMember &>(*member));
                                                break;
                                        case StructMember::Kind::Constructor:
                                                registerConstructor(qualified, static_cast<const ConstructorMember &>(*member));
                                                break;
                                        case StructMember::Kind::Operator:
                                                registerOperator(qualified, static_cast<const OperatorMember &>(*member));
                                                break;
                                }
                        }

                        if (!info.hasUserDefaultConstructor)
                        {
                                if (!info.hasExplicitConstructor)
                                {
                                        FunctionSignature defaultCtor;
                                        defaultCtor.nameToken = aggregate.name;
                                        defaultCtor.displayName = qualified + "()";
                                        defaultCtor.returnType = TypeInfo{qualified};
                                        info.constructors.push_back(defaultCtor);
                                }
                        }
                }

                void registerField(const std::string &aggregateName, const FieldMember &field)
                {
                        AggregateInfo &info = state.aggregates[aggregateName];
                        for (const VariableDeclarator &declarator : field.declaration.declarators)
                        {
                                TypeInfo type = resolveType(
                                    field.declaration.type, declarator.isReference, declarator.arraySize.get(), declarator.hasArraySuffix);
                                const bool unsizedArray = type.isArray && !type.hasArraySize;
                                if (unsizedArray && !info.allowUnsizedArrays)
                                {
                                        emitError("Unsized arrays are only allowed inside DataBlocks", declarator.name);
                                        continue;
                                }

                                if (declarator.hasTextureBinding && type.name != "Texture")
                                {
                                        emitError(
                                            "Only Texture declarations can use 'as constant' or 'as attribute'",
                                            textureBindingToken(declarator));
                                }

                                AggregateField entry;
                                entry.nameToken = declarator.name;
                                entry.type = type;
                                info.fields.emplace(declarator.name.content, entry);
                        }
                }

                void registerMethod(const std::string &aggregateName, const MethodMember &method)
                {
                        AggregateInfo &info = state.aggregates[aggregateName];
                        FunctionSignature signature;
                        signature.nameToken = method.name;
                        signature.returnType = resolveType(method.returnType, method.returnsReference, nullptr);
                        signature.returnsReference = method.returnsReference;
                        signature.displayName = aggregateName + "::" + method.name.content;
                        signature.isMethod = true;
                        signature.isConstMethod = method.isConst;
                        fillSignatureParameters(signature, method.parameters);

                        auto &overloads = info.methods[method.name.content];
                        enforceOverloadConsistency(overloads, signature);
                        overloads.push_back(signature);
                }

                void registerConstructor(const std::string &aggregateName, const ConstructorMember &constructor)
                {
                        AggregateInfo &info = state.aggregates[aggregateName];
                        FunctionSignature signature;
                        signature.nameToken = constructor.name;
                        signature.returnType = TypeInfo{aggregateName};
                        signature.displayName = aggregateName;
                        fillSignatureParameters(signature, constructor.parameters);

                        if (constructor.parameters.empty())
                        {
                                info.hasUserDefaultConstructor = true;
                        }
                        info.hasExplicitConstructor = true;

                        enforceOverloadConsistency(info.constructors, signature);
                        info.constructors.push_back(signature);
                }

                void registerOperator(const std::string &aggregateName, const OperatorMember &op)
                {
                        AggregateInfo &info = state.aggregates[aggregateName];
                        FunctionSignature signature;
                        signature.nameToken = op.symbol;
                        signature.returnType = resolveType(op.returnType, op.returnsReference, nullptr);
                        signature.returnsReference = op.returnsReference;
                        signature.displayName = aggregateName + "::operator" + op.symbol.content;
                        signature.isMethod = true;
                        fillSignatureParameters(signature, op.parameters);

                        std::string opName = "operator" + op.symbol.content;
                        auto &overloads = info.operators[opName];
                        enforceOverloadConsistency(overloads, signature);
                        overloads.push_back(signature);
                }

                void registerVariable(const VariableInstruction &variable)
                {
                        for (const VariableDeclarator &declarator : variable.declaration.declarators)
                        {
                                TypeInfo type = resolveType(
                                    variable.declaration.type, declarator.isReference, declarator.arraySize.get(), declarator.hasArraySuffix);
                                Symbol symbol;
                                symbol.token = declarator.name;
                                symbol.type = type;
                                symbol.isAssignable = !type.isConst;
                                const std::string qualified = qualify(declarator.name);
                                const auto [it, inserted] = state.globals.emplace(qualified, symbol);
                                if (!inserted)
                                {
                                        emitError("Variable '" + qualified + "' already defined", declarator.name);
                                }
                        }
                }

                void registerFunction(const FunctionInstruction &function)
                {
                        FunctionSignature signature;
                        signature.nameToken = function.name;
                        signature.returnType = resolveType(function.returnType, function.returnsReference, nullptr);
                        signature.returnsReference = function.returnsReference;
                        signature.displayName = qualify(function.name);
                        fillSignatureParameters(signature, function.parameters);

                        std::string qualified = qualify(function.name);
                        auto &overloads = state.functions[qualified];
                        enforceOverloadConsistency(overloads, signature);
                        overloads.push_back(signature);
                }

                void registerStageFunction(const StageFunctionInstruction &stageFunction)
                {
                        StageState &slot = (stageFunction.stage == Stage::VertexPass) ? state.vertex : state.fragment;
                        if (slot.defined)
                        {
                                std::string message = "Duplicate " + stageToString(stageFunction.stage) + "() definition";
                                emitError(message, stageFunction.stageToken);
                                return;
                        }

                        slot.defined = true;
                        slot.token = stageFunction.stageToken;
                }

                void registerPipeline(const PipelineInstruction &pipeline)
                {
                        if (!pipeline.payloadType.name.parts.empty())
                        {
                                TypeInfo payloadType = resolveType(pipeline.payloadType, false, nullptr);
                                if (payloadType.isArray && !payloadType.hasArraySize)
                                {
                                        emitError("Unsized arrays are only allowed inside DataBlocks",
                                            pipeline.payloadType.name.parts.front());
                                }
                                const std::string name = pipeline.variable.content;
                                Symbol symbol;
                                symbol.token = pipeline.variable;
                                symbol.type = payloadType;
                                symbol.isAssignable = !payloadType.isConst;
                                const auto [it, inserted] = state.pipelineVariables.emplace(name, symbol);
                                if (!inserted)
                                {
                                        emitError("Pipeline variable '" + name + "' already defined", pipeline.variable);
                                }

                                const std::size_t source = stageIndex(pipeline.source);
                                const std::size_t destination = stageIndex(pipeline.destination);
                                state.stagePipeline[source][name] = symbol;
                                state.stagePipeline[destination][name] = symbol;
				if (pipeline.source == Stage::VertexPass && pipeline.destination == Stage::FragmentPass)
				{
					state.stageRequiredBuiltins[stageIndex(Stage::VertexPass)].insert(name);
				}
				if (pipeline.source == Stage::FragmentPass && pipeline.destination == Stage::Output)
				{
					state.stageRequiredBuiltins[stageIndex(Stage::FragmentPass)].insert(name);
				}

                                if (!isAllowedPipelineType(payloadType.name))
                                {
                                        emitError(
                                            "Pipeline payload type must be a native scalar, vector, matrix, or Color",
                                            pipeline.payloadType.name.parts.front());
                                }
                                if (payloadType.name == "Texture")
                                {
                                        emitError("Textures cannot travel through the pipeline flow",
                                            pipeline.payloadType.name.parts.front());
                                }
                        }
                }

                std::string qualify(const Token &token) const
                {
                        const std::string ns = currentNamespace();
                        if (ns.empty())
                        {
                                return token.content;
                        }
                        return ns + "::" + token.content;
                }

                bool isBuiltinType(const std::string &name) const
                {
                        return builtinTypes.find(name) != builtinTypes.end();
                }

                bool isAllowedPipelineType(const std::string &name) const
                {
                        return pipelineAllowedTypes.find(name) != pipelineAllowedTypes.end();
                }

                bool isNumericType(const std::string &name) const
                {
                        return numericTypes.find(name) != numericTypes.end();
                }

                bool isBooleanType(const std::string &name) const
                {
                        return name == "bool";
                }

                bool canExplicitlyConvert(const TypeInfo &from, const std::string &to) const
                {
                        if (from.isArray)
                        {
                                return false;
                        }

                        TypeInfo base = stripReference(from);
                        if (base.name == to)
                        {
                                return true;
                        }

                        if (isNumericType(base.name) && isNumericType(to))
                        {
                                return true;
                        }

                        if (to == "bool" && isNumericType(base.name))
                        {
                                return true;
                        }

                        return false;
                }

                void ensureDefaultConstructorAvailable(const TypeInfo &type, const Token &token) const
                {
                        TypeInfo base = stripReference(type);
                        if (base.name.empty() || base.name == "Texture")
                        {
                                return;
                        }

                        auto it = state.aggregates.find(base.name);
                        if (it == state.aggregates.end())
                        {
                                return;
                        }

                        for (const FunctionSignature &signature : it->second.constructors)
                        {
                                if (signature.parameters.empty())
                                {
                                        return;
                                }
                        }

                        emitError("No default constructor available for type '" + base.name + "'", token);
                }

                TypeInfo resolveType(const TypeName &type, bool isReference, const Expression *arrayExpr,
                    bool hasArraySuffix = false)
                {
                        TypeInfo info;
                        info.isConst = type.isConst;
                        info.isReference = isReference;
                        if (!type.name.parts.empty())
                        {
                                info.name = resolveTypeName(type.name, type.name.parts.front());
                        }
                        if (hasArraySuffix)
                        {
                                info.isArray = true;
                                info.hasArraySize = (arrayExpr != nullptr);
                                if (arrayExpr)
                                {
                                        if (const auto *literal = dynamic_cast<const LiteralExpression *>(arrayExpr))
                                        {
                                                try
                                                {
                                                        info.arraySize = static_cast<std::size_t>(std::stoul(literal->literal.content));
                                                }
                                                catch (...)
                                                {
                                                        info.arraySize.reset();
                                                }
                                        }
                                        else
                                        {
                                                info.arraySize.reset();
                                        }
                                }
                        }
                        return info;
                }

                std::string resolveTypeName(const Name &typeName, const Token &errorToken)
                {
                        if (auto resolved = lookupTypeName(typeName))
                        {
                                return *resolved;
                        }

                        std::string unknown;
                        if (typeName.parts.empty())
                        {
                                unknown = "<anonymous>";
                        }
                        else if (typeName.parts.size() > 1)
                        {
                                unknown = joinName(typeName);
                        }
                        else
                        {
                                unknown = typeName.parts.front().content;
                        }

                        emitError("Unknown type '" + unknown + "'", errorToken);
                        return {};
                }

                void fillSignatureParameters(FunctionSignature &signature, const std::vector<Parameter> &parameters)
                {
                        signature.parameters.reserve(parameters.size());

                        std::ostringstream label;
                        label << signature.displayName << '(';

                        for (std::size_t i = 0; i < parameters.size(); ++i)
                        {
                                if (i > 0)
                                {
                                        label << ", ";
                                }
                                const Parameter &param = parameters[i];
                                TypeInfo type = resolveType(param.type, param.isReference, nullptr);
                                signature.parameters.push_back(type);
                                label << typeToString(type);
                        }

                        label << ')';
                        signature.displayName = label.str();
                }

                void enforceOverloadConsistency(std::vector<FunctionSignature> &existing, const FunctionSignature &candidate)
                {
                        for (const FunctionSignature &signature : existing)
                        {
                                if (!typeEquals(signature.returnType, candidate.returnType) ||
                                    signature.returnsReference != candidate.returnsReference)
                                {
                                        emitError("All overloads of '" + signature.displayName + "' must share the same return type",
                                            candidate.nameToken);
                                }
                                if (signature.parameters.size() == candidate.parameters.size())
                                {
                                        bool same = true;
                                        for (std::size_t i = 0; i < signature.parameters.size(); ++i)
                                        {
                                                if (!typeEquals(signature.parameters[i], candidate.parameters[i]))
                                                {
                                                        same = false;
                                                        break;
                                                }
                                        }
                                        if (same)
                                        {
                                                emitError("Duplicate overload of '" + signature.displayName + "'", candidate.nameToken);
                                        }
                                }
                        }
                }

                void finalize()
                {
                        if (!state.vertex.defined)
                        {
                                emitError("Missing VertexPass() stage function", makeSyntheticStageToken(Stage::VertexPass));
                        }
                        if (!state.fragment.defined)
                        {
                                emitError("Missing FragmentPass() stage function", makeSyntheticStageToken(Stage::FragmentPass));
                        }
                }

                struct Scope
                {
                        std::unordered_map<std::string, Symbol> symbols;
                };

		struct FunctionContext
		{
			std::vector<Scope> scopes;
			const AggregateInfo *aggregate = nullptr;
			bool methodConst = false;
                        TypeInfo returnType;
                        bool returnsReference = false;
			bool requiresValue = false;
			bool sawReturn = false;
			Token ownerToken;
			std::string displayName;
			bool inConstructor = false;
			std::unordered_map<std::string, bool> requiredBuiltins;
		};

		std::vector<std::string> collectFunctionSignatures(const std::string &qualifiedName) const;
		std::string formatArgumentTypes(const std::vector<std::unique_ptr<Expression>> &arguments,
		    FunctionContext &context);

                void analyzeInstruction(const Instruction &instruction)
                {
                        switch (instruction.type)
                        {
                                case Instruction::Type::Pipeline:
                                        analyzePipeline(static_cast<const PipelineInstruction &>(instruction));
                                        break;
                                case Instruction::Type::Variable:
                                        analyzeVariable(static_cast<const VariableInstruction &>(instruction));
                                        break;
                                case Instruction::Type::Function:
                                        analyzeFunction(static_cast<const FunctionInstruction &>(instruction));
                                        break;
                                case Instruction::Type::StageFunction:
                                        analyzeStageFunction(static_cast<const StageFunctionInstruction &>(instruction));
                                        break;
                                case Instruction::Type::Aggregate:
                                        analyzeAggregate(static_cast<const AggregateInstruction &>(instruction));
                                        break;
                                case Instruction::Type::Namespace:
                                        analyzeNamespace(static_cast<const NamespaceInstruction &>(instruction));
                                        break;
                        }
                }

                void analyzePipeline(const PipelineInstruction &pipeline)
                {
                        if (!currentNamespace().empty())
                        {
                                emitError("Pipeline declarations must be placed at the global scope", pipeline.sourceToken);
                        }

                        const bool isValidFlow = (pipeline.source == Stage::Input && pipeline.destination == Stage::VertexPass) ||
                                                 (pipeline.source == Stage::VertexPass && pipeline.destination == Stage::FragmentPass) ||
                                                 (pipeline.source == Stage::FragmentPass && pipeline.destination == Stage::Output);
                        if (!isValidFlow)
                        {
                                std::string message = "Invalid pipeline flow " + stageToString(pipeline.source) + " -> " +
                                                      stageToString(pipeline.destination);
                                emitError(message, pipeline.destinationToken);
                        }
                }

                void analyzeVariable(const VariableInstruction &variable)
                {
                        for (const VariableDeclarator &declarator : variable.declaration.declarators)
                        {
                                TypeInfo type = resolveType(
                                    variable.declaration.type, declarator.isReference, declarator.arraySize.get(), declarator.hasArraySuffix);
                                const bool typeValid = type.valid();
                                const bool isTexture = (type.name == "Texture");
                                const bool unsizedArray = typeValid && type.isArray && !type.hasArraySize;

                                if (declarator.hasTextureBinding && !isTexture)
                                {
                                        emitError(
                                            "Only Texture declarations can use 'as constant' or 'as attribute'",
                                            textureBindingToken(declarator));
                                }

                                if (unsizedArray)
                                {
                                        emitError("Unsized arrays are only allowed inside DataBlocks", declarator.name);
                                        continue;
                                }

                                if (typeValid && !isTexture && !declarator.initializer && !declarator.isReference)
                                {
                                        ensureDefaultConstructorAvailable(type, declarator.name);
                                }
                                if (declarator.initializer)
                                {
                                        FunctionContext context;
                                        pushScope(context);
                                        declareSymbol(context, declarator.name, type, !type.isConst);
                                        TypedValue value = evaluateExpression(*declarator.initializer, context, false);
                                        if (typeValid &&
                                            !typeAssignable(stripReference(type), stripReference(value.type)))
                                        {
                                                emitError("Cannot assign type '" + typeToString(value.type) + "' to variable '" +
                                                          declarator.name.content + "' of type '" + typeToString(type) + "'",
                                                    declarator.name);
                                        }
                                        popScope(context);
                                }
                        }
                }

                void analyzeFunction(const FunctionInstruction &function)
                {
                        FunctionContext context;
                        context.returnType = resolveType(function.returnType, function.returnsReference, nullptr);
                        context.returnsReference = function.returnsReference;
                        context.requiresValue = !isVoidType(context.returnType);
                        context.ownerToken = function.name;
                        context.displayName = qualify(function.name);

                        pushScope(context);
                        for (const Parameter &parameter : function.parameters)
                        {
                                TypeInfo type = resolveType(parameter.type, parameter.isReference, nullptr);
                                declareSymbol(context, parameter.name, type, !type.isConst);
                        }

                        if (function.body)
                        {
                                analyzeBlock(*function.body, context);
                        }

                        if (context.requiresValue && !context.sawReturn)
                        {
                                emitError("Function '" + context.displayName + "' must return a value", function.name);
                        }
                }

                void analyzeStageFunction(const StageFunctionInstruction &stageFunction)
                {
                        if (!currentNamespace().empty())
                        {
                                emitError("Stage functions must be declared in the global scope", stageFunction.stageToken);
                                return;
                        }

			FunctionContext context;
			context.returnType = TypeInfo{"void"};
			context.ownerToken = stageFunction.stageToken;
			context.displayName = stageToString(stageFunction.stage);
			const auto &requiredBuiltins = state.stageRequiredBuiltins[stageIndex(stageFunction.stage)];
			for (const std::string &name : requiredBuiltins)
			{
				context.requiredBuiltins[name] = false;
			}

			pushScope(context);

                        const auto &builtins = state.stageBuiltins[stageIndex(stageFunction.stage)];
                        for (const auto &[name, symbol] : builtins)
                        {
                                declareSymbol(context, symbol.token, symbol.type, false, &name);
                        }

                        const auto &pipelineSymbols = state.stagePipeline[stageIndex(stageFunction.stage)];
                        for (const auto &[name, symbol] : pipelineSymbols)
                        {
                                declareSymbol(context, symbol.token, symbol.type, !symbol.type.isConst, &name);
                        }

                        for (const auto &[name, symbol] : state.globals)
                        {
                                declareSymbol(context, symbol.token, symbol.type, !symbol.type.isConst, &name);
                        }

                        for (const Parameter &parameter : stageFunction.parameters)
                        {
                                TypeInfo type = resolveType(parameter.type, parameter.isReference, nullptr);
                                declareSymbol(context, parameter.name, type, !type.isConst);
                        }

			if (stageFunction.body)
			{
				analyzeBlock(*stageFunction.body, context);
			}

			for (const auto &[name, assigned] : context.requiredBuiltins)
			{
				if (!assigned)
				{
					emitError("Stage '" + stageToString(stageFunction.stage) + "' must set " + name, stageFunction.stageToken);
				}
			}
		}

                void analyzeAggregate(const AggregateInstruction &aggregate)
                {
                        const std::string qualified = qualify(aggregate.name);
                        AggregateInfo *info = nullptr;
                        auto it = state.aggregates.find(qualified);
                        if (it != state.aggregates.end())
                        {
                                info = &it->second;
                        }

                        for (const std::unique_ptr<StructMember> &member : aggregate.members)
                        {
                                if (!member)
                                {
                                        continue;
                                }

                                switch (member->kind)
                                {
                                        case StructMember::Kind::Field:
                                        {
                                                const auto &field = static_cast<const FieldMember &>(*member);
                                                TypeInfo type = resolveType(field.declaration.type, false, nullptr);
                                                if (type.name == "Texture")
                                                {
                                                        emitError(
                                                            "Textures cannot be declared inside struct fields",
                                                            field.declaration.type.name.parts.front());
                                                }
                                                break;
                                        }
                                        case StructMember::Kind::Method:
                                                analyzeMethod(qualified, info, static_cast<const MethodMember &>(*member));
                                                break;
                                        case StructMember::Kind::Constructor:
                                                analyzeConstructor(qualified, info, static_cast<const ConstructorMember &>(*member));
                                                break;
                                        case StructMember::Kind::Operator:
                                                analyzeOperator(qualified, info, static_cast<const OperatorMember &>(*member));
                                                break;
                                }
                        }

                        if (aggregate.kind == AggregateInstruction::Kind::AttributeBlock ||
                            aggregate.kind == AggregateInstruction::Kind::ConstantBlock)
                        {
                                Symbol symbol;
                                symbol.token = aggregate.name;
                                symbol.type = TypeInfo{qualified};
                                symbol.isAssignable = false;
                                state.globals[qualified] = symbol;
                        }
                }

                void analyzeNamespace(const NamespaceInstruction &ns)
                {
                        pushNamespace(ns.name);
                        for (const std::unique_ptr<Instruction> &child : ns.instructions)
                        {
                                if (child)
                                {
                                        analyzeInstruction(*child);
                                }
                        }
                        popNamespace();
                }

                void analyzeMethod(const std::string &qualifiedName, const AggregateInfo *info, const MethodMember &method)
                {
                        FunctionContext context;
                        context.aggregate = info;
                        context.methodConst = method.isConst;
                        context.returnType = resolveType(method.returnType, method.returnsReference, nullptr);
                        context.returnsReference = method.returnsReference;
                        context.requiresValue = !isVoidType(context.returnType);
                        context.ownerToken = method.name;
                        context.displayName = qualifiedName + "::" + method.name.content;

                        pushScope(context);

                        if (info)
                        {
                                static const std::string thisName = "this";
                                TypeInfo thisType;
                                thisType.name = qualifiedName;
                                thisType.isReference = true;
                                thisType.isConst = method.isConst;
                                declareSymbol(context, method.name, thisType, !method.isConst, &thisName);
                                for (const auto &[name, field] : info->fields)
                                {
                                        declareSymbol(context, field.nameToken, field.type, !field.type.isConst, &name);
                                }
                        }

                        for (const Parameter &parameter : method.parameters)
                        {
                                TypeInfo type = resolveType(parameter.type, parameter.isReference, nullptr);
                                declareSymbol(context, parameter.name, type, !type.isConst);
                        }

                        if (method.body)
                        {
                                analyzeBlock(*method.body, context);
                        }

                        if (context.requiresValue && !context.sawReturn)
                        {
                                emitError("Function '" + context.displayName + "' must return a value", method.name);
                        }
                }

                void analyzeConstructor(const std::string &qualifiedName, const AggregateInfo *info,
                    const ConstructorMember &constructor)
                {
                        FunctionContext context;
                        context.aggregate = info;
                        context.inConstructor = true;
                        context.returnType = TypeInfo{"void"};
                        context.ownerToken = constructor.name;
                        context.displayName = qualifiedName;

                        pushScope(context);

                        if (info)
                        {
                                static const std::string thisName = "this";
                                TypeInfo thisType;
                                thisType.name = qualifiedName;
                                thisType.isReference = true;
                                declareSymbol(context, constructor.name, thisType, true, &thisName);
                                for (const auto &[name, field] : info->fields)
                                {
                                        declareSymbol(context, field.nameToken, field.type, !field.type.isConst, &name);
                                }
                        }

                        for (const Parameter &parameter : constructor.parameters)
                        {
                                TypeInfo type = resolveType(parameter.type, parameter.isReference, nullptr);
                                declareSymbol(context, parameter.name, type, !type.isConst);
                        }

                        if (constructor.body)
                        {
                                analyzeBlock(*constructor.body, context);
                        }
                }

                void analyzeOperator(const std::string &qualifiedName, const AggregateInfo *info, const OperatorMember &op)
                {
                        FunctionContext context;
                        context.aggregate = info;
                        context.returnType = resolveType(op.returnType, op.returnsReference, nullptr);
                        context.returnsReference = op.returnsReference;
                        context.requiresValue = !isVoidType(context.returnType);
                        context.ownerToken = op.symbol;
                        context.displayName = qualifiedName + "::operator" + op.symbol.content;

                        pushScope(context);

                        if (info)
                        {
                                static const std::string thisName = "this";
                                TypeInfo thisType;
                                thisType.name = qualifiedName;
                                thisType.isReference = true;
                                declareSymbol(context, op.symbol, thisType, true, &thisName);
                                for (const auto &[name, field] : info->fields)
                                {
                                        declareSymbol(context, field.nameToken, field.type, !field.type.isConst, &name);
                                }
                        }

                        for (const Parameter &parameter : op.parameters)
                        {
                                TypeInfo type = resolveType(parameter.type, parameter.isReference, nullptr);
                                declareSymbol(context, parameter.name, type, !type.isConst);
                        }

                        if (op.body)
                        {
                                analyzeBlock(*op.body, context);
                        }

                        if (context.requiresValue && !context.sawReturn)
                        {
                                emitError("Function '" + context.displayName + "' must return a value", op.symbol);
                        }
                }

                void pushScope(FunctionContext &context)
                {
                        context.scopes.emplace_back();
                }

                void popScope(FunctionContext &context)
                {
                        if (!context.scopes.empty())
                        {
                                context.scopes.pop_back();
                        }
                }

                void declareSymbol(FunctionContext &context, const Token &name, const TypeInfo &type, bool assignable,
                    const std::string *overrideName = nullptr)
                {
                        if (context.scopes.empty())
                        {
                                context.scopes.emplace_back();
                        }

                        const std::string key = overrideName ? *overrideName : qualify(name);
                        for (const Scope &scope : context.scopes)
                        {
                                if (scope.symbols.find(key) != scope.symbols.end())
                                {
                                        const std::string display = overrideName ? *overrideName : name.content;
                                        emitError("Identifier '" + display + "' is already declared in this scope", name);
                                        return;
                                }
                        }

                        auto &symbols = context.scopes.back().symbols;
                        Symbol symbol;
                        symbol.token = name;
                        symbol.type = type;
                        symbol.isAssignable = assignable;
                        symbols.emplace(key, symbol);
                }

		Symbol *lookupSymbol(FunctionContext &context, const Name &name)
                {
                        if (name.parts.empty())
                        {
			return nullptr;
		}

                        if (name.parts.size() == 1)
                        {
                                const std::string simple = name.parts.front().content;
                                const std::string key = qualify(name.parts.front());
                                for (auto it = context.scopes.rbegin(); it != context.scopes.rend(); ++it)
                                {
                                        auto found = it->symbols.find(key);
                                        if (found != it->symbols.end())
                                        {
                                                return &found->second;
                                        }
                                        auto unqualified = it->symbols.find(simple);
                                        if (unqualified != it->symbols.end())
                                        {
                                                return &unqualified->second;
                                        }
                                }

                                const auto candidates = namespaceCandidates(simple);
                                for (const std::string &candidate : candidates)
                                {
                                        auto global = state.globals.find(candidate);
                                        if (global != state.globals.end())
                                        {
                                                return &global->second;
                                        }
                                }
                        }
                        else
                        {
                                const std::string joined = joinName(name);
                                auto global = state.globals.find(joined);
                                if (global != state.globals.end())
                                {
                                        return &global->second;
                                }
                        }

                        if (context.aggregate && name.parts.size() == 1)
                        {
                                auto field = context.aggregate->fields.find(name.parts.front().content);
                                if (field != context.aggregate->fields.end())
                                {
                                        static Symbol temp;
                                        temp.token = field->second.nameToken;
                                        temp.type = field->second.type;
                                        if (context.methodConst && !context.inConstructor)
                                        {
                                                temp.type.isConst = true;
                                        }
                                        temp.isAssignable = !temp.type.isConst;
                                        return &temp;
                                }
                        }

                        return nullptr;
                }

		void markStageBuiltinAssignment(FunctionContext &context, const Expression &target)
		{
			if (context.requiredBuiltins.empty() || target.kind != Expression::Kind::Identifier)
			{
				const Expression *current = &target;
				while (current)
				{
					if (current->kind == Expression::Kind::Identifier)
					{
						break;
					}
					if (current->kind == Expression::Kind::MemberAccess)
					{
						current = static_cast<const MemberExpression *>(current)->object.get();
						continue;
					}
					if (current->kind == Expression::Kind::IndexAccess)
					{
						current = static_cast<const IndexExpression *>(current)->object.get();
						continue;
					}
					return;
				}
				if (!current || current->kind != Expression::Kind::Identifier)
				{
					return;
				}
				const auto &identifier = static_cast<const IdentifierExpression &>(*current);
				if (identifier.name.parts.size() != 1)
				{
					return;
				}
				const std::string &name = identifier.name.parts.front().content;
				auto it = context.requiredBuiltins.find(name);
				if (it != context.requiredBuiltins.end())
				{
					it->second = true;
				}
				return;
			}

			const auto &identifier = static_cast<const IdentifierExpression &>(target);
			if (identifier.name.parts.size() != 1)
			{
				return;
			}

			const std::string &name = identifier.name.parts.front().content;
			auto it = context.requiredBuiltins.find(name);
			if (it != context.requiredBuiltins.end())
			{
				it->second = true;
			}
		}

                void analyzeBlock(const BlockStatement &block, FunctionContext &context)
                {
                        pushScope(context);
                        for (const std::unique_ptr<Statement> &statement : block.statements)
                        {
                                if (statement)
                                {
                                        analyzeStatement(*statement, context);
                                }
                        }
                        popScope(context);
                }

                void analyzeStatement(const Statement &statement, FunctionContext &context)
                {
                        switch (statement.kind)
                        {
                                case Statement::Kind::Block:
                                        analyzeBlock(static_cast<const BlockStatement &>(statement), context);
                                        break;
                                case Statement::Kind::Expression:
                                        if (const auto &expr = static_cast<const ExpressionStatement &>(statement).expression)
                                        {
                                                evaluateExpression(*expr, context, false);
                                        }
                                        break;
                                case Statement::Kind::Variable:
                                        analyzeVariableStatement(static_cast<const VariableStatement &>(statement), context);
                                        break;
                                case Statement::Kind::If:
                                        analyzeIf(static_cast<const IfStatement &>(statement), context);
                                        break;
                                case Statement::Kind::While:
                                        analyzeLoop(static_cast<const WhileStatement &>(statement).condition,
                                            static_cast<const WhileStatement &>(statement).body.get(), context);
                                        break;
                                case Statement::Kind::DoWhile:
                                        analyzeLoop(static_cast<const DoWhileStatement &>(statement).condition,
                                            static_cast<const DoWhileStatement &>(statement).body.get(), context);
                                        break;
                                case Statement::Kind::For:
                                        analyzeFor(static_cast<const ForStatement &>(statement), context);
                                        break;
                                case Statement::Kind::Return:
                                        analyzeReturn(static_cast<const ReturnStatement &>(statement), context);
                                        break;
                                default:
                                        break;
                        }
                }

                void analyzeVariableStatement(const VariableStatement &statement, FunctionContext &context)
                {
                        for (const VariableDeclarator &declarator : statement.declaration.declarators)
                        {
                                TypeInfo type = resolveType(
                                    statement.declaration.type, declarator.isReference, declarator.arraySize.get(), declarator.hasArraySuffix);
                                const bool typeValid = type.valid();
                                const bool unsizedArray = typeValid && type.isArray && !type.hasArraySize;
                                if (unsizedArray)
                                {
                                        emitError("Unsized arrays are only allowed inside DataBlocks", declarator.name);
                                        if (declarator.initializer)
                                        {
                                                evaluateExpression(*declarator.initializer, context, false);
                                        }
                                        continue;
                                }
                                if (declarator.hasTextureBinding && type.name != "Texture")
                                {
                                        emitError(
                                            "Only Texture declarations can use 'as constant' or 'as attribute'",
                                            textureBindingToken(declarator));
                                }
                                if (type.name == "Texture")
                                {
                                        emitError("Textures can only be declared at the global scope", declarator.name);
                                        if (declarator.initializer)
                                        {
                                                evaluateExpression(*declarator.initializer, context, false);
                                        }
                                        continue;
                                }

                                if (typeValid && !declarator.initializer && !declarator.isReference)
                                {
                                        ensureDefaultConstructorAvailable(type, declarator.name);
                                }

                                declareSymbol(context, declarator.name, type, !type.isConst);

                        if (typeValid && declarator.initializer)
                        {
                                TypedValue value = evaluateExpression(*declarator.initializer, context, false);
                                if (value.type.valid() &&
                                    !typeAssignable(stripReference(type), stripReference(value.type)))
                                {
                                        emitError("Cannot assign type '" + typeToString(value.type) + "' to variable '" +
                                                  declarator.name.content + "' of type '" + typeToString(type) + "'",
                                            declarator.name);
                                }
                        }
                        }
                }

                void analyzeIf(const IfStatement &statement, FunctionContext &context)
                {
                        if (statement.condition)
                        {
                                TypedValue condition = evaluateExpression(*statement.condition, context, false);
                                if (condition.type.valid() && !isBooleanType(stripReference(condition.type).name))
                                {
                                        emitError("If condition must be boolean", context.ownerToken);
                                }
                        }
                        if (statement.thenBranch)
                        {
                                analyzeStatement(*statement.thenBranch, context);
                        }
                        if (statement.elseBranch)
                        {
                                analyzeStatement(*statement.elseBranch, context);
                        }
                }

                void analyzeLoop(const std::unique_ptr<Expression> &condition, const Statement *body, FunctionContext &context)
                {
                        if (condition)
                        {
                                TypedValue value = evaluateExpression(*condition, context, false);
                                if (value.type.valid() && !isBooleanType(stripReference(value.type).name))
                                {
                                        emitError("Loop condition must be boolean", context.ownerToken);
                                }
                        }
                        if (body)
                        {
                                analyzeStatement(*body, context);
                        }
                }

                void analyzeFor(const ForStatement &statement, FunctionContext &context)
                {
                        pushScope(context);
                        if (statement.initializer)
                        {
                                analyzeStatement(*statement.initializer, context);
                        }
                        if (statement.condition)
                        {
                                TypedValue condition = evaluateExpression(*statement.condition, context, false);
                                if (condition.type.valid() && !isBooleanType(stripReference(condition.type).name))
                                {
                                        emitError("For-loop condition must be boolean", context.ownerToken);
                                }
                        }
                        if (statement.body)
                        {
                                analyzeStatement(*statement.body, context);
                        }
                        if (statement.increment)
                        {
                                evaluateExpression(*statement.increment, context, false);
                        }
                        popScope(context);
                }

                void analyzeReturn(const ReturnStatement &statement, FunctionContext &context)
                {
                        if (statement.value)
                        {
                                TypedValue value = evaluateExpression(*statement.value, context, false);
                                if (!value.type.valid())
                                {
                                        context.sawReturn = true;
                                        return;
                                }
                                if (context.inConstructor)
                                {
                                        emitError("Constructors may not return a value", context.ownerToken);
                                }
                                else if (isVoidType(context.returnType))
                                {
                                        emitError("Void functions may not return a value", context.ownerToken);
                                }
                                else if (!typeEquals(stripReference(context.returnType), stripReference(value.type)))
                                {
                                        emitError("Function '" + context.displayName + "' must return a value of type '" +
                                                      typeToString(context.returnType) + "'",
                                            context.ownerToken);
                                }
                                else if (!context.returnsReference && value.type.isReference)
                                {
                                        emitError("Function '" + context.displayName + "' cannot return a reference value",
                                            context.ownerToken);
                                }
                                else if (context.returnsReference && !value.isLValue)
                                {
                                        emitError("Function '" + context.displayName + "' must return a reference value",
                                            context.ownerToken);
                                }
                                else
                                {
                                        context.sawReturn = true;
                                }
                        }
                        else
                        {
                                if (!isVoidType(context.returnType) && !context.inConstructor)
                                {
                                        emitError("Function '" + context.displayName + "' must return a value", context.ownerToken);
                                }
                                context.sawReturn = true;
                        }
                }

		TypedValue evaluateExpression(const Expression &expression, FunctionContext &context, bool isCallee)
		{
			TypedValue value;
			switch (expression.kind)
			{
				case Expression::Kind::Literal:
					value = evaluateLiteral(static_cast<const LiteralExpression &>(expression));
					break;
				case Expression::Kind::ArrayLiteral:
					value = evaluateArrayLiteral(static_cast<const ArrayLiteralExpression &>(expression), context);
					break;
				case Expression::Kind::Identifier:
					value = evaluateIdentifier(static_cast<const IdentifierExpression &>(expression), context, isCallee);
					break;
				case Expression::Kind::Unary:
					value = evaluateUnary(static_cast<const UnaryExpression &>(expression), context);
					break;
				case Expression::Kind::Binary:
					value = evaluateBinary(static_cast<const BinaryExpression &>(expression), context);
					break;
				case Expression::Kind::Assignment:
					value = evaluateAssignment(static_cast<const AssignmentExpression &>(expression), context);
					break;
				case Expression::Kind::Conditional:
					value = evaluateConditional(static_cast<const ConditionalExpression &>(expression), context);
					break;
				case Expression::Kind::Call:
					value = evaluateCall(static_cast<const CallExpression &>(expression), context);
					break;
				case Expression::Kind::MemberAccess:
					value = evaluateMember(static_cast<const MemberExpression &>(expression), context);
					break;
				case Expression::Kind::IndexAccess:
					value = evaluateIndex(static_cast<const IndexExpression &>(expression), context);
					break;
				case Expression::Kind::Postfix:
					value = evaluatePostfix(static_cast<const PostfixExpression &>(expression), context);
					break;
			}
			recordExpression(expression, value);
			return value;
		}

		TypedValue evaluateLiteral(const LiteralExpression &literal)
		{
			const std::string &text = literal.literal.content;
			if (text.size() > 2 && text[0] == '0' && (text[1] == 'x' || text[1] == 'X'))
			{
				return {TypeInfo{"int"}, false};
			}
			if (text == "true" || text == "false")
			{
				return {TypeInfo{"bool"}, false};
			}
			if (text.find('"') != std::string::npos)
			{
				return {TypeInfo{"string"}, false};
			}
			const bool hasFloatMarker =
			    text.find('.') != std::string::npos || text.find('e') != std::string::npos || text.find('E') != std::string::npos;
			if (hasFloatMarker || (!text.empty() && (text.back() == 'f' || text.back() == 'F')))
			{
				return {TypeInfo{"float"}, false};
			}

			return {TypeInfo{"int"}, false};
		}

		TypedValue evaluateArrayLiteral(const ArrayLiteralExpression &literal, FunctionContext &context)
		{
			if (literal.elements.empty())
			{
				emitError("Array literal must have at least one element", literal.leftBrace);
				return {};
			}

			std::vector<TypedValue> elements;
			elements.reserve(literal.elements.size());
			for (const std::unique_ptr<Expression> &element : literal.elements)
			{
				if (element)
				{
					elements.push_back(evaluateExpression(*element, context, false));
				}
				else
				{
					elements.push_back({});
				}
			}

			TypeInfo common = stripReference(elements.front().type);
			common.isConst = false;
			common.isReference = false;
			if (!common.valid())
			{
				return {};
			}
			if (common.isArray)
			{
				emitError("Array literal elements cannot be arrays", literal.leftBrace);
				return {};
			}

			for (std::size_t i = 0; i < elements.size(); ++i)
			{
				TypeInfo current = stripReference(elements[i].type);
				current.isConst = false;
				current.isReference = false;
				if (!current.valid())
				{
					return {};
				}
				if (current.isArray)
				{
					const Token &token =
					    literal.elements[i] ? expressionToken(*literal.elements[i], literal.leftBrace) : literal.leftBrace;
					emitError("Array literal elements cannot be arrays", token);
					return {};
				}
				if (!typeEquals(common, current))
				{
					const Token &token =
					    literal.elements[i] ? expressionToken(*literal.elements[i], literal.leftBrace) : literal.leftBrace;
					emitError("Array literal elements must share the same type", token);
					return {};
				}
			}

			TypeInfo resultType = common;
			resultType.isArray = true;
			resultType.hasArraySize = true;
			resultType.arraySize = elements.size();
			resultType.isConst = false;
			resultType.isReference = false;
			TypedValue result;
			result.type = resultType;
			result.isLValue = false;
			return result;
		}

                TypedValue evaluateIdentifier(const IdentifierExpression &identifier, FunctionContext &context, bool isCallee)
                {
                        if (identifier.name.parts.empty())
                        {
                                return {};
                        }

                        if (identifier.name.parts.size() == 1 && identifier.name.parts.front().content == "this")
                        {
                                if (!context.aggregate)
                                {
                                        emitError("'this' can only be used inside aggregate methods", identifier.name.parts.front());
                                        return {};
                                }

                                TypeInfo thisType;
                                thisType.name = context.aggregate->qualifiedName;
                                thisType.isReference = true;
                                thisType.isConst = context.methodConst && !context.inConstructor;
                                return {thisType, true};
                        }

                        Symbol *symbol = lookupSymbol(context, identifier.name);
                        if (symbol)
                        {
                                return {symbol->type, true};
                        }

                        if (!isCallee)
                        {
                                emitError("Identifier '" + joinName(identifier.name) + "' is not declared", identifier.name.parts.front());
                        }
                        return {};
                }

                TypedValue evaluateUnary(const UnaryExpression &unary, FunctionContext &context)
                {
                        TypedValue operand = evaluateExpression(*unary.operand, context, false);
                        if (!operand.type.valid())
                        {
                                return operand;
                        }
                        const Token &operandToken =
                            unary.operand ? expressionToken(*unary.operand, context.ownerToken) : context.ownerToken;
                        TypeInfo base = stripReference(operand.type);
                        switch (unary.op)
                        {
                                case UnaryOperator::Positive:
                                case UnaryOperator::Negate:
                                case UnaryOperator::PreIncrement:
                                case UnaryOperator::PreDecrement:
                                        if (!isNumericType(base.name))
                                        {
                                                emitError("Unary numeric operator is not defined for type '" +
                                                              typeToString(operand.type) + "'",
                                                    operandToken);
                                        }
                                        break;
                                case UnaryOperator::LogicalNot:
                                        if (!isBooleanType(base.name))
                                        {
                                                emitError("Logical not requires a boolean operand", operandToken);
                                        }
                                        operand.type = TypeInfo{"bool"};
                                        break;
                                case UnaryOperator::BitwiseNot:
                                        if (!isNumericType(base.name))
                                        {
                                                emitError("Bitwise not requires a numeric operand", operandToken);
                                        }
                                        break;
                        }
                        operand.isLValue = false;
                        return operand;
                }

                std::optional<TypedValue> tryResolveUserOperator(BinaryOperator op, const TypedValue &left,
                    const TypedValue &right, const Token &token, bool &reportedError)
                {
                        reportedError = false;
                        if (!left.type.valid() || !right.type.valid())
                        {
                                return std::nullopt;
                        }

                        const std::string symbol = binaryOperatorSymbol(op);
                        if (symbol.empty())
                        {
                                return std::nullopt;
                        }

                        TypeInfo leftType = stripReference(left.type);
                        auto aggregateIt = state.aggregates.find(leftType.name);
                        if (aggregateIt == state.aggregates.end())
                        {
                                return std::nullopt;
                        }

                        std::string opName = "operator" + symbol;
                        auto overloadIt = aggregateIt->second.operators.find(opName);
                        if (overloadIt == aggregateIt->second.operators.end())
                        {
                                reportedError = true;
                                emitError("Operator '" + symbol + "' is not defined for type '" + aggregateIt->second.qualifiedName + "'",
                                    token);
                                return std::nullopt;
                        }

                        const bool objectConst = leftType.isConst;
                        for (const FunctionSignature &signature : overloadIt->second)
                        {
                                if (objectConst && !signature.isConstMethod)
                                {
                                        continue;
                                }
                                if (signature.parameters.size() != 1)
                                {
                                        continue;
                                }
                                const TypeInfo &parameter = signature.parameters.front();
                                if (parameter.isReference && !right.isLValue)
                                {
                                        continue;
                                }
                                if (!typeEquals(stripReference(parameter), stripReference(right.type)))
                                {
                                        continue;
                                }

                                TypedValue result;
                                result.type = signature.returnType;
                                result.isLValue = signature.returnsReference;
                                return result;
                        }

                        reportedError = true;
                        std::string displayName = aggregateIt->second.qualifiedName + "::operator" + symbol;
                        emitError("No overload of '" + displayName + "' matches provided arguments", token);
                        return std::nullopt;
                }

                std::optional<TypedValue> tryResolveCompoundAssignmentOperator(AssignmentOperator op, const TypedValue &target,
                    const TypedValue &value, const Token &token, bool &reportedError)
                {
                        reportedError = false;
                        if (!target.type.valid() || !value.type.valid())
                        {
                                return std::nullopt;
                        }

                        const std::string symbol = assignmentOperatorSymbol(op);
                        if (symbol.empty() || op == AssignmentOperator::Assign)
                        {
                                return std::nullopt;
                        }

                        TypeInfo targetType = stripReference(target.type);
                        auto aggregateIt = state.aggregates.find(targetType.name);
                        if (aggregateIt == state.aggregates.end())
                        {
                                return std::nullopt;
                        }

                        auto overloadIt = aggregateIt->second.operators.find("operator" + symbol);
                        if (overloadIt == aggregateIt->second.operators.end())
                        {
                                reportedError = true;
                                emitError("Operator '" + symbol + "' is not defined for type '" + aggregateIt->second.qualifiedName + "'",
                                    token);
                                return std::nullopt;
                        }

                        const bool targetConst = targetType.isConst;
                        for (const FunctionSignature &signature : overloadIt->second)
                        {
                                if (targetConst && !signature.isConstMethod)
                                {
                                        continue;
                                }
                                if (signature.parameters.size() != 1)
                                {
                                        continue;
                                }
                                const TypeInfo &parameter = signature.parameters.front();
                                if (parameter.isReference && !value.isLValue)
                                {
                                        continue;
                                }
                                if (!typeEquals(stripReference(parameter), stripReference(value.type)))
                                {
                                        continue;
                                }

                                TypedValue result;
                                result.type = signature.returnType;
                                result.isLValue = signature.returnsReference;
                                return result;
                        }

                        reportedError = true;
                        std::string displayName = aggregateIt->second.qualifiedName + "::operator" + symbol;
                        emitError("No overload of '" + displayName + "' matches provided arguments", token);
                        return std::nullopt;
                }

                TypedValue evaluateBinary(const BinaryExpression &binary, FunctionContext &context)
                {
                        TypedValue left = evaluateExpression(*binary.left, context, false);
                        TypedValue right = evaluateExpression(*binary.right, context, false);
                        if (!left.type.valid() || !right.type.valid())
                        {
                                return {};
                        }

                        const Token &binaryToken = tokenOrFallback(
                            binary.operatorToken,
                            binary.left ? expressionToken(*binary.left, context.ownerToken) : context.ownerToken);
                        bool operatorErrorReported = false;
                        if (auto userOperator =
                                tryResolveUserOperator(binary.op, left, right, binaryToken, operatorErrorReported))
                        {
                                return *userOperator;
                        }
                        if (operatorErrorReported)
                        {
                                return {};
                        }

                        TypeInfo leftBase = stripReference(left.type);
                        TypeInfo rightBase = stripReference(right.type);
                        leftBase.isConst = false;
                        rightBase.isConst = false;

			bool usedBuiltinResolution = false;
			TypeInfo resolvedType = leftBase;
			if (!typeEquals(leftBase, rightBase))
			{
				if (auto builtin = resolveBuiltinBinaryType(leftBase, rightBase, binary.op))
				{
					usedBuiltinResolution = true;
					resolvedType = *builtin;
				}
				else
				{
					emitError("Binary operands must share the same type", binaryToken);
				}
			}

			TypedValue result = left;
			result.type = resolvedType;
			result.isLValue = false;

			switch (binary.op)
			{
				case BinaryOperator::Add:
				case BinaryOperator::Subtract:
				case BinaryOperator::Multiply:
				case BinaryOperator::Divide:
				case BinaryOperator::Modulo:
					if (!isArithmeticTypeName(leftBase.name) || !isArithmeticTypeName(rightBase.name))
					{
						emitError("Arithmetic operators require homogenous operands", binaryToken);
					}
					break;
                                case BinaryOperator::Less:
                                case BinaryOperator::LessEqual:
                                case BinaryOperator::Greater:
                                case BinaryOperator::GreaterEqual:
                                        if (!isNumericType(leftBase.name))
                                        {
                                                emitError("Comparison operators require numeric operands", binaryToken);
                                        }
                                        result.type = TypeInfo{"bool"};
                                        break;
                                case BinaryOperator::Equal:
                                case BinaryOperator::NotEqual:
                                        result.type = TypeInfo{"bool"};
                                        break;
                                case BinaryOperator::LogicalAnd:
                                case BinaryOperator::LogicalOr:
                                        if (!isBooleanType(leftBase.name))
                                        {
                                                emitError("Logical operators require boolean operands", binaryToken);
                                        }
                                        result.type = TypeInfo{"bool"};
                                        break;
                                case BinaryOperator::BitwiseAnd:
                                case BinaryOperator::BitwiseOr:
                                case BinaryOperator::BitwiseXor:
                                        if (!isNumericType(leftBase.name))
                                        {
                                                emitError("Bitwise operators require numeric operands", binaryToken);
                                        }
                                        break;
                                case BinaryOperator::ShiftLeft:
                                case BinaryOperator::ShiftRight:
                                        if (!isIntLikeTypeName(leftBase.name) && !isUIntLikeTypeName(leftBase.name))
                                        {
                                                emitError("Shift operators require integer operands", binaryToken);
                                        }
                                        if (!isIntLikeTypeName(rightBase.name) && !isUIntLikeTypeName(rightBase.name))
                                        {
                                                emitError("Shift operators require integer operands", binaryToken);
                                        }
                                        break;
                        }

                        return result;
                }

                TypedValue evaluateAssignment(const AssignmentExpression &assignment, FunctionContext &context)
                {
                        TypedValue target = evaluateExpression(*assignment.target, context, false);
                        const Token &targetToken =
                            assignment.target ? expressionToken(*assignment.target, context.ownerToken) : context.ownerToken;
                        const Token &operatorToken = tokenOrFallback(assignment.operatorToken, targetToken);
                        if (!target.type.valid())
                        {
                                return {};
                        }
                        if (!target.isLValue)
                        {
                                emitError("Assignment target must be an lvalue", targetToken);
                        }
                        if (target.type.isConst)
                        {
                                emitError("Cannot assign to constant value", targetToken);
                        }
                        TypedValue value = evaluateExpression(*assignment.value, context, false);
                        if (!value.type.valid())
                        {
                                return {};
                        }
                        bool handledByUserOperator = false;
                        TypedValue userOperatorResult;
                        bool typeMismatch = false;
                        if (assignment.op != AssignmentOperator::Assign)
                        {
                                bool userOperatorError = false;
                                if (auto opResult = tryResolveCompoundAssignmentOperator(
                                        assignment.op, target, value, operatorToken, userOperatorError))
                                {
                                        handledByUserOperator = true;
                                        userOperatorResult = *opResult;
                                }
                                else if (userOperatorError)
                                {
                                        return {};
                                }
                        }

			if (!handledByUserOperator &&
			    !typeAssignable(stripReference(target.type), stripReference(value.type)))
			{
				emitError("Cannot assign type '" + typeToString(value.type) + "' to target of type '" +
				          typeToString(target.type) + "'",
                                    operatorToken);
                                typeMismatch = true;
                        }
                        if (!handledByUserOperator && !typeMismatch && assignment.op != AssignmentOperator::Assign)
                        {
                                TypeInfo base = stripReference(target.type);
				if (!isArithmeticTypeName(base.name))
			{
				emitError("Compound assignments require arithmetic operands", operatorToken);
			}
		}
		if (!typeMismatch && assignment.target)
		{
			markStageBuiltinAssignment(context, *assignment.target);
		}
		TypedValue result = handledByUserOperator ? userOperatorResult : target;
		result.isLValue = false;
		return result;
	}

                TypedValue evaluateConditional(const ConditionalExpression &conditional, FunctionContext &context)
                {
                        TypedValue condition = evaluateExpression(*conditional.condition, context, false);
                        if (!condition.type.valid())
                        {
                                return {};
                        }
                        if (!isBooleanType(stripReference(condition.type).name))
                        {
                                emitError("Conditional expression requires a boolean condition", context.ownerToken);
                        }
                        TypedValue thenValue = evaluateExpression(*conditional.thenBranch, context, false);
                        TypedValue elseValue = evaluateExpression(*conditional.elseBranch, context, false);
                        if (!thenValue.type.valid() || !elseValue.type.valid())
                        {
                                return {};
                        }
                        TypeInfo thenBase = stripReference(thenValue.type);
                        TypeInfo elseBase = stripReference(elseValue.type);
                        thenBase.isConst = false;
                        elseBase.isConst = false;
                        if (!typeAssignable(thenBase, elseBase))
                        {
                                emitError("Conditional branches must produce the same type", context.ownerToken);
                        }
                        TypedValue result = thenValue;
                        result.isLValue = false;
                        return result;
                }

                TypedValue evaluateCall(const CallExpression &call, FunctionContext &context)
                {
                        if (!call.callee)
                        {
                                return {};
                        }

                        if (call.callee->kind == Expression::Kind::Identifier)
                        {
                                return evaluateIdentifierCall(static_cast<const IdentifierExpression &>(*call.callee), call.arguments,
                                    context);
                        }
                        if (call.callee->kind == Expression::Kind::MemberAccess)
                        {
                                return evaluateMemberCall(static_cast<const MemberExpression &>(*call.callee), call.arguments, context);
                        }

                        TypedValue callee = evaluateExpression(*call.callee, context, true);
                        return callee;
                }

		bool resolveBuiltinFunctionCall(const IdentifierExpression &identifier,
		    const std::vector<std::unique_ptr<Expression>> &arguments, FunctionContext &context, TypedValue &result)
		{
			if (identifier.name.parts.size() != 1)
			{
				return false;
			}

			const Token &token = identifier.name.parts.front();
			const std::string name = token.content;

			const auto emitArgError = [&](const std::string &message) -> bool {
				emitError(message, token);
				result = {};
				return true;
			};

			std::vector<TypedValue> evaluatedArgs;
			evaluatedArgs.reserve(arguments.size());
			for (const std::unique_ptr<Expression> &argument : arguments)
			{
				if (argument)
				{
					evaluatedArgs.push_back(evaluateExpression(*argument, context, false));
				}
				else
				{
					evaluatedArgs.push_back({});
				}
			}

			const auto requireArgCount = [&](std::size_t expected) -> bool {
				if (evaluatedArgs.size() != expected)
				{
					return emitArgError(name + "() expects " + std::to_string(expected) + " argument" +
					    (expected == 1 ? "" : "s"));
				}
				return true;
			};

			const auto baseTypeName = [&](std::size_t index) -> std::string {
				if (index >= evaluatedArgs.size() || !evaluatedArgs[index].type.valid())
				{
					return {};
				}
				return stripReference(evaluatedArgs[index].type).name;
			};

			const auto sharedType = [&](std::initializer_list<std::size_t> indices) -> std::string {
				std::string candidate;
				for (std::size_t index : indices)
				{
					std::string typeName = baseTypeName(index);
					if (typeName.empty())
					{
						return {};
					}
					if (candidate.empty())
					{
						candidate = typeName;
					}
					else if (candidate != typeName)
					{
						return {};
					}
				}
				return candidate;
			};

			const auto setResult = [&](const std::string &typeName) {
				TypeInfo returnType;
				returnType.name = typeName;
				result.type = returnType;
				result.isLValue = false;
			};

			const auto handleBinarySameType = [&](bool allowFloat, bool allowInt, bool allowUInt) -> bool {
				if (!requireArgCount(2))
				{
					return true;
				}

				std::string typeName = sharedType({0, 1});
				if (typeName.empty())
				{
					return emitArgError(name + "() arguments must share the same type");
				}

				if ((allowFloat && isFloatLikeTypeName(typeName)) || (allowInt && isIntLikeTypeName(typeName)) ||
				    (allowUInt && isUIntLikeTypeName(typeName)))
				{
					setResult(typeName);
					return true;
				}

				return emitArgError(name + "() is not defined for type '" + typeName + "'");
			};

			const auto handleTernarySameType = [&](bool allowFloat, bool allowInt, bool allowUInt) -> bool {
				if (!requireArgCount(3))
				{
					return true;
				}

				std::string typeName = sharedType({0, 1, 2});
				if (typeName.empty())
				{
					return emitArgError(name + "() arguments must share the same type");
				}

				if ((allowFloat && isFloatLikeTypeName(typeName)) || (allowInt && isIntLikeTypeName(typeName)) ||
				    (allowUInt && isUIntLikeTypeName(typeName)))
				{
					setResult(typeName);
					return true;
				}

				return emitArgError(name + "() is not defined for type '" + typeName + "'");
			};

			if (name == "abs" || name == "sign")
			{
				if (!requireArgCount(1))
				{
					return true;
				}
				std::string typeName = baseTypeName(0);
				if (typeName.empty())
				{
					return true;
				}
				if (isFloatLikeTypeName(typeName) || isIntLikeTypeName(typeName))
				{
					setResult(typeName);
					return true;
				}
				return emitArgError(name + "() argument must be a numeric scalar or vector");
			}

			if (name == "floor" || name == "ceil" || name == "fract" || name == "exp" || name == "log" ||
			    name == "exp2" || name == "log2" || name == "sqrt" || name == "inversesqrt" || name == "sin" ||
			    name == "cos" || name == "tan" || name == "asin" || name == "acos" || name == "atan")
			{
				if (!requireArgCount(1))
				{
					return true;
				}
				std::string typeName = baseTypeName(0);
				if (typeName.empty())
				{
					return true;
				}
				if (isFloatLikeTypeName(typeName))
				{
					setResult(typeName);
					return true;
				}
				return emitArgError(name + "() argument must be float-based");
			}

			if (name == "mod")
			{
				return handleBinarySameType(true, true, true);
			}

			if (name == "min" || name == "max")
			{
				return handleBinarySameType(true, true, true);
			}

			if (name == "pow")
			{
				return handleBinarySameType(true, false, false);
			}

			if (name == "step")
			{
				if (!requireArgCount(2))
				{
					return true;
				}
				std::string typeName = sharedType({0, 1});
				if (typeName.empty())
				{
					return emitArgError("step() arguments must share the same type");
				}
				if (isFloatLikeTypeName(typeName))
				{
					setResult(typeName);
					return true;
				}
				return emitArgError("step() is only defined for float types");
			}

			if (name == "clamp")
			{
				return handleTernarySameType(true, true, true);
			}

			if (name == "smoothstep")
			{
				if (!requireArgCount(3))
				{
					return true;
				}
				std::string typeName = sharedType({0, 1, 2});
				if (typeName.empty())
				{
					return emitArgError("smoothstep() arguments must share the same type");
				}
				if (isFloatLikeTypeName(typeName))
				{
					setResult(typeName);
					return true;
				}
				return emitArgError("smoothstep() is only defined for float types");
			}

			if (name == "mix")
			{
				if (!requireArgCount(3))
				{
					return true;
				}

				std::string typeName = sharedType({0, 1});
				if (typeName.empty())
				{
					return emitArgError("mix() first two arguments must share the same type");
				}
				if (!isFloatLikeTypeName(typeName))
				{
					return emitArgError("mix() is only defined for float types");
				}

				const std::string factorType = baseTypeName(2);
				if (factorType.empty())
				{
					return true;
				}
				if (!isFloatTypeName(factorType))
				{
					return emitArgError("mix() third argument must be 'float'");
				}

				setResult(typeName);
				return true;
			}

			if (name == "dot")
			{
				if (!requireArgCount(2))
				{
					return true;
				}
				std::string typeName = sharedType({0, 1});
				if (typeName.empty())
				{
					return emitArgError("dot() arguments must share the same type");
				}
				if (!isFloatVectorOrColorTypeName(typeName))
				{
					return emitArgError("dot() requires float vector arguments");
				}

				TypeInfo returnType;
				returnType.name = "float";
				result.type = returnType;
				result.isLValue = false;
				return true;
			}

			if (name == "length")
			{
				if (!requireArgCount(1))
				{
					return true;
				}
				std::string typeName = baseTypeName(0);
				if (typeName.empty())
				{
					return true;
				}
				if (!isFloatVectorOrColorTypeName(typeName))
				{
					return emitArgError("length() requires a float vector argument");
				}

				TypeInfo returnType;
				returnType.name = "float";
				result.type = returnType;
				result.isLValue = false;
				return true;
			}

			if (name == "distance")
			{
				if (!requireArgCount(2))
				{
					return true;
				}
				std::string typeName = sharedType({0, 1});
				if (typeName.empty())
				{
					return emitArgError("distance() arguments must share the same type");
				}
				if (!isFloatVectorOrColorTypeName(typeName))
				{
					return emitArgError("distance() requires float vector arguments");
				}

				TypeInfo returnType;
				returnType.name = "float";
				result.type = returnType;
				result.isLValue = false;
				return true;
			}

			if (name == "normalize")
			{
				if (!requireArgCount(1))
				{
					return true;
				}
				std::string typeName = baseTypeName(0);
				if (typeName.empty())
				{
					return true;
				}
				if (!isFloatVectorOrColorTypeName(typeName))
				{
					return emitArgError("normalize() requires a float vector argument");
				}

				setResult(typeName);
				return true;
			}

			if (name == "cross")
			{
				if (!requireArgCount(2))
				{
					return true;
				}
				std::string typeName = sharedType({0, 1});
				if (typeName.empty())
				{
					return emitArgError("cross() arguments must share the same type");
				}
				if (typeName != "Vector3")
				{
					return emitArgError("cross() is only defined for 'Vector3'");
				}

				setResult("Vector3");
				return true;
			}

			if (name == "reflect")
			{
				if (!requireArgCount(2))
				{
					return true;
				}
				std::string typeName = sharedType({0, 1});
				if (typeName.empty())
				{
					return emitArgError("reflect() arguments must share the same type");
				}
				if (!isFloatVectorOrColorTypeName(typeName))
				{
					return emitArgError("reflect() requires float vector arguments");
				}

				setResult(typeName);
				return true;
			}

			return false;
		}

                TypedValue evaluateIdentifierCall(const IdentifierExpression &identifier,
                    const std::vector<std::unique_ptr<Expression>> &arguments, FunctionContext &context)
                {
                        if (identifier.name.parts.empty())
                        {
                                return {};
                        }

                        const std::string calleeName = joinName(identifier.name);
                        if (auto resolvedType = lookupTypeName(identifier.name))
                        {
                                return evaluateConstructorCall(*resolvedType, identifier.name.parts.front(), arguments, context);
                        }

                        std::vector<std::string> candidates = resolveQualifiedCandidates(identifier.name);
                        for (const std::string &candidate : candidates)
                        {
                                auto it = state.functions.find(candidate);
                                if (it != state.functions.end())
                                {
                                        return resolveCall(candidate, it->second, arguments, context, identifier.name.parts.front());
                                }
                        }

                        if (context.aggregate)
                        {
                                auto methodIt = context.aggregate->methods.find(identifier.name.parts.front().content);
                                if (methodIt != context.aggregate->methods.end())
                                {
                                        return resolveCall(calleeName, methodIt->second, arguments, context,
                                            identifier.name.parts.front(), context.methodConst);
                                }
                        }

			TypedValue builtinResult;
			if (resolveBuiltinFunctionCall(identifier, arguments, context, builtinResult))
			{
				return builtinResult;
			}

                        emitError("No overload of '" + calleeName + "' matches provided arguments",
                            identifier.name.parts.front());

                        const std::string qualifiedName = joinName(identifier.name);
                        const auto signatures = collectFunctionSignatures(qualifiedName);
                        if (!signatures.empty())
                        {
                                std::cout << "  Expected overloads:\n";
                                for (const std::string &sig : signatures)
                                {
                                        std::cout << "    " << sig << '\n';
                                }
                        }
                        else
                        {
                                std::cout << "  No overloads were defined for '" << qualifiedName << "'\n";
                        }

                        std::cout << "  Provided: " << formatArgumentTypes(arguments, context) << '\n';
                        return {};
                }

                std::optional<std::string> lookupTypeName(const Name &name) const
                {
                        if (name.parts.empty())
                        {
                                return std::nullopt;
                        }

                        const std::string joined = joinName(name);
                        if (name.parts.size() > 1)
                        {
                                if (isBuiltinType(joined) || state.types.find(joined) != state.types.end())
                                {
                                        return joined;
                                }
                                return std::nullopt;
                        }

                        const std::string simple = name.parts.front().content;
                        if (isBuiltinType(simple))
                        {
                                return simple;
                        }

                        std::vector<std::string> candidates = namespaceCandidates(simple);
                        for (const std::string &candidate : candidates)
                        {
                                if (state.types.find(candidate) != state.types.end())
                                {
                                        return candidate;
                                }
                        }
                        return std::nullopt;
                }

                bool isTypeName(const Name &name) const { return lookupTypeName(name).has_value(); }

                TypedValue evaluateConstructorCall(const std::string &typeName, const Token &token,
                    const std::vector<std::unique_ptr<Expression>> &arguments, FunctionContext &context)
                {
                        if (typeName.empty())
                        {
                                return {};
                        }

                        auto aggregateIt = state.aggregates.find(typeName);
                        if (aggregateIt == state.aggregates.end())
                        {
                                if (isBuiltinType(typeName))
                                {
                                        if (arguments.size() == 1 && arguments.front())
                                        {
                                                TypedValue value = evaluateExpression(*arguments.front(), context, false);
                                                if (!canExplicitlyConvert(value.type, typeName))
                                                {
                                                        emitError("Cannot convert type '" + typeToString(value.type) + "' to '" +
                                                                      typeName + "'",
                                                            token);
                                                }
                                        }
                                        else
                                        {
                                                for (const std::unique_ptr<Expression> &argument : arguments)
                                                {
                                                        if (argument)
                                                        {
                                                                evaluateExpression(*argument, context, false);
                                                        }
                                                }
                                        }
                                        TypedValue result;
                                        result.type = TypeInfo{typeName};
                                        result.isLValue = false;
                                        return result;
                                }
                                emitError("Unknown constructor '" + typeName + "'", token);
                                return {};
                        }
                        return resolveCall(typeName, aggregateIt->second.constructors, arguments, context, token);
                }

		bool resolveFloatBuiltinMethod(
		    const TypedValue &object,
		    const MemberExpression &member,
		    const std::vector<std::unique_ptr<Expression>> &arguments,
		    FunctionContext &context,
		    TypedValue &result)
		{
			TypeInfo base = stripReference(object.type);
			if (!isFloatTypeName(base.name))
			{
				return false;
			}

			std::vector<TypedValue> evaluatedArgs;
			evaluatedArgs.reserve(arguments.size());
			for (const std::unique_ptr<Expression> &argument : arguments)
			{
				if (argument)
				{
					evaluatedArgs.push_back(evaluateExpression(*argument, context, false));
				}
				else
				{
					evaluatedArgs.push_back({});
				}
			}

			const std::string methodName = member.member.content;
			const auto emitArgError = [&](const std::string &message) -> bool {
				emitError(message, member.member);
				result = {};
				return true;
			};

			const auto requireArgCount = [&](std::size_t expected) -> bool {
				if (evaluatedArgs.size() != expected)
				{
					return emitArgError(
					    methodName + "() expects " + std::to_string(expected) + " argument" +
					    (expected == 1 ? "" : "s"));
				}
				return true;
			};

			const auto isFloatArg = [&](std::size_t index) -> bool {
				if (index >= evaluatedArgs.size() || !evaluatedArgs[index].type.valid())
				{
					return false;
				}
				return stripReference(evaluatedArgs[index].type).name == "float";
			};

			const auto setResult = [&]() {
				TypeInfo returnType;
				returnType.name = "float";
				result.type = returnType;
				result.isLValue = false;
			};

			if (methodName == "abs" || methodName == "sign" || methodName == "floor" || methodName == "ceil" ||
			    methodName == "fract" || methodName == "exp" || methodName == "log" || methodName == "exp2" ||
			    methodName == "log2" || methodName == "sqrt" || methodName == "inversesqrt" ||
			    methodName == "sin" || methodName == "cos" || methodName == "tan" || methodName == "asin" ||
			    methodName == "acos" || methodName == "atan")
			{
				if (!requireArgCount(0))
				{
					return true;
				}

				setResult();
				return true;
			}

			if (methodName == "mod" || methodName == "min" || methodName == "max" || methodName == "pow")
			{
				if (!requireArgCount(1))
				{
					return true;
				}
				if (!isFloatArg(0))
				{
					return emitArgError(methodName + "() argument must be float");
				}
				setResult();
				return true;
			}

			if (methodName == "clamp")
			{
				if (!requireArgCount(2))
				{
					return true;
				}
				if (!isFloatArg(0) || !isFloatArg(1))
				{
					return emitArgError("clamp() arguments must be float");
				}
				setResult();
				return true;
			}

			if (methodName == "mix")
			{
				if (!requireArgCount(2))
				{
					return true;
				}
				if (!isFloatArg(0) || !isFloatArg(1))
				{
					return emitArgError("mix() arguments must be float");
				}
				setResult();
				return true;
			}

			if (methodName == "step")
			{
				if (!requireArgCount(1))
				{
					return true;
				}
				if (!isFloatArg(0))
				{
					return emitArgError("step() argument must be float");
				}
				setResult();
				return true;
			}

			if (methodName == "smoothstep")
			{
				if (!requireArgCount(2))
				{
					return true;
				}
				if (!isFloatArg(0) || !isFloatArg(1))
				{
					return emitArgError("smoothstep() arguments must be float");
				}
				setResult();
				return true;
			}

			return false;
		}

		bool resolveVectorBuiltinMethod(
		    const TypedValue &object,
		    const MemberExpression &member,
		    const std::vector<std::unique_ptr<Expression>> &arguments,
		    FunctionContext &context,
		    TypedValue &result)
		{
			TypeInfo base = stripReference(object.type);
			const std::string typeName = base.name;
			if (!isFloatVectorTypeName(typeName) && !isColorTypeName(typeName))
			{
				return false;
			}

			const auto descriptorIt = builtinSwizzleTypes.find(typeName);
			if (descriptorIt == builtinSwizzleTypes.end())
			{
				return false;
			}

			const SwizzleDescriptor &descriptor = descriptorIt->second;
			if (descriptor.scalarType != "float")
			{
				return false;
			}

			std::vector<TypedValue> evaluatedArgs;
			evaluatedArgs.reserve(arguments.size());
			for (const std::unique_ptr<Expression> &argument : arguments)
			{
				if (argument)
				{
					evaluatedArgs.push_back(evaluateExpression(*argument, context, false));
				}
				else
				{
					evaluatedArgs.push_back({});
				}
			}

			const std::string methodName = member.member.content;
			const auto emitArgError = [&](const std::string &message) -> bool {
				emitError(message, member.member);
				result = {};
				return true;
			};

			const auto requireArgCount = [&](std::size_t expected) -> bool {
				if (evaluatedArgs.size() != expected)
				{
					return emitArgError(
					    methodName + "() expects " + std::to_string(expected) + " argument" +
					    (expected == 1 ? "" : "s"));
				}
				return true;
			};

			const auto matchesBaseType = [&](std::size_t index) -> bool {
				if (index >= evaluatedArgs.size() || !evaluatedArgs[index].type.valid())
				{
					return false;
				}
				return stripReference(evaluatedArgs[index].type).name == typeName;
			};

			const auto isFloatArg = [&](std::size_t index) -> bool {
				if (index >= evaluatedArgs.size() || !evaluatedArgs[index].type.valid())
				{
					return false;
				}
				return stripReference(evaluatedArgs[index].type).name == "float";
			};

			TypeInfo vectorResult = base;
			vectorResult.isReference = false;
			vectorResult.isConst = false;

			if (methodName == "dot")
			{
				if (!requireArgCount(1))
				{
					return true;
				}

				if (!matchesBaseType(0))
				{
					return emitArgError("dot() argument must be of type '" + typeName + "'");
				}

				TypeInfo returnType;
				returnType.name = descriptor.scalarType;
				result.type = returnType;
				result.isLValue = false;
				return true;
			}

			if (methodName == "length" || methodName == "distance")
			{
				if (!requireArgCount(methodName == "length" ? 0 : 1))
				{
					return true;
				}

				if (methodName == "distance" && !matchesBaseType(0))
				{
					return emitArgError("distance() argument must be of type '" + typeName + "'");
				}

				TypeInfo returnType;
				returnType.name = "float";
				result.type = returnType;
				result.isLValue = false;
				return true;
			}

			if (methodName == "normalize")
			{
				if (!requireArgCount(0))
				{
					return true;
				}

				result.type = vectorResult;
				result.isLValue = false;
				return true;
			}

			if (methodName == "cross")
			{
				if (typeName != "Vector3")
				{
					return false;
				}
				if (!requireArgCount(1))
				{
					return true;
				}
				if (!matchesBaseType(0))
				{
					return emitArgError("cross() argument must be of type 'Vector3'");
				}

				result.type = vectorResult;
				result.isLValue = false;
				return true;
			}

			if (methodName == "reflect")
			{
				if (!requireArgCount(1))
				{
					return true;
				}
				if (!matchesBaseType(0))
				{
					return emitArgError("reflect() argument must be of type '" + typeName + "'");
				}

				result.type = vectorResult;
				result.isLValue = false;
				return true;
			}

			if (methodName == "abs" || methodName == "floor" || methodName == "ceil" || methodName == "fract" ||
			    methodName == "exp" || methodName == "log" || methodName == "exp2" || methodName == "log2" ||
			    methodName == "sqrt" || methodName == "inversesqrt" || methodName == "sin" || methodName == "cos" ||
			    methodName == "tan" || methodName == "asin" || methodName == "acos" || methodName == "atan")
			{
				if (!requireArgCount(0))
				{
					return true;
				}

				result.type = vectorResult;
				result.isLValue = false;
				return true;
			}

			if (methodName == "mod" || methodName == "min" || methodName == "max" || methodName == "pow")
			{
				if (!requireArgCount(1))
				{
					return true;
				}
				if (!matchesBaseType(0))
				{
					return emitArgError(methodName + "() argument must be of type '" + typeName + "'");
				}

				result.type = vectorResult;
				result.isLValue = false;
				return true;
			}

			if (methodName == "clamp")
			{
				if (!requireArgCount(2))
				{
					return true;
				}
				if (!matchesBaseType(0) || !matchesBaseType(1))
				{
					return emitArgError("clamp() arguments must be of type '" + typeName + "'");
				}

				result.type = vectorResult;
				result.isLValue = false;
				return true;
			}

			if (methodName == "lerp")
			{
				if (!requireArgCount(2))
				{
					return true;
				}
				if (!matchesBaseType(0) || !isFloatArg(1))
				{
					return emitArgError("lerp() arguments must be '" + typeName + "' and 'float'");
				}

				result.type = vectorResult;
				result.isLValue = false;
				return true;
			}

			if (methodName == "step")
			{
				if (!requireArgCount(1))
				{
					return true;
				}
				if (!matchesBaseType(0))
				{
					return emitArgError("step() argument must be of type '" + typeName + "'");
				}

				result.type = vectorResult;
				result.isLValue = false;
				return true;
			}

			if (methodName == "smoothstep")
			{
				if (!requireArgCount(2))
				{
					return true;
				}
				if (!matchesBaseType(0) || !matchesBaseType(1))
				{
					return emitArgError("smoothstep() arguments must be of type '" + typeName + "'");
				}

				result.type = vectorResult;
				result.isLValue = false;
				return true;
			}

			if (methodName == "saturate" && isColorTypeName(typeName))
			{
				if (!requireArgCount(0))
				{
					return true;
				}

				result.type = vectorResult;
				result.isLValue = false;
				return true;
			}

			return false;
		}

		bool resolveBuiltinMethod(
		    const TypedValue &object,
		    const MemberExpression &member,
		    const std::vector<std::unique_ptr<Expression>> &arguments,
		    FunctionContext &context,
		    TypedValue &result)
		{
			const std::string typeName = stripReference(object.type).name;
			if (isFloatTypeName(typeName))
			{
				return resolveFloatBuiltinMethod(object, member, arguments, context, result);
			}
			return resolveVectorBuiltinMethod(object, member, arguments, context, result);
		}

		TypedValue evaluateMemberCall(const MemberExpression &member,
		    const std::vector<std::unique_ptr<Expression>> &arguments, FunctionContext &context)
                {
                        TypedValue object = evaluateExpression(*member.object, context, false);
                        if (!object.type.valid())
                        {
                                return {};
                        }
                        const std::string typeName = stripReference(object.type).name;
		auto aggregateIt = state.aggregates.find(typeName);
		if (aggregateIt == state.aggregates.end())
		{
			TypedValue builtinResult;
			if (resolveBuiltinMethod(object, member, arguments, context, builtinResult))
			{
				return builtinResult;
			}

			emitError("Type '" + typeName + "' has no members", member.member);
			return {};
		}

                        std::string methodName = member.member.content;
                        auto methodIt = aggregateIt->second.methods.find(methodName);
                        if (methodIt == aggregateIt->second.methods.end())
                        {
                                emitError("Type '" + typeName + "' has no member named '" + methodName + "'", member.member);
                                return {};
                        }

                        const bool objectConst = stripReference(object.type).isConst;
                        return resolveCall(methodName, methodIt->second, arguments, context, member.member, objectConst);
                }

                TypedValue resolveCall(const std::string &name, const std::vector<FunctionSignature> &overloads,
                    const std::vector<std::unique_ptr<Expression>> &arguments, FunctionContext &context, const Token &token,
                    bool objectIsConst = false)
                {
                        std::vector<TypedValue> argumentTypes;
                        argumentTypes.reserve(arguments.size());
                        for (const std::unique_ptr<Expression> &argument : arguments)
                        {
                                if (argument)
                                {
                                        argumentTypes.push_back(evaluateExpression(*argument, context, false));
                                }
                                else
                                {
                                        argumentTypes.push_back({});
                                }
                        }

                        const FunctionSignature *match = nullptr;
                        for (const FunctionSignature &signature : overloads)
                        {
                                if (signature.parameters.size() != argumentTypes.size())
                                {
                                        continue;
                                }

                                if (signature.isMethod && objectIsConst && !signature.isConstMethod)
                                {
                                        continue;
                                }

                                bool compatible = true;
                                for (std::size_t i = 0; i < signature.parameters.size(); ++i)
                                {
                                        if (!argumentTypes[i].type.valid())
                                        {
                                                compatible = false;
                                                break;
                                        }
                                        if (!argumentTypes[i].isLValue && signature.parameters[i].isReference)
                                        {
                                                compatible = false;
                                                break;
                                        }
                                        if (!typeAssignable(stripReference(signature.parameters[i]),
                                                stripReference(argumentTypes[i].type)))
                                        {
                                                compatible = false;
                                                break;
                                        }
                                }

                                if (compatible)
                                {
                                        match = &signature;
                                        break;
                                }
                        }

		if (!match)
		{
			 std::ostringstream provided;
			 provided << "(";
			 for (std::size_t i = 0; i < argumentTypes.size(); ++i)
			 {
				 if (i > 0)
				 {
					 provided << ", ";
				 }
				 if (argumentTypes[i].type.valid())
				 {
					 provided << typeToString(stripReference(argumentTypes[i].type));
				 }
				 else
				 {
					 provided << "?";
				 }
			 }
			 provided << ")";

			 std::vector<std::string> candidates;
			 candidates.reserve(overloads.size());
			 for (const FunctionSignature &signature : overloads)
			 {
				 std::ostringstream candidate;
				 candidate << "(";
				 for (std::size_t i = 0; i < signature.parameters.size(); ++i)
				 {
					 if (i > 0)
					 {
						 candidate << ", ";
					 }
					 candidate << typeToString(signature.parameters[i]);
				 }
				 candidate << ")";
				 candidates.push_back(candidate.str());
			 }

			 emitError("No overload of '" + name + "' matches provided arguments", token);

			 if (!candidates.empty())
			 {
				 std::cout << "  Expected overloads:\n";
				 for (const std::string &candidate : candidates)
				 {
					 std::cout << "    " << candidate << '\n';
				 }
			 }
			 else
			 {
				 std::cout << "  No overloads were defined for '" << name << "'\n";
			 }

			 std::cout << "  Provided: " << provided.str() << '\n';
			 return {};
		}

		TypedValue result;
                        result.type = match->returnType;
                        result.isLValue = match->returnsReference;
                        return result;
                }

                TypedValue evaluateMember(const MemberExpression &member, FunctionContext &context)
                {
                        TypedValue object = evaluateExpression(*member.object, context, false);
                        if (!object.type.valid())
                        {
                                return {};
                        }
                        const TypeInfo objectType = stripReference(object.type);
			if (objectType.isArray && member.member.content == "size")
			{
				if (objectType.hasArraySize)
				{
					emitError("Array size is only available on unsized arrays", member.member);
					return {};
				}
				TypedValue value;
				value.type = TypeInfo{"uint"};
				if (objectType.isConst)
				{
					value.type.isConst = true;
				}
				value.isLValue = false;
				return value;
			}
                        const std::string typeName = objectType.name;
                        auto aggregateIt = state.aggregates.find(typeName);
                        if (aggregateIt != state.aggregates.end())
                        {
                                auto fieldIt = aggregateIt->second.fields.find(member.member.content);
                                if (fieldIt == aggregateIt->second.fields.end())
                                {
                                        emitError("Identifier '" + member.member.content + "' is not declared in this scope",
                                            member.member);
                                        return {};
                                }

                                TypedValue value;
                                value.type = fieldIt->second.type;
                                if (objectType.isConst)
                                {
                                        value.type.isConst = true;
                                }
                                value.isLValue = true;
                                return value;
                        }

                        std::optional<TypeInfo> builtinField = resolveBuiltinFieldType(typeName, member.member.content);
                        if (!builtinField)
                        {
                                emitError("Type '" + typeName + "' has no fields", member.member);
                                return {};
                        }

                        TypedValue value;
                        value.type = *builtinField;
                        if (objectType.isConst)
                        {
                                value.type.isConst = true;
                        }
                        value.isLValue = member.member.content.size() == 1;
                        return value;
                }


		TypedValue evaluateIndex(const IndexExpression &index, FunctionContext &context)
		{
			TypedValue object = evaluateExpression(*index.object, context, false);
			if (!object.type.isArray)
			{
				emitError("Index operator is only valid on arrays", context.ownerToken);
			}
			evaluateExpression(*index.index, context, false);
			object.type.isArray = false;
			object.type.hasArraySize = false;
			object.type.arraySize.reset();
			object.isLValue = true;
			return object;
		}

                TypedValue evaluatePostfix(const PostfixExpression &postfix, FunctionContext &context)
                {
                        TypedValue operand = evaluateExpression(*postfix.operand, context, false);
                        if (!operand.isLValue)
                        {
                                emitError("Postfix operator requires an lvalue", context.ownerToken);
                        }
                        if (!isNumericType(stripReference(operand.type).name))
                        {
                                emitError("Postfix operators require numeric operands", context.ownerToken);
                        }
                        return operand;
                }

        };

std::vector<std::string> Analyzer::collectFunctionSignatures(const std::string &qualifiedName) const
{
	std::vector<std::string> signatures;
	const auto appendSignatures = [&](const std::string &qualified, const std::vector<FunctionSignature> &overloads) {
		for (const FunctionSignature &signature : overloads)
		{
			std::ostringstream oss;
			oss << qualified << "(";
			for (std::size_t i = 0; i < signature.parameters.size(); ++i)
			{
				if (i > 0)
				{
					oss << ", ";
				}
				oss << typeToString(signature.parameters[i]);
			}
			oss << ")";
			signatures.push_back(oss.str());
		}
	};

	if (auto it = state.functions.find(qualifiedName); it != state.functions.end())
	{
		appendSignatures(qualifiedName, it->second);
	}

	if (!signatures.empty())
	{
		return signatures;
	}

	const std::size_t lastSeparator = qualifiedName.rfind("::");
	const std::string_view simple = (lastSeparator == std::string::npos) ?
	    std::string_view(qualifiedName) : std::string_view(qualifiedName).substr(lastSeparator + 2);

	for (const auto &[qualified, overloads] : state.functions)
	{
		if (overloads.empty())
		{
			continue;
		}

		std::string_view base = qualified;
		const std::size_t sep = qualified.rfind("::");
		if (sep != std::string::npos)
		{
			base.remove_prefix(sep + 2);
		}

		if (base != simple)
		{
			continue;
		}

		appendSignatures(qualified, overloads);
	}

	return signatures;
}

std::string Analyzer::formatArgumentTypes(const std::vector<std::unique_ptr<Expression>> &arguments,
    Analyzer::FunctionContext &context)
{
        std::ostringstream oss;
        oss << "(";
        for (std::size_t i = 0; i < arguments.size(); ++i)
        {
                if (i > 0)
                {
                        oss << ", ";
                }
                if (arguments[i])
                {
                        TypedValue value = evaluateExpression(*arguments[i], context, false);
                        if (value.type.valid())
                        {
                                oss << typeToString(stripReference(value.type));
                                continue;
                        }
                }
                oss << "?";
        }
        oss << ")";
        return oss.str();
}
}

SemanticParser::SemanticParser() = default;

SemanticParseResult SemanticParser::operator()(std::vector<std::unique_ptr<Instruction>> p_rawInstructions)
{
        Analyzer analyzer;
        return analyzer(std::move(p_rawInstructions));
}
