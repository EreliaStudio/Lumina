#include "lumina_descriptors.hpp"

namespace Lumina
{
    void TypeDescriptor::append(const Lumina::Token& p_newToken)
    {
        if (value.content.empty())
        {
            value = p_newToken;
        }
        else
        {
            value.content += p_newToken.content;
        }
    }
}
