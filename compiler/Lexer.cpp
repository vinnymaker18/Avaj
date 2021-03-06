#include "SyntaxError.h"
#include "SourceBuffer.h"
#include "Lexer.h"

Lexer::Lexer(const std::string& sourceFile):
    sourceBuffer(sourceFile)
{
}

Token Lexer::getNextToken()
{
    CharUnit chrUnit = sourceBuffer.getCharUnit();
    if (chrUnit.chr == EOF) {
        return _Token(TokenType::END_OF_FILE, "");
    }
    else if (chrUnit.chr == ';') {
        return _Token(TokenType::SEMI_COLON, ";");
    }
    else if (chrUnit.chr == ',') {
        return _Token(TokenType::COMMA, ",");
    }
    else if (chrUnit.chr == '.') {
        // A dot character starts a field member/method reference.
        Token ref = readIdentifier();
        if (ref.lexeme.empty()) {
            throw SyntaxError(ref.lineNo, 
                    "Invalid member reference.");
        }

        return _Token(TokenType::FIELD_REF, "." + ref.lexeme);
    }
    else if (chrUnit.chr == '?') {
        return _Token(TokenType::QUESTION_MARK, "?");
    }
    else if (chrUnit.chr == ':') {
        return _Token(TokenType::COLON, ":");
    }
    else if (chrUnit.chr == '"') {
        // Read a string literal.
        int lineNo = sourceBuffer.line(), columnNo = sourceBuffer.column();
        std::string lexeme = readStringLiteral();
        return Token(TokenType::STRING_LITERAL, lexeme, lineNo, columnNo);
    }
    else if (chrUnit.chr == '\'') {
        // Read a character literal.
        int lineNo = sourceBuffer.line(), columnNo = sourceBuffer.column();
        std::string lexeme = readStringLiteral('\'');

        if (lexeme.size() != 1 and (lexeme.size() != 2 or lexeme[0] != '\\')) {
            return _Token(TokenType::FAILURE, "Invalid char literal");
        }

        return Token(TokenType::CHAR_LITERAL, lexeme, lineNo, columnNo);
    }
    else if (chrUnit.chr == '=') {
        CharUnit newUnit = sourceBuffer.getCharUnit();
        if (newUnit.chr != '=') {
            sourceBuffer.pushCharBack(newUnit);
            return _Token(TokenType::ASSIGNMENT, "=");
        }
        return _Token(TokenType::EQ, "==");
    }     
    else if (isBraceOrBracket(chrUnit)) {
        char c = chrUnit.chr;
        TokenType tokenType = c == '{' ? TokenType::LEFT_BRACE :
                              c == '}' ? TokenType::RIGHT_BRACE :
                              c == '[' ? TokenType::LEFT_SQUARE_BKT :
                              c == ']' ? TokenType::RIGHT_SQUARE_BKT :
                              c == '(' ? TokenType::LEFT_BRACKET :
                              TokenType::RIGHT_BRACKET;

        return _Token(tokenType, std::string(1, c));
    }
    else if (isspace(chrUnit.chr)) {
        // Read all the contiguous whitespace.
        while (true) {
            CharUnit unit = sourceBuffer.getCharUnit();
            if (!isspace(unit.chr)) {
                sourceBuffer.pushCharBack(unit);
                break;
            }
        }

        CharUnit unit = sourceBuffer.getCharUnit();
        if (unit.chr == '.') {
            sourceBuffer.pushCharBack(unit);
            throw SyntaxError(sourceBuffer.line(), 
                    "Unexpected dot symbol.");
        }
        sourceBuffer.pushCharBack(unit);
        
        // Ignore this whitespace and return the next token.
        return getNextToken();
    }
    else if (alpha_(chrUnit)) {
        // Read an identifier.
        sourceBuffer.pushCharBack(chrUnit);
        return readIdentifier();
    }
    else if (chrUnit.chr == '>' || chrUnit.chr == '<') {
        // >, <, >=, <=
        CharUnit newUnit = sourceBuffer.getCharUnit();
        if (newUnit.chr == '=') {
            std::string op(2, '=');
            op[0] = chrUnit.chr;
            auto opType = chrUnit.chr == '>' ? TokenType::GE : TokenType::LE;
            return _Token(opType, op);
        } else {
            sourceBuffer.pushCharBack(newUnit);
            auto opType = chrUnit.chr == '>' ? TokenType::GT : TokenType::LT;
            return _Token(opType, std::string(1, chrUnit.chr));
        }
    }
    else if (chrUnit.chr == '!') {
        // Could be '!' or '!='
        CharUnit newUnit = sourceBuffer.getCharUnit();
        if (newUnit.chr == '=') {
            return _Token(TokenType::NE, "!=");
        }
        sourceBuffer.pushCharBack(newUnit);
        return _Token(TokenType::NOT, "!");
    }
    else if (chrUnit.chr == '~') {
        // Could be '~' or '~='
        CharUnit newUnit = sourceBuffer.getCharUnit();
        if (newUnit.chr == '=') {
            return _Token(TokenType::BITWISE_NOT_EQ, "~=");
        }
        sourceBuffer.pushCharBack(newUnit);
        return _Token(TokenType::BITWISE_NOT, "~");
    }
    else if (chrUnit.chr == '^') {
        // ^ or ^=
        CharUnit newUnit = sourceBuffer.getCharUnit();
        if (newUnit.chr == '=') {
            return _Token(TokenType::BITWISE_XOR_EQ, "^=");
        }
        sourceBuffer.pushCharBack(newUnit);
        return _Token(TokenType::BITWISE_XOR, "^");
    }
    else if (chrUnit.chr == '%' || chrUnit.chr == '*' || chrUnit.chr == '/') {
        // *, %, /, *=, %=, /=
        CharUnit newUnit = sourceBuffer.getCharUnit();
        if (newUnit.chr == '=') {
            std::string op(2, '=');
            op[0] = chrUnit.chr;
            TokenType type = chrUnit.chr == '%' ? TokenType::MOD_EQ :
                             chrUnit.chr == '*' ? TokenType::MULT_EQ :
                             TokenType::DIV_EQ;
            return _Token(type, op);
        } else {
            sourceBuffer.pushCharBack(newUnit);
            std::string op(1, chrUnit.chr);
            TokenType type = chrUnit.chr == '%' ? TokenType::MODULO :
                             chrUnit.chr == '*' ? TokenType::MULTIPLY :
                             TokenType::DIVIDE;

            return _Token(type, op);
        }
    }
    else if (chrUnit.chr == '+' || chrUnit.chr == '-') {
        // Could be an operator +, -, +=, -=, ++, --.
        // Could be a number.

        CharUnit newUnit = sourceBuffer.getCharUnit();
        if (isdigit(newUnit.chr)) {
            sourceBuffer.pushCharBack(newUnit);
            sourceBuffer.pushCharBack(chrUnit);
            return readNumber();
        }
        else if (newUnit.chr == chrUnit.chr) {
            if (newUnit.chr == '+')
                return _Token(TokenType::INCREMENT, "++");
            else
                return _Token(TokenType::DECREMENT, "--");
        }
        else if (newUnit.chr == '=') {
            // += or -=
            if (chrUnit.chr == '+')
                return _Token(TokenType::PLUS_EQ, "+=");
            else
                return _Token(TokenType::MINUS_EQ, "-=");
        }
        else {
            sourceBuffer.pushCharBack(newUnit);
            TokenType type = chrUnit.chr == '+' ? TokenType::PLUS :
                                    TokenType::MINUS;
            return _Token(type, std::string(1, chrUnit.chr));
        }
    }
    else if (chrUnit.chr == '&') {
        // &, &&, &=, &&=
        CharUnit newUnit = sourceBuffer.getCharUnit();
        if (newUnit.chr == '=') {
            return _Token(TokenType::BITWISE_AND_EQ, "&=");
        } else if (newUnit.chr == '&') {
            CharUnit nxtUnit = sourceBuffer.getCharUnit();
            if (nxtUnit.chr == '=') {
                return _Token(TokenType::AND_EQ, "&&=");
            } else {
                sourceBuffer.pushCharBack(nxtUnit);
                return _Token(TokenType::AND, "&&");
            }
        } else {
            sourceBuffer.pushCharBack(newUnit);
            return _Token(TokenType::BITWISE_AND, "&");
        }
    }
    else if (chrUnit.chr == '|') {
        // |, ||, |=, ||=
        CharUnit newUnit = sourceBuffer.getCharUnit();
        if (newUnit.chr == '=') {
            return _Token(TokenType::BITWISE_OR_EQ, "|=");
        } else if (newUnit.chr == '|') {
            CharUnit nxtUnit = sourceBuffer.getCharUnit();
            if (nxtUnit.chr == '=') {
                return _Token(TokenType::OR_EQ, "||=");
            } else {
                sourceBuffer.pushCharBack(nxtUnit);
                return _Token(TokenType::OR, "||");
            }
        } else {
            sourceBuffer.pushCharBack(newUnit);
            return _Token(TokenType::BITWISE_OR, "|");
        }
    }
    // All numerical tokens must start with a digit.
    else if (isdigit(chrUnit.chr)) {
        sourceBuffer.pushCharBack(chrUnit);
        return readNumber();
    }
    else {
        return _Token(TokenType::FAILURE, std::string(1, chrUnit.chr));
    }
}

