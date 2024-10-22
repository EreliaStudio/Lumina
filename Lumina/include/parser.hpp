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

		struct Parameter
		{
			const Type* type;
			bool isReference;
			std::string name;
			std::vector<size_t> arraySize;

			bool operator ==(const Parameter& p_other) const
			{
				if ((type != p_other.type) ||
					(arraySize.size() != p_other.arraySize.size()) ||
					(isReference != p_other.isReference))
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

		struct SymbolBody
		{
		};

		struct Function
		{
			ExpressionType returnType;
			std::string name;
			std::vector<Parameter> parameters;

			SymbolBody body;
		};

		struct Type
		{
			using Attribute = Variable;
			using Method = Function;
			using Operator = Function;
			
			struct Constructor
			{
				std::vector<Parameter> parameters;

				SymbolBody body;

				bool operator ==(const Constructor& p_other) const
				{
					if (parameters.size() != p_other.parameters.size())
					{
						return (false);
					}

					for (size_t i = 0; i < parameters.size(); i++)
					{
						if (parameters[i] != p_other.parameters[i])
						{
							return (false);
						}
					}

					return (true);
				}
			};

			std::string name;
			std::set<Attribute> attributes;

			std::vector<Constructor> constructors;
			std::map<std::string, std::vector<Method>> methods;
			std::map<std::string, std::vector<Operator>> operators;

			operator std::string() const
			{
				return name;
			}
		};


		Product _product;

		std::set<std::string> _reservedIdentifiers;

		std::map<std::string, Type> _availibleTypes;
		std::vector<const Type*> _attributesTypes;
		std::vector<const Type*> _constantsTypes;
		std::map<std::string, std::vector<Function>> _availibleFunctions;

		std::set<Variable> _globalVariables;
		std::set<Variable> _vertexVariables;
		std::set<Variable> _fragmentVariables;

		std::deque<std::string> _nspaces;

		Function _vertexPassMain;
		Function _fragmentPassMain;

		void composeStandardTypes();
		void composeScalarTypes();
		void composeVector2Types();
		void composeVector3Types();
		void composeVector4Types();
		void composeTextureType();
		Parser();

		std::string _composeTypeName(const TypeInfo& p_typeInfo);
		std::vector<size_t> _composeSizeArray(const ArraySizeInfo& p_arraySizeInfo);
		std::string _composeIdentifierName(const std::string& p_identifierName);

		Type* _insertType(const Type& p_inputType);
		const Type* _findType(const std::string& p_objectName);
		const Type* _findType(const TypeInfo& p_typeInfo);

		void _insertVariable(const Variable& p_variable);

		ExpressionType _composeExpressionType(const ExpressionTypeInfo& p_expressionType);

		Variable _composeVariable(const VariableInfo& p_variableInfo);
		Type _composeType(const BlockInfo& p_block, const std::string& p_suffix = "");

		SymbolBody _composeSymbolBody(const SymbolBodyInfo& p_symbolInfo);
		Function _composeMethodFunction(const FunctionInfo& p_functionInfo);
		Type::Constructor _composeConstructorFunction(const ConstructorInfo& p_constructorInfo);
		Function _composeOperatorFunction(const OperatorInfo& p_operatorInfo);

		void _computeMethodAndOperator(Type* p_originator, const BlockInfo& p_block);

		void _parseStructure(const BlockInfo& p_blockInfo);
		void _parseAttribute(const BlockInfo& p_blockInfo);
		void _parseConstant(const BlockInfo& p_blockInfo);
		void _parseTexture(const TextureInfo& p_textureInfo);
		void _parseFunction(const FunctionInfo& p_functionInfo);
		void _parseNamespace(const NamespaceInfo& p_namespaceInfo);

		void _parsePipelineFlow(const PipelineFlowInfo& p_pipelineFlow);

		Function _composePipelinePass(const PipelinePassInfo& p_pipelinePass);
		void _parsePipelinePass(const PipelinePassInfo& p_pipelinePass);

		Product _parse(const Lexer::Output& p_input);

		void printArraySizes(const std::vector<size_t>& arraySize) const;
		void printParameters(const std::vector<Parameter>& parameters) const;
		void printVariable(const Variable& var, const std::string& indent = "") const;
		void printExpressionType(const ExpressionType& exprType) const;
		void printMethods(const std::map<std::string, std::vector<Type::Method>>& methods, const std::string& title) const;
		void printConstructors(const std::string& p_constructedType, const std::vector<Type::Constructor>& constructors) const;
		void printParsedData() const;

	public:
		static Product parse(const Lexer::Output& p_input);
	};
}