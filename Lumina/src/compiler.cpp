#include "compiler.hpp"

namespace Lumina
{
	std::ostream& operator << (std::ostream& p_os, const ShaderImpl& p_shader)
	{
		p_os << ShaderImpl::LAYOUTS_DELIMITER << std::endl;
		p_os << p_shader.layoutContent << std::endl;
		p_os << std::endl;
		p_os << ShaderImpl::CONSTANTS_DELIMITER << std::endl;
		p_os << p_shader.constantContent << std::endl;
		p_os << std::endl;
		p_os << ShaderImpl::ATTRIBUTES_DELIMITER << std::endl;
		p_os << p_shader.attributeContent << std::endl;
		p_os << std::endl;
		p_os << ShaderImpl::TEXTURES_DELIMITER << std::endl;
		p_os << p_shader.textureContent << std::endl;
		p_os << std::endl;
		p_os << ShaderImpl::VERTEX_DELIMITER << std::endl;
		p_os << p_shader.vertexCodeContent << std::endl;
		p_os << std::endl;
		p_os << ShaderImpl::FRAGMENT_DELIMITER << std::endl;
		p_os << p_shader.fragmentCodeContent << std::endl;
		p_os << std::endl;

		return (p_os);
	}

	ShaderImpl Compiler::_compile(const Parser::Output& p_input)
	{
		return (_product);
	}

	ShaderImpl Compiler::compile(const Parser::Output& p_input)
	{
		return (Compiler()._compile(p_input));
	}
}