Token Lexer::readNumber()
{
    // Numbers are of the form -> [+-]?[0-9]+(.[0-9]*)?(E[0-9]+)?)
    std::string prefix = readSignedDigitString();
    if (prefix.empty() || prefix == "+" || prefix == "-") {
        return _Token(TokenType::FAILURE, "");
    }
    std::ostringstream out;
    out << prefix;
    
    CharUnit unit = sourceBuffer.getCharUnit();
    if (unit.chr == '.') {
        out << ".";
        out << readDigitString();

        CharUnit unit = sourceBuffer.getCharUnit();
        if (unit.chr == 'E' || unit.chr == 'e') {
            out << unit.chr;
            out << readSignedDigitString();
            return _Token(TokenType::EXPONENT, out.str());
        } else {
            sourceBuffer.pushCharBack(unit);
            return _Token(TokenType::FLOATING, out.str());
        }
    } else if (unit.chr == 'E' || unit.chr == 'e') {
        out << unit.chr;
        std::string exponent = readSignedDigitString();
        out << exponent;
        return _Token(TokenType::EXPONENT, out.str());
    } else {
        sourceBuffer.pushCharBack(unit);
        return _Token(TokenType::INTEGER, out.str());
    }
}

Token Lexer::readIdentifier()
{
    std::ostringstream out;
    while (true) {
        CharUnit unit = sourceBuffer.getCharUnit();
        if (!alpha_(unit)) {
            sourceBuffer.pushCharBack(unit);
            break;
        }
        out << unit.chr;
    }

    std::string id = out.str();
    if (KEYWORDS.count(id)) {
        // This is a keyword.
        return _Token(TokenType::KEYWORD, id);
    }
    return _Token(TokenType::IDENTIFIER, id);
}
