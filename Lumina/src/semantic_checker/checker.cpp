#include "lumina_semantic_checker.hpp"

namespace Lumina
{
	SemanticChecker::Result SemanticChecker::checkSemantic(const std::filesystem::path& p_file, std::vector<std::shared_ptr<AbstractInstruction>>& p_instructions)
	{
		return (SemanticChecker().check(p_file, p_instructions));
	}

	void SemanticChecker::setup()
	{
		setupTypes();
		setupStructures();
		setupSymbols();

		_vertexPassVariables["pixelPosition"] = type("Vector4");
		_fragmentPassVariables["pixelColor"] = type("Vector4");
	}

	SemanticChecker::Result SemanticChecker::check(const std::filesystem::path& p_file, std::vector<std::shared_ptr<AbstractInstruction>>& p_instructions)
	{
		_result = Result();
		_index = 0;

		setup();

		for (auto& instruction : p_instructions)
		{
			_elements.push_back(Element{ p_file, instruction });
		}

		while (_index < _elements.size())
		{
			auto& element = _elements[_index];
			try
			{
				std::shared_ptr<AbstractInstruction> instruction = element.instruction;

				switch (instruction->type)
				{
				case Instruction::Type::Include:
				{
					checkIncludeInstruction(element.filePath, static_pointer_cast<IncludeInstruction>(instruction));
					break;
				}
				case Instruction::Type::PipelineFlow:
				{
					checkPipelineFlowInstruction(element.filePath, static_pointer_cast<PipelineFlowInstruction>(instruction));
					break;
				}
				case Instruction::Type::StructureBlock:
				{
					checkStructureInstruction(element.filePath, static_pointer_cast<StructureBlockInstruction>(instruction));
					break;
				}
				case Instruction::Type::AttributeBlock:
				{
					checkAttributeInstruction(element.filePath, static_pointer_cast<AttributeBlockInstruction>(instruction));
					break;
				}
				case Instruction::Type::ConstantBlock:
				{
					checkConstantInstruction(element.filePath, static_pointer_cast<ConstantBlockInstruction>(instruction));
					break;
				}
				case Instruction::Type::Texture:
				{
					checkTextureInstruction(element.filePath, static_pointer_cast<TextureInstruction>(instruction));
					break;
				}
				case Instruction::Type::Symbol:
				{
					checkSymbolInstruction(element.filePath, static_pointer_cast<SymbolInstruction>(instruction));
					break;
				}
				case Instruction::Type::Namespace:
				{
					checkNamespaceInstruction(element.filePath, static_pointer_cast<NamespaceInstruction>(instruction));
					break;
				}
				case Instruction::Type::PipelineBody:
				{
					checkPipelineBodyInstruction(element.filePath, static_pointer_cast<PipelineBodyInstruction>(instruction));
					break;
				}
				default:
				{
					throw TokenBasedError(element.filePath, "Unexpected instruction type : " + ::to_string(instruction->type) + DEBUG_INFORMATION, Token());
				}
				}
			}
			catch (const TokenBasedError& e)
			{
				_result.errors.push_back(e);
			}

			_index++;
		}

		return _result;
	}
}