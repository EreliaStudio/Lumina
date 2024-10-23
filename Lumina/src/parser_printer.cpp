#include "parser.hpp"

namespace Lumina
{
    void Parser::_printArraySizes(const std::vector<size_t>& arraySize) const {
        if (!arraySize.empty()) {
            std::cout << "[";
            for (size_t i = 0; i < arraySize.size(); ++i) {
                if (i > 0) std::cout << ", ";
                std::cout << arraySize[i];
            }
            std::cout << "]";
        }
    }

    void Parser::_printParameters(const std::vector<Parameter>& parameters) const {
        for (size_t i = 0; i < parameters.size(); ++i) {
            if (i != 0) {
                std::cout << ", ";
            }
            const auto& param = parameters[i];
            if (param.type == nullptr) {
                std::cout << "[No type]";
            }
            else {
                std::cout << param.type->name;
                _printArraySizes(param.arraySize);
            }
            std::cout << " " << (param.isReference ? "in " : "") << param.name;
        }
    }

    void Parser::_printVariable(const Variable& var, const std::string& indent) const {
        if (var.type == nullptr) {
            std::cout << indent << "[No type] " << var.name << "\n";
        }
        else {
            std::cout << indent << var.type->name << " " << var.name;
            _printArraySizes(var.arraySize);
            std::cout << "\n";
        }
    }

    void Parser::_printExpressionType(const ExpressionType& exprType) const {
        if (exprType.type == nullptr) {
            std::cout << "[No type]";
        }
        else {
            std::cout << exprType.type->name;
            _printArraySizes(exprType.arraySize);
        }
    }

    void Parser::_printMethods(const std::map<std::string, std::vector<Type::Method>>& methods, const std::string& title) const {
        if (!methods.empty()) {
            std::cout << "        " << title << ":\n";
            for (const auto& methodPair : methods) {
                for (const auto& method : methodPair.second) {
                    std::cout << "            ";
                    _printExpressionType(method.returnType);
                    std::cout << " " << method.name << "(";
                    _printParameters(method.parameters);
                    std::cout << ")";
                    if (!method.body.statements.empty()) {
                        std::cout << " {\n";
                        _printSymbolBody(method.body, "                ");
                        std::cout << "            }\n";
                    }
                    else {
                        std::cout << ";\n";
                    }
                }
            }
        }
    }

    void Parser::_printConstructors(const std::string& p_constructedType, const std::vector<Type::Constructor>& constructors) const {
        if (!constructors.empty()) {
            std::cout << "        Constructors:\n";
            for (const auto& constructor : constructors) {
                std::cout << "            " << p_constructedType << "(";
                _printParameters(constructor.parameters);
                std::cout << ")";
                if (!constructor.body.statements.empty()) {
                    std::cout << " {\n";
                    _printSymbolBody(constructor.body, "                ");
                    std::cout << "            }\n";
                }
                else {
                    std::cout << ";\n";
                }
            }
        }
    }

    void Parser::_printSymbolBody(const SymbolBody& body, const std::string& indent) const {
        for (const auto& stmt : body.statements) {
            _printStatement(stmt, indent);
        }
    }

    void Parser::_printStatement(const std::shared_ptr<Statement>& stmt, const std::string& indent) const {
        if (auto varDecl = std::dynamic_pointer_cast<VariableDeclarationStatement>(stmt)) {
            _printVariableDeclarationStatement(*varDecl, indent);
        }
        else if (auto exprStmt = std::dynamic_pointer_cast<ExpressionStatement>(stmt)) {
            _printExpressionStatement(*exprStmt, indent);
        }
        else if (auto assignStmt = std::dynamic_pointer_cast<AssignmentStatement>(stmt)) {
            _printAssignmentStatement(*assignStmt, indent);
        }
        else if (auto returnStmt = std::dynamic_pointer_cast<ReturnStatement>(stmt)) {
            _printReturnStatement(*returnStmt, indent);
        }
        else if (auto discardStmt = std::dynamic_pointer_cast<DiscardStatement>(stmt)) {
            std::cout << indent << "discard;\n";
        }
        else if (auto ifStmt = std::dynamic_pointer_cast<IfStatement>(stmt)) {
            _printIfStatement(*ifStmt, indent);
        }
        else if (auto whileStmt = std::dynamic_pointer_cast<WhileStatement>(stmt)) {
            _printWhileStatement(*whileStmt, indent);
        }
        else if (auto forStmt = std::dynamic_pointer_cast<ForStatement>(stmt)) {
            _printForStatement(*forStmt, indent);
        }
        else if (auto compoundStmt = std::dynamic_pointer_cast<CompoundStatement>(stmt)) {
            _printCompoundStatement(*compoundStmt, indent);
        }
        else {
            std::cout << indent << "[Unknown Statement]\n";
        }
    }

