#pragma once

#include "lumina_token.hpp"

namespace Lumina
{
    struct TypeDescriptor
    {
        Lumina::Token value;

        void append(const Lumina::Token& p_newToken);
    };

    struct VariableDescriptor
    {
        TypeDescriptor type;
        Lumina::Token name;
        size_t arraySize;
    };

    struct ReturnTypeDescriptor
    {
        TypeDescriptor type;
        size_t arraySize;
    };
}