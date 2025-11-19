
#include "parser.hpp"

#include <algorithm>
#include <initializer_list>
#include <string>
#include <string_view>
#include <utility>

struct Parser::Impl
{
    using InstructionPtr = std::unique_ptr<Instruction>;
    using StatementPtr = std::unique_ptr<Statement>;
    using ExpressionPtr = std::unique_ptr<Expression>;
    using StructMemberPtr = std::unique_ptr<StructMember>;

    enum class IdentifierContext
    {
        General,
        Type
    };

    std::vector<Token> tokens;
    std::size_t current = 0;

    std::vector<InstructionPtr> parse(std::vector<Token> input);

private:
    InstructionPtr parseInstruction();
    InstructionPtr parsePipelineInstruction();
    InstructionPtr parseStageFunction();
    InstructionPtr parseNamespaceInstruction();
	InstructionPtr parseAggregateInstruction(AggregateInstruction::Kind kind);
	InstructionPtr parseDataBlockInstruction();
	InstructionPtr finishAggregateInstruction(AggregateInstruction::Kind kind, Token name);
	InstructionPtr parseFunctionOrVariable();
    InstructionPtr parseFunctionDefinition(TypeName returnType, Token name, bool returnsReference);
    InstructionPtr parseVariableInstruction(TypeName type);

    StructMemberPtr parseAggregateMember(const Token &aggregateName);
    StructMemberPtr parseConstructorMember(const Token &aggregateName);
    StructMemberPtr parseMethodMember(TypeName returnType, Token name, bool returnsReference);
    StructMemberPtr parseOperatorMember(TypeName returnType, bool returnsReference);

    bool isPipelineStart() const;
    bool isStageFunctionStart() const;
    bool isFunctionDefinitionAhead() const;
    bool isConstructorStart(const Token &aggregateName) const;

    Stage stageFromToken(const Token &token);
    bool isStageToken(Token::Type type) const;
    Token consumeStageToken(std::string_view message);

    TypeName parseTypeName();
    Name parseQualifiedName(IdentifierContext ctx, std::string_view message);
    Token consumeIdentifierToken(IdentifierContext ctx, std::string_view message);
    bool isIdentifierToken(const Token &token, IdentifierContext ctx) const;
    bool isTypeToken(const Token &token) const;

    Parameter parseParameter();
    std::vector<Parameter> parseParameterList();

    std::unique_ptr<BlockStatement> parseBlock();
    StatementPtr parseStatement();
    StatementPtr parseIfStatement();
    StatementPtr parseWhileStatement();
    StatementPtr parseDoWhileStatement();
    StatementPtr parseForStatement();
    StatementPtr parseReturnStatement();
    StatementPtr parseBreakStatement();
    StatementPtr parseContinueStatement();
    StatementPtr parseDiscardStatement();
    StatementPtr parseExpressionStatement();
    StatementPtr parseVariableStatement();

    bool looksLikeDeclaration() const;

	VariableDeclarator parseSingleDeclarator(const TypeName &type, bool allowDirectInit);
	VariableDeclarator parseDeclaratorWithConsumedName(Token nameToken, bool isReference, const TypeName &type,
	    bool allowDirectInit);
	void parseArraySuffix(VariableDeclarator &decl);
	void parseDeclaratorInitializer(VariableDeclarator &decl, const TypeName &type, bool allowDirectInit);
	void parseTextureBindingQualifier(VariableDeclarator &decl);
	VariableDeclaration parseVariableDeclaration(TypeName type, bool allowDirectInit);
	VariableDeclaration parseVariableDeclarationFromExisting(TypeName type, VariableDeclarator first,
	    bool allowDirectInit);

    ExpressionPtr parseExpression();
    ExpressionPtr parseAssignment();
    ExpressionPtr parseConditional();
    ExpressionPtr parseLogicalOr();
    ExpressionPtr parseLogicalAnd();
    ExpressionPtr parseBitwiseOr();
    ExpressionPtr parseBitwiseXor();
    ExpressionPtr parseBitwiseAnd();
    ExpressionPtr parseEquality();
    ExpressionPtr parseComparison();
    ExpressionPtr parseTerm();
    ExpressionPtr parseFactor();
    ExpressionPtr parseUnary();
    ExpressionPtr parsePostfix();
    ExpressionPtr parsePrimary();

    ExpressionPtr finishCall(ExpressionPtr callee);
    std::vector<ExpressionPtr> parseArgumentListAfterLeftParen();
    ExpressionPtr parseDirectInitializer(const TypeName &type);
    ExpressionPtr makeTypeExpression(const TypeName &type);
    ExpressionPtr parseIdentifierExpression(Token firstToken);
    ExpressionPtr makeLiteralExpression(const Token &token);

    BinaryOperator binaryOperatorFromToken(Token::Type type) const;
    AssignmentOperator assignmentOperatorFromToken(Token::Type type) const;
    UnaryOperator unaryOperatorFromToken(Token::Type type) const;
    PostfixOperator postfixOperatorFromToken(Token::Type type) const;
    bool isAssignmentOperator(Token::Type type) const;

    bool match(Token::Type type);
    bool match(std::initializer_list<Token::Type> types);
    bool check(Token::Type type) const;
    bool checkNext(Token::Type type) const;
    const Token &advance();
    bool isAtEnd() const;
    const Token &peek(std::size_t offset = 0) const;
    const Token &previous() const;

    Token consume(Token::Type type, std::string_view message);

    void reportError(const std::string &message, const Token &token);
    void skipToNextLine(std::size_t line);
};

Parser::Parser() : m_impl(std::make_unique<Impl>()) {}
Parser::~Parser() = default;

std::vector<std::unique_ptr<Instruction>> Parser::operator()(std::vector<Token> p_tokens)
{
    return m_impl->parse(std::move(p_tokens));
}
std::vector<Parser::Impl::InstructionPtr> Parser::Impl::parse(std::vector<Token> input)
{
    tokens = std::move(input);
    current = 0;

    std::vector<InstructionPtr> instructions;

    while (!isAtEnd())
    {
        while (match(Token::Type::Semicolon))
        {
            // Skip stray semicolons at the top level.
        }

        if (isAtEnd())
        {
            break;
        }

        const std::size_t startIndex = current;
        if (InstructionPtr instruction = parseInstruction())
        {
            instructions.push_back(std::move(instruction));
        }

        if (isAtEnd())
        {
            break;
        }

        if (current == startIndex)
        {
            advance();
        }
    }

    return instructions;
}

