#pragma once

#include <string>

namespace Lumina
{
	struct Shader
	{
		std::string inputLayouts;
		std::string outputLayouts;
		std::string constants;
		std::string attributes;
		std::string textures;
		std::string vertexShaderCode;
		std::string fragmentShaderCode;

		friend std::ostream& operator << (std::ostream& p_os, const Shader& p_shader)
		{
			const std::string INPUT_LAYOUTS_DELIMITER = "## INPUT LAYOUTS DEFINITION ##";
			const std::string OUTPUT_LAYOUTS_DELIMITER = "## OUTPUT LAYOUTS DEFINITION ##";
			const std::string CONSTANTS_DELIMITER = "## CONSTANTS DEFINITION ##";
			const std::string ATTRIBUTES_DELIMITER = "## ATTRIBUTES DEFINITION ##";
			const std::string TEXTURES_DELIMITER = "## TEXTURES DEFINITION ##";
			const std::string VERTEX_DELIMITER = "## VERTEX SHADER CODE ##";
			const std::string FRAGMENT_DELIMITER = "## FRAGMENT SHADER CODE ##";

			p_os << INPUT_LAYOUTS_DELIMITER << std::endl;
			p_os << p_shader.inputLayouts << std::endl;
			p_os << OUTPUT_LAYOUTS_DELIMITER << std::endl;
			p_os << p_shader.outputLayouts << std::endl;
			p_os << CONSTANTS_DELIMITER << std::endl;
			p_os << p_shader.constants << std::endl;
			p_os << ATTRIBUTES_DELIMITER << std::endl;
			p_os << p_shader.attributes << std::endl;
			p_os << TEXTURES_DELIMITER << std::endl;
			p_os << p_shader.textures << std::endl;
			p_os << VERTEX_DELIMITER << std::endl;
			p_os << p_shader.vertexShaderCode << std::endl;
			p_os << FRAGMENT_DELIMITER << std::endl;
			p_os << p_shader.fragmentShaderCode << std::endl;

			return (p_os);
		}
	};
}