#include "lumina_compiler.hpp"

namespace Lumina
{

	std::string Compiler::compileSymbolBody(SymbolBody p_metaToken, std::vector<Variable> p_availableVariables)
	{
		std::string result;

		for (const auto& instruction : p_metaToken.instructions)
		{
			try
			{
				switch (instruction->type)
				{
				case Instruction::Type::VariableDeclaration:
				{
					result += parseVariableDeclaration(dynamic_pointer_cast<VariableDeclaration>(instruction), p_availableVariables);
					break;
				}

				case Instruction::Type::VariableAssignation:
				{
					result += parseVariableAssignation(dynamic_pointer_cast<VariableAssignation>(instruction));
					break;
				}
				case Instruction::Type::SymbolCall:
				{
					result += parseSymbolCall(dynamic_pointer_cast<SymbolCall>(instruction));
					break;
				}
				case Instruction::Type::IfStatement:
				{
					result += parseIfStatement(dynamic_pointer_cast<IfStatement>(instruction));
					break;
				}
				case Instruction::Type::WhileStatement:
				{
					result += parseWhileStatement(dynamic_pointer_cast<WhileStatement>(instruction));
					break;
				}
				case Instruction::Type::ForStatement:
				{
					result += parseForStatement(dynamic_pointer_cast<ForStatement>(instruction));
					break;
				}
				case Instruction::Type::ReturnStatement:
				{
					result += parseReturnStatement(dynamic_pointer_cast<ReturnStatement>(instruction));
					break;
				}
				case Instruction::Type::DiscardStatement:
				{
					result += parseDiscardStatement(dynamic_pointer_cast<DiscardStatement>(instruction));
					break;
				}
				default:
					throw TokenBasedError("Unknown instruction type", Token());
				}
			}
			catch (TokenBasedError& e)
			{
				_result.errors.push_back(e);
			}
		}

		return (result);
	}

	void Compiler::compileFunction(std::shared_ptr<FunctionMetaToken> p_metaToken)
	{
		Function newFunction;

		newFunction.returnType = { type(p_metaToken->returnType.type.value), p_metaToken->returnType.arraySizes };
		newFunction.name = namespacePrefix() + p_metaToken->name.content;

		for (const auto& parameter : p_metaToken->parameters)
		{
			Variable newParameter;

			newParameter.type = type(parameter.type.value);
			newParameter.name = parameter.name.content;
			newParameter.arraySizes = parameter.arraySizes;

			newFunction.parameters.push_back(newParameter);
		}

		if (_functions.contains(newFunction.name) == true)
		{
			const Function& tmpFunction = _functions[newFunction.name].front();

			if (tmpFunction.returnType != newFunction.returnType)
			{
				throw TokenBasedError("Function [" + p_metaToken->name.content + "] already defined with a different return type.", p_metaToken->name);
			}

			for (const auto& function : _functions[newFunction.name])
			{
				if (function.parameters.size() == newFunction.parameters.size())
				{
					bool different = false;

					for (size_t i = 0; different == false && i < function.parameters.size(); i++)
					{
						if (function.parameters[i].isSame(newFunction.parameters[i]) == false)
						{
							different = true;
						}
					}

					if (different == false)
					{
						throw TokenBasedError("Function [" + p_metaToken->name.content + "] already defined with a the same parameters types.", p_metaToken->name);
					}
				}
			}
		}

		_functions[newFunction.name].push_back(newFunction);

		std::string functionCode = "";

		std::vector<Variable> availableVariables;

		functionCode += newFunction.returnType.type->name + " " + newFunction.name + "(";
		for (size_t i = 0; i < newFunction.parameters.size(); i++)
		{
			if (i != 0)
				functionCode += ", ";
			functionCode += newFunction.parameters[i].type->name + " " + newFunction.parameters[i].name;
			for (const auto& size : newFunction.parameters[i].arraySizes)
			{
				functionCode += "[" + std::to_string(size) + "]";
			}
			availableVariables.push_back(newFunction.parameters[i]);
		}
		functionCode += "){\n";
		functionCode += compileSymbolBody(p_metaToken->body, availableVariables);
		functionCode += "};\n";

		_result.value.vertexShaderCode += functionCode;
		_result.value.fragmentShaderCode += functionCode;

	}

	void Compiler::compilePipelineBody(std::shared_ptr<PipelineBodyMetaToken> p_metaToken)
	{
		std::string functionCode = "";

		functionCode += "void main(){\n";
		functionCode += compileSymbolBody(p_metaToken->body, (p_metaToken->target == "VertexPass" ? _vertexVariables : _fragmentVariables));
		functionCode += "};\n";

		if (p_metaToken->target == "VertexPass")
		{
			_result.value.vertexShaderCode += functionCode;
		}
		else if (p_metaToken->target == "FragmentPass")
		{
			_result.value.fragmentShaderCode += functionCode;
		}
		else
		{
			throw TokenBasedError("Invalid pipeline pass definition.", p_metaToken->target);
		}
	}
}