    void Parser::_printVariableDeclarationStatement(const VariableDeclarationStatement& stmt, const std::string& indent) const {
        std::cout << indent;
        if (stmt.variable.type) {
            std::cout << stmt.variable.type->name << " ";
        }
        else {
            std::cout << "[Unknown Type] ";
        }
        std::cout << stmt.variable.name;
        _printArraySizes(stmt.variable.arraySize);
        if (stmt.initializer) {
            std::cout << " = ";
            _printExpression(stmt.initializer, "");
        }
        std::cout << ";\n";
    }

    void Parser::_printExpressionStatement(const ExpressionStatement& stmt, const std::string& indent) const {
        std::cout << indent;
        _printExpression(stmt.expression, "");
        std::cout << ";\n";
    }
    
    std::string Parser::_assignatorOperatorToString(AssignatorOperator op) {
        switch (op) {
        case AssignatorOperator::Equal: return "=";
        case AssignatorOperator::PlusEqual: return "+=";
        case AssignatorOperator::MinusEqual: return "-=";
        case AssignatorOperator::MultiplyEqual: return "*=";
        case AssignatorOperator::DivideEqual: return "/=";
        case AssignatorOperator::ModuloEqual: return "%=";
        default: return "?Assignator";
        }
    }

    void Parser::_printAssignmentStatement(const AssignmentStatement& stmt, const std::string& indent) const {
        std::cout << indent;
        _printExpression(stmt.target, "");
        std::cout << " " << _assignatorOperatorToString(stmt.op) << " ";
        _printExpression(stmt.value, "");
        std::cout << ";\n";
    }

    void Parser::_printReturnStatement(const ReturnStatement& stmt, const std::string& indent) const {
        std::cout << indent << "return";
        if (stmt.expression) {
            std::cout << " ";
            _printExpression(stmt.expression, "");
        }
        std::cout << ";\n";
    }

    void Parser::_printIfStatement(const IfStatement& stmt, const std::string& indent) const {
        for (size_t i = 0; i < stmt.branches.size(); ++i) {
            const auto& branch = stmt.branches[i];
            if (i == 0) {
                std::cout << indent << "if (";
            }
            else {
                std::cout << indent << "else if (";
            }
            _printExpression(branch.condition, "");
            std::cout << ") {\n";
            _printSymbolBody(branch.body, indent + "    ");
            std::cout << indent << "}\n";
        }
        if (!stmt.elseBody.statements.empty()) {
            std::cout << indent << "else {\n";
            _printSymbolBody(stmt.elseBody, indent + "    ");
            std::cout << indent << "}\n";
        }
    }

    void Parser::_printWhileStatement(const WhileStatement& stmt, const std::string& indent) const {
        std::cout << indent << "while (";
        _printExpression(stmt.condition, "");
        std::cout << ") {\n";
        _printSymbolBody(stmt.body, indent + "    ");
        std::cout << indent << "}\n";
    }

    void Parser::_printForStatement(const ForStatement& stmt, const std::string& indent) const {
        std::cout << indent << "for (";
        if (stmt.initializer) {
            _printStatement(stmt.initializer, "");
        }
        else {
            std::cout << "; ";
        }
        if (stmt.condition) {
            _printExpression(stmt.condition, "");
        }
        std::cout << "; ";
        if (stmt.increment) {
            _printExpression(stmt.increment, "");
        }
        std::cout << ") {\n";
        _printSymbolBody(stmt.body, indent + "    ");
        std::cout << indent << "}\n";
    }

