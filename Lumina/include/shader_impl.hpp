#pragma once

#include <string>
#include <set>
#include <vector>

namespace Lumina
{
	struct VariableImpl;

	struct TypeImpl
	{
		std::string name = "UnknownTypeName";
		std::set<VariableImpl> attributes;

		bool operator < (const TypeImpl& p_other) const
		{
			return (name < p_other.name);
		}

		bool operator == (const TypeImpl& p_other) const
		{
			return (name == p_other.name);
		}

		bool operator != (const TypeImpl& p_other) const
		{
			return (!(this->operator==(p_other)));
		}
	};

	struct VariableImpl
	{
		TypeImpl type;
		std::string name;
		std::vector<size_t> arraySizes;

		bool operator < (const VariableImpl& p_other) const
		{
			return (name < p_other.name);
		}
	};

	struct PipelineFlowImpl
	{
		enum class Direction
		{
			In,
			Out
		};

		Direction direction;
		VariableImpl variable;
	};

	struct ExpressionTypeImpl
	{
		TypeImpl type;
		std::vector<size_t> arraySizes;

		bool operator == (const ExpressionTypeImpl& p_other) const
		{
			if (type != p_other.type)
				return (false);

			if (arraySizes.size() != p_other.arraySizes.size())
				return (false);

			for (size_t i = 0; i < arraySizes.size(); i++)
			{
				if (arraySizes[i] != p_other.arraySizes[i])
					return (false);
			}
			return (true);
		}

		bool operator != (const ExpressionTypeImpl& p_other) const
		{
			return (!(this->operator==(p_other)));
		}
	};

	struct ParameterImpl
	{
		TypeImpl type;
		bool isReference;
		std::string name;
		std::vector<size_t> arraySizes;

		bool operator<(const ParameterImpl& p_other) const
		{
			if (type < p_other.type) return true;
			if (p_other.type < type) return false;

			if (arraySizes.size() < p_other.arraySizes.size()) return true;
			if (arraySizes.size() > p_other.arraySizes.size()) return false;

			for (size_t i = 0; i < arraySizes.size(); ++i)
			{
				if (arraySizes[i] < p_other.arraySizes[i]) return true;
				if (arraySizes[i] > p_other.arraySizes[i]) return false;
			}

			return false;
		}
	};

	struct SymbolBodyImpl
	{
		std::string code;
	}; 

	struct FunctionImpl
	{
		bool isPrototype;
		ExpressionTypeImpl returnType;
		std::string name;
		std::vector<ParameterImpl> parameters;

		SymbolBodyImpl body;

		bool operator<(const FunctionImpl& p_other) const
		{
			if (name < p_other.name) return true;
			if (p_other.name < name) return false;

			if (parameters.size() < p_other.parameters.size()) return true;
			if (parameters.size() > p_other.parameters.size()) return false;

			for (size_t i = 0; i < parameters.size(); ++i)
			{
				if (parameters[i] < p_other.parameters[i]) return true;
				if (p_other.parameters[i] < parameters[i]) return false;
			}

			return false;
		}
	};

	struct PipelinePassImpl
	{
		bool isDefined = false;
		SymbolBodyImpl body;
	};

	struct ShaderImpl
	{
		std::vector<PipelineFlowImpl> vertexPipelineFlows;
		std::vector<PipelineFlowImpl> fragmentPipelineFlows;
		std::vector<PipelineFlowImpl> outputPipelineFlows;

		std::vector<TypeImpl> structures;
		std::vector<TypeImpl> attributes;
		std::vector<TypeImpl> constants;

		std::vector<FunctionImpl> functions;

		std::vector<VariableImpl> textures;

		PipelinePassImpl vertexPipelinePass;
		PipelinePassImpl fragmentPipelinePass;
	};

	std::ostream& operator<<(std::ostream& os, const ShaderImpl& shader);
}