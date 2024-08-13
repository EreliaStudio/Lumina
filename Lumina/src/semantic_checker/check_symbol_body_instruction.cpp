#include "lumina_semantic_checker.hpp"

namespace Lumina
{
	void SemanticChecker::checkSymbolBodyInstruction(const std::filesystem::path& p_file, const std::shared_ptr<SymbolBodyInstruction>& p_instruction, std::unordered_map<std::string, Type*> p_variables, Type* returnType)
	{
		for (const auto& element : p_instruction->elements)
		{
			try
			{
				switch (element->type)
				{
				case Instruction::Type::VariableDeclaration:
				{
					checkVariableDeclarationInstruction(p_file, std::static_pointer_cast<VariableDeclarationInstruction>(element), p_variables);
					break;
				}
				case Instruction::Type::VariableAssignation:
				{
					checkVariableAssignationInstruction(p_file, std::static_pointer_cast<VariableAssignationInstruction>(element), p_variables);
					break;
				}
				case Instruction::Type::Return:
				{
					checkReturnInstruction(p_file, std::static_pointer_cast<ReturnInstruction>(element), p_variables, returnType);
					break;
				}
				case Instruction::Type::Discard:
				{
					checkDiscardInstruction(p_file, std::static_pointer_cast<DiscardInstruction>(element), p_variables);
					break;
				}
				case Instruction::Type::IfStatement:
				{
					checkIfStatementInstruction(p_file, std::static_pointer_cast<IfStatementInstruction>(element), p_variables, returnType);
					break;
				}
				case Instruction::Type::WhileLoop:
				{
					checkWhileLoopInstruction(p_file, std::static_pointer_cast<WhileLoopInstruction>(element), p_variables, returnType);
					break;
				}
				case Instruction::Type::SymbolCall:
				{
					checkSymbolCallInstruction(p_file, std::static_pointer_cast<SymbolCallInstruction>(element), p_variables, returnType);
				}
				default:
				{
					throw TokenBasedError(p_file, "Unexpected instruction type in symbol body - type : " + ::to_string(element->type), element->mergedToken());
				}
				}
			}
			catch (TokenBasedError& e)
			{
				_result.errors.push_back(e);
			}
			
		}
	}
}