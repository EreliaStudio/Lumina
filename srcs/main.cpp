#include "compiler.hpp"
#include "parser.hpp"
#include "semantic_parser.hpp"
#include "source_manager.hpp"
#include "token.hpp"

#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
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
			return "<anonymous>";
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
		return "UnknownStage";
	}

	std::string indentString(std::size_t indent);
	std::string expressionToString(const Expression &expression);
	void printStatementSummary(const Statement &statement, std::size_t indent);
	void printBlockSummary(const BlockStatement &block, std::size_t indent);

	std::string aggregateKindToString(AggregateInstruction::Kind kind)
	{
		switch (kind)
		{
			case AggregateInstruction::Kind::Struct:
				return "Struct";
			case AggregateInstruction::Kind::AttributeBlock:
				return "DataBlock(attribute)";
			case AggregateInstruction::Kind::ConstantBlock:
				return "DataBlock(constant)";
		}
		return "Aggregate";
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

	std::string formatParameter(const Parameter &parameter)
	{
		std::string text = formatTypeName(parameter.type);
		if (parameter.isReference)
		{
			text += " &";
		}
		text += " ";
		text += safeTokenContent(parameter.name);
		return text;
	}

	std::string formatParameters(const std::vector<Parameter> &parameters)
	{
		std::ostringstream oss;
		for (std::size_t i = 0; i < parameters.size(); ++i)
		{
			if (i > 0)
			{
				oss << ", ";
			}
			oss << formatParameter(parameters[i]);
		}
		return oss.str();
	}

	std::string formatVariableDeclarator(const VariableDeclarator &declarator, bool verbose = false)
	{
		std::string text;
		if (declarator.isReference)
		{
			text += "& ";
		}
		text += safeTokenContent(declarator.name);
		if (declarator.hasArraySuffix)
		{
			text += "[";
			if (declarator.arraySize)
			{
				text += expressionToString(*declarator.arraySize);
			}
			else
			{
				text += verbose ? "<dynamic>" : "dynamic";
			}
			text += "]";
		}
		if (declarator.initializer)
		{
			text += " = ";
			if (verbose && declarator.initializer)
			{
				text += expressionToString(*declarator.initializer);
			}
			else
			{
				text += "<expr>";
			}
		}
		if (declarator.hasTextureBinding)
		{
			text += " as ";
			text += (declarator.textureBindingScope == TextureBindingScope::Attribute) ? "attribute" : "constant";
		}
		return text;
	}

	std::string formatDeclarators(const std::vector<VariableDeclarator> &declarators, bool verbose = false)
	{
		std::ostringstream oss;
		for (std::size_t i = 0; i < declarators.size(); ++i)
		{
			if (i > 0)
			{
				oss << ", ";
			}
			oss << formatVariableDeclarator(declarators[i], verbose);
		}
		return oss.str();
	}

	std::string assignmentOperatorToString(AssignmentOperator op)
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

	std::string binaryOperatorToString(BinaryOperator op)
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
		return "?";
	}

	std::string unaryOperatorToString(UnaryOperator op)
	{
		switch (op)
		{
			case UnaryOperator::Positive:
				return "+";
			case UnaryOperator::Negate:
				return "-";
			case UnaryOperator::LogicalNot:
				return "!";
			case UnaryOperator::BitwiseNot:
				return "~";
			case UnaryOperator::PreIncrement:
				return "++";
			case UnaryOperator::PreDecrement:
				return "--";
		}
		return "";
	}

	std::string postfixOperatorToString(PostfixOperator op)
	{
		switch (op)
		{
			case PostfixOperator::Increment:
				return "++";
			case PostfixOperator::Decrement:
				return "--";
		}
		return "";
	}

	std::string expressionToString(const Expression &expression)
	{
		switch (expression.kind)
		{
			case Expression::Kind::Literal:
			{
				const auto &literal = static_cast<const LiteralExpression &>(expression);
				return literal.literal.content.empty() ? "<literal>" : literal.literal.content;
			}
			case Expression::Kind::ArrayLiteral:
			{
				const auto &literal = static_cast<const ArrayLiteralExpression &>(expression);
				std::string text = "{";
				for (std::size_t i = 0; i < literal.elements.size(); ++i)
				{
					if (i > 0)
					{
						text += ", ";
					}
					text += literal.elements[i] ? expressionToString(*literal.elements[i]) : "<expr>";
				}
				text += "}";
				return text;
			}
			case Expression::Kind::Identifier:
			{
				const auto &identifier = static_cast<const IdentifierExpression &>(expression);
				return formatName(identifier.name);
			}
			case Expression::Kind::Unary:
			{
				const auto &unary = static_cast<const UnaryExpression &>(expression);
				const std::string operand =
				    unary.operand ? expressionToString(*unary.operand) : std::string("<expr>");
				const std::string op = unaryOperatorToString(unary.op);
				if (unary.op == UnaryOperator::PreIncrement || unary.op == UnaryOperator::PreDecrement ||
				    unary.op == UnaryOperator::Positive || unary.op == UnaryOperator::Negate ||
				    unary.op == UnaryOperator::LogicalNot || unary.op == UnaryOperator::BitwiseNot)
				{
					return op + operand;
				}
				return operand;
			}
			case Expression::Kind::Binary:
			{
				const auto &binary = static_cast<const BinaryExpression &>(expression);
				const std::string left = binary.left ? expressionToString(*binary.left) : "<lhs>";
				const std::string right = binary.right ? expressionToString(*binary.right) : "<rhs>";
				return left + " " + binaryOperatorToString(binary.op) + " " + right;
			}
			case Expression::Kind::Assignment:
			{
				const auto &assignExpr = static_cast<const AssignmentExpression &>(expression);
				const std::string target = assignExpr.target ? expressionToString(*assignExpr.target) : "<target>";
				const std::string value = assignExpr.value ? expressionToString(*assignExpr.value) : "<value>";
				return target + " " + assignmentOperatorToString(assignExpr.op) + " " + value;
			}
			case Expression::Kind::Conditional:
			{
				const auto &cond = static_cast<const ConditionalExpression &>(expression);
				const std::string c = cond.condition ? expressionToString(*cond.condition) : "<cond>";
				const std::string t = cond.thenBranch ? expressionToString(*cond.thenBranch) : "<then>";
				const std::string e = cond.elseBranch ? expressionToString(*cond.elseBranch) : "<else>";
				return c + " ? " + t + " : " + e;
			}
			case Expression::Kind::Call:
			{
				const auto &call = static_cast<const CallExpression &>(expression);
				std::string text = call.callee ? expressionToString(*call.callee) : "<callee>";
				text += "(";
				for (std::size_t i = 0; i < call.arguments.size(); ++i)
				{
					if (i > 0)
					{
						text += ", ";
					}
					text += call.arguments[i] ? expressionToString(*call.arguments[i]) : "<arg>";
				}
				text += ")";
				return text;
			}
			case Expression::Kind::MemberAccess:
			{
				const auto &member = static_cast<const MemberExpression &>(expression);
				const std::string object = member.object ? expressionToString(*member.object) : "<object>";
				return object + "." + safeTokenContent(member.member);
			}
			case Expression::Kind::IndexAccess:
			{
				const auto &index = static_cast<const IndexExpression &>(expression);
				const std::string object = index.object ? expressionToString(*index.object) : "<object>";
				const std::string idx = index.index ? expressionToString(*index.index) : "<index>";
				return object + "[" + idx + "]";
			}
			case Expression::Kind::Postfix:
			{
				const auto &postfix = static_cast<const PostfixExpression &>(expression);
				const std::string operand =
				    postfix.operand ? expressionToString(*postfix.operand) : std::string("<expr>");
				return operand + postfixOperatorToString(postfix.op);
			}
		}

		return "<expr>";
	}

	void printStatementSummary(const Statement &statement, std::size_t indent)
	{
		const std::string pad = indentString(indent);
		switch (statement.kind)
		{
			case Statement::Kind::Block:
			{
				const auto &block = static_cast<const BlockStatement &>(statement);
				printBlockSummary(block, indent);
				break;
			}
			case Statement::Kind::Expression:
			{
				const auto &exprStmt = static_cast<const ExpressionStatement &>(statement);
				if (exprStmt.expression)
				{
					std::cout << pad << expressionToString(*exprStmt.expression) << '\n';
				}
				else
				{
					std::cout << pad << "(expression)\n";
				}
				break;
			}
			case Statement::Kind::Variable:
			{
				const auto &varStmt = static_cast<const VariableStatement &>(statement);
				std::cout << pad << formatTypeName(varStmt.declaration.type) << " "
				          << formatDeclarators(varStmt.declaration.declarators, true) << ";\n";
				break;
			}
			case Statement::Kind::If:
			{
				const auto &ifStmt = static_cast<const IfStatement &>(statement);
				const std::string cond =
				    ifStmt.condition ? expressionToString(*ifStmt.condition) : std::string("<cond>");
				std::cout << pad << "if (" << cond << ")\n";
				if (ifStmt.thenBranch)
				{
					printStatementSummary(*ifStmt.thenBranch, indent + 2);
				}
				else
				{
					std::cout << indentString(indent + 2) << "(missing then-branch)\n";
				}
				if (ifStmt.elseBranch)
				{
					std::cout << pad << "else\n";
					printStatementSummary(*ifStmt.elseBranch, indent + 2);
				}
				break;
			}
			case Statement::Kind::While:
			{
				const auto &whileStmt = static_cast<const WhileStatement &>(statement);
				const std::string cond =
				    whileStmt.condition ? expressionToString(*whileStmt.condition) : std::string("<cond>");
				std::cout << pad << "while (" << cond << ")\n";
				if (whileStmt.body)
				{
					printStatementSummary(*whileStmt.body, indent + 2);
				}
				break;
			}
			case Statement::Kind::DoWhile:
			{
				const auto &doWhile = static_cast<const DoWhileStatement &>(statement);
				std::cout << pad << "do\n";
				if (doWhile.body)
				{
					printStatementSummary(*doWhile.body, indent + 2);
				}
				const std::string cond =
				    doWhile.condition ? expressionToString(*doWhile.condition) : std::string("<cond>");
				std::cout << pad << "while (" << cond << ");\n";
				break;
			}
			case Statement::Kind::For:
			{
				const auto &forStmt = static_cast<const ForStatement &>(statement);
				std::cout << pad << "for\n";
				if (forStmt.initializer)
				{
					std::cout << indentString(indent + 2) << "initializer:\n";
					printStatementSummary(*forStmt.initializer, indent + 4);
				}
				else
				{
					std::cout << indentString(indent + 2) << "initializer: (none)\n";
				}
				if (forStmt.condition)
				{
					std::cout << indentString(indent + 2)
					          << "condition: " << expressionToString(*forStmt.condition) << '\n';
				}
				else
				{
					std::cout << indentString(indent + 2) << "condition: (none)\n";
				}
				if (forStmt.increment)
				{
					std::cout << indentString(indent + 2)
					          << "increment: " << expressionToString(*forStmt.increment) << '\n';
				}
				else
				{
					std::cout << indentString(indent + 2) << "increment: (none)\n";
				}
				if (forStmt.body)
				{
					printStatementSummary(*forStmt.body, indent + 2);
				}
				break;
			}
			case Statement::Kind::Return:
			{
				const auto &ret = static_cast<const ReturnStatement &>(statement);
				if (ret.value)
				{
					std::cout << pad << "return " << expressionToString(*ret.value) << '\n';
				}
				else
				{
					std::cout << pad << "return\n";
				}
				break;
			}
			case Statement::Kind::Break:
				std::cout << pad << "break\n";
				break;
			case Statement::Kind::Continue:
				std::cout << pad << "continue\n";
				break;
			case Statement::Kind::Discard:
				std::cout << pad << "discard\n";
				break;
		}
	}

	void printBlockSummary(const BlockStatement &block, std::size_t indent)
	{
		const std::string pad = indentString(indent);
		std::cout << pad << "{\n";
		if (block.statements.empty())
		{
			std::cout << indentString(indent + 2) << "(empty)\n";
		}
		else
		{
			for (const std::unique_ptr<Statement> &statement : block.statements)
			{
				if (statement)
				{
					printStatementSummary(*statement, indent + 2);
				}
			}
		}
		std::cout << pad << "}\n";
	}

	void printOptionalBody(const BlockStatement *body, std::size_t indent)
	{
		if (body)
		{
			printBlockSummary(*body, indent);
		}
		else
		{
			std::cout << indentString(indent) << "(no body)\n";
		}
	}

	std::string operatorSymbolToString(const Token &symbol)
	{
		if (symbol.type == Token::Type::LeftBracket)
		{
			return "[]";
		}
		return symbol.content;
	}

	std::string indentString(std::size_t indent)
	{
		return std::string(indent, ' ');
	}

	void printStructMember(const StructMember &member, std::size_t indent);
	void printInstruction(const Instruction &instruction, std::size_t indent);

	void printTokens(const std::vector<Token> &tokens)
	{
		using Row = std::array<std::string, 5>;
		const Row headers = {"File name", "Line", "Column", "Type", "Content"};
		std::array<std::size_t, headers.size()> widths;

		for (std::size_t i = 0; i < headers.size(); ++i)
		{
			widths[i] = headers[i].size();
		}

		std::vector<Row> rows;
		rows.reserve(tokens.size());

		for (const Token &token : tokens)
		{
			Row row = {token.origin.string(),
			    std::to_string(token.start.line + 1),
			    std::to_string(token.start.column + 1),
			    std::string(tokenTypeToString(token.type)),
			    token.content};

			for (std::size_t i = 0; i < row.size(); ++i)
			{
				widths[i] = std::max(widths[i], row[i].size());
			}

			rows.emplace_back(std::move(row));
		}

		const auto printRow = [&](const Row &row) {
			for (std::size_t i = 0; i < row.size(); ++i)
			{
				const std::size_t padding = widths[i] - row[i].size();
				std::cout << "| " << row[i] << std::string(padding, ' ') << ' ';
			}
			std::cout << "|\n";
		};

		const auto printSeparator = [&]() {
			std::size_t totalWidth = 1;
			for (std::size_t width : widths)
			{
				totalWidth += width + 3;
			}
			std::cout << std::string(totalWidth, '-') << '\n';
		};

		printRow(headers);
		printSeparator();
		for (const Row &row : rows)
		{
			printRow(row);
		}
		std::cout << std::flush;
	}

	void printStructMember(const StructMember &member, std::size_t indent)
	{
		const std::string pad = indentString(indent);
		switch (member.kind)
		{
			case StructMember::Kind::Field:
			{
				const auto &field = static_cast<const FieldMember &>(member);
				std::cout << pad << "* Field " << formatTypeName(field.declaration.type) << " : "
				          << formatDeclarators(field.declaration.declarators) << "\n";
				break;
			}
			case StructMember::Kind::Method:
			{
				const auto &method = static_cast<const MethodMember &>(member);
				std::cout << pad << "* Method " << safeTokenContent(method.name) << "("
				          << formatParameters(method.parameters) << ") -> " << formatTypeName(method.returnType);
				if (method.returnsReference)
				{
					std::cout << " &";
				}
				if (method.isConst)
				{
					std::cout << " const";
				}
				std::cout << "\n";
				printOptionalBody(method.body.get(), indent + 2);
				break;
			}
			case StructMember::Kind::Constructor:
			{
				const auto &ctor = static_cast<const ConstructorMember &>(member);
				std::cout << pad << "* Constructor(" << formatParameters(ctor.parameters) << ")\n";
				printOptionalBody(ctor.body.get(), indent + 2);
				break;
			}
			case StructMember::Kind::Operator:
			{
				const auto &op = static_cast<const OperatorMember &>(member);
				std::cout << pad << "* Operator " << operatorSymbolToString(op.symbol) << "("
				          << formatParameters(op.parameters) << ") -> " << formatTypeName(op.returnType);
				if (op.returnsReference)
				{
					std::cout << " &";
				}
				std::cout << "\n";
				printOptionalBody(op.body.get(), indent + 2);
				break;
			}
		}
	}

	void printInstruction(const Instruction &instruction, std::size_t indent)
	{
		const std::string pad = indentString(indent);
		switch (instruction.type)
		{
			case Instruction::Type::Pipeline:
			{
				const auto &pipeline = static_cast<const PipelineInstruction &>(instruction);
				std::cout << pad << "- Pipeline " << stageToString(pipeline.source) << " -> "
				          << stageToString(pipeline.destination) << " : " << formatTypeName(pipeline.payloadType) << " "
				          << safeTokenContent(pipeline.variable) << "\n";
				break;
			}
			case Instruction::Type::Variable:
			{
				const auto &variable = static_cast<const VariableInstruction &>(instruction);
				std::cout << pad << "- Variable " << formatTypeName(variable.declaration.type) << " : "
				          << formatDeclarators(variable.declaration.declarators) << "\n";
				break;
			}
			case Instruction::Type::Function:
			{
				const auto &function = static_cast<const FunctionInstruction &>(instruction);
				std::cout << pad << "- Function " << formatTypeName(function.returnType);
				if (function.returnsReference)
				{
					std::cout << " &";
				}
				std::cout << " " << safeTokenContent(function.name) << "("
				          << formatParameters(function.parameters) << ")\n";
				printOptionalBody(function.body.get(), indent + 2);
				break;
			}
			case Instruction::Type::StageFunction:
			{
				const auto &stageFunction = static_cast<const StageFunctionInstruction &>(instruction);
				std::cout << pad << "- Stage " << stageToString(stageFunction.stage) << "("
				          << formatParameters(stageFunction.parameters) << ")\n";
				printOptionalBody(stageFunction.body.get(), indent + 2);
				break;
			}
			case Instruction::Type::Aggregate:
			{
				const auto &aggregate = static_cast<const AggregateInstruction &>(instruction);
				std::cout << pad << "- " << aggregateKindToString(aggregate.kind) << " "
				          << safeTokenContent(aggregate.name) << "\n";
				for (const std::unique_ptr<StructMember> &member : aggregate.members)
				{
					if (member)
					{
						printStructMember(*member, indent + 2);
					}
				}
				break;
			}
			case Instruction::Type::Namespace:
			{
				const auto &ns = static_cast<const NamespaceInstruction &>(instruction);
				std::cout << pad << "- Namespace " << safeTokenContent(ns.name) << "\n";
				for (const std::unique_ptr<Instruction> &child : ns.instructions)
				{
					if (child)
					{
						printInstruction(*child, indent + 2);
					}
				}
				break;
			}
		}
	}

	void printInstructions(const std::vector<std::unique_ptr<Instruction>> &instructions)
	{
		if (instructions.empty())
		{
			std::cout << "\nNo parsed instructions.\n";
			return;
		}

		std::cout << "\nParsed instructions:\n";
		for (const std::unique_ptr<Instruction> &instruction : instructions)
		{
			if (instruction)
			{
				printInstruction(*instruction, 0);
			}
		}
		std::cout << std::flush;
	}
}

