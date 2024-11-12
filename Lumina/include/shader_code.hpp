#pragma once

#include <string>
#include <iostream>

namespace Lumina
{
	struct ShaderCode
	{
		static inline const std::string LAYOUTS_DELIMITER = "## LAYOUTS DEFINITION ##";
		static inline const std::string FRAMEBUFFERS_DELIMITER = "## FRAMEBUFFER DEFINITION ##";
		static inline const std::string CONSTANTS_DELIMITER = "## CONSTANTS DEFINITION ##";
		static inline const std::string ATTRIBUTES_DELIMITER = "## ATTRIBUTES DEFINITION ##";
		static inline const std::string TEXTURES_DELIMITER = "## TEXTURES DEFINITION ##";
		static inline const std::string VERTEX_DELIMITER = "## VERTEX SHADER CODE ##";
		static inline const std::string FRAGMENT_DELIMITER = "## FRAGMENT SHADER CODE ##";

		std::string layoutContent = "";
		std::string frameBufferContent = "";
		std::string constantContent = "";
		std::string attributeContent = "";
		std::string textureContent = "";
		std::string vertexCodeContent = "#version 450\n\n";
		std::string fragmentCodeContent = "#version 450\n\n";

		friend std::ostream& operator << (std::ostream& p_os, const ShaderCode& p_shader)
		{
			p_os << ShaderCode::LAYOUTS_DELIMITER << std::endl;
			p_os << p_shader.layoutContent << std::endl;
			p_os << std::endl;
			p_os << ShaderCode::FRAMEBUFFERS_DELIMITER << std::endl;
			p_os << p_shader.frameBufferContent << std::endl;
			p_os << std::endl;
			p_os << ShaderCode::CONSTANTS_DELIMITER << std::endl;
			p_os << p_shader.constantContent << std::endl;
			p_os << std::endl;
			p_os << ShaderCode::ATTRIBUTES_DELIMITER << std::endl;
			p_os << p_shader.attributeContent << std::endl;
			p_os << std::endl;
			p_os << ShaderCode::TEXTURES_DELIMITER << std::endl;
			p_os << p_shader.textureContent << std::endl;
			p_os << std::endl;
			p_os << ShaderCode::VERTEX_DELIMITER << std::endl;
			p_os << p_shader.vertexCodeContent << std::endl;
			p_os << std::endl;
			p_os << ShaderCode::FRAGMENT_DELIMITER << std::endl;
			p_os << p_shader.fragmentCodeContent << std::endl;
			p_os << std::endl;

			return (p_os);
		}
	};
}