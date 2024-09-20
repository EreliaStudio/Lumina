#pragma once

#include "lumina_metatokens.hpp"
#include "lumina_descriptors.hpp"
#include "lumina_exception.hpp"

namespace Lumina
{
    class MetaTokenizer
    {
    public:
        using TokenType = Lumina::Token::Type;
        using Product = Lumina::Expected<std::vector<std::shared_ptr<MetaToken>>>;

    private:
        Product _result;
        std::vector<Lumina::Token> _tokens;
        size_t _index = 0;
        Lumina::Token noToken;

        MetaTokenizer();

        void expendInclude();
        TypeDescriptor parseTypeDescriptor();
        size_t parseArraySize();
        VariableDescriptor parseVariableDescriptor();
        std::shared_ptr<PipelineFlowMetaToken> parsePipelineFlowMetaToken();
        std::shared_ptr<BlockMetaToken> parseBlockMetaToken(const TokenType& p_tokenType);
        std::shared_ptr<TextureMetaToken> parseTextureMetaToken();
        ReturnTypeDescriptor parseReturnTypeDescriptor();
        SymbolBody parseSymbolBody();
        std::shared_ptr<FunctionMetaToken> parseFunctionMetaToken();
        std::shared_ptr<PipelineBodyMetaToken> parsePipelineBodyMetaToken();
        std::shared_ptr<NamespaceMetaToken> parseNamespaceMetaToken();

        Product _analyse(const std::vector<Lumina::Token>& p_tokens);
        bool hasTokenLeft() const;
        void backOff();
        void advance();
        const Lumina::Token& currentToken() const;
        const Lumina::Token& tokenAtIndex(size_t p_index) const;
        const Lumina::Token& nextToken() const;
        void skipToken();
        void skipLine();
        void skipUntilReach(const TokenType& p_type);
        void skipUntilReach(const std::vector<TokenType>& p_types);
        const Lumina::Token& expect(Lumina::Token::Type p_expectedType, const std::string& p_errorMessage);
        const Lumina::Token& expect(std::vector<Lumina::Token::Type> p_expectedTypes, const std::string& p_errorMessage);

    public:
        static Product analyse(const std::vector<Lumina::Token>& p_tokens);
    };
}
