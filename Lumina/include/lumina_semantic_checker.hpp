#pragma once

#include <iostream>
#include <fstream>
#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <string>
#include <vector>
#include <filesystem>

#include "lumina_token.hpp"
#include "lumina_tokenizer.hpp"
#include "lumina_lexer.hpp"
#include "lumina_exception.hpp"

namespace Lumina
{
	class SemanticChecker
	{
	public:
		struct Result
		{
			std::vector<TokenBasedError> errors;
		};

		struct Type
		{
			struct Attribute
			{
				Type* type;
				std::string name;
				size_t size;
			};

			std::string name = "";
			std::vector<Attribute> attributes = {};
			std::unordered_set<Type*> acceptedConversions = {}; //Point to type convertible from this type
			std::unordered_set<std::string> operators; //Operator +, -, *, /
			std::unordered_set<std::string> comparaisonOperators; //Operator <, >, <=, ==, etc
			std::vector<std::vector<Type*>> constructors = {};

			friend std::ostream& operator<<(std::ostream& os, const Type& type);
		};

		struct Symbol
		{
			struct Parameter
			{
				std::string name;
				Type* type;
			};
			Type* returnType;
			std::string name;
			std::vector<Parameter> parameters;
		};

	private:
		struct Element
		{
			std::filesystem::path filePath;
			std::shared_ptr<AbstractInstruction> instruction;
		};

		Result _result;
		std::vector<Element> _elements;
		size_t _index;

		std::vector<Lumina::Token> _currentNamespace;

		std::unordered_set<std::filesystem::path> _loadedIncludes;

		std::list<Type> _types;
		std::unordered_set<Type*> _standardTypes;
		std::unordered_set<Type*> _structures;
		std::unordered_set<Type*> _attributes;
		std::unordered_set<Type*> _constants;

		std::unordered_set<std::string> _textures;

		std::unordered_map<std::string, std::vector<Symbol>> _symbols;

		bool _vertexParsed = false;
		std::unordered_map<std::string, Type*> _vertexPassVariables;
		bool _fragmentParsed = false;
		std::unordered_map<std::string, Type*> _fragmentPassVariables;

	public:
		static Result checkSemantic(const std::filesystem::path& p_file, std::vector<std::shared_ptr<AbstractInstruction>>& p_instructions);

	private:
		void setupTypes();
		void setupStructures();
		void setupSymbols();
		void setup();

		void addType(const Type& p_type);
		void addStandardType(const Type& p_structure);
		void addStructure(const Type& p_structure);
		void addAttribute(const Type& p_attribute);
		void addConstant(const Type& p_constant);

		Type* type(const std::string& p_typeName);
		Type* standardType(const std::string& p_standardTypeName);
		Type* structure(const std::string& p_structureName);
		Type* attribute(const std::string& p_attributeName);
		Type* constant(const std::string& p_constantName);

		std::vector<Symbol>* symbolArray(const std::string& p_symbolName);
		void addSymbol(const Symbol& p_symbol);
		
		std::string createNamespacePrefix() const;
		void throwException(const std::filesystem::path& p_filePath, const std::string& p_errorMessage, const Token& p_errorToken);
		
		void checkIncludeInstruction(const std::filesystem::path& p_file, const std::shared_ptr<IncludeInstruction>& p_instruction);
		void checkPipelineFlowInstruction(const std::filesystem::path& p_file, const std::shared_ptr<PipelineFlowInstruction>& p_instruction);
		void checkStructureInstruction(const std::filesystem::path& p_file, const std::shared_ptr<StructureBlockInstruction>& p_instruction);
		void checkAttributeInstruction(const std::filesystem::path& p_file, const std::shared_ptr<AttributeBlockInstruction>& p_instruction);
		void checkConstantInstruction(const std::filesystem::path& p_file, const std::shared_ptr<ConstantBlockInstruction>& p_instruction);
		void checkTextureInstruction(const std::filesystem::path& p_file, const std::shared_ptr<TextureInstruction>& p_instruction);

		void checkSymbolInstruction(const std::filesystem::path& p_file, const std::shared_ptr<SymbolInstruction>& p_instruction);

