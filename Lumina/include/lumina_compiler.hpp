#pragma once

#include <fstream>
#include <set>
#include <map>

#include "lumina_shader.hpp"
#include "lumina_tokenizer.hpp"
#include "lumina_exception.hpp"
#include "lumina_utils.hpp"
#include "lumina_metatokenizer.hpp"

namespace Lumina
{
	struct Compiler
	{
	public:
		using Product = Expected<Shader>;
	private:
		struct Type;

		struct Variable
		{
			const Type* type;
			std::string name;
			std::vector<size_t> arraySizes;

			bool isSame(const Variable& p_other) const
			{
				if (type != p_other.type)
					return (false);
				if (arraySizes.size() != p_other.arraySizes.size())
					return(false);

				for (size_t i = 0; i < arraySizes.size(); i++)
				{
					if (arraySizes[i] != p_other.arraySizes[i])
						return (false);
				}

				return (true);
			}

			bool operator<(const Variable& p_other) const
			{
				if (type != p_other.type)
					return type < p_other.type;
				if (name != p_other.name)
					return name < p_other.name;
				return arraySizes < p_other.arraySizes;
			}
		};
		struct Type
		{
			std::string name = "";
			size_t cpuSize;
			size_t gpuSize;
			size_t padding = 0;

			struct Element
			{
				Variable variable;
				size_t cpuOffset;
				size_t gpuOffset;
			};

			std::vector<Element> innerElements;

			bool contains(const std::string& p_name)
			{
				for (const auto& element : innerElements)
				{
					if (element.variable.name == p_name)
						return (true);
				}
				return (false);
			}

			bool operator<(const Type& other) const
			{
				return name < other.name;
			}
		};


		struct Function
		{
			struct Return
			{
				const Type* type;
				std::vector<size_t> arraySizes;

				bool operator == (const Return& p_other)
				{
					if (type != p_other.type)
						return (false);
					if (arraySizes.size() != p_other.arraySizes.size())
						return(false);

					for (size_t i = 0; i < arraySizes.size(); i++)
					{
						if (arraySizes[i] != p_other.arraySizes[i])
							return (false);
					}

					return (true);
				}

				bool operator != (const Return& p_other)
				{
					return (!(operator==(p_other)));
				}
			};

			Return returnType;
			std::string name;
			std::vector<Variable> parameters;
		};

		enum class BlockType
		{
			Constant,
			Attribute,
			Structure
		};

		Product _result;

		size_t nbVertexLayout = 0;
		size_t nbFragmentLayout = 0;
		size_t nbOutputLayout = 0;
		size_t nbTexture = 0;

		std::vector<std::string> _namespaceNames;

		std::string namespacePrefix();

		std::set<Type> _types;
		std::set<Type> _standardTypes;

		std::map<std::string, std::vector<Function>> _functions;

		std::set<Variable> _vertexVariables;
		std::set<Variable> _fragmentVariables;

		Variable composeVariable(const VariableDescriptor& p_descriptor);

		void compilePipelineFlow(std::shared_ptr<PipelineFlowMetaToken> p_metaToken);

		Type composeType(std::shared_ptr<BlockMetaToken> p_metaToken);
		Variable composeVariable(std::shared_ptr<BlockMetaToken> p_metaToken);

		std::string composeBlockCode(const BlockType& p_blockType, const Variable& p_variable);

		void insertElement(std::string& p_stringToFill, const Type::Element& p_elementToInsert, size_t p_nbSpace);
		std::string composeDataDescriptor(const Variable& p_variable);

		void compileStructure(std::shared_ptr<StructureMetaToken> p_metaToken);
		void compileAttribute(std::shared_ptr<AttributeMetaToken> p_metaToken);
		void compileConstant(std::shared_ptr<ConstantMetaToken> p_metaToken);

		void compileTexture(std::shared_ptr<TextureMetaToken> p_metaToken);

		std::string parseNumberElement(const std::shared_ptr<Expression::NumberElement>& element);
		std::string parseBooleanElement(const std::shared_ptr<Expression::BooleanElement>& element);
		std::string parseVariableDesignationElement(const std::shared_ptr<Expression::VariableDesignationElement>& element);
		std::string parseOperatorElement(const std::shared_ptr<Expression::OperatorElement>& element);
		std::string parseComparatorOperatorElement(const std::shared_ptr<Expression::ComparatorOperatorElement>& element);
		std::string parseConditionOperatorElement(const std::shared_ptr<Expression::ConditionOperatorElement>& element);
		std::string parseIncrementorElement(const std::shared_ptr<Expression::IncrementorElement>& element);
		std::string parseSymbolCallElement(const std::shared_ptr<Expression::SymbolCallElement>& element);
		std::string parseExpression(const std::shared_ptr<Expression> p_expression);

		std::string parseVariableDeclaration(const std::shared_ptr<Instruction>& instruction);
		std::string parseVariableAssignation(const std::shared_ptr<Instruction>& instruction);
		std::string parseSymbolCall(const std::shared_ptr<Instruction>& instruction);
		std::string parseIfStatement(const std::shared_ptr<Instruction>& instruction);
		std::string parseWhileStatement(const std::shared_ptr<Instruction>& instruction);
		std::string parseForStatement(const std::shared_ptr<Instruction>& instruction);
		std::string parseReturnStatement(const std::shared_ptr<Instruction>& instruction);
		std::string parseDiscardStatement(const std::shared_ptr<Instruction>& instruction);

		std::string compileSymbolBody(SymbolBody p_metaToken);

		void compileFunction(std::shared_ptr<FunctionMetaToken> p_metaToken);

		void compilePipelineBody(std::shared_ptr<PipelineBodyMetaToken> p_metaToken);

		void compileNamespace(std::shared_ptr<NamespaceMetaToken> p_metaToken);
		Product _compile(const std::vector<std::shared_ptr<MetaToken>>& p_metaTokens);

		Compiler();

		void createScalarTypes();
		void createFloatVectorTypes();
		void createIntVectorTypes();
		void createUIntVectorTypes();
		void createMatrixTypes();
		void createLuminaTypes();

		void addType(const Type& p_type);
		void addStandardType(const Type& p_type);
		const Type* _type(const std::string& p_typeName) const;
		const Type* type(const Lumina::Token& p_typeToken) const;

	public:
		static Expected<Shader> compile(const std::vector<std::shared_ptr<MetaToken>>& p_metaTokens)
		{
			return (Compiler()._compile(p_metaTokens));
		}
	};
}
