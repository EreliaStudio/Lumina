#include "tokenizer.hpp"

#include "file_io.hpp"
#include "utils.hpp"

#include <stdexcept>
#include <string_view>
#include <utility>

namespace
{
	struct ScanContext
	{
		explicit ScanContext(std::string_view p_source) : source(p_source) {}

		bool eof(std::size_t lookahead = 0) const
		{
			return (cursor.offset + lookahead) >= source.size();
		}

		char peek(std::size_t lookahead = 0) const
		{
			if (cursor.offset + lookahead >= source.size())
			{
				return '\0';
			}
			return source[cursor.offset + lookahead];
		}

		char advance()
		{
			const char c = peek();
			if (!eof())
			{
				advanceCursor(cursor, c);
			}
			return c;
		}

		std::string_view slice(std::size_t begin) const
		{
			return source.substr(begin, cursor.offset - begin);
		}

		std::string_view source;
		Cursor cursor;
	};

	Token::Location makeLocation(const Cursor &cursor)
	{
		return Token::Location{cursor.line, cursor.column};
	}

	void throwTokenizerError(const std::filesystem::path &origin, const Cursor &cursor, const std::string &message)
	{
		throw std::runtime_error(
		    origin.string() + ":" + std::to_string(cursor.line) + ":" + std::to_string(cursor.column) + ": " + message);
	}

	Token makeToken(const std::filesystem::path &origin, const ScanContext &ctx, std::size_t tokenStart, Token::Type type,
	    const Token::Location &startLoc)
	{
		Token token;
		token.origin = origin;
		token.type = type;
		token.content.assign(ctx.source.data() + tokenStart, ctx.source.data() + ctx.cursor.offset);
		token.start = startLoc;
		token.end = makeLocation(ctx.cursor);
		return token;
	}

	void skipTrivia(ScanContext &ctx, const std::filesystem::path &origin)
	{
		while (!ctx.eof())
		{
			const char ch = ctx.peek();
			if (isWhitespace(ch))
			{
				ctx.advance();
				continue;
			}

			if (ch == '/' && ctx.peek(1) == '/')
			{
				ctx.advance();
				ctx.advance();
				while (!ctx.eof() && ctx.peek() != '\n')
				{
					ctx.advance();
				}
				continue;
			}

			if (ch == '/' && ctx.peek(1) == '*')
			{
				const Token::Location startLoc = makeLocation(ctx.cursor);
				ctx.advance();
				ctx.advance();

				bool closed = false;
				while (!ctx.eof())
				{
					if (ctx.peek() == '*' && ctx.peek(1) == '/')
					{
						ctx.advance();
						ctx.advance();
						closed = true;
						break;
					}
					ctx.advance();
				}

				if (!closed)
				{
					throwTokenizerError(origin, ctx.cursor,
					    "Unterminated block comment that started at line " + std::to_string(startLoc.line));
				}
				continue;
			}

			break;
		}
	}

	Token lexIdentifier(const std::filesystem::path &origin, ScanContext &ctx)
	{
		const Token::Location startLoc = makeLocation(ctx.cursor);
		const std::size_t begin = ctx.cursor.offset;

		ctx.advance();
		while (!ctx.eof() && isIdentifierBody(ctx.peek()))
		{
			ctx.advance();
		}

		std::string_view lexeme = ctx.slice(begin);
		const std::optional<Token::Type> keyword = lookupKeyword(lexeme);
		const Token::Type type = keyword.value_or(Token::Type::Identifier);
		return makeToken(origin, ctx, begin, type, startLoc);
	}

	Token lexNumber(const std::filesystem::path &origin, ScanContext &ctx, bool leadingDot)
	{
		const Token::Location startLoc = makeLocation(ctx.cursor);
		const std::size_t begin = ctx.cursor.offset;

		bool isFloat = false;

	if (leadingDot)
	{
		isFloat = true;
		ctx.advance();
		if (!isDigit(ctx.peek()))
		{
			throwTokenizerError(origin, ctx.cursor, "Malformed floating-point literal");
		}
	}

	if (!leadingDot && ctx.peek() == '0' && (ctx.peek(1) == 'x' || ctx.peek(1) == 'X'))
	{
		ctx.advance();
		ctx.advance();
		if (!isHexDigit(ctx.peek()))
		{
			throwTokenizerError(origin, ctx.cursor, "Malformed hexadecimal literal");
		}
		while (!ctx.eof() && isHexDigit(ctx.peek()))
		{
			ctx.advance();
		}
		if (ctx.peek() == 'u' || ctx.peek() == 'U')
		{
			ctx.advance();
		}
		return makeToken(origin, ctx, begin, Token::Type::IntegerLiteral, startLoc);
	}

	while (!ctx.eof() && isDigit(ctx.peek()))
	{
		ctx.advance();
	}

		if (!leadingDot && ctx.peek() == '.')
		{
			isFloat = true;
			ctx.advance();
			while (!ctx.eof() && isDigit(ctx.peek()))
			{
				ctx.advance();
			}
		}

		if (ctx.peek() == 'e' || ctx.peek() == 'E')
		{
			isFloat = true;
			ctx.advance();
			if (ctx.peek() == '+' || ctx.peek() == '-')
			{
				ctx.advance();
			}
			if (!isDigit(ctx.peek()))
			{
				throwTokenizerError(origin, ctx.cursor, "Malformed exponent in numeric literal");
			}
			while (!ctx.eof() && isDigit(ctx.peek()))
			{
				ctx.advance();
			}
		}

		if (ctx.peek() == 'f' || ctx.peek() == 'F')
		{
			isFloat = true;
			ctx.advance();
		}
		else if (!isFloat && (ctx.peek() == 'u' || ctx.peek() == 'U'))
		{
			ctx.advance();
		}

		return makeToken(origin, ctx, begin, isFloat ? Token::Type::FloatLiteral : Token::Type::IntegerLiteral, startLoc);
	}