Parser::Impl::InstructionPtr Parser::Impl::parseInstruction()
{
    if (isPipelineStart())
    {
        return parsePipelineInstruction();
    }

    if (isStageFunctionStart())
    {
        return parseStageFunction();
    }

    const Token &token = peek();
    switch (token.type)
    {
        case Token::Type::KeywordNamespace:
            advance();
            return parseNamespaceInstruction();
        case Token::Type::KeywordStruct:
            advance();
            return parseAggregateInstruction(AggregateInstruction::Kind::Struct);
        case Token::Type::KeywordAttributeBlock:
            advance();
            return parseAggregateInstruction(AggregateInstruction::Kind::AttributeBlock);
        case Token::Type::KeywordConstantBlock:
            advance();
            return parseAggregateInstruction(AggregateInstruction::Kind::ConstantBlock);
        case Token::Type::KeywordDataBlock:
            advance();
            return parseDataBlockInstruction();
        default:
            break;
    }

    return parseFunctionOrVariable();
}

Parser::Impl::InstructionPtr Parser::Impl::parsePipelineInstruction()
{
    Token sourceToken = consumeStageToken("Expected stage name at the beginning of a pipeline declaration");
    Stage source = stageFromToken(sourceToken);

    consume(Token::Type::Arrow, "Expected '->' in pipeline declaration");

    Token destinationToken = consumeStageToken("Expected stage name after '->' in pipeline declaration");
    Stage destination = stageFromToken(destinationToken);

    consume(Token::Type::Colon, "Expected ':' after pipeline stages");

	TypeName payloadType = parseTypeName();
	Token variable = consumeIdentifierToken(IdentifierContext::General, "Expected variable name in pipeline declaration");
	consume(Token::Type::Semicolon, "Expected ';' at the end of a pipeline declaration");

	auto result = std::make_unique<PipelineInstruction>();
	result->sourceToken = std::move(sourceToken);
	result->source = source;
	result->destinationToken = std::move(destinationToken);
	result->destination = destination;
	result->payloadType = std::move(payloadType);
	result->variable = std::move(variable);
	return result;
}

Parser::Impl::InstructionPtr Parser::Impl::parseStageFunction()
{
	Token keyword = consumeStageToken("Expected stage keyword");
	Stage stage = stageFromToken(keyword);

	if (stage == Stage::Input || stage == Stage::Output)
	{
		reportError("Stage functions are only allowed for VertexPass or FragmentPass", keyword);
		return nullptr;
	}

    consume(Token::Type::LeftParen, "Expected '(' after stage name");
    std::vector<Parameter> parameters = parseParameterList();
    consume(Token::Type::RightParen, "Expected ')' after parameter list");

    auto body = parseBlock();
	if (!body)
	{
		return nullptr;
	}

	auto result = std::make_unique<StageFunctionInstruction>();
	result->stageToken = std::move(keyword);
	result->stage = stage;
	result->parameters = std::move(parameters);
	result->body = std::move(body);
	return result;
}

Parser::Impl::InstructionPtr Parser::Impl::parseNamespaceInstruction()
{
    Token name = consumeIdentifierToken(IdentifierContext::General, "Expected namespace name");
    consume(Token::Type::LeftBrace, "Expected '{' to open namespace body");

    auto result = std::make_unique<NamespaceInstruction>();
    result->name = std::move(name);

    while (!check(Token::Type::RightBrace) && !isAtEnd())
    {
        if (InstructionPtr instruction = parseInstruction())
        {
            result->instructions.push_back(std::move(instruction));
        }
    }

    consume(Token::Type::RightBrace, "Expected '}' to close namespace");
    return result;
}
Parser::Impl::InstructionPtr Parser::Impl::parseAggregateInstruction(AggregateInstruction::Kind kind)
{
    Token name = consumeIdentifierToken(IdentifierContext::General, "Expected name after aggregate keyword");
    return finishAggregateInstruction(kind, std::move(name));
}

Parser::Impl::InstructionPtr Parser::Impl::parseDataBlockInstruction()
{
    Token name = consumeIdentifierToken(IdentifierContext::General, "Expected name after 'DataBlock'");
    AggregateInstruction::Kind kind = AggregateInstruction::Kind::ConstantBlock;
    if (match(Token::Type::KeywordAs))
    {
        if (match(Token::Type::KeywordAttribute))
        {
            kind = AggregateInstruction::Kind::AttributeBlock;
        }
        else if (match(Token::Type::KeywordConstant))
        {
            kind = AggregateInstruction::Kind::ConstantBlock;
        }
        else
        {
            reportError("Expected 'constant' or 'attribute' after 'as'", peek());
        }
    }
    return finishAggregateInstruction(kind, std::move(name));
}

Parser::Impl::InstructionPtr Parser::Impl::finishAggregateInstruction(AggregateInstruction::Kind kind, Token name)
{
    auto result = std::make_unique<AggregateInstruction>(kind);
    result->name = std::move(name);

    consume(Token::Type::LeftBrace, "Expected '{' to open aggregate body");
    while (!check(Token::Type::RightBrace) && !isAtEnd())
    {
        if (StructMemberPtr member = parseAggregateMember(result->name))
        {
            result->members.push_back(std::move(member));
        }
        else if (isAtEnd())
        {
            break;
        }
    }
    consume(Token::Type::RightBrace, "Expected '}' to close aggregate body");
    consume(Token::Type::Semicolon, "Expected ';' after aggregate declaration");

    return result;
}

Parser::Impl::InstructionPtr Parser::Impl::parseFunctionOrVariable()
{
	TypeName type = parseTypeName();

	if (!isIdentifierToken(peek(), IdentifierContext::General))
	{
		reportError("Expected identifier after type name", peek());
		return nullptr;
	}

	if (isFunctionDefinitionAhead())
	{
		bool returnsReference = match(Token::Type::Ampersand);
		Token name = consumeIdentifierToken(IdentifierContext::General, "Expected function name");
		return parseFunctionDefinition(std::move(type), std::move(name), returnsReference);
	}

	return parseVariableInstruction(std::move(type));
}

