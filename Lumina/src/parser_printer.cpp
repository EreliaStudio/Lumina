#include "parser.hpp"

namespace Lumina
{
    void Parser::printArraySizes(const std::vector<size_t>& arraySize) const {
        if (!arraySize.empty()) {
            std::cout << "[";
            for (size_t i = 0; i < arraySize.size(); ++i) {
                if (i > 0) std::cout << ", ";
                std::cout << arraySize[i];
            }
            std::cout << "]";
        }
    }

    void Parser::printParameters(const std::vector<Parameter>& parameters) const {
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
                printArraySizes(param.arraySize);
            }
            std::cout << " " << (param.isReference ? "in " : "") << param.name;
        }
    }

    void Parser::printVariable(const Variable& var, const std::string& indent) const {
        if (var.type == nullptr) {
            std::cout << indent << "[No type] " << var.name << "\n";
        }
        else {
            std::cout << indent << var.type->name << " " << var.name;
            printArraySizes(var.arraySize);
            std::cout << "\n";
        }
    }

    void Parser::printExpressionType(const ExpressionType& exprType) const {
        if (exprType.type == nullptr) {
            std::cout << "[No type]";
        }
        else {
            std::cout << exprType.type->name;
            printArraySizes(exprType.arraySize);
        }
    }

    void Parser::printMethods(const std::map<std::string, std::vector<Type::Method>>& methods, const std::string& title) const {
        if (!methods.empty()) {
            std::cout << "        " << title << ":\n";
            for (const auto& methodPair : methods) {
                for (const auto& method : methodPair.second) {
                    std::cout << "            ";
                    printExpressionType(method.returnType);
                    std::cout << " " << method.name << "(";
                    printParameters(method.parameters);
                    std::cout << ")\n";
                }
            }
        }
    }

    void Parser::printConstructors(const std::string& p_constructedType, const std::vector<Type::Constructor>& constructors) const {
        if (!constructors.empty()) {
            std::cout << "        Constructors:\n";
            for (const auto& constructor : constructors) {
                std::cout << "            " << p_constructedType << "(";
                printParameters(constructor.parameters);
                std::cout << ")\n";
            }
        }
    }

    void Parser::printParsedData() const {
        std::cout << "Available Types:\n";
        for (const auto& typePair : _availibleTypes) {
            const auto& type = typePair.second;

            std::cout << "    Type: " << type.name << "\n";

            if (!type.attributes.empty()) {
                std::cout << "        Attributes:\n";
                for (const auto& attr : type.attributes) {
                    printVariable(attr, "            ");
                }
            }

            printConstructors(type.name, type.constructors);
            printMethods(type.methods, "Methods");
            printMethods(type.operators, "Operators");
        }

        std::cout << "\nGlobal variables:\n";
        for (const auto& var : _globalVariables) {
            printVariable(var, "    ");
        }

        std::cout << "\nVertex variables:\n";
        for (const auto& var : _vertexVariables) {
            printVariable(var, "    ");
        }

        std::cout << "\nFragment variables:\n";
        for (const auto& var : _fragmentVariables) {
            printVariable(var, "    ");
        }

        std::cout << "\nFunctions:\n";
        for (const auto& funcPair : _availibleFunctions) {
            for (const auto& func : funcPair.second) {
                std::cout << "    ";
                printExpressionType(func.returnType);
                std::cout << " " << func.name << "(";
                printParameters(func.parameters);
                std::cout << ")\n";
            }
        }

        std::cout << "\nVertexPass main()\n";

        std::cout << "\nFragmentPass main()\n";

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