    void Parser::_printCompoundStatement(const CompoundStatement& stmt, const std::string& indent) const {
        std::cout << indent << "{\n";
        _printSymbolBody(stmt.body, indent + "    ");
        std::cout << indent << "}\n";
    }

    void Parser::_printExpression(const std::shared_ptr<Expression>& expr, const std::string& indent) const {
        if (auto literalExpr = std::dynamic_pointer_cast<LiteralExpression>(expr)) {
            _printLiteralExpression(*literalExpr, indent);
        }
        else if (auto varExpr = std::dynamic_pointer_cast<VariableExpression>(expr)) {
            _printVariableExpression(*varExpr, indent);
        }
        else if (auto binaryExpr = std::dynamic_pointer_cast<BinaryExpression>(expr)) {
            _printBinaryExpression(*binaryExpr, indent);
        }
        else if (auto unaryExpr = std::dynamic_pointer_cast<UnaryExpression>(expr)) {
            _printUnaryExpression(*unaryExpr, indent);
        }
        else if (auto funcCallExpr = std::dynamic_pointer_cast<FunctionCallExpression>(expr)) {
            _printFunctionCallExpression(*funcCallExpr, indent);
        }
        else if (auto memberAccessExpr = std::dynamic_pointer_cast<MemberAccessExpression>(expr)) {
            _printMemberAccessExpression(*memberAccessExpr, indent);
        }
        else if (auto arrayAccessExpr = std::dynamic_pointer_cast<ArrayAccessExpression>(expr)) {
            _printArrayAccessExpression(*arrayAccessExpr, indent);
        }
        else if (auto castExpr = std::dynamic_pointer_cast<CastExpression>(expr)) {
            _printCastExpression(*castExpr, indent);
        }
        else {
            std::cout << indent << "[Unknown Expression]";
        }
    }

    void Parser::_printLiteralExpression(const LiteralExpression& expr, const std::string& indent) const {
        std::visit([](auto&& value) {
            std::cout << value;
            }, expr.value);
    }

    void Parser::_printVariableExpression(const VariableExpression& expr, const std::string& indent) const {
        std::cout << expr.variableName;
    }

    std::string Parser::_arithmeticOperatorToString(ArithmeticOperator op) {
        switch (op) {
        case ArithmeticOperator::Plus: return "+";
        case ArithmeticOperator::Minus: return "-";
        case ArithmeticOperator::Multiply: return "*";
        case ArithmeticOperator::Divide: return "/";
        case ArithmeticOperator::Modulo: return "%";
        case ArithmeticOperator::ConditionEqual: return "==";
        case ArithmeticOperator::NotEqual: return "!=";
        case ArithmeticOperator::Less: return "<";
        case ArithmeticOperator::Greater: return ">";
        case ArithmeticOperator::LessEqual: return "<=";
        case ArithmeticOperator::GreaterEqual: return ">=";
        case ArithmeticOperator::LogicalAnd: return "&&";
        case ArithmeticOperator::LogicalOr: return "||";
        case ArithmeticOperator::Equal: return "=";
        case ArithmeticOperator::PlusEqual: return "+=";
        case ArithmeticOperator::MinusEqual: return "-=";
        case ArithmeticOperator::MultiplyEqual: return "*=";
        case ArithmeticOperator::DivideEqual: return "/=";
        case ArithmeticOperator::ModuloEqual: return "%=";
        default: return "?Arithmetic";
        }
    }

    void Parser::_printBinaryExpression(const BinaryExpression& expr, const std::string& indent) const {
        std::cout << "(";
        _printExpression(expr.left, "");
        std::cout << " " << _arithmeticOperatorToString(expr.op) << " ";
        _printExpression(expr.right, "");
        std::cout << ")";
    }

