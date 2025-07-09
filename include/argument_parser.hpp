#pragma once

#include <filesystem>
#include <fstream>
#include <vector>
#include <iostream>

class ArgumentParser
{
public:
	struct Option
	{
		bool activated = false;
		std::string name;
		std::string reduceName;
		std::string description;
		size_t nbParameter = 0;
		bool isOptional = false;

		std::vector<std::string> parameters;
	};

	std::string _usage;
	std::vector<std::string> _parameters;
	std::vector<Option> _options;

private:
	void _parseArgument(char **argumentList, size_t argumentListSize, size_t& cursor)
	{
		std::string currentArgument = argumentList[cursor];

		if (contains(currentArgument) == true)
		{
			Option& currentOption = option(currentArgument);

			currentOption.activated = true;
			if (currentOption.nbParameter > 0)
			{
				if (cursor + currentOption.nbParameter >= argumentListSize)
				{
					throw std::runtime_error("Not enough parameters for option: " + currentArgument);
				}

				for (size_t i = 0; i < currentOption.nbParameter; ++i)
				{
					currentOption.parameters.push_back(argumentList[cursor + i + 1]);
				}
				cursor += currentOption.nbParameter;
			}			
		}
		else
		{
			if (currentArgument[0] == '-')
			{
				throw std::runtime_error("Unknown option: " + currentArgument);
			}

			_parameters.push_back(currentArgument);
		}
	}

public:
	void setUsage(const std::string& usage)
	{
		_usage = usage;
	}

	const std::vector<std::string>& parameters() const
	{
		return _parameters;
	}

	void addOption(const std::string& name, const std::string& reduceName, const std::string& description, bool isOptional, size_t nbParameter)
	{
		Option arg;
		arg.name = name;
		arg.reduceName = reduceName;
		arg.description = description;
		arg.isOptional = isOptional;
		arg.nbParameter = nbParameter;

		_options.push_back(arg);
	}

	bool contains(const std::string& name) const
	{
		for (const auto& arg : _options)
		{
			if (arg.name == name || arg.reduceName == name)
			{
				return true;
			}
		}
		return false;
	}

	Option& option(const std::string& name)
	{
		for (auto& arg : _options)
		{
			if (arg.name == name || arg.reduceName == name)
			{
				return arg;
			}
		}

		throw std::runtime_error("Option not found: " + name);
	}

	const Option& option(const std::string& name) const
	{
		for (const auto& arg : _options)
		{
			if (arg.name == name || arg.reduceName == name)
			{
				return arg;
			}
		}

		throw std::runtime_error("Option not found: " + name);
	}

	void parse(int argc, char** argv)
	{
		size_t cursor = 1;
		while (cursor < argc)
		{
			_parseArgument(argv, argc, cursor);
			cursor++;
		}
	}

	void print() const
	{
		std::cout << "Parameters:";
		if (_parameters.empty())
		{
			std::cout << " <none>";
		}
		else
		{
			for (const auto& p : _parameters)
				std::cout << ' ' << p;
		}
		std::cout << std::endl;

		for (const auto& opt : _options)
		{
			std::cout << opt.name;
			if (!opt.reduceName.empty())
				std::cout << " (" << opt.reduceName << ')';

			std::cout << " : " << (opt.activated ? "active" : "inactive");

			if (!opt.parameters.empty())
			{
				std::cout << " | parameters:";
				for (const auto& param : opt.parameters)
					std::cout << ' ' << param;
			}
			std::cout << std::endl;
		}
	}

	void printHelp() const
	{
		std::cout << "Usage : " << _usage << std::endl;
		std::cout << "Options :" << std::endl;
		for (const auto& option : _options)
		{
			std::cout << option.name << " (" << option.reduceName << ") : " << option.description;
			if (option.isOptional)
			{
				std::cout << " [optional]";
			}
			if (option.nbParameter > 0)
			{
				std::cout << " [requires " << option.nbParameter << " parameters]";
			}
			std::cout << std::endl;
		}
	}
};