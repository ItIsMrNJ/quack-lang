#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include "lexer.hpp"
#include "parser.hpp"

using namespace std;

static void printUsage(const char *progName)
{
    cout << "Usage: " << progName << " <source_file> [-d | --debug]" << endl;
}

static string tokenTypeName(enum type t)
{
    switch (t)
    {
        case TOKEN_ID:              return "ID";
        case TOKEN_INT:             return "INT";
        case TOKEN_FLOAT:           return "FLOAT";
        case TOKEN_BOOL:            return "BOOL";
        case TOKEN_EQUALS:          return "EQUALS";
        case TOKEN_SEMICOLON:       return "SEMICOLON";
        case TOKEN_LEFT_PAREN:      return "LEFT_PAREN";
        case TOKEN_RIGHT_PAREN:     return "RIGHT_PAREN";
        case TOKEN_PLUS:            return "PLUS";
        case TOKEN_MINUS:           return "MINUS";
        case TOKEN_MULTIPLY:        return "MULTIPLY";
        case TOKEN_DIVIDE:          return "DIVIDE";
        case TOKEN_STRING:          return "STRING";
        case TOKEN_COMMA:           return "COMMA";
        case TOKEN_LEFT_BRACE:      return "LEFT_BRACE";
        case TOKEN_RIGHT_BRACE:     return "RIGHT_BRACE";
        case TOKEN_EQUALS_EQUALS:   return "EQUALS_EQUALS";
        case TOKEN_LESS:            return "LESS";
        case TOKEN_GREATER:         return "GREATER";
        case TOKEN_LESS_EQUALS:     return "LESS_EQUALS";
        case TOKEN_GREATER_EQUALS:  return "GREATER_EQUALS";
        case TOKEN_PLUS_EQUALS:     return "PLUS_EQUALS";
        case TOKEN_MINUS_EQUALS:    return "MINUS_EQUALS";
        case TOKEN_MULTIPLY_EQUALS: return "MULTIPLY_EQUALS";
        case TOKEN_DIVIDE_EQUALS:   return "DIVIDE_EQUALS";
        default:                    return "UNKNOWN";
    }
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printUsage(argv[0]);
        return 1;
    }

    string sourcePath = argv[1];
    bool debugMode = false;

    for (int i = 2; i < argc; i++)
    {
        string arg = argv[i];
        if (arg == "-d" || arg == "--debug")
            debugMode = true;
    }

    ifstream sourceFileStream(sourcePath);
    if (!sourceFileStream.is_open())
    {
        cout << "[ERROR] Could not open source file: " << sourcePath << endl;
        return 1;
    }

    stringstream buffer;
    buffer << sourceFileStream.rdbuf();
    string sourceCode = buffer.str();
    sourceFileStream.close();

    if (debugMode)
    {
        cout << "Reading from the file name: " << sourcePath << endl;
        cout << "-= Source Code =-" << endl << sourceCode << endl;
    }

    Lexer lexer(sourceCode);
    vector<Token*> tokens = lexer.tokenize();

    if (debugMode)
    {
        cout << "=== Tokens ===" << endl;
        int counter = 0;
        for (Token* t : tokens)
        {
            counter++;
            cout << counter << ") " << tokenTypeName(t->TYPE) << " : " << t->VALUE << endl;
        }
        cout << endl;
    }

    Parser parser(tokens);
    vector<Stmt*> ast = parser.parse();

    // Program output (quack(...) calls) now prints straight to the console.
    Interpreter interpreter;
    interpreter.interpret(ast);

    for (Stmt* stmt : ast)
        delete stmt;
    for (Token* t : tokens)
        delete t;

    if (debugMode)
        cout << "-= End of Program =-" << endl;

    return 0;
}
