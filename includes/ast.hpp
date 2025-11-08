#pragma once

#include "token.hpp"

#include <memory>
#include <vector>

struct Name
{
	std::vector<Token> parts;
};

struct TypeName
{
	bool isConst = false;
	Name name;
};

struct Parameter
{
	TypeName type;
	Token name;
	bool isReference = false;
};

struct Expression
{
	enum class Kind
	{
		Literal,
		Identifier,
		Unary,
		Binary,
		Assignment,
		Conditional,
		Call,
		MemberAccess,
		IndexAccess,
		Postfix
	};

	explicit Expression(Kind k) : kind(k) {}
	virtual ~Expression() = default;

	Kind kind;
};

struct LiteralExpression final : public Expression
{
	LiteralExpression();

	Token literal;
};

struct IdentifierExpression final : public Expression
{
	IdentifierExpression();

	Name name;
};

enum class UnaryOperator
{
	Positive,
	Negate,
	LogicalNot,
	BitwiseNot,
	PreIncrement,
	PreDecrement
};

struct UnaryExpression final : public Expression
{
	UnaryExpression();

	UnaryOperator op;
	std::unique_ptr<Expression> operand;
};

enum class BinaryOperator
{
	Add,
	Subtract,
	Multiply,
	Divide,
	Modulo,
	Less,
	LessEqual,
	Greater,
	GreaterEqual,
	Equal,
	NotEqual,
	LogicalAnd,
	LogicalOr,
	BitwiseAnd,
	BitwiseOr,
	BitwiseXor
};

struct BinaryExpression final : public Expression
{
	BinaryExpression();

	Token operatorToken;
	BinaryOperator op;
	std::unique_ptr<Expression> left;
	std::unique_ptr<Expression> right;
};

enum class AssignmentOperator
{
	Assign,
	AddAssign,
	SubtractAssign,
	MultiplyAssign,
	DivideAssign,
	ModuloAssign,
	BitwiseAndAssign,
	BitwiseOrAssign,
	BitwiseXorAssign
};

struct AssignmentExpression final : public Expression
{
	AssignmentExpression();

	Token operatorToken;
	AssignmentOperator op;
	std::unique_ptr<Expression> target;
	std::unique_ptr<Expression> value;
};

struct ConditionalExpression final : public Expression
{
	ConditionalExpression();

	std::unique_ptr<Expression> condition;
	std::unique_ptr<Expression> thenBranch;
	std::unique_ptr<Expression> elseBranch;
};

struct CallExpression final : public Expression
{
	CallExpression();

	std::unique_ptr<Expression> callee;
	std::vector<std::unique_ptr<Expression>> arguments;
};

struct MemberExpression final : public Expression
{
	MemberExpression();

	std::unique_ptr<Expression> object;
	Token member;
};

struct IndexExpression final : public Expression
{
	IndexExpression();

	std::unique_ptr<Expression> object;
	std::unique_ptr<Expression> index;
};

enum class PostfixOperator
{
	Increment,
	Decrement
};

struct PostfixExpression final : public Expression
{
	PostfixExpression();

	PostfixOperator op;
	std::unique_ptr<Expression> operand;
};

struct VariableDeclarator
{
	Token name;
	bool isReference = false;
	bool hasArraySuffix = false;
	bool hasArraySize = false;
	std::unique_ptr<Expression> arraySize;
	std::unique_ptr<Expression> initializer;
};

struct VariableDeclaration
{
	TypeName type;
	std::vector<VariableDeclarator> declarators;
};

struct Statement
{
	enum class Kind
	{
		Block,
		Expression,
		Variable,
		If,
		While,
		DoWhile,
		For,
		Return,
		Break,
		Continue,
		Discard
	};

	explicit Statement(Kind k) : kind(k) {}
	virtual ~Statement() = default;

	Kind kind;
};

struct BlockStatement final : public Statement
{
	BlockStatement();

	std::vector<std::unique_ptr<Statement>> statements;
};

struct ExpressionStatement final : public Statement
{
	ExpressionStatement();

	std::unique_ptr<Expression> expression;
};

struct VariableStatement final : public Statement
{
	VariableStatement();

	VariableDeclaration declaration;
};

struct IfStatement final : public Statement
{
	IfStatement();

	std::unique_ptr<Expression> condition;
	std::unique_ptr<Statement> thenBranch;
	std::unique_ptr<Statement> elseBranch;
};

struct WhileStatement final : public Statement
{
	WhileStatement();

	std::unique_ptr<Expression> condition;
	std::unique_ptr<Statement> body;
};

struct DoWhileStatement final : public Statement
{
	DoWhileStatement();

	std::unique_ptr<Statement> body;
	std::unique_ptr<Expression> condition;
};

struct ForStatement final : public Statement
{
	ForStatement();

	std::unique_ptr<Statement> initializer;
	std::unique_ptr<Expression> condition;
	std::unique_ptr<Expression> increment;
	std::unique_ptr<Statement> body;
};

struct ReturnStatement final : public Statement
{
	ReturnStatement();

	std::unique_ptr<Expression> value;
};

struct BreakStatement final : public Statement
{
	BreakStatement();
};

struct ContinueStatement final : public Statement
{
	ContinueStatement();
};

struct DiscardStatement final : public Statement
{
	DiscardStatement();
};

