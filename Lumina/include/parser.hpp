#pragma once

#include "lexer.hpp"

#include "shader_impl.hpp"

#include <deque>
#include <set>

namespace Lumina
{
	struct Parser
	{
	public:
		using Output = ShaderImpl;
		using Product = Lumina::Expected<Output>;

	private:
		struct Type;

		struct Variable
		{
			const Type* type;
			std::string name;
			std::vector<size_t> arraySize;

			bool operator <(const Variable& p_other) const
			{
				return (name < p_other.name);
			}
		};

		struct ExpressionType
		{
			const Type* type;
			std::vector<size_t> arraySize;

			bool operator ==(const Variable& p_other) const
			{
				if ((type != p_other.type) ||
					(arraySize.size() != p_other.arraySize.size()))
				{
					return (false);
				}

				for (size_t i = 0; i < arraySize.size(); i++)
				{
					if (arraySize[i] != p_other.arraySize[i])
					{
						return (false);
					}
				}

				return (true);
			}
		};

		struct Function
		{
			ExpressionType returnType;
			std::string name;
			std::set<Variable> parameters;
		};

		struct Type
		{
			using Attribute = Variable;
			using Method = Function;
			using Operator = Function;

			std::string name;
			std::set<Attribute> attributes;

			std::map<std::string, std::vector<Method>> methods;
			std::map<std::string, std::vector<Operator>> operators;

			bool operator <(const Type& p_other) const
			{
				return (name < p_other.name);
			}
		};


		Product _product;

		std::set<std::string> _reservedIdentifiers;

		std::set<Type> _availibleTypes;
		std::set<const Type*> _attributesTypes;
		std::set<const Type*> _constantsTypes;
		std::map<std::string, std::vector<Function>> _availibleFunctions;

		std::set<Variable> _variables;

		std::deque<std::string> _nspaces;

		void composeStandardTypes();
		void composeScalarTypes();
		void composeVector2Types();
		void composeVector3Types();
		void composeVector4Types();
		Parser();

		std::string _composeTypeName(const TypeInfo& p_typeInfo);
		std::vector<size_t> _composeSizeArray(const ArraySizeInfo& p_arraySizeInfo);
		std::string _composeIdentifierName(const std::string& p_identifierName);

		const Type* _insertType(const Type& p_inputType);
		const Type* _findType(const std::string& p_objectName);

		void _insertVariable(const Variable& p_variable);

		Variable _composeVariable(const VariableInfo& p_variableInfo);
		Type _composeType(const BlockInfo& p_block, const std::string& p_suffix = "");
		Function _composeMethodFunction(const Type* p_originator, const FunctionInfo& p_functionInfo);
		Function _composeOperatorFunction(const Type* p_originator, const OperatorInfo& p_operatorInfo);

		void _computeMethodAndOperator(const Type* p_originator, const BlockInfo& p_block);

		void _parseStructure(const BlockInfo& p_block);
		void _parseAttribute(const BlockInfo& p_block);
		void _parseConstant(const BlockInfo& p_block);
		void _parseTexture(const TextureInfo& p_texture);
		void _parseFunction(const FunctionInfo& p_function);
		void _parseNamespace(const NamespaceInfo& p_namespace);

		Product _parse(const Lexer::Output& p_input);
		void printParsedData() const;

	public:
		static Product parse(const Lexer::Output& p_input);
	};
}