#pragma once

#include <string>
#include <vector>
#include <set>
#include <map>
#include <memory>
#include <list>
#include <deque>
#include <variant>
#include <iostream>
#include <sstream>

namespace Lumina
{
	struct ShaderRepresentation
	{
		// Enums
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

		// Classes and Structures
		struct Type;

		struct Variable
		{
			const Type* type;
			std::string name;
			std::vector<size_t> arraySize;

			bool operator<(const Variable& other) const
			{
				return name < other.name;
			}
		};

		struct ExpressionType
		{
			const Type* type;
			std::vector<size_t> arraySize;

			bool operator==(const Variable& other) const
			{
				if ((type != other.type) ||
					(arraySize.size() != other.arraySize.size()))
				{
					return false;
				}

				for (size_t i = 0; i < arraySize.size(); i++)
				{
					if (arraySize[i] != other.arraySize[i])
					{
						return false;
					}
				}

				return true;
			}
		};

		struct Parameter
		{
			const Type* type;
			bool isReference;
			std::string name;
			std::vector<size_t> arraySize;

			bool operator==(const Parameter& other) const
			{
				if ((type != other.type) ||
					(arraySize.size() != other.arraySize.size()) ||
					(isReference != other.isReference))
				{
					return false;
				}

				for (size_t i = 0; i < arraySize.size(); i++)
				{
					if (arraySize[i] != other.arraySize[i])
					{
						return false;
					}
				}

				return true;
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

			std::string toString() const;
		};

		// Expression and Statement derived classes

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
			bool isPrototype;
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
				bool isPrototype;
				std::vector<Parameter> parameters;
				SymbolBody body;

				Constructor() = default;

				bool operator==(const Constructor& other) const
				{
					if (parameters.size() != other.parameters.size())
					{
						return false;
					}

					for (size_t i = 0; i < parameters.size(); i++)
					{
						if (parameters[i] != other.parameters[i])
						{
							return false;
						}
					}

					return true;
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

		// Data members
		std::set<std::string> reservedIdentifiers;
		std::list<Type> availableTypes;
		std::vector<const Type*> structureTypes;
		std::vector<const Type*> attributesTypes;
		std::vector<const Type*> constantsTypes;
		std::map<std::string, std::vector<Function>> availableFunctions;

		std::set<Variable> globalVariables;

		std::set<Variable> vertexVariables;
		std::set<Variable> fragmentVariables;
		std::set<Variable> outputVariables;

		Function vertexPassMain;
		Function fragmentPassMain;

		// Methods
		const Type* findType(const std::string& name) const
		{
			for (const auto& type : availableTypes)
			{
				if (type.name == name)
				{
					return &type;
				}
			}
			return nullptr;
		}

		Type* insertType(const Type& inputType)
		{
			availableTypes.push_back(inputType);
			
			reservedIdentifiers.insert(inputType.name);
			return &availableTypes.back();
		}

		void insertVariable(const Variable& variable)
		{
			globalVariables.insert(variable);
			reservedIdentifiers.insert(variable.name);
		}

		bool variableExists(const std::string& name) const
		{
			return reservedIdentifiers.find(name) != reservedIdentifiers.end();
		}

		bool typeExists(const std::string& name) const
		{
			return findType(name) != nullptr;
		}
	};

	std::ostream& operator<<(std::ostream& os, const ShaderRepresentation& shaderRepresentation);
}