Parser::Impl::InstructionPtr Parser::Impl::parseFunctionDefinition(TypeName returnType, Token name, bool returnsReference)
{
    consume(Token::Type::LeftParen, "Expected '(' after function name");
    std::vector<Parameter> parameters = parseParameterList();
    consume(Token::Type::RightParen, "Expected ')' after parameter list");
    match(Token::Type::KeywordConst);

    auto body = parseBlock();
    if (!body)
    {
        return nullptr;
    }

    auto result = std::make_unique<FunctionInstruction>();
    result->returnType = std::move(returnType);
    result->name = std::move(name);
    result->parameters = std::move(parameters);
    result->body = std::move(body);
    result->returnsReference = returnsReference;
    return result;
}

Parser::Impl::InstructionPtr Parser::Impl::parseVariableInstruction(TypeName type)
{
    VariableDeclaration declaration = parseVariableDeclaration(std::move(type), true);
    consume(Token::Type::Semicolon, "Expected ';' after variable declaration");
    auto result = std::make_unique<VariableInstruction>();
    result->declaration = std::move(declaration);
    return result;
}

Parser::Impl::StructMemberPtr Parser::Impl::parseAggregateMember(const Token &aggregateName)
{
    if (match(Token::Type::Semicolon))
    {
        return nullptr;
    }

    if (isConstructorStart(aggregateName))
    {
        return parseConstructorMember(aggregateName);
    }

    TypeName type = parseTypeName();
    bool returnsReference = match(Token::Type::Ampersand);

    if (!isIdentifierToken(peek(), IdentifierContext::General))
    {
        reportError("Expected identifier in aggregate member", peek());
        return nullptr;
    }

    Token name = consumeIdentifierToken(IdentifierContext::General, "Expected member name");
    if (name.content == "operator")
    {
        return parseOperatorMember(std::move(type), returnsReference);
    }

    if (check(Token::Type::LeftParen))
    {
        return parseMethodMember(std::move(type), std::move(name), returnsReference);
    }

    VariableDeclarator first = parseDeclaratorWithConsumedName(std::move(name), returnsReference, type, true);
    VariableDeclaration declaration = parseVariableDeclarationFromExisting(std::move(type), std::move(first), true);
    consume(Token::Type::Semicolon, "Expected ';' after field declaration");

    auto field = std::make_unique<FieldMember>();
    field->declaration = std::move(declaration);
    return field;
}
Parser::Impl::StructMemberPtr Parser::Impl::parseConstructorMember(const Token &aggregateName)
{
    Token name = consumeIdentifierToken(IdentifierContext::General, "Expected constructor name");
    (void)name;

    consume(Token::Type::LeftParen, "Expected '(' after constructor name");
    std::vector<Parameter> parameters = parseParameterList();
    consume(Token::Type::RightParen, "Expected ')' after constructor parameters");

    auto body = parseBlock();
    if (!body)
    {
        return nullptr;
    }

    auto constructor = std::make_unique<ConstructorMember>();
    constructor->name = aggregateName;
    constructor->parameters = std::move(parameters);
    constructor->body = std::move(body);
    return constructor;
}

Parser::Impl::StructMemberPtr Parser::Impl::parseMethodMember(TypeName returnType, Token name, bool returnsReference)
{
    consume(Token::Type::LeftParen, "Expected '(' after method name");
    std::vector<Parameter> parameters = parseParameterList();
    consume(Token::Type::RightParen, "Expected ')' after method parameters");

    bool isConst = match(Token::Type::KeywordConst);
    auto body = parseBlock();
    if (!body)
    {
        return nullptr;
    }

    auto method = std::make_unique<MethodMember>();
    method->returnType = std::move(returnType);
    method->name = std::move(name);
    method->parameters = std::move(parameters);
    method->body = std::move(body);
    method->returnsReference = returnsReference;
    method->isConst = isConst;
    return method;
}

Parser::Impl::StructMemberPtr Parser::Impl::parseOperatorMember(TypeName returnType, bool returnsReference)
{
    Token symbol;
    if (match(Token::Type::LeftBracket))
    {
        symbol = previous();
        consume(Token::Type::RightBracket, "Expected ']' to complete operator[]");
        symbol.type = Token::Type::LeftBracket;
    }
    else if (match({Token::Type::Plus,
                 Token::Type::Minus,
                 Token::Type::Star,
                 Token::Type::Slash,
                 Token::Type::Percent,
                 Token::Type::Equal,
                 Token::Type::Less,
                 Token::Type::Greater,
                 Token::Type::Bang,
                 Token::Type::Ampersand,
                 Token::Type::Pipe,
                 Token::Type::Caret,
                 Token::Type::PlusEqual,
                 Token::Type::MinusEqual,
                 Token::Type::StarEqual,
                 Token::Type::SlashEqual,
                 Token::Type::PercentEqual,
                 Token::Type::AmpersandEqual,
                 Token::Type::PipeEqual,
                 Token::Type::CaretEqual}))
    {
        symbol = previous();
    }
    else
    {
        reportError("Unsupported operator symbol", peek());
        return nullptr;
    }

    consume(Token::Type::LeftParen, "Expected '(' after operator symbol");
    std::vector<Parameter> parameters = parseParameterList();
    consume(Token::Type::RightParen, "Expected ')' after operator parameters");
    match(Token::Type::KeywordConst);

    auto body = parseBlock();
    if (!body)
    {
        return nullptr;
    }

    auto op = std::make_unique<OperatorMember>();
    op->returnType = std::move(returnType);
    op->symbol = std::move(symbol);
    op->parameters = std::move(parameters);
    op->body = std::move(body);
    op->returnsReference = returnsReference;
    return op;
}
bool Parser::Impl::isPipelineStart() const
{
    if (!isStageToken(peek().type))
    {
        return false;
    }
    return checkNext(Token::Type::Arrow);
}

bool Parser::Impl::isStageFunctionStart() const
{
    const Token::Type type = peek().type;
    if (type != Token::Type::KeywordVertexPass && type != Token::Type::KeywordFragmentPass)
    {
        return false;
    }
    return checkNext(Token::Type::LeftParen);
}

