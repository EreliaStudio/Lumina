#pragma once

#include "lexer.hpp"

#include "shader_impl.hpp"

#include <list>
#include <deque>
#include <set>

namespace Lumina
{
	struct Parser
	{
	public:
		using Output = ShaderImpl;
		using Product = Lumina::Expected<Output>;

	private:
		struct Type;

		enum class ArithmeticOperator
		{
			Plus,
			Minus,
			Multiply,
			Divide,
			Modulo,
			ConditionEqual,
			NotEqual,
			Less,
			Greater,
			LessEqual,
			GreaterEqual,
			LogicalAnd,
			LogicalOr,
			Equal,
			PlusEqual,
			MinusEqual,
			MultiplyEqual,
			DivideEqual,
			ModuloEqual
		};

		enum class UnaryOperator
		{
			Increment,
			Decrement
		};

		enum class AssignatorOperator
		{
			Equal,
			PlusEqual,
			MinusEqual,
			MultiplyEqual,
			DivideEqual,
			ModuloEqual
		};

		struct Variable
		{
			const Type* type;
			std::string name;
			std::vector<size_t> arraySize;

			bool operator <(const Variable& p_other) const
			{
				return (name < p_other.name);
			}
		};

		struct ExpressionType
		{
			const Type* type;
			std::vector<size_t> arraySize;

			bool operator ==(const Variable& p_other) const
			{
				if ((type != p_other.type) ||
					(arraySize.size() != p_other.arraySize.size()))
				{
					return (false);
				}

				for (size_t i = 0; i < arraySize.size(); i++)
				{
					if (arraySize[i] != p_other.arraySize[i])
					{
						return (false);
					}
				}

				return (true);
			}
		};

		struct Parameter
		{
			const Type* type;
			bool isReference;
			std::string name;
			std::vector<size_t> arraySize;

			bool operator ==(const Parameter& p_other) const
			{
				if ((type != p_other.type) ||
					(arraySize.size() != p_other.arraySize.size()) ||
					(isReference != p_other.isReference))
				{
					return (false);
				}

				for (size_t i = 0; i < arraySize.size(); i++)
				{
					if (arraySize[i] != p_other.arraySize[i])
					{
						return (false);
					}
				}

				return (true);
			}
		};

		struct Expression
		{
			virtual ~Expression() = default;
		};

		struct Statement
		{
			virtual ~Statement() = default;
		};

		struct SymbolBody
		{
			std::vector<std::shared_ptr<Statement>> statements;
		};

		struct LiteralExpression : public Expression
		{
			std::variant<int, float, std::string, bool> value;
		};

		struct VariableExpression : public Expression
		{
			std::string variableName;
		};

		struct BinaryExpression : public Expression
		{
			std::shared_ptr<Expression> left;
			ArithmeticOperator op;
			std::shared_ptr<Expression> right;
		};

		struct UnaryExpression : public Expression
		{
			UnaryOperator op;
			std::shared_ptr<Expression> operand;
		};

		struct FunctionCallExpression : public Expression
		{
			std::string functionName;
			std::vector<std::shared_ptr<Expression>> arguments;
		};

		struct MemberAccessExpression : public Expression
		{
			std::shared_ptr<Expression> object;
			std::string memberName;
		};

		struct ArrayAccessExpression : public Expression
		{
			std::shared_ptr<Expression> array;
			std::shared_ptr<Expression> index;
		};

		struct VariableDeclarationStatement : public Statement
		{
			Variable variable;
			std::shared_ptr<Expression> initializer;
		};

		struct ExpressionStatement : public Statement
		{
			std::shared_ptr<Expression> expression;
		};

		struct AssignmentStatement : public Statement
		{
			std::shared_ptr<Expression> target;
			AssignatorOperator op;
			std::shared_ptr<Expression> value;
		};

		struct ReturnStatement : public Statement
		{
			std::shared_ptr<Expression> expression;
		};

		struct DiscardStatement : public Statement
		{
		};

		struct IfStatement : public Statement
		{
			struct ConditionalBranch
			{
				std::shared_ptr<Expression> condition;
				SymbolBody body;
			};

			std::vector<ConditionalBranch> branches;
			SymbolBody elseBody;
		};

		struct WhileStatement : public Statement
		{
			std::shared_ptr<Expression> condition;
			SymbolBody body;
		};

		struct ForStatement : public Statement
		{
			std::shared_ptr<Statement> initializer;
			std::shared_ptr<Expression> condition;
			std::shared_ptr<Expression> increment;
			SymbolBody body;
		};

		struct RaiseExceptionStatement : public Statement
		{

		};

		struct CompoundStatement : public Statement
		{
			SymbolBody body;
		};

