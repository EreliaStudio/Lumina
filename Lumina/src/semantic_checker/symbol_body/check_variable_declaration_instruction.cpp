#include "lumina_semantic_checker.hpp"

namespace Lumina
{
	void SemanticChecker::checkVariableDeclarationInstruction(const std::filesystem::path& p_file, const std::shared_ptr<VariableDeclarationInstruction>& p_instruction, std::unordered_map<std::string, Variable>& p_variables)
	{
		Token typeToken = Token::merge(p_instruction->type->tokens, Token::Type::Identifier);
		
		std::string typeName = typeToken.content;
		
		Type* varType = type(typeName);

		if (varType == nullptr)
		{
			throw TokenBasedError(p_file, "Type [" + typeName + "] not found", typeToken);
		}

		if (p_variables.contains(p_instruction->name.content))
		{
			throw TokenBasedError(p_file, "Variable [" + p_instruction->name.content + "] already declared in this scope", p_instruction->name);
		}

		p_variables[p_instruction->name.content] = { varType, p_instruction->size};

		if (p_instruction->initializer)
		{
			checkExpression(p_file, p_instruction->initializer, p_variables, varType, p_instruction->size);
		}
	}
}