	Token lexString(const std::filesystem::path &origin, ScanContext &ctx)
	{
		const Token::Location startLoc = makeLocation(ctx.cursor);
		const std::size_t begin = ctx.cursor.offset;
		ctx.advance();

		bool closed = false;
		bool escaping = false;

		while (!ctx.eof())
		{
			const char c = ctx.advance();
			if (!escaping && c == '\n')
			{
				throwTokenizerError(origin, ctx.cursor, "Unterminated string literal");
			}
			if (!escaping && c == '"')
			{
				closed = true;
				break;
			}

			if (!escaping && c == '\\')
			{
				escaping = true;
			}
			else
			{
				escaping = false;
			}
		}

		if (!closed)
		{
			throwTokenizerError(origin, ctx.cursor, "Unterminated string literal");
		}

		return makeToken(origin, ctx, begin, Token::Type::StringLiteral, startLoc);
	}

	Token lexHeader(const std::filesystem::path &origin, ScanContext &ctx)
	{
		const Token::Location startLoc = makeLocation(ctx.cursor);
		const std::size_t begin = ctx.cursor.offset;
		ctx.advance();

		bool closed = false;
		while (!ctx.eof())
		{
			const char c = ctx.advance();
			if (c == '>')
			{
				closed = true;
				break;
			}
			if (c == '\n')
			{
				throwTokenizerError(origin, ctx.cursor, "Unterminated header literal");
			}
		}

		if (!closed)
		{
			throwTokenizerError(origin, ctx.cursor, "Unterminated header literal");
		}

		return makeToken(origin, ctx, begin, Token::Type::HeaderLiteral, startLoc);
	}
}

Tokenizer::Tokenizer() = default;