bool Parser::Impl::isFunctionDefinitionAhead() const
{
    std::size_t index = current;
    if (index >= tokens.size())
    {
        return false;
    }

    if (tokens[index].type == Token::Type::Ampersand)
    {
        ++index;
    }

    if (index >= tokens.size() || !isIdentifierToken(tokens[index], IdentifierContext::General))
    {
        return false;
    }
    ++index;

    if (index >= tokens.size() || tokens[index].type != Token::Type::LeftParen)
    {
        return false;
    }

    int depth = 1;
    ++index;
    while (index < tokens.size() && depth > 0)
    {
        Token::Type type = tokens[index].type;
        if (type == Token::Type::LeftParen)
        {
            ++depth;
        }
        else if (type == Token::Type::RightParen)
        {
            --depth;
        }
        else if (type == Token::Type::EndOfFile)
        {
            return false;
        }
        ++index;
    }

    if (depth != 0 || index >= tokens.size())
    {
        return false;
    }

    if (tokens[index].type == Token::Type::KeywordConst)
    {
        ++index;
    }

    return index < tokens.size() && tokens[index].type == Token::Type::LeftBrace;
}

bool Parser::Impl::isConstructorStart(const Token &aggregateName) const
{
    if (!isIdentifierToken(peek(), IdentifierContext::General))
    {
        return false;
    }
    if (peek().content != aggregateName.content)
    {
        return false;
    }
    return checkNext(Token::Type::LeftParen);
}

Stage Parser::Impl::stageFromToken(const Token &token)
{
    switch (token.type)
    {
        case Token::Type::KeywordInput:
            return Stage::Input;
        case Token::Type::KeywordVertexPass:
            return Stage::VertexPass;
        case Token::Type::KeywordFragmentPass:
            return Stage::FragmentPass;
        case Token::Type::KeywordOutput:
            return Stage::Output;
        default:
            break;
    }
    reportError("Invalid stage token", token);
    return Stage::Input;
}

bool Parser::Impl::isStageToken(Token::Type type) const
{
    return type == Token::Type::KeywordInput || type == Token::Type::KeywordVertexPass ||
           type == Token::Type::KeywordFragmentPass || type == Token::Type::KeywordOutput;
}

Token Parser::Impl::consumeStageToken(std::string_view message)
{
    if (!isStageToken(peek().type))
    {
        return consume(Token::Type::Identifier, message);
    }
    Token token = peek();
    advance();
    return token;
}

TypeName Parser::Impl::parseTypeName()
{
    TypeName type;
    if (match(Token::Type::KeywordConst))
    {
        type.isConst = true;
    }
    type.name = parseQualifiedName(IdentifierContext::Type, "Expected type name");
    return type;
}

Name Parser::Impl::parseQualifiedName(IdentifierContext ctx, std::string_view message)
{
    Name name;
    name.parts.push_back(consumeIdentifierToken(ctx, message));
    while (match(Token::Type::DoubleColon))
    {
        name.parts.push_back(consumeIdentifierToken(ctx, "Expected identifier after '::'"));
    }
    return name;
}

Token Parser::Impl::consumeIdentifierToken(IdentifierContext ctx, std::string_view message)
{
    if (isIdentifierToken(peek(), ctx))
    {
        Token token = peek();
        advance();
        return token;
    }
    return consume(Token::Type::Identifier, message);
}

bool Parser::Impl::isIdentifierToken(const Token &token, IdentifierContext ctx) const
{
    if (token.type == Token::Type::Identifier)
    {
        return true;
    }
    if (ctx == IdentifierContext::Type && token.type == Token::Type::KeywordTexture)
    {
        return true;
    }
    return false;
}

bool Parser::Impl::isTypeToken(const Token &token) const
{
    return token.type == Token::Type::Identifier || token.type == Token::Type::KeywordTexture;
}
Parameter Parser::Impl::parseParameter()
{
    Parameter parameter;
    parameter.type = parseTypeName();
    if (match(Token::Type::Ampersand))
    {
        parameter.isReference = true;
    }
    parameter.name = consumeIdentifierToken(IdentifierContext::General, "Expected parameter name");
    return parameter;
}

std::vector<Parameter> Parser::Impl::parseParameterList()
{
    std::vector<Parameter> parameters;
    if (!check(Token::Type::RightParen))
    {
        do
        {
            parameters.emplace_back(parseParameter());
        } while (match(Token::Type::Comma));
    }
    return parameters;
}

std::unique_ptr<BlockStatement> Parser::Impl::parseBlock()
{
    consume(Token::Type::LeftBrace, "Expected '{' to begin block");

    auto block = std::make_unique<BlockStatement>();
    while (!check(Token::Type::RightBrace) && !isAtEnd())
    {
        if (StatementPtr statement = parseStatement())
        {
            block->statements.push_back(std::move(statement));
        }
        else if (isAtEnd())
        {
            break;
        }
    }

    consume(Token::Type::RightBrace, "Expected '}' to close block");
    return block;
}

Parser::Impl::StatementPtr Parser::Impl::parseStatement()
{
    if (check(Token::Type::LeftBrace))
    {
        return parseBlock();
    }

    switch (peek().type)
    {
        case Token::Type::KeywordIf:
            return parseIfStatement();
        case Token::Type::KeywordWhile:
            return parseWhileStatement();
        case Token::Type::KeywordDo:
            return parseDoWhileStatement();
        case Token::Type::KeywordFor:
            return parseForStatement();
        case Token::Type::KeywordReturn:
            return parseReturnStatement();
        case Token::Type::KeywordBreak:
            return parseBreakStatement();
        case Token::Type::KeywordContinue:
            return parseContinueStatement();
        case Token::Type::KeywordDiscard:
            return parseDiscardStatement();
        default:
            break;
    }

    if (looksLikeDeclaration())
    {
        return parseVariableStatement();
    }

    return parseExpressionStatement();
}
Parser::Impl::StatementPtr Parser::Impl::parseIfStatement()
{
    consume(Token::Type::KeywordIf, "Expected 'if'");
    consume(Token::Type::LeftParen, "Expected '(' after 'if'");
    ExpressionPtr condition = parseExpression();
    consume(Token::Type::RightParen, "Expected ')' after condition");

    StatementPtr thenBranch = parseStatement();
    StatementPtr elseBranch;
    if (match(Token::Type::KeywordElse))
    {
        elseBranch = parseStatement();
    }

    auto statement = std::make_unique<IfStatement>();
    statement->condition = std::move(condition);
    statement->thenBranch = std::move(thenBranch);
    statement->elseBranch = std::move(elseBranch);
    return statement;
}

Parser::Impl::StatementPtr Parser::Impl::parseWhileStatement()
{
    consume(Token::Type::KeywordWhile, "Expected 'while'");
    consume(Token::Type::LeftParen, "Expected '(' after 'while'");
    ExpressionPtr condition = parseExpression();
    consume(Token::Type::RightParen, "Expected ')' after condition");
    StatementPtr body = parseStatement();

    auto statement = std::make_unique<WhileStatement>();
    statement->condition = std::move(condition);
    statement->body = std::move(body);
    return statement;
}