		struct Function
		{
			ExpressionType returnType;
			std::string name;
			std::vector<Parameter> parameters;

			SymbolBody body;
		};

		struct Type
		{
			using Attribute = Variable;
			using Method = Function;
			using Operator = Function;

			struct Constructor
			{
				std::vector<Parameter> parameters;

				SymbolBody body;

				Constructor() = default;

				Constructor(const std::string& p_sourceCode);

				bool operator ==(const Constructor& p_other) const
				{
					if (parameters.size() != p_other.parameters.size())
					{
						return (false);
					}

					for (size_t i = 0; i < parameters.size(); i++)
					{
						if (parameters[i] != p_other.parameters[i])
						{
							return (false);
						}
					}

					return (true);
				}
			};

			std::string name;
			std::set<Attribute> attributes;

			std::vector<const Type*> acceptedConvertions;
			std::vector<Constructor> constructors;
			std::map<std::string, std::vector<Method>> methods;
			std::map<std::string, std::vector<Operator>> operators;

			operator std::string() const
			{
				return name;
			}
		};


		Product _product;

		std::set<std::string> _reservedIdentifiers;

		std::list<Type> _availibleTypes;
		std::vector<const Type*> _attributesTypes;
		std::vector<const Type*> _constantsTypes;
		std::map<std::string, std::vector<Function>> _availibleFunctions;

		std::set<Variable> _globalVariables;
		std::set<Variable> _vertexVariables;
		std::set<Variable> _fragmentVariables;

		std::deque<std::string> _nspaces;

		Function _vertexPassMain;
		Function _fragmentPassMain;

		Type::Constructor composeConstructor(const Type* p_originatorType, const std::string& p_sourceCode);
		Type::Operator composeOperator(const Type* p_originatorType, const std::string& p_sourceCode);
		Type::Method composeMethod(const Type* p_originatorType, const std::string& p_sourceCode);

		void composeStandardTypes();
		void composePredefinedTypes();
		void composeComplexStandardTypes();
		Parser();

		static inline ArithmeticOperator _stringToOperator(const std::string& opStr);
		static inline std::string _arithmeticOperatorToString(ArithmeticOperator op);
		static inline UnaryOperator _stringToUnaryOperator(const std::string& opStr);
		static inline std::string _unaryOperatorToString(UnaryOperator op);
		static inline AssignatorOperator _stringToAssignatorOperator(const std::string& opStr);
		static inline std::string _assignatorOperatorToString(AssignatorOperator op);

		std::string _composeTypeName(const TypeInfo& p_typeInfo);
		std::vector<size_t> _composeSizeArray(const ArraySizeInfo& p_arraySizeInfo);
		std::string currentNamespace();

		std::string _extractNameInfo(const NameInfo& p_nameInfo);

		Type* _insertType(const Type& p_inputType);
		const Type* _findType(const std::string& p_objectName);
		const Type* _findType(const TypeInfo& p_typeInfo);

		void _insertVariable(const Variable& p_variable);

		ExpressionType _composeExpressionType(const ExpressionTypeInfo& p_expressionType);

		Variable _composeVariable(const VariableInfo& p_variableInfo);
		Type _composeType(const BlockInfo& p_block, const std::string& p_suffix = "");

		SymbolBody _composeSymbolBody(const SymbolBodyInfo& p_symbolBodyInfo);
		std::shared_ptr<Statement> _composeStatement(const StatementInfo& p_statementInfo);
		std::shared_ptr<Expression> _composeExpression(const ExpressionInfo& p_expressionInfo);

		std::shared_ptr<Statement> _composeVariableDeclarationStatement(const VariableDeclarationStatementInfo& p_variableDeclarationStatementInfo);
		std::shared_ptr<Statement> _composeExpressionStatement(const ExpressionStatementInfo& p_expressionStatementInfo);
		std::shared_ptr<Statement> _composeAssignmentStatement(const AssignmentStatementInfo& p_assignmentStatementInfo);
		std::shared_ptr<Statement> _composeReturnStatement(const ReturnStatementInfo& p_returnStatementInfo);
		std::shared_ptr<Statement> _composeDiscardStatement(const DiscardStatementInfo& p_discardStatementInfo);
		std::shared_ptr<Statement> _composeIfStatement(const IfStatementInfo& p_ifStatementInfo);
		std::shared_ptr<Statement> _composeWhileStatement(const WhileStatementInfo& p_whileStatementInfo);
		std::shared_ptr<Statement> _composeForStatement(const ForStatementInfo& p_forStatementInfo);
		std::shared_ptr<Statement> _composeRaiseExceptionStatement(const RaiseExceptionStatementInfo& p_raiseExceptionStatementInfo);
		std::shared_ptr<Statement> _composeCompoundStatement(const CompoundStatementInfo& p_compoundStatementInfo);

