#pragma once

#include "parser.hpp"

namespace Lumina
{
	struct ShaderImpl
	{
		static inline const std::string LAYOUTS_DELIMITER = "## LAYOUTS DEFINITION ##";
		static inline const std::string CONSTANTS_DELIMITER = "## CONSTANTS DEFINITION ##";
		static inline const std::string ATTRIBUTES_DELIMITER = "## ATTRIBUTES DEFINITION ##";
		static inline const std::string TEXTURES_DELIMITER = "## TEXTURES DEFINITION ##";
		static inline const std::string VERTEX_DELIMITER = "## VERTEX SHADER CODE ##";
		static inline const std::string FRAGMENT_DELIMITER = "## FRAGMENT SHADER CODE ##";

		std::string layoutContent = "Default layout content";
		std::string constantContent = "Default constant content";
		std::string attributeContent = "Default attribute content";
		std::string textureContent = "Default texture content";
		std::string vertexCodeContent = "Default vertex pipeline";
		std::string fragmentCodeContent = "Default fragment pipeline";

		friend std::ostream& operator << (std::ostream& p_os, const ShaderImpl& p_shader);
	};

	struct Compiler
	{
	private:
		ShaderImpl _product;

		Compiler() = default;

		ShaderImpl _compile(const Parser::Output& p_input);

	public:
		static ShaderImpl compile(const Parser::Output& p_input);
	};
}