Parser::Impl::StatementPtr Parser::Impl::parseDoWhileStatement()
{
    consume(Token::Type::KeywordDo, "Expected 'do'");
    StatementPtr body = parseStatement();
    consume(Token::Type::KeywordWhile, "Expected 'while' after do-while body");
    consume(Token::Type::LeftParen, "Expected '(' after 'while'");
    ExpressionPtr condition = parseExpression();
    consume(Token::Type::RightParen, "Expected ')' after condition");
    consume(Token::Type::Semicolon, "Expected ';' after do-while statement");

    auto statement = std::make_unique<DoWhileStatement>();
    statement->body = std::move(body);
    statement->condition = std::move(condition);
    return statement;
}

Parser::Impl::StatementPtr Parser::Impl::parseForStatement()
{
    consume(Token::Type::KeywordFor, "Expected 'for'");
    consume(Token::Type::LeftParen, "Expected '(' after 'for'");

    StatementPtr initializer;
    if (!check(Token::Type::Semicolon))
    {
        if (looksLikeDeclaration())
        {
            initializer = parseVariableStatement();
        }
        else
        {
            initializer = parseExpressionStatement();
        }
    }
    else
    {
        consume(Token::Type::Semicolon, "Expected ';' in for-loop");
    }

    ExpressionPtr condition;
    if (!check(Token::Type::Semicolon))
    {
        condition = parseExpression();
    }
    consume(Token::Type::Semicolon, "Expected ';' in for-loop");

    ExpressionPtr increment;
    if (!check(Token::Type::RightParen))
    {
        increment = parseExpression();
    }
    consume(Token::Type::RightParen, "Expected ')' after for components");

    StatementPtr body = parseStatement();

    auto statement = std::make_unique<ForStatement>();
    statement->initializer = std::move(initializer);
    statement->condition = std::move(condition);
    statement->increment = std::move(increment);
    statement->body = std::move(body);
    return statement;
}

Parser::Impl::StatementPtr Parser::Impl::parseReturnStatement()
{
    consume(Token::Type::KeywordReturn, "Expected 'return'");

    auto statement = std::make_unique<ReturnStatement>();
    if (!check(Token::Type::Semicolon))
    {
        statement->value = parseExpression();
    }
    consume(Token::Type::Semicolon, "Expected ';' after return statement");
    return statement;
}

Parser::Impl::StatementPtr Parser::Impl::parseBreakStatement()
{
    consume(Token::Type::KeywordBreak, "Expected 'break'");
    consume(Token::Type::Semicolon, "Expected ';' after 'break'");
    return std::make_unique<BreakStatement>();
}

Parser::Impl::StatementPtr Parser::Impl::parseContinueStatement()
{
    consume(Token::Type::KeywordContinue, "Expected 'continue'");
    consume(Token::Type::Semicolon, "Expected ';' after 'continue'");
    return std::make_unique<ContinueStatement>();
}

Parser::Impl::StatementPtr Parser::Impl::parseDiscardStatement()
{
    consume(Token::Type::KeywordDiscard, "Expected 'discard'");
    consume(Token::Type::Semicolon, "Expected ';' after 'discard'");
    return std::make_unique<DiscardStatement>();
}

Parser::Impl::StatementPtr Parser::Impl::parseExpressionStatement()
{
    ExpressionPtr expression = parseExpression();
    consume(Token::Type::Semicolon, "Expected ';' after expression");
    auto statement = std::make_unique<ExpressionStatement>();
    statement->expression = std::move(expression);
    return statement;
}

Parser::Impl::StatementPtr Parser::Impl::parseVariableStatement()
{
	TypeName type = parseTypeName();
	VariableDeclaration declaration = parseVariableDeclaration(std::move(type), true);

	consume(Token::Type::Semicolon, "Expected ';' after declaration");

	auto statement = std::make_unique<VariableStatement>();
	statement->declaration = std::move(declaration);
    return statement;
}
bool Parser::Impl::looksLikeDeclaration() const
{
    std::size_t index = current;
    if (index >= tokens.size())
    {
        return false;
    }

    if (tokens[index].type == Token::Type::KeywordConst)
    {
        ++index;
    }

    if (index >= tokens.size() || !isTypeToken(tokens[index]))
    {
        return false;
    }
    ++index;

    while (index < tokens.size() && tokens[index].type == Token::Type::DoubleColon)
    {
        ++index;
        if (index >= tokens.size() || !isTypeToken(tokens[index]))
        {
            return false;
        }
        ++index;
    }

    if (index < tokens.size() && tokens[index].type == Token::Type::Ampersand)
    {
        ++index;
    }

    return index < tokens.size() && isIdentifierToken(tokens[index], IdentifierContext::General);
}

VariableDeclarator Parser::Impl::parseSingleDeclarator(const TypeName &type, bool allowDirectInit)
{
    bool isReference = match(Token::Type::Ampersand);
    Token name = consumeIdentifierToken(IdentifierContext::General, "Expected identifier");
    return parseDeclaratorWithConsumedName(std::move(name), isReference, type, allowDirectInit);
}

VariableDeclarator Parser::Impl::parseDeclaratorWithConsumedName(Token nameToken, bool isReference,
    const TypeName &type, bool allowDirectInit)
{
    VariableDeclarator declarator;
	declarator.name = std::move(nameToken);
	declarator.isReference = isReference;
	parseArraySuffix(declarator);
	parseDeclaratorInitializer(declarator, type, allowDirectInit);
	parseTextureBindingQualifier(declarator);
	return declarator;
}

void Parser::Impl::parseArraySuffix(VariableDeclarator &decl)
{
    if (match(Token::Type::LeftBracket))
    {
        decl.hasArraySuffix = true;
        if (!check(Token::Type::RightBracket))
        {
            decl.hasArraySize = true;
            decl.arraySize = parseExpression();
        }
        consume(Token::Type::RightBracket, "Expected ']' after array size");
    }
}

void Parser::Impl::parseDeclaratorInitializer(VariableDeclarator &decl, const TypeName &type, bool allowDirectInit)
{
    if (match(Token::Type::Assign))
    {
        decl.initializer = parseExpression();
        return;
    }

    if (allowDirectInit && check(Token::Type::LeftParen))
    {
        decl.initializer = parseDirectInitializer(type);
    }
}

