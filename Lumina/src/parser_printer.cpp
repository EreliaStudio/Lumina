#include "parser.hpp"

namespace Lumina
{
	void Parser::printParsedData() const
	{
		std::cout << "Available Types:\n";
		for (const auto& typeIterator : _availibleTypes)
		{
			const auto& type = typeIterator.second;

			std::cout << "	Type: " << type.name << "\n";

			if (!type.attributes.empty())
			{
				std::cout << "		Attributes:\n";
				for (const auto& attr : type.attributes)
				{
					if (attr.type == nullptr)
					{
						std::cout << "			[No type] " << attr.name << "\n";
					}
					else
					{
						std::cout << "			" << attr.type->name << " " << attr.name;
						if (!attr.arraySize.empty())
						{
							std::cout << "[";
							for (size_t i = 0; i < attr.arraySize.size(); ++i)
							{
								if (i > 0) std::cout << ", ";
								std::cout << attr.arraySize[i];
							}
							std::cout << "]";
						}
						std::cout << "\n";
					}

				}
			}

			if (!type.methods.empty())
			{
				std::cout << "		Methods:\n";
				for (const auto& methodPair : type.methods)
				{
					for (const auto& method : methodPair.second)
					{
						std::cout << "			" << method.returnType.type->name;
						if (!method.returnType.arraySize.empty())
						{
							std::cout << "[";
							for (size_t i = 0; i < method.returnType.arraySize.size(); ++i)
							{
								if (i > 0) std::cout << ", ";
								std::cout << method.returnType.arraySize[i];
							}
							std::cout << "]";
						}
						std::cout << " " << method.name << "(";
						for (size_t i = 0; i < method.parameters.size(); i++)
						{
							if (i != 0)
							{
								std::cout << ", ";
							}
							const auto& param = method.parameters[i];
							std::cout << param.type->name;
							if (!param.arraySize.empty())
							{
								std::cout << "[";
								for (size_t i = 0; i < param.arraySize.size(); ++i)
								{
									if (i > 0) std::cout << ", ";
									std::cout << param.arraySize[i];
								}
								std::cout << "]";
							}
							std::cout << " " << (param.isReference == true ? "in" : "out") << " " << param.name;
						}
						std::cout << ")\n";
					}
				}
			}

			if (!type.operators.empty())
			{
				std::cout << "		Operators:\n";
				for (const auto& operatorPair : type.operators)
				{
					for (const auto& op : operatorPair.second)
					{
						std::cout << "			" << op.returnType.type->name;
						if (!op.returnType.arraySize.empty())
						{
							std::cout << "[";
							for (size_t i = 0; i < op.returnType.arraySize.size(); ++i)
							{
								if (i > 0) std::cout << ", ";
								std::cout << op.returnType.arraySize[i];
							}
							std::cout << "]";
						}
						std::cout << " operator " << op.name << "(";
						for (size_t i = 0; i < op.parameters.size(); i++)
						{
							if (i != 0)
							{
								std::cout << ", ";
							}
							const auto& param = op.parameters[i];
							std::cout << param.type->name;
							if (!param.arraySize.empty())
							{
								std::cout << "[";
								for (size_t i = 0; i < param.arraySize.size(); ++i)
								{
									if (i > 0) std::cout << ", ";
									std::cout << param.arraySize[i];
								}
								std::cout << "]";
							}
							std::cout << " " << (param.isReference == true ? "in" : "out") << " " << param.name;
						}
						std::cout << ")\n";
					}
				}
			}
		}

		std::cout << "\n	Variables:\n";
		for (const auto& var : _variables)
		{
			if (var.type == nullptr)
			{
				std::cout << "			[No type] " << var.name << "\n";
			}
			else
			{
				std::cout << "			" << var.type->name << " " << var.name;
				if (!var.arraySize.empty())
				{
					std::cout << "[";
					for (size_t i = 0; i < var.arraySize.size(); ++i)
					{
						if (i > 0) std::cout << ", ";
						std::cout << var.arraySize[i];
					}
					std::cout << "]";
				}
				std::cout << "\n";
			}
		}

		std::cout << "\n	Functions:\n";
		for (const auto& funcPair : _availibleFunctions)
		{
			for (const auto& func : funcPair.second)
			{
				std::cout << "		" << func.returnType.type->name;
				if (!func.returnType.arraySize.empty())
				{
					std::cout << "[";
					for (size_t i = 0; i < func.returnType.arraySize.size(); ++i)
					{
						if (i > 0) std::cout << ", ";
						std::cout << func.returnType.arraySize[i];
					}
					std::cout << "]";
				}
				std::cout << " " << func.name << "(";
				for (size_t i = 0; i < func.parameters.size(); i++)
				{
					if (i != 0)
					{
						std::cout << ", ";
					}
					const auto& param = func.parameters[i];
					std::cout << param.type->name;
					if (!param.arraySize.empty())
					{
						std::cout << "[";
						for (size_t i = 0; i < param.arraySize.size(); ++i)
						{
							if (i > 0) std::cout << ", ";
							std::cout << param.arraySize[i];
						}
						std::cout << "]";
					}
					std::cout << " " << (param.isReference == true ? "in" : "out") << " " << param.name;
				}
				std::cout << ")\n";
			}
		}

		std::cout << "\nAttribute Types:\n";
		for (const auto& attrType : _attributesTypes)
		{
			if (attrType == nullptr)
			{
				std::cout << "	Inserted a nullptr type in attribute" << "\n";
			}
			else
			{
				std::cout << "	" << attrType->name << "\n";
			}
		}

		std::cout << "\nConstants Types:\n";
		for (const auto& constType : _constantsTypes)
		{
			if (constType == nullptr)
			{
				std::cout << "Inserted a nullptr type in Constant" << "\n";
			}
			else
			{
				std::cout << "  " << constType->name << "\n";
			}
		}
	}
}