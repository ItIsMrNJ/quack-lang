#ifndef __LEXER_H
#define __LEXER_H
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <cctype>

using namespace std;

enum type
{
    TOKEN_ID,
    TOKEN_INT,
    TOKEN_FLOAT,
    TOKEN_BOOL,
    TOKEN_EQUALS,
    TOKEN_SEMICOLON,
    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULTIPLY,
    TOKEN_DIVIDE,
    TOKEN_STRING,
    TOKEN_COMMA,
    TOKEN_LEFT_BRACE,
    TOKEN_RIGHT_BRACE,
    TOKEN_EQUALS_EQUALS,
    TOKEN_LESS,
    TOKEN_GREATER,
    TOKEN_LESS_EQUALS,
    TOKEN_GREATER_EQUALS,
    TOKEN_PLUS_EQUALS,
    TOKEN_MINUS_EQUALS,
    TOKEN_MULTIPLY_EQUALS,
    TOKEN_DIVIDE_EQUALS
};

struct Token
{
    enum type TYPE;
    string VALUE;
};

class Lexer
{
public:
    Lexer(string sourceCode)
    {
        source = sourceCode;
        cursor = 0;
        size = sourceCode.length();
        current = (size > 0) ? sourceCode[0] : '\0';
    }

    char advance()
    {
        if (cursor < size)
        {
            char temp = current;
            cursor++;
            current = (cursor < size) ? source[cursor] : '\0';
            return temp;
        }
        return '\0';
    }

    void checkAndSkips()
    {
        while (cursor < size && (current == ' ' || current == '\t' || current == '\r' || current == '\n'))
        {
            advance();
        }
    }

    Token * tokenizeID()
    {
        stringstream buffer;
        buffer << current;
        advance();

        while (isalnum(current) || current == '_')
        {
            buffer << current;
            advance();
        }

        string word = buffer.str();

        Token * newToken = new Token();
        if (word == "true" || word == "false")
        {
            newToken -> TYPE = TOKEN_BOOL;
        }
        else
        {
            newToken -> TYPE = TOKEN_ID;
        }
        newToken -> VALUE = word;

        return newToken;
    }

    Token * tokenizeSTRING()
    {
        stringstream buffer;
        advance();

        while (current != '"' && current != '\0')
        {
            buffer << current;
            advance();
        }

        if (current == '"') {
            advance();
        }

        Token * newToken = new Token();
        newToken -> TYPE = TOKEN_STRING;
        newToken -> VALUE = buffer.str();

        return newToken;
    }

    Token * tokenizeSPECIAL(enum type TYPE, string val)
    {
        Token * newToken = new Token();
        newToken -> TYPE = TYPE;
        newToken -> VALUE = val;
        advance();
        return newToken;
    }

    Token *tokenizeINT()
    {
        stringstream buffer;
        bool isFloat = false;

        while(isdigit(current))
        {
            buffer << current;
            advance();
        }

        // A '.' followed by a digit turns this into a float literal.
        // A '.' NOT followed by a digit is left alone (not consumed here),
        // so it can't accidentally swallow something else later.
        if (current == '.' && cursor + 1 < size && isdigit(source[cursor + 1]))
        {
            isFloat = true;
            buffer << current;
            advance();
            while (isdigit(current))
            {
                buffer << current;
                advance();
            }
        }

        Token * newToken = new Token();
        newToken -> TYPE = isFloat ? TOKEN_FLOAT : TOKEN_INT;
        newToken -> VALUE = buffer.str();

        return newToken;
    }