int main(int argc, char **argv)
{
	try
	{
		bool debug = false;
		std::vector<std::string_view> positionalArgs;
		for (int i = 1; i < argc; ++i)
		{
			const std::string_view arg = argv[i];
			if (arg == "-d" || arg == "--debug")
			{
				debug = true;
				continue;
			}

			if (!arg.empty() && arg[0] == '-')
			{
				std::cerr << "unknown option '" << arg << "'\n";
				return 2;
			}

			positionalArgs.push_back(arg);
		}

		if (positionalArgs.size() != 2)
		{
			std::cerr << "usage: lumina-compiler [-d|--debug] <input.lumina> <output.glsl>\n";
			return 2;
		}

		const std::filesystem::path inputPath(positionalArgs[0]);
		const std::filesystem::path outputPath(positionalArgs[1]);

		resetErrorCount();
		constexpr int kStageErrorExit = 5;

		const auto abortOnErrors = [&](const char *stage, int previousCount) {
			if (getErrorCount() > previousCount)
			{
				std::cerr << "Compilation aborted after " << stage << " due to errors.\n";
				return true;
			}
			return false;
		};

		// 1) Retrieve tokens
		const int lexingErrors = getErrorCount();
		std::vector<Token> tokens = SourceManager::loadFile(inputPath);
		if (abortOnErrors("lexing", lexingErrors))
		{
			return kStageErrorExit;
		}

		if (debug)
		{
			printTokens(tokens);
		}

		// 2) Parse instruction syntaxically
		Parser parser;
		const int parseErrors = getErrorCount();
		std::vector<std::unique_ptr<Instruction>> raw = parser(std::move(tokens));
		if (abortOnErrors("syntax analysis", parseErrors))
		{
			return kStageErrorExit;
		}

		if (debug)
		{
			printInstructions(raw);
		}

		// 3) Semantic checks
		SemanticParser sema;
		const int semanticErrors = getErrorCount();
		SemanticParseResult semantic = sema(std::move(raw));
		if (abortOnErrors("semantic analysis", semanticErrors))
		{
			return kStageErrorExit;
		}

		// 4) Codegen
		Compiler codegen(debug);
		std::string glsl = codegen(semantic);

		// 5) Output
		std::ofstream out(outputPath, std::ios::binary);
		if (!out)
		{
			std::cerr << "cannot open output: " << outputPath.string() << "\n";
			return 3;
		}
		out.write(glsl.data(), static_cast<std::streamsize>(glsl.size()));
		if (!out)
		{
			std::cerr << "write failed: " << outputPath.string() << "\n";
			return 4;
		}

		std::cout << "Compilation complete: " << outputPath.string() << "\n";
		return 0;
	} catch (const std::exception &e)
	{
		std::cerr << "error: " << e.what() << "\n";
		return 1;
	}
}