struct StructMember
{
	enum class Kind
	{
		Field,
		Method,
		Constructor,
		Operator
	};

	explicit StructMember(Kind k) : kind(k) {}
	virtual ~StructMember() = default;

	Kind kind;
};

struct FieldMember final : public StructMember
{
	FieldMember();

	VariableDeclaration declaration;
};

struct MethodMember final : public StructMember
{
	MethodMember();

	TypeName returnType;
	Token name;
	std::vector<Parameter> parameters;
	std::unique_ptr<BlockStatement> body;
	bool returnsReference = false;
	bool isConst = false;
};

struct ConstructorMember final : public StructMember
{
	ConstructorMember();

	Token name;
	std::vector<Parameter> parameters;
	std::unique_ptr<BlockStatement> body;
};

struct OperatorMember final : public StructMember
{
	OperatorMember();

	TypeName returnType;
	Token symbol;
	std::vector<Parameter> parameters;
	std::unique_ptr<BlockStatement> body;
	bool returnsReference = false;
};

enum class Stage
{
	Input,
	VertexPass,
	FragmentPass,
	Output
};

struct Instruction
{
	enum class Type
	{
		Pipeline,
		Variable,
		Function,
		StageFunction,
		Aggregate,
		Namespace
	};

	explicit Instruction(Type t) : type(t) {}
	virtual ~Instruction() = default;

	Type type;
};

struct PipelineInstruction final : public Instruction
{
	PipelineInstruction();

	Token sourceToken;
	Stage source;
	Token destinationToken;
	Stage destination;
	TypeName payloadType;
	Token variable;
};

struct VariableInstruction final : public Instruction
{
	VariableInstruction();

	VariableDeclaration declaration;
};

struct FunctionInstruction final : public Instruction
{
	FunctionInstruction();

	TypeName returnType;
	Token name;
	std::vector<Parameter> parameters;
	std::unique_ptr<BlockStatement> body;
	bool returnsReference = false;
};

struct StageFunctionInstruction final : public Instruction
{
	StageFunctionInstruction();

	Token stageToken;
	Stage stage;
	std::vector<Parameter> parameters;
	std::unique_ptr<BlockStatement> body;
};

struct NamespaceInstruction final : public Instruction
{
	NamespaceInstruction();

	Token name;
	std::vector<std::unique_ptr<Instruction>> instructions;
};

struct AggregateInstruction final : public Instruction
{
	enum class Kind
	{
		Struct,
		AttributeBlock,
		ConstantBlock
	};

	explicit AggregateInstruction(Kind k);

	Kind kind;
	Token name;
	std::vector<std::unique_ptr<StructMember>> members;
};

inline LiteralExpression::LiteralExpression() : Expression(Kind::Literal) {}
inline IdentifierExpression::IdentifierExpression() : Expression(Kind::Identifier) {}
inline UnaryExpression::UnaryExpression() : Expression(Kind::Unary) {}
inline BinaryExpression::BinaryExpression() : Expression(Kind::Binary) {}
inline AssignmentExpression::AssignmentExpression() : Expression(Kind::Assignment) {}
inline ConditionalExpression::ConditionalExpression() : Expression(Kind::Conditional) {}
inline CallExpression::CallExpression() : Expression(Kind::Call) {}
inline MemberExpression::MemberExpression() : Expression(Kind::MemberAccess) {}
inline IndexExpression::IndexExpression() : Expression(Kind::IndexAccess) {}
inline PostfixExpression::PostfixExpression() : Expression(Kind::Postfix) {}

inline BlockStatement::BlockStatement() : Statement(Kind::Block) {}
inline ExpressionStatement::ExpressionStatement() : Statement(Kind::Expression) {}
inline VariableStatement::VariableStatement() : Statement(Kind::Variable) {}
inline IfStatement::IfStatement() : Statement(Kind::If) {}
inline WhileStatement::WhileStatement() : Statement(Kind::While) {}
inline DoWhileStatement::DoWhileStatement() : Statement(Kind::DoWhile) {}
inline ForStatement::ForStatement() : Statement(Kind::For) {}
inline ReturnStatement::ReturnStatement() : Statement(Kind::Return) {}
inline BreakStatement::BreakStatement() : Statement(Kind::Break) {}
inline ContinueStatement::ContinueStatement() : Statement(Kind::Continue) {}
inline DiscardStatement::DiscardStatement() : Statement(Kind::Discard) {}

inline FieldMember::FieldMember() : StructMember(Kind::Field) {}
inline MethodMember::MethodMember() : StructMember(Kind::Method) {}
inline ConstructorMember::ConstructorMember() : StructMember(Kind::Constructor) {}
inline OperatorMember::OperatorMember() : StructMember(Kind::Operator) {}

inline PipelineInstruction::PipelineInstruction() : Instruction(Type::Pipeline) {}
inline VariableInstruction::VariableInstruction() : Instruction(Type::Variable) {}
inline FunctionInstruction::FunctionInstruction() : Instruction(Type::Function) {}
inline StageFunctionInstruction::StageFunctionInstruction() : Instruction(Type::StageFunction) {}
inline NamespaceInstruction::NamespaceInstruction() : Instruction(Type::Namespace) {}
inline AggregateInstruction::AggregateInstruction(Kind k) : Instruction(Type::Aggregate), kind(k) {}