		void checkPipelineBodyInstruction(const std::filesystem::path& p_file, const std::shared_ptr<PipelineBodyInstruction>& p_instruction);

		Type* getNumberExpressionElementType(const std::filesystem::path& p_file, const std::shared_ptr<NumberExpressionValueInstruction>& p_instruction);
		Type* getVariableExpressionElementType(const std::filesystem::path& p_file, const std::shared_ptr<VariableExpressionValueInstruction>& p_instruction, const std::unordered_map<std::string, Type*> p_variables);
		Type* getSymbolCallExpressionElementType(const std::filesystem::path& p_file, const std::shared_ptr<ExpressionElementInstruction>& p_instruction);
		Type* getExpressionElementType(const std::filesystem::path& p_file, const std::shared_ptr<ExpressionElementInstruction>& p_instruction, const std::unordered_map<std::string, Type*> p_variables);
		Type* getExpressionType(const std::filesystem::path& p_file, const std::shared_ptr<ExpressionInstruction>& p_instruction, const std::unordered_map<std::string, Type*> p_variables);

		void checkNumberExpressionValueInstruction(const std::filesystem::path& p_file, const std::shared_ptr<NumberExpressionValueInstruction>& p_instruction, Type* p_expectedType);
		void checkVariableExpressionValueInstruction(const std::filesystem::path& p_file, const std::shared_ptr<VariableExpressionValueInstruction>& p_instruction, const std::unordered_map<std::string, Type*> p_variables, Type* p_expectedType);
		void checkSymbolCallInstruction(const std::filesystem::path& p_file, const std::shared_ptr<SymbolCallInstruction>& p_instruction, const std::unordered_map<std::string, Type*> p_variables, Type* p_expectedType);
		void checkExpressionInstruction(const std::filesystem::path& p_file, const std::shared_ptr<ExpressionInstruction>& p_instruction, const std::unordered_map<std::string, Type*> p_variables, Type* p_expectedType);
		
		void checkConditionElementInstruction(const std::filesystem::path& p_file, const std::shared_ptr<ConditionElementInstruction>& p_instruction, std::unordered_map<std::string, Type*> p_variables);
		void checkConditionInstruction(const std::filesystem::path& p_file, const std::shared_ptr<ConditionInstruction>& p_instruction, std::unordered_map<std::string, Type*> p_variables);
		
		void checkVariableDeclarationInstruction(const std::filesystem::path& p_file, const std::shared_ptr<VariableDeclarationInstruction>& p_instruction, std::unordered_map<std::string, Type*>& p_variables);
		void checkVariableAssignationInstruction(const std::filesystem::path& p_file, const std::shared_ptr<VariableAssignationInstruction>& p_instruction, std::unordered_map<std::string, Type*> p_variables);
		void checkIfStatementInstruction(const std::filesystem::path& p_file, const std::shared_ptr<IfStatementInstruction>& p_instruction, std::unordered_map<std::string, Type*> p_variables, Type* returnType);
		void checkWhileLoopInstruction(const std::filesystem::path& p_file, const std::shared_ptr<WhileLoopInstruction>& p_instruction, std::unordered_map<std::string, Type*> p_variables, Type* returnType);
		void checkReturnInstruction(const std::filesystem::path& p_file, const std::shared_ptr<ReturnInstruction>& p_instruction, std::unordered_map<std::string, Type*> p_variables, Type* returnType);
		void checkDiscardInstruction(const std::filesystem::path& p_file, const std::shared_ptr<DiscardInstruction>& p_instruction, std::unordered_map<std::string, Type*> p_variables);
		
		void checkSymbolBodyInstruction(const std::filesystem::path& p_file, const std::shared_ptr<SymbolBodyInstruction>& p_instruction, std::unordered_map<std::string, Type*> p_variables, Type* returnType);
		
		void checkNamespaceInstruction(const std::filesystem::path& p_file, const std::shared_ptr<NamespaceInstruction>& p_instruction);

		Result check(const std::filesystem::path& p_file, std::vector<std::shared_ptr<AbstractInstruction>>& p_instructions);
	};
}