    vector<Token *> tokenize()
    {
        vector<Token *> tokens;

        while (cursor < size)
        {
            checkAndSkips();

            if (cursor >= size || current == '\0') 
                break;

            if (isalpha(current) || current == '_')
            {
                tokens.push_back(tokenizeID());
                continue;
            }
            if (isdigit(current))
            {
                tokens.push_back(tokenizeINT());
                continue;
            }
            if (current == '"')
            {
                tokens.push_back(tokenizeSTRING());
                continue;
            }

            switch (current)
            {
                case ';':
                    tokens.push_back(tokenizeSPECIAL(TOKEN_SEMICOLON, ";"));
                    break;
                case '=':
                    if (cursor + 1 < size && source[cursor + 1] == '=') {
                        Token * newToken = new Token();
                        newToken -> TYPE = TOKEN_EQUALS_EQUALS;
                        newToken -> VALUE = "==";
                        advance(); advance();
                        tokens.push_back(newToken);
                    } else {
                        tokens.push_back(tokenizeSPECIAL(TOKEN_EQUALS, "="));
                    }
                    break;
                case '<':
                    if (cursor + 1 < size && source[cursor + 1] == '=') {
                        Token * newToken = new Token();
                        newToken -> TYPE = TOKEN_LESS_EQUALS;
                        newToken -> VALUE = "<=";
                        advance(); advance();
                        tokens.push_back(newToken);
                    } else {
                        tokens.push_back(tokenizeSPECIAL(TOKEN_LESS, "<"));
                    }
                    break;
                case '>':
                    if (cursor + 1 < size && source[cursor + 1] == '=') {
                        Token * newToken = new Token();
                        newToken -> TYPE = TOKEN_GREATER_EQUALS;
                        newToken -> VALUE = ">=";
                        advance(); advance();
                        tokens.push_back(newToken);
                    } else {
                        tokens.push_back(tokenizeSPECIAL(TOKEN_GREATER, ">"));
                    }
                    break;
                case '(':
                    tokens.push_back(tokenizeSPECIAL(TOKEN_LEFT_PAREN, "("));
                    break;
                case ')':
                    tokens.push_back(tokenizeSPECIAL(TOKEN_RIGHT_PAREN, ")"));
                    break;
                case '{':
                    tokens.push_back(tokenizeSPECIAL(TOKEN_LEFT_BRACE, "{"));
                    break;
                case '}':
                    tokens.push_back(tokenizeSPECIAL(TOKEN_RIGHT_BRACE, "}"));
                    break;
                case ',':
                    tokens.push_back(tokenizeSPECIAL(TOKEN_COMMA, ","));
                    break;
                case '+':
                    if (cursor + 1 < size && source[cursor + 1] == '=') {
                        Token * newToken = new Token();
                        newToken -> TYPE = TOKEN_PLUS_EQUALS;
                        newToken -> VALUE = "+=";
                        advance(); advance();
                        tokens.push_back(newToken);
                    } else {
                        tokens.push_back(tokenizeSPECIAL(TOKEN_PLUS, "+"));
                    }
                    break;
                case '-':
                    if (cursor + 1 < size && source[cursor + 1] == '=') {
                        Token * newToken = new Token();
                        newToken -> TYPE = TOKEN_MINUS_EQUALS;
                        newToken -> VALUE = "-=";
                        advance(); advance();
                        tokens.push_back(newToken);
                    } else {
                        tokens.push_back(tokenizeSPECIAL(TOKEN_MINUS, "-"));
                    }
                    break;
                case '*':
                    if (cursor + 1 < size && source[cursor + 1] == '=') {
                        Token * newToken = new Token();
                        newToken -> TYPE = TOKEN_MULTIPLY_EQUALS;
                        newToken -> VALUE = "*=";
                        advance(); advance();
                        tokens.push_back(newToken);
                    } else {
                        tokens.push_back(tokenizeSPECIAL(TOKEN_MULTIPLY, "*"));
                    }
                    break;
                case '/':
                    if (cursor + 1 < size && source[cursor + 1] == '=') {
                        Token * newToken = new Token();
                        newToken -> TYPE = TOKEN_DIVIDE_EQUALS;
                        newToken -> VALUE = "/=";
                        advance(); advance();
                        tokens.push_back(newToken);
                    } else {
                        tokens.push_back(tokenizeSPECIAL(TOKEN_DIVIDE, "/"));
                    }
                    break;
                default:
                    cout << "[1] PARSER ERROR : Unidentified symbol '" << current 
                         << "' (ASCII " << (int)current << ") at position " << cursor << endl;
                    exit(1);
            }
        }
        return tokens;
    }

private:
    string source;
    int cursor;
    int size;
    char current;
};

#endif