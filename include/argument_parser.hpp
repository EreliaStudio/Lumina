#pragma once

#include <iostream>
#include <vector>
#include <filesystem>

class ArgumentParser
{
private:
	bool _verboseMode = false;
	std::filesystem::path _outputFile = "a.out";
	std::filesystem::path _inputFile = "";
	std::vector<std::filesystem::path> _additionalIncludePaths;

public:
	ArgumentParser()
	{

	}

	bool isVerboseMode() const
	{
		return _verboseMode;
	}

	const std::filesystem::path& inputFile() const
	{
		return _inputFile;
	}

	const std::filesystem::path& outputFile() const
	{
		return _outputFile;
	}

	const std::vector<std::filesystem::path>& additionalIncludePaths() const
	{
		return _additionalIncludePaths;
	}

	static void printHelp(const std::string& programName)
	{
		std::cout << "Usage : " << programName << " [path to your lumina shader code]" << std::endl;
		std::cout << "Options :" << std::endl;
		std::cout << "  -o, --output\t\tSpecify the output file for the compiled shader" << std::endl;
		std::cout << "  -v, --verbose\t\tEnable verbose output" << std::endl;
		std::cout << "  -h, --help\t\tShow this help message" << std::endl;
		std::cout << "  -i, --includePath\tSpecify additionnal include paths for shader files" << std::endl;
	}

	void parseArgument(char **argumentList, int argumentListSize, int& counter)
	{
		std::string argument = argumentList[counter];
		if (argumentList[counter][0] == '-')
		{
			if (argument == "-v" || argument == "--verbose")
			{
				_verboseMode = true;
			}
			else if (argument == "-h" || argument == "--help")
			{
				printHelp(argumentList[0]);
				exit(0);
			}
			else if (argument == "-o" || argument == "--output")
			{
				counter++;

				if (counter >= argumentListSize)
				{
					throw std::runtime_error("No output file specified after -o or --output option.");
				}
				else if (argumentList[counter][0] == '-')
				{
					throw std::runtime_error("Output file cannot start with a dash (-). Please specify a valid output file name.");
				}
				else
				{
					_outputFile = argumentList[counter];
				}
			}
			else if (argument == "-i" || argument == "--includePath")
			{
				counter++;

				if (counter >= argumentListSize)
				{
					throw std::runtime_error("No include path specified after -i or --includePath option.");
				}
				else if (argumentList[counter][0] == '-')
				{
					throw std::runtime_error("Include folder path cannot start with a dash (-). Please specify a valid folder path.");
				}
				else
				{
					_additionalIncludePaths.push_back(argumentList[counter]);
				}
			}
			else
			{
				throw std::runtime_error("Unknown option: " + std::string(argumentList[counter]));
			}
		}
		else
		{
			if (_inputFile.empty())
			{
				_inputFile = argumentList[counter];
			}
			else
			{
				throw std::runtime_error("Multiple input files specified. Only one input file is allowed.");
			}
		}
	}

	void parseArguments(int argc, char ** argv)
	{
		for (int i = 1; i < argc; i++)
		{
			parseArgument(argv, argc, i);
		}
	}

	void print()
	{
		std::cout << " - Verbose mode: " << (isVerboseMode() ? "Enabled" : "Disabled") << std::endl;

		std::cout << " - Input file: " << _inputFile << std::endl;
		std::cout << " - Output file: " << _outputFile << std::endl;

		if (_additionalIncludePaths.empty() == false)
		{
			std::cout << " - Additional include paths:" << std::endl;
			for (const auto& path : _additionalIncludePaths)
			{
				std::cout << "   - " << path << std::endl;
			}
		}
	}
};

ArgumentParser readArguments(int argc, char **argv)
{
	ArgumentParser argumentParser;

	for (int i = 1; i < argc; i++)
	{
		argumentParser.parseArgument(argv, argc, i);
	}

	if (argumentParser.isVerboseMode())
	{
		std::cout << "Compilation call : " << std::endl;
		std::cout << "Arguments :" << std::endl;
		argumentParser.print();
	}

	return argumentParser;
}