void Parser::Impl::parseTextureBindingQualifier(VariableDeclarator &decl)
{
    if (!match(Token::Type::KeywordAs))
    {
        return;
    }

    decl.hasTextureBinding = true;
    if (match(Token::Type::KeywordConstant))
    {
        decl.textureBindingScope = TextureBindingScope::Constant;
        decl.textureBindingToken = previous();
        return;
    }
    if (match(Token::Type::KeywordAttribute))
    {
        decl.textureBindingScope = TextureBindingScope::Attribute;
        decl.textureBindingToken = previous();
        return;
    }

    reportError("Expected 'constant' or 'attribute' after 'as'", peek());
}

VariableDeclaration Parser::Impl::parseVariableDeclaration(TypeName type, bool allowDirectInit)
{
    VariableDeclaration declaration;
    declaration.type = std::move(type);
    declaration.declarators.push_back(parseSingleDeclarator(declaration.type, allowDirectInit));
    while (match(Token::Type::Comma))
    {
        declaration.declarators.push_back(parseSingleDeclarator(declaration.type, allowDirectInit));
    }
    return declaration;
}

VariableDeclaration Parser::Impl::parseVariableDeclarationFromExisting(TypeName type, VariableDeclarator first,
    bool allowDirectInit)
{
    VariableDeclaration declaration;
    declaration.type = std::move(type);
    declaration.declarators.push_back(std::move(first));
    while (match(Token::Type::Comma))
    {
        declaration.declarators.push_back(parseSingleDeclarator(declaration.type, allowDirectInit));
    }
    return declaration;
}
Parser::Impl::ExpressionPtr Parser::Impl::parseExpression()
{
    return parseAssignment();
}

Parser::Impl::ExpressionPtr Parser::Impl::parseAssignment()
{
    ExpressionPtr left = parseConditional();
    if (!left)
    {
        return nullptr;
    }

    if (isAssignmentOperator(peek().type))
    {
        Token opToken = advance();
        ExpressionPtr value = parseAssignment();
        if (!value)
        {
            return nullptr;
        }

        auto assignment = std::make_unique<AssignmentExpression>();
        assignment->operatorToken = opToken;
        assignment->op = assignmentOperatorFromToken(opToken.type);
        assignment->target = std::move(left);
        assignment->value = std::move(value);
        return assignment;
    }

    return left;
}

Parser::Impl::ExpressionPtr Parser::Impl::parseConditional()
{
    ExpressionPtr condition = parseLogicalOr();
    if (!condition)
    {
        return nullptr;
    }

    if (match(Token::Type::Question))
    {
        ExpressionPtr thenBranch = parseExpression();
        consume(Token::Type::Colon, "Expected ':' in conditional expression");
        ExpressionPtr elseBranch = parseExpression();

        auto expression = std::make_unique<ConditionalExpression>();
        expression->condition = std::move(condition);
        expression->thenBranch = std::move(thenBranch);
        expression->elseBranch = std::move(elseBranch);
        return expression;
    }

    return condition;
}

Parser::Impl::ExpressionPtr Parser::Impl::parseLogicalOr()
{
    ExpressionPtr expression = parseLogicalAnd();
    while (match(Token::Type::PipePipe))
    {
        Token opToken = previous();
        ExpressionPtr right = parseLogicalAnd();
        auto binary = std::make_unique<BinaryExpression>();
        binary->operatorToken = opToken;
        binary->op = BinaryOperator::LogicalOr;
        binary->left = std::move(expression);
        binary->right = std::move(right);
        expression = std::move(binary);
    }
    return expression;
}

Parser::Impl::ExpressionPtr Parser::Impl::parseLogicalAnd()
{
    ExpressionPtr expression = parseBitwiseOr();
    while (match(Token::Type::AmpersandAmpersand))
    {
        Token opToken = previous();
        ExpressionPtr right = parseBitwiseOr();
        auto binary = std::make_unique<BinaryExpression>();
        binary->operatorToken = opToken;
        binary->op = BinaryOperator::LogicalAnd;
        binary->left = std::move(expression);
        binary->right = std::move(right);
        expression = std::move(binary);
    }
    return expression;
}

Parser::Impl::ExpressionPtr Parser::Impl::parseBitwiseOr()
{
    ExpressionPtr expression = parseBitwiseXor();
    while (match(Token::Type::Pipe))
    {
        Token opToken = previous();
        ExpressionPtr right = parseBitwiseXor();
        auto binary = std::make_unique<BinaryExpression>();
        binary->operatorToken = opToken;
        binary->op = BinaryOperator::BitwiseOr;
        binary->left = std::move(expression);
        binary->right = std::move(right);
        expression = std::move(binary);
    }
    return expression;
}

Parser::Impl::ExpressionPtr Parser::Impl::parseBitwiseXor()
{
    ExpressionPtr expression = parseBitwiseAnd();
    while (match(Token::Type::Caret))
    {
        Token opToken = previous();
        ExpressionPtr right = parseBitwiseAnd();
        auto binary = std::make_unique<BinaryExpression>();
        binary->operatorToken = opToken;
        binary->op = BinaryOperator::BitwiseXor;
        binary->left = std::move(expression);
        binary->right = std::move(right);
        expression = std::move(binary);
    }
    return expression;
}

Parser::Impl::ExpressionPtr Parser::Impl::parseBitwiseAnd()
{
    ExpressionPtr expression = parseEquality();
    while (match(Token::Type::Ampersand))
    {
        Token opToken = previous();
        ExpressionPtr right = parseEquality();
        auto binary = std::make_unique<BinaryExpression>();
        binary->operatorToken = opToken;
        binary->op = BinaryOperator::BitwiseAnd;
        binary->left = std::move(expression);
        binary->right = std::move(right);
        expression = std::move(binary);
    }
    return expression;
}
Parser::Impl::ExpressionPtr Parser::Impl::parseEquality()
{
    ExpressionPtr expression = parseComparison();
    while (match({Token::Type::Equal, Token::Type::BangEqual}))
    {
        Token opToken = previous();
        Token::Type opType = opToken.type;
        ExpressionPtr right = parseComparison();
        auto binary = std::make_unique<BinaryExpression>();
        binary->operatorToken = opToken;
        binary->op = (opType == Token::Type::Equal) ? BinaryOperator::Equal : BinaryOperator::NotEqual;
        binary->left = std::move(expression);
        binary->right = std::move(right);
        expression = std::move(binary);
    }
    return expression;
}

