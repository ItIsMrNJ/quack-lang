#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "lexer.hpp"
#include "parser.hpp"

using namespace std;

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        cout << "Please supply the source file." << endl;
        return 1;
    }

    cout << "Reading from the file name: " << argv[1] << endl;
    ifstream sourceFileStream(argv[1]);

    stringstream buffer;
    char temp;
    while (sourceFileStream.get(temp))
    {
        buffer << temp;
    }

    string sourceCode = buffer.str();
    cout << "-=This is a SourceCode=-" << endl << sourceCode << endl;

    Lexer lexer(sourceCode);
    vector<Token*> tokens = lexer.tokenize();

    cout << "=== Tokens ===" << endl;
    int counter = 0;
    for (Token* t : tokens)
    {
        counter++;
        cout << counter << ") " << t->TYPE << " : " << t->VALUE << endl;
    }
    cout << endl;

    Parser parser(tokens);
    vector<Stmt*> ast = parser.parse();

    ofstream outputFile("output.txt");
    if (!outputFile.is_open()) {
        cout << "Failed to open output file." << endl;
        return 1;
    }

    Interpreter interpreter;
    
    streambuf* oldCout = cout.rdbuf();
    cout.rdbuf(outputFile.rdbuf());

    interpreter.interpret(ast);

    cout.rdbuf(oldCout);
    outputFile.close();

    for (Stmt* stmt : ast)
    {
        delete stmt;
    }

    cout << "-= End of Program =-" << endl;
    
    return 0;
}