    std::string Parser::_unaryOperatorToString(UnaryOperator op) {
        switch (op) {
        case UnaryOperator::Increment: return "++";
        case UnaryOperator::Decrement: return "--";
        default: return "?Unary";
        }
    }

    void Parser::_printUnaryExpression(const UnaryExpression& expr, const std::string& indent) const {
        std::cout << _unaryOperatorToString(expr.op);
        _printExpression(expr.operand, "");
    }

    void Parser::_printFunctionCallExpression(const FunctionCallExpression& expr, const std::string& indent) const {
        std::cout << expr.functionName << "(";
        for (size_t i = 0; i < expr.arguments.size(); ++i) {
            if (i > 0) std::cout << ", ";
            _printExpression(expr.arguments[i], "");
        }
        std::cout << ")";
    }

    void Parser::_printMemberAccessExpression(const MemberAccessExpression& expr, const std::string& indent) const {
        _printExpression(expr.object, "");
        std::cout << "." << expr.memberName;
    }

    void Parser::_printArrayAccessExpression(const ArrayAccessExpression& expr, const std::string& indent) const {
        _printExpression(expr.array, "");
        std::cout << "[";
        _printExpression(expr.index, "");
        std::cout << "]";
    }

    void Parser::_printCastExpression(const CastExpression& expr, const std::string& indent) const {
        std::cout << "(" << expr.targetType.type->name << ")";
        std::cout << "(";
        for (size_t i = 0; i < expr.arguments.size(); ++i) {
            if (i > 0) std::cout << ", ";
            _printExpression(expr.arguments[i], "");
        }
        std::cout << ")";
    }


    void Parser::_printParsedData() const {
        std::cout << "Available Types:\n";
        for (const auto& typePair : _availibleTypes) {
            const auto& type = typePair.second;

            std::cout << "    Type: " << type.name << "\n";

            if (!type.attributes.empty()) {
                std::cout << "        Attributes:\n";
                for (const auto& attr : type.attributes) {
                    _printVariable(attr, "            ");
                }
            }

            _printConstructors(type.name, type.constructors);
            _printMethods(type.methods, "Methods");
            _printMethods(type.operators, "Operators");
        }

        std::cout << "\nGlobal variables:\n";
        for (const auto& var : _globalVariables) {
            _printVariable(var, "    ");
        }

        std::cout << "\nVertex variables:\n";
        for (const auto& var : _vertexVariables) {
            _printVariable(var, "    ");
        }

        std::cout << "\nFragment variables:\n";
        for (const auto& var : _fragmentVariables) {
            _printVariable(var, "    ");
        }

        std::cout << "\nFunctions:\n";
        for (const auto& funcPair : _availibleFunctions) {
            for (const auto& func : funcPair.second) {
                std::cout << "    ";
                _printExpressionType(func.returnType);
                std::cout << " " << func.name << "(";
                _printParameters(func.parameters);
                std::cout << ")";

                if (!func.body.statements.empty()) {
                    std::cout << " {\n";
                    _printSymbolBody(func.body, "        ");
                    std::cout << "    }\n";
                }
                else {
                    std::cout << ";\n";
                }
            }
        }

        std::cout << "\nVertexPass main() {\n";
        _printSymbolBody(_vertexPassMain.body, "    ");
        std::cout << "}\n";

        std::cout << "\nFragmentPass main() {\n";
        _printSymbolBody(_fragmentPassMain.body, "    ");
        std::cout << "}\n";

        std::cout << "\nAttribute Types:\n";
        for (const auto& attrType : _attributesTypes) {
            if (attrType == nullptr) {
                std::cout << "    Inserted a nullptr type in attribute\n";
            }
            else {
                std::cout << "    " << attrType->name << "\n";
            }
        }

        std::cout << "\nConstants Types:\n";
        for (const auto& constType : _constantsTypes) {
            if (constType == nullptr) {
                std::cout << "    Inserted a nullptr type in Constant\n";
            }
            else {
                std::cout << "    " << constType->name << "\n";
            }
        }
    }

}