Parser::Impl::ExpressionPtr Parser::Impl::parseComparison()
{
    ExpressionPtr expression = parseTerm();
    while (match({Token::Type::Less, Token::Type::LessEqual, Token::Type::Greater, Token::Type::GreaterEqual}))
    {
        Token opToken = previous();
        Token::Type opType = opToken.type;
        ExpressionPtr right = parseTerm();
        auto binary = std::make_unique<BinaryExpression>();
        binary->operatorToken = opToken;
        binary->op = binaryOperatorFromToken(opType);
        binary->left = std::move(expression);
        binary->right = std::move(right);
        expression = std::move(binary);
    }
    return expression;
}

Parser::Impl::ExpressionPtr Parser::Impl::parseTerm()
{
    ExpressionPtr expression = parseFactor();
    while (match({Token::Type::Plus, Token::Type::Minus}))
    {
        Token opToken = previous();
        Token::Type opType = opToken.type;
        ExpressionPtr right = parseFactor();
        auto binary = std::make_unique<BinaryExpression>();
        binary->operatorToken = opToken;
        binary->op = (opType == Token::Type::Plus) ? BinaryOperator::Add : BinaryOperator::Subtract;
        binary->left = std::move(expression);
        binary->right = std::move(right);
        expression = std::move(binary);
    }
    return expression;
}

Parser::Impl::ExpressionPtr Parser::Impl::parseFactor()
{
    ExpressionPtr expression = parseUnary();
    while (match({Token::Type::Star, Token::Type::Slash, Token::Type::Percent}))
    {
        Token opToken = previous();
        Token::Type opType = opToken.type;
        ExpressionPtr right = parseUnary();
        auto binary = std::make_unique<BinaryExpression>();
        binary->operatorToken = opToken;
        switch (opType)
        {
            case Token::Type::Star:
                binary->op = BinaryOperator::Multiply;
                break;
            case Token::Type::Slash:
                binary->op = BinaryOperator::Divide;
                break;
            case Token::Type::Percent:
                binary->op = BinaryOperator::Modulo;
                break;
            default:
                break;
        }
        binary->left = std::move(expression);
        binary->right = std::move(right);
        expression = std::move(binary);
    }
    return expression;
}

Parser::Impl::ExpressionPtr Parser::Impl::parseUnary()
{
    if (match({Token::Type::Plus, Token::Type::Minus, Token::Type::Bang, Token::Type::Tilde, Token::Type::PlusPlus,
            Token::Type::MinusMinus}))
    {
        Token::Type opType = previous().type;
        ExpressionPtr operand = parseUnary();
        auto unary = std::make_unique<UnaryExpression>();
        unary->op = unaryOperatorFromToken(opType);
        unary->operand = std::move(operand);
        return unary;
    }

    return parsePostfix();
}

Parser::Impl::ExpressionPtr Parser::Impl::parsePostfix()
{
    ExpressionPtr expression = parsePrimary();
    if (!expression)
    {
        return nullptr;
    }

    while (true)
    {
        if (match(Token::Type::LeftParen))
        {
            expression = finishCall(std::move(expression));
            continue;
        }
        if (match(Token::Type::Dot))
        {
            Token member = consumeIdentifierToken(IdentifierContext::General, "Expected member name after '.'");
            auto access = std::make_unique<MemberExpression>();
            access->object = std::move(expression);
            access->member = std::move(member);
            expression = std::move(access);
            continue;
        }
        if (match(Token::Type::LeftBracket))
        {
            ExpressionPtr index = parseExpression();
            consume(Token::Type::RightBracket, "Expected ']' after index expression");
            auto access = std::make_unique<IndexExpression>();
            access->object = std::move(expression);
            access->index = std::move(index);
            expression = std::move(access);
            continue;
        }
        if (match(Token::Type::PlusPlus))
        {
            auto postfix = std::make_unique<PostfixExpression>();
            postfix->op = PostfixOperator::Increment;
            postfix->operand = std::move(expression);
            expression = std::move(postfix);
            continue;
        }
        if (match(Token::Type::MinusMinus))
        {
            auto postfix = std::make_unique<PostfixExpression>();
            postfix->op = PostfixOperator::Decrement;
            postfix->operand = std::move(expression);
            expression = std::move(postfix);
            continue;
        }
        break;
    }

    return expression;
}

Parser::Impl::ExpressionPtr Parser::Impl::parsePrimary()
{
    const Token &token = peek();
    switch (token.type)
    {
        case Token::Type::IntegerLiteral:
        case Token::Type::FloatLiteral:
        case Token::Type::StringLiteral:
        case Token::Type::HeaderLiteral:
        case Token::Type::KeywordTrue:
        case Token::Type::KeywordFalse:
            advance();
            return makeLiteralExpression(token);
		case Token::Type::Identifier:
		case Token::Type::KeywordTexture:
		case Token::Type::KeywordThis:
		{
			Token first = token;
			advance();
            return parseIdentifierExpression(first);
        }
        case Token::Type::LeftParen:
        {
            advance();
            ExpressionPtr expression = parseExpression();
            consume(Token::Type::RightParen, "Expected ')' after expression");
            return expression;
        }
        default:
            break;
    }

    reportError("Unexpected token in expression", token);
    return nullptr;
}
Parser::Impl::ExpressionPtr Parser::Impl::finishCall(ExpressionPtr callee)
{
    auto call = std::make_unique<CallExpression>();
    call->callee = std::move(callee);
    call->arguments = parseArgumentListAfterLeftParen();
    return call;
}

std::vector<Parser::Impl::ExpressionPtr> Parser::Impl::parseArgumentListAfterLeftParen()
{
    std::vector<ExpressionPtr> arguments;
    if (!check(Token::Type::RightParen))
    {
        do
        {
            arguments.push_back(parseExpression());
        } while (match(Token::Type::Comma));
    }
    consume(Token::Type::RightParen, "Expected ')' after arguments");
    return arguments;
}

Parser::Impl::ExpressionPtr Parser::Impl::parseDirectInitializer(const TypeName &type)
{
    consume(Token::Type::LeftParen, "Expected '(' to start initializer");
    auto call = std::make_unique<CallExpression>();
    call->callee = makeTypeExpression(type);
    call->arguments = parseArgumentListAfterLeftParen();
    return call;
}

