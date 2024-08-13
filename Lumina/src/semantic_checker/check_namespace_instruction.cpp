#include "lumina_semantic_checker.hpp"

namespace Lumina
{
	void SemanticChecker::checkNamespaceInstruction(const std::filesystem::path& p_file, const std::shared_ptr<NamespaceInstruction>& p_instruction)
	{
		size_t namespaceIndex = 0;

		_currentNamespace.push_back(p_instruction->name);

		while (namespaceIndex < p_instruction->instructions.size())
		{
			try
			{
				std::shared_ptr<AbstractInstruction> instruction = p_instruction->instructions[namespaceIndex];

				switch (instruction->type)
				{
				case Instruction::Type::StructureBlock:
				{
					checkStructureInstruction(p_file, static_pointer_cast<StructureBlockInstruction>(instruction));
					break;
				}
				case Instruction::Type::AttributeBlock:
				{
					checkAttributeInstruction(p_file, static_pointer_cast<AttributeBlockInstruction>(instruction));
					break;
				}
				case Instruction::Type::ConstantBlock:
				{
					checkConstantInstruction(p_file, static_pointer_cast<ConstantBlockInstruction>(instruction));
					break;
				}
				case Instruction::Type::Texture:
				{
					checkTextureInstruction(p_file, static_pointer_cast<TextureInstruction>(instruction));
					break;
				}
				case Instruction::Type::Symbol:
				{
					checkSymbolInstruction(p_file, static_pointer_cast<SymbolInstruction>(instruction));
					break;
				}
				case Instruction::Type::Namespace:
				{
					checkNamespaceInstruction(p_file, static_pointer_cast<NamespaceInstruction>(instruction));
					break;
				}
				default:
				{
					throw TokenBasedError(p_file, "Unexpected instruction type : " + ::to_string(instruction->type) + " inside namespace [" + p_instruction->name.content + "]" + DEBUG_INFORMATION, Token());
				}
				}
			}
			catch (const TokenBasedError& e)
			{
				_result.errors.push_back(e);
			}

			namespaceIndex++;
		}

		_currentNamespace.pop_back();
	}
}