		std::shared_ptr<Expression> _composeLiteralExpression(const LiteralExpressionInfo& p_literalExpressionInfo);
		std::shared_ptr<Expression> _composeVariableExpression(const VariableExpressionInfo& p_variableExpressionInfo);
		std::shared_ptr<Expression> _composeBinaryExpression(const BinaryExpressionInfo& p_binaryExpressionInfo);
		std::shared_ptr<Expression> _composeUnaryExpression(const UnaryExpressionInfo& p_unaryExpressionInfo);
		std::shared_ptr<Expression> _composeUnaryExpression(const PostfixExpressionInfo& p_postfixExpressionInfo);
		std::shared_ptr<Expression> _composeFunctionCallExpression(const FunctionCallExpressionInfo& p_functionCallExpressionInfo);
		std::shared_ptr<Expression> _composeMemberAccessExpression(const MemberAccessExpressionInfo& p_memberAccessExpressionInfo);
		std::shared_ptr<Expression> _composeArrayAccessExpression(const ArrayAccessExpressionInfo& p_arrayAccessExpressionInfo);

		Parameter _composeParameter(const ParameterInfo& p_parameterInfo);
		Function _composeMethodFunction(const Type* p_originatorType, const FunctionInfo& p_functionInfo);
		Type::Constructor _composeConstructorFunction(const Type* p_originatorType, const ConstructorInfo& p_constructorInfo);
		Function _composeOperatorFunction(const Type* p_originatorType, const OperatorInfo& p_operatorInfo);

		void _computeMethodAndOperator(Type* p_originator, const BlockInfo& p_block);

		void _parseStructure(const BlockInfo& p_blockInfo);
		void _parseAttribute(const BlockInfo& p_blockInfo);
		void _parseConstant(const BlockInfo& p_blockInfo);
		void _parseTexture(const TextureInfo& p_textureInfo);
		void _parseFunction(const FunctionInfo& p_functionInfo);
		void _parseNamespace(const NamespaceInfo& p_namespaceInfo);

		void _parsePipelineFlow(const PipelineFlowInfo& p_pipelineFlow);

		Function _composePipelinePass(const PipelinePassInfo& p_pipelinePass);
		void _parsePipelinePass(const PipelinePassInfo& p_pipelinePass);

		Product _parse(const Lexer::Output& p_input);

		void _printArraySizes(const std::vector<size_t>& arraySize) const;
		void _printParameters(const std::vector<Parameter>& parameters) const;
		void _printVariable(const Variable& var, const std::string& indent = "") const;
		void _printExpressionType(const ExpressionType& exprType) const;
		void _printMethods(const std::map<std::string, std::vector<Type::Method>>& methods, const std::string& title) const;
		void _printConstructors(const std::string& p_constructedType, const std::vector<Type::Constructor>& constructors) const;
		void _printConvertions(const std::string& p_constructedType, const std::vector<const Type*>& convertions) const;

		void _printSymbolBody(const SymbolBody& body, const std::string& indent = "") const;
		void _printStatement(const std::shared_ptr<Statement>& stmt, const std::string& indent) const;
		void _printExpression(const std::shared_ptr<Expression>& expr, const std::string& indent) const;

		void _printVariableDeclarationStatement(const VariableDeclarationStatement& stmt, const std::string& indent) const;
		void _printExpressionStatement(const ExpressionStatement& stmt, const std::string& indent) const;
		void _printAssignmentStatement(const AssignmentStatement& stmt, const std::string& indent) const;
		void _printReturnStatement(const ReturnStatement& stmt, const std::string& indent) const;
		void _printIfStatement(const IfStatement& stmt, const std::string& indent) const;
		void _printWhileStatement(const WhileStatement& stmt, const std::string& indent) const;
		void _printForStatement(const ForStatement& stmt, const std::string& indent) const;
		void _printCompoundStatement(const CompoundStatement& stmt, const std::string& indent) const;

		void _printLiteralExpression(const LiteralExpression& expr, const std::string& indent) const;
		void _printVariableExpression(const VariableExpression& expr, const std::string& indent) const;
		void _printBinaryExpression(const BinaryExpression& expr, const std::string& indent) const;
		void _printUnaryExpression(const UnaryExpression& expr, const std::string& indent) const;
		void _printFunctionCallExpression(const FunctionCallExpression& expr, const std::string& indent) const;
		void _printMemberAccessExpression(const MemberAccessExpression& expr, const std::string& indent) const;
		void _printArrayAccessExpression(const ArrayAccessExpression& expr, const std::string& indent) const;

		void _printParsedData() const;

	public:
		static Product parse(const Lexer::Output& p_input);
	};
}