std::vector<Token> Tokenizer::operator()(const std::filesystem::path &p_path) const
{
	std::string sanitized = normalizeLineEndings(readFile(p_path));
	ScanContext ctx(sanitized);

	std::vector<Token> tokens;
	tokens.reserve(sanitized.empty() ? 0 : sanitized.size() / 4 + 8);

	while (true)
	{
		skipTrivia(ctx, p_path);
		if (ctx.eof())
		{
			break;
		}

		const char ch = ctx.peek();

		if (isIdentifierStart(ch))
		{
			tokens.emplace_back(lexIdentifier(p_path, ctx));
			continue;
		}
		if (isDigit(ch) || (ch == '.' && isDigit(ctx.peek(1))))
		{
			tokens.emplace_back(lexNumber(p_path, ctx, ch == '.'));
			continue;
		}

		const Token::Location startLoc = makeLocation(ctx.cursor);
		const std::size_t tokenStart = ctx.cursor.offset;
		Token::Type tokenType = Token::Type::EndOfFile;

		switch (ch)
		{
			case '#':
				ctx.advance();
				tokenType = Token::Type::Hash;
				break;
			case '"':
				tokens.emplace_back(lexString(p_path, ctx));
				continue;
			case '<':
				if (!tokens.empty() && tokens.back().type == Token::Type::KeywordInclude)
				{
					tokens.emplace_back(lexHeader(p_path, ctx));
					continue;
				}
				ctx.advance();
				tokenType = Token::Type::Less;
				if (ctx.peek() == '<')
				{
					ctx.advance();
					tokenType = Token::Type::ShiftLeft;
					if (ctx.peek() == '=')
					{
						ctx.advance();
						tokenType = Token::Type::ShiftLeftEqual;
					}
				}
				else if (ctx.peek() == '=')
				{
					ctx.advance();
					tokenType = Token::Type::LessEqual;
				}
				break;
			case '>':
				ctx.advance();
				tokenType = Token::Type::Greater;
				if (ctx.peek() == '>')
				{
					ctx.advance();
					tokenType = Token::Type::ShiftRight;
					if (ctx.peek() == '=')
					{
						ctx.advance();
						tokenType = Token::Type::ShiftRightEqual;
					}
				}
				else if (ctx.peek() == '=')
				{
					ctx.advance();
					tokenType = Token::Type::GreaterEqual;
				}
				break;
			case '(':
				ctx.advance();
				tokenType = Token::Type::LeftParen;
				break;
			case ')':
				ctx.advance();
				tokenType = Token::Type::RightParen;
				break;
			case '{':
				ctx.advance();
				tokenType = Token::Type::LeftBrace;
				break;
			case '}':
				ctx.advance();
				tokenType = Token::Type::RightBrace;
				break;
			case '[':
				ctx.advance();
				tokenType = Token::Type::LeftBracket;
				break;
			case ']':
				ctx.advance();
				tokenType = Token::Type::RightBracket;
				break;
			case ';':
				ctx.advance();
				tokenType = Token::Type::Semicolon;
				break;
			case ',':
				ctx.advance();
				tokenType = Token::Type::Comma;
				break;
			case '.':
				ctx.advance();
				tokenType = Token::Type::Dot;
				break;
			case ':':
				ctx.advance();
				tokenType = Token::Type::Colon;
				if (ctx.peek() == ':')
				{
					ctx.advance();
					tokenType = Token::Type::DoubleColon;
				}
				break;
			case '+':
				ctx.advance();
				tokenType = Token::Type::Plus;
				if (ctx.peek() == '+')
				{
					ctx.advance();
					tokenType = Token::Type::PlusPlus;
				}
				else if (ctx.peek() == '=')
				{
					ctx.advance();
					tokenType = Token::Type::PlusEqual;
				}
				break;
			case '-':
				ctx.advance();
				tokenType = Token::Type::Minus;
				if (ctx.peek() == '>')
				{
					ctx.advance();
					tokenType = Token::Type::Arrow;
				}
				else if (ctx.peek() == '-')
				{
					ctx.advance();
					tokenType = Token::Type::MinusMinus;
				}
				else if (ctx.peek() == '=')
				{
					ctx.advance();
					tokenType = Token::Type::MinusEqual;
				}
				break;
			case '*':
				ctx.advance();
				tokenType = Token::Type::Star;
				if (ctx.peek() == '=')
				{
					ctx.advance();
					tokenType = Token::Type::StarEqual;
				}
				break;
			case '/':
				ctx.advance();
				tokenType = Token::Type::Slash;
				if (ctx.peek() == '=')
				{
					ctx.advance();
					tokenType = Token::Type::SlashEqual;
				}
				break;
			case '%':
				ctx.advance();
				tokenType = Token::Type::Percent;
				if (ctx.peek() == '=')
				{
					ctx.advance();
					tokenType = Token::Type::PercentEqual;
				}
				break;
			case '!':
				ctx.advance();
				tokenType = Token::Type::Bang;
				if (ctx.peek() == '=')
				{
					ctx.advance();
					tokenType = Token::Type::BangEqual;
				}
				break;
			case '=':
				ctx.advance();
				tokenType = Token::Type::Assign;
				if (ctx.peek() == '=')
				{
					ctx.advance();
					tokenType = Token::Type::Equal;
				}
				break;
			case '&':
				ctx.advance();
				tokenType = Token::Type::Ampersand;
				if (ctx.peek() == '&')
				{
					ctx.advance();
					tokenType = Token::Type::AmpersandAmpersand;
				}
				else if (ctx.peek() == '=')
				{
					ctx.advance();
					tokenType = Token::Type::AmpersandEqual;
				}
				break;
			case '|':
				ctx.advance();
				tokenType = Token::Type::Pipe;
				if (ctx.peek() == '|')
				{
					ctx.advance();
					tokenType = Token::Type::PipePipe;
				}
				else if (ctx.peek() == '=')
				{
					ctx.advance();
					tokenType = Token::Type::PipeEqual;
				}
				break;
			case '^':
				ctx.advance();
				tokenType = Token::Type::Caret;
				if (ctx.peek() == '=')
				{
					ctx.advance();
					tokenType = Token::Type::CaretEqual;
				}
				break;
			case '?':
				ctx.advance();
				tokenType = Token::Type::Question;
				break;
			case '~':
				ctx.advance();
				tokenType = Token::Type::Tilde;
				break;
			default:
				throwTokenizerError(p_path, ctx.cursor, "Unexpected character '" + std::string(1, ch) + "'");
		}

		tokens.emplace_back(makeToken(p_path, ctx, tokenStart, tokenType, startLoc));
	}

	Token eof;
	eof.origin = p_path;
	eof.type = Token::Type::EndOfFile;
	eof.start = eof.end = makeLocation(ctx.cursor);
	tokens.emplace_back(std::move(eof));

	return tokens;
}
