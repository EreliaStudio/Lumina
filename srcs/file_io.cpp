#include "file_io.hpp"

#include <fstream>
#include <stdexcept>

std::string readFile(const std::filesystem::path &p_path)
{
	std::ifstream in(p_path, std::ios::binary);
	if (!in)
	{
		throw std::runtime_error("Failed to open file: " + p_path.string());
	}

	in.seekg(0, std::ios::end);
	const std::streamoff end = in.tellg();
	if (end < 0)
	{
		throw std::runtime_error("Failed to get size: " + p_path.string());
	}

	std::string data(static_cast<std::size_t>(end), '\0');
	in.seekg(0, std::ios::beg);
	in.read(data.data(), static_cast<std::streamsize>(data.size()));
	if (!in)
	{
		throw std::runtime_error("Failed to read file: " + p_path.string());
	}

	return data;
}
