#include "parser.hpp"

namespace Lumina
{
	TypeImpl Parser::_findTypeImpl(const ShaderRepresentation::Type* p_type)
	{
		if (p_type == nullptr)
			return (TypeImpl());

		for (const auto& type : _product.value.structures)
		{
			if (type.name == p_type->name)
			{
				return (type);
			}
		}

		for (const auto& type : _product.value.attributes)
		{
			if (type.name == p_type->name)
			{
				return (type);
			}
		}

		for (const auto& type : _product.value.constants)
		{
			if (type.name == p_type->name)
			{
				return (type);
			}
		}

		return (TypeImpl());
	}
	
	VariableImpl Parser::_convertVariable(const ShaderRepresentation::Variable& p_variable)
	{
		VariableImpl result;

		result.type = _findTypeImpl(p_variable.type);
		result.arraySize = p_variable.arraySize;
		result.name = p_variable.name;

		return (result);
	}

	ExpressionTypeImpl Parser::_convertExpressionType(const ShaderRepresentation::ExpressionType& p_expressionType)
	{
		ExpressionTypeImpl result;

		result.type = _findTypeImpl(p_expressionType.type);
		result.arraySize = p_expressionType.arraySize;

		return (result);
	}

	ParameterImpl Parser::_convertParameter(const ShaderRepresentation::Parameter& p_parameter)
	{
		ParameterImpl result;

		result.type = _findTypeImpl(p_parameter.type);
		result.isReference = p_parameter.isReference;
		result.arraySize = p_parameter.arraySize;
		result.name = p_parameter.name;

		return (result);
	}

	FunctionBodyImpl Parser::_convertFunctionBody(const ShaderRepresentation::SymbolBody& p_symbolBody)
	{
		FunctionBodyImpl result;

		result.code = p_symbolBody.toString();

		return (result);
	}

	FunctionImpl Parser::_convertConstructor(const TypeImpl& p_originator, const ShaderRepresentation::Type::Constructor& p_constructor)
	{
		FunctionImpl result;

		result.name = p_originator.name + "_Constructor";
		result.returnType = { p_originator, {} };
		for (const auto& param : p_constructor.parameters)
		{
			result.parameters.push_back(_convertParameter(param));
		}
		result.body = _convertFunctionBody(p_constructor.body);

		return (result);
	}

	FunctionImpl Parser::_convertFunction(const TypeImpl& p_originator, const ShaderRepresentation::Function& p_function)
	{
		FunctionImpl result;

		result.name = p_originator.name + "_" + p_function.name;
		result.returnType = _convertExpressionType(p_function.returnType);
		result.parameters.push_back({
				.type = p_originator,
				.isReference = true,
				.name = "self",
				.arraySize = {}
			});
		result.parameters.push_back({
				.type = p_originator,
				.isReference = true,
				.name = "this",
				.arraySize = {}
			});
		for (const auto& param : p_function.parameters)
		{
			result.parameters.push_back(_convertParameter(param));
		}
		result.body = _convertFunctionBody(p_function.body);

		return (result);
	}
	
	TypeImpl Parser::_convertType(const ShaderRepresentation::Type* p_type)
	{
		TypeImpl result;

		result.name = p_type->name;

		for (const auto& element : p_type->attributes)
		{
			result.attributes.push_back(_convertVariable(element));
		}

		return (result);
	}

	void Parser::_composeTypeArray(const std::vector<const ShaderRepresentation::Type*>& typeArray, std::vector<TypeImpl>& p_destination)
	{
		for (const auto* type : typeArray)
		{
			TypeImpl tmpType = _convertType(type);

			p_destination.push_back(tmpType);

			for (const auto& funct : type->constructors)
			{
				if (funct.isPrototype == false)
				{
					_product.value.functions.push_back(_convertConstructor(tmpType, funct));
				}
			}

			for (const auto& [key, functArray] : type->methods)
			{
				for (const auto& funct : functArray)
				{
					if (funct.isPrototype == false)
					{
						_product.value.functions.push_back(_convertFunction(tmpType, funct));
					}
				}
			}

			for (const auto& [key, opeArray] : type->operators)
			{
				for (const auto& ope : opeArray)
				{
					if (ope.isPrototype == false)
					{
						_product.value.functions.push_back(_convertFunction(tmpType, ope));
					}
				}
			}
		}
	}

	void Parser::_composeShaderTypes()
	{
		_composeTypeArray(_shaderRepresentation.structureTypes, _product.value.structures);
		_composeTypeArray(_shaderRepresentation.attributesTypes, _product.value.attributes);
		_composeTypeArray(_shaderRepresentation.constantsTypes, _product.value.constants);
	}
	
	void Parser::_composeShaderPipelineFlows()
	{
		for (const auto& variable : _shaderRepresentation.vertexVariables)
		{
			_product.value.vertexPipelineFlows.push_back({
					.direction = PipelineFlowImpl::Direction::In,
					.variable = _convertVariable(variable)
				});
		}

		for (const auto& variable : _shaderRepresentation.fragmentVariables)
		{
			_product.value.vertexPipelineFlows.push_back({
					.direction = PipelineFlowImpl::Direction::Out,
					.variable = _convertVariable(variable)
				});
			_product.value.fragmentPipelineFlows.push_back({
					.direction = PipelineFlowImpl::Direction::In,
					.variable = _convertVariable(variable)
				});
		}

		for (const auto& variable : _shaderRepresentation.outputVariables)
		{
			_product.value.fragmentPipelineFlows.push_back({
					.direction = PipelineFlowImpl::Direction::Out,
					.variable = _convertVariable(variable)
				});
		}
	}
	
	void Parser::_composeShaderImpl()
	{
		_product.value = ShaderImpl();

		_composeShaderTypes();

		_composeShaderPipelineFlows();
	}
}
