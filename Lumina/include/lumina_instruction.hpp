#pragma once

#include <vector>
#include <memory>
#include <optional>
#include <variant>
#include <string>
#include "lumina_token.hpp"
#include "lumina_descriptors.hpp"

namespace Lumina
{
    // Base class for all instructions
    struct Instruction {
        enum class Type {
            Unknown,
            VariableDeclaration,
            VariableAssignation,
            SymbolCall,
            IfStatement,
            WhileStatement,
            ForStatement,
            ReturnStatement,
            DiscardStatement,
            SymbolBody
        };

        Type type;

        Instruction(Type p_type) : type(p_type) {}
        virtual ~Instruction() = default;
    };

    // Represents an expression, which is composed of multiple elements
    struct Expression : public Instruction {
        // Base class for elements inside an expression
        struct Element {
            enum class Type {
                Unknown,
                Number,
                Boolean,
                VariableDesignation,
                Operator,
                SymbolCall
            };

            Type type;

            Element(Type p_type) : type(p_type) {}
            virtual ~Element() = default;
        };

        // Represents a numeric value inside an expression
        struct NumberElement : public Element {
            Lumina::Token value; // Token representing the number

            NumberElement() : Element(Type::Number) {}
        };

        // Represents a boolean value inside an expression
        struct BooleanElement : public Element {
            Lumina::Token value; // Token representing true or false

            BooleanElement() : Element(Type::Boolean) {}
        };

        // Represents a variable or member access inside an expression
        struct VariableDesignationElement : public Element {
            std::vector<Lumina::Token> namespaceChain; // Optional namespace or struct member chain
            Lumina::Token name; // The actual variable name
            std::optional<std::shared_ptr<Expression>> index; // Optional array index or expression

            VariableDesignationElement() : Element(Type::VariableDesignation) {}
        };

        // Represents an operator inside an expression
        struct OperatorElement : public Element {
            Lumina::Token operatorToken; // Token representing the operator, e.g., +, -, *, /

            OperatorElement() : Element(Type::Operator) {}
        };

        // Represents a function or symbol call inside an expression
        struct SymbolCallElement : public Element {
            std::vector<Lumina::Token> namespaceChain; // Optional namespace chain
            Lumina::Token functionName; // The function or symbol being called
            std::vector<std::shared_ptr<Expression>> parameters; // Parameters for the function call

            SymbolCallElement() : Element(Type::SymbolCall) {}
        };

        std::vector<std::shared_ptr<Element>> elements; // Elements making up the expression

        Expression() : Instruction(Type::SymbolBody) {} // Adjust the type based on your needs
    };

    // Represents a variable declaration (including type)
    struct VariableDeclaration : public Instruction {
        VariableDescriptor descriptor; // Contains type and name (e.g., int value)
        std::optional<std::shared_ptr<Expression>> initialValue; // Optional initial value (e.g., int value = 5)

        VariableDeclaration() : Instruction(Type::VariableDeclaration) {}
    };

    // Represents a variable assignment
    struct VariableAssignation : public Instruction {
        std::shared_ptr<Expression::VariableDesignationElement> target; // The target variable being assigned (without type)
        std::shared_ptr<Expression> value; // The value being assigned (e.g., value = expression)

        VariableAssignation() : Instruction(Type::VariableAssignation) {}
    };

    // Represents a function or symbol call
    struct SymbolCall : public Instruction {
        std::vector<Lumina::Token> namespaceChain; // Optional namespace chain
        Lumina::Token functionName; // The function or symbol being called
        std::vector<std::shared_ptr<Expression>> parameters; // Parameters for the function call

        SymbolCall() : Instruction(Type::SymbolCall) {}
    };

    // Represents a single branch in an if-else structure (if, else if, else)
    struct ConditionalBranch {
        std::shared_ptr<Expression> condition; // Optional condition, null for 'else'
        std::vector<std::shared_ptr<Instruction>> body; // Instructions executed in this branch

        ConditionalBranch() = default;
    };

    // Represents an if-else statement with multiple branches
    struct IfStatement : public Instruction {
        std::vector<ConditionalBranch> branches; // List of all branches (if, else if, else)

        IfStatement() : Instruction(Type::IfStatement) {}
    };

    // Represents a while statement
    struct WhileStatement : public Instruction {
        std::shared_ptr<Expression> condition; // The loop condition
        std::vector<std::shared_ptr<Instruction>> body; // Instructions executed each loop iteration

        WhileStatement() : Instruction(Type::WhileStatement) {}
    };

    // Represents a for statement
    struct ForStatement : public Instruction {
        std::shared_ptr<VariableDeclaration> initializer; // The initial variable declaration
        std::shared_ptr<Expression> condition; // The loop condition
        std::shared_ptr<VariableAssignation> increment; // The increment expression
        std::vector<std::shared_ptr<Instruction>> body; // Instructions executed each loop iteration

        ForStatement() : Instruction(Type::ForStatement) {}
    };

    // Represents a return statement
    struct ReturnStatement : public Instruction {
        std::optional<std::shared_ptr<Expression>> returnValue; // Optional value to return

        ReturnStatement() : Instruction(Type::ReturnStatement) {}
    };

    // Represents a discard statement
    struct DiscardStatement : public Instruction {
        DiscardStatement() : Instruction(Type::DiscardStatement) {}
    };

    // Composite class for symbol body, containing multiple instructions
    struct SymbolBody : public Instruction {
        std::vector<std::shared_ptr<Instruction>> instructions;

        SymbolBody() : Instruction(Type::SymbolBody) {}
    };
}