Parser::Impl::ExpressionPtr Parser::Impl::makeTypeExpression(const TypeName &type)
{
    auto identifier = std::make_unique<IdentifierExpression>();
    identifier->name = type.name;
    return identifier;
}

Parser::Impl::ExpressionPtr Parser::Impl::parseIdentifierExpression(Token firstToken)
{
    Name name;
    name.parts.push_back(std::move(firstToken));
    while (match(Token::Type::DoubleColon))
    {
        name.parts.push_back(consumeIdentifierToken(IdentifierContext::General, "Expected identifier after '::'"));
    }

    auto expression = std::make_unique<IdentifierExpression>();
    expression->name = std::move(name);
    return expression;
}

Parser::Impl::ExpressionPtr Parser::Impl::makeLiteralExpression(const Token &token)
{
    auto literal = std::make_unique<LiteralExpression>();
    literal->literal = token;
    return literal;
}

BinaryOperator Parser::Impl::binaryOperatorFromToken(Token::Type type) const
{
    switch (type)
    {
        case Token::Type::Plus:
            return BinaryOperator::Add;
        case Token::Type::Minus:
            return BinaryOperator::Subtract;
        case Token::Type::Star:
            return BinaryOperator::Multiply;
        case Token::Type::Slash:
            return BinaryOperator::Divide;
        case Token::Type::Percent:
            return BinaryOperator::Modulo;
        case Token::Type::Less:
            return BinaryOperator::Less;
        case Token::Type::LessEqual:
            return BinaryOperator::LessEqual;
        case Token::Type::Greater:
            return BinaryOperator::Greater;
        case Token::Type::GreaterEqual:
            return BinaryOperator::GreaterEqual;
        case Token::Type::Equal:
            return BinaryOperator::Equal;
        case Token::Type::BangEqual:
            return BinaryOperator::NotEqual;
        case Token::Type::AmpersandAmpersand:
            return BinaryOperator::LogicalAnd;
        case Token::Type::PipePipe:
            return BinaryOperator::LogicalOr;
        case Token::Type::Ampersand:
            return BinaryOperator::BitwiseAnd;
        case Token::Type::Pipe:
            return BinaryOperator::BitwiseOr;
        case Token::Type::Caret:
            return BinaryOperator::BitwiseXor;
        default:
            break;
    }
    return BinaryOperator::Add;
}

AssignmentOperator Parser::Impl::assignmentOperatorFromToken(Token::Type type) const
{
    switch (type)
    {
        case Token::Type::Assign:
            return AssignmentOperator::Assign;
        case Token::Type::PlusEqual:
            return AssignmentOperator::AddAssign;
        case Token::Type::MinusEqual:
            return AssignmentOperator::SubtractAssign;
        case Token::Type::StarEqual:
            return AssignmentOperator::MultiplyAssign;
        case Token::Type::SlashEqual:
            return AssignmentOperator::DivideAssign;
        case Token::Type::PercentEqual:
            return AssignmentOperator::ModuloAssign;
        case Token::Type::AmpersandEqual:
            return AssignmentOperator::BitwiseAndAssign;
        case Token::Type::PipeEqual:
            return AssignmentOperator::BitwiseOrAssign;
        case Token::Type::CaretEqual:
            return AssignmentOperator::BitwiseXorAssign;
        default:
            break;
    }
    return AssignmentOperator::Assign;
}

UnaryOperator Parser::Impl::unaryOperatorFromToken(Token::Type type) const
{
    switch (type)
    {
        case Token::Type::Plus:
            return UnaryOperator::Positive;
        case Token::Type::Minus:
            return UnaryOperator::Negate;
        case Token::Type::Bang:
            return UnaryOperator::LogicalNot;
        case Token::Type::Tilde:
            return UnaryOperator::BitwiseNot;
        case Token::Type::PlusPlus:
            return UnaryOperator::PreIncrement;
        case Token::Type::MinusMinus:
            return UnaryOperator::PreDecrement;
        default:
            break;
    }
    return UnaryOperator::Positive;
}

PostfixOperator Parser::Impl::postfixOperatorFromToken(Token::Type type) const
{
    return (type == Token::Type::PlusPlus) ? PostfixOperator::Increment : PostfixOperator::Decrement;
}

bool Parser::Impl::isAssignmentOperator(Token::Type type) const
{
    switch (type)
    {
        case Token::Type::Assign:
        case Token::Type::PlusEqual:
        case Token::Type::MinusEqual:
        case Token::Type::StarEqual:
        case Token::Type::SlashEqual:
        case Token::Type::PercentEqual:
        case Token::Type::AmpersandEqual:
        case Token::Type::PipeEqual:
        case Token::Type::CaretEqual:
            return true;
        default:
            return false;
    }
}
bool Parser::Impl::match(Token::Type type)
{
    if (check(type))
    {
        advance();
        return true;
    }
    return false;
}

bool Parser::Impl::match(std::initializer_list<Token::Type> types)
{
    for (Token::Type type : types)
    {
        if (check(type))
        {
            advance();
            return true;
        }
    }
    return false;
}

bool Parser::Impl::check(Token::Type type) const
{
    if (isAtEnd())
    {
        return false;
    }
    return peek().type == type;
}

bool Parser::Impl::checkNext(Token::Type type) const
{
    if (current + 1 >= tokens.size())
    {
        return false;
    }
    return tokens[current + 1].type == type;
}

const Token &Parser::Impl::advance()
{
    if (!isAtEnd())
    {
        ++current;
    }
    return previous();
}

bool Parser::Impl::isAtEnd() const
{
    return peek().type == Token::Type::EndOfFile;
}

const Token &Parser::Impl::peek(std::size_t offset) const
{
    const std::size_t index = std::min(current + offset, tokens.size() - 1);
    return tokens[index];
}

const Token &Parser::Impl::previous() const
{
    return tokens[current - 1];
}

Token Parser::Impl::consume(Token::Type type, std::string_view message)
{
    if (check(type))
    {
        Token token = peek();
        advance();
        return token;
    }

    Token token = peek();
    reportError(std::string(message), token);
    return token;
}

void Parser::Impl::reportError(const std::string &message, const Token &token)
{
    emitError(message, token);
    skipToNextLine(token.start.line + 1);
}

void Parser::Impl::skipToNextLine(std::size_t line)
{
    while (!isAtEnd() && peek().start.line < line)
    {
        advance();
    }
}
