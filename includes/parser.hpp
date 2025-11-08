#pragma once

#include "ast.hpp"

#include <memory>
#include <vector>

struct Parser
{
	Parser();
	~Parser();

	std::vector<std::unique_ptr<Instruction>> operator()(std::vector<Token> p_tokens);

private:
	struct Impl;
	std::unique_ptr<Impl> m_impl;
};
