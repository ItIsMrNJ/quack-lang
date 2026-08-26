#ifndef __PARSER_H
#define __PARSER_H

#include "lexer.hpp"
#include <vector>
#include <iostream>
#include <map>
#include <variant>

using namespace std;

struct Value;
class Stmt;

struct Function {
    vector<string> params;
    vector<Stmt*> body;
};

struct Value {
    variant<int, double, string, bool, Function*> val;

    bool isInt() const { return holds_alternative<int>(val); }
    bool isFloat() const { return holds_alternative<double>(val); }
    bool isNumber() const { return isInt() || isFloat(); }
    bool isString() const { return holds_alternative<string>(val); }
    bool isBool() const { return holds_alternative<bool>(val); }
    bool isFunc() const { return holds_alternative<Function*>(val); }

    int asInt() const { return get<int>(val); }
    double asFloat() const { return get<double>(val); }
    // Use when you just need a numeric value regardless of int/float.
    double asNumber() const { return isInt() ? (double)get<int>(val) : get<double>(val); }
    string asString() const { return get<string>(val); }
    bool asBool() const { return get<bool>(val); }
    Function* asFunc() const { return get<Function*>(val); }

    void print() const {
        if (isInt()) cout << asInt() << endl;
        else if (isFloat()) cout << asFloat() << endl;
        else if (isString()) cout << asString() << endl;
        else if (isBool()) cout << (asBool() ? "true" : "false") << endl;
        else if (isFunc()) cout << "<function>" << endl;
    }
};

// Thrown by ReturnStmt::execute to unwind the call stack back up to the
// CallExpr that invoked the current function, carrying the returned value.
struct ReturnSignal {
    Value value;
};

class Expr {
public:
    virtual ~Expr() {}
    virtual Value evaluate(map<string, Value>& variables) = 0;
};

class LiteralExpr : public Expr {
private:
    Token* token;
public:
    LiteralExpr(Token* t) : token(t) {}
    Value evaluate(map<string, Value>& variables) override {
        if (token->TYPE == TOKEN_INT) return Value{stoi(token->VALUE)};
        if (token->TYPE == TOKEN_FLOAT) return Value{stod(token->VALUE)};
        if (token->TYPE == TOKEN_BOOL) return Value{token->VALUE == "true"};
        if (token->TYPE == TOKEN_STRING) return Value{token->VALUE};
        if (token->TYPE == TOKEN_ID) {
            if (variables.count(token->VALUE)) return variables[token->VALUE];
            cout << "[RUNTIME ERROR] Undefined variable '" << token->VALUE << "'" << endl;
            exit(1);
        }
        return Value{0};
    }
};

class BinaryExpr : public Expr {
private:
    Expr* left;
    enum type op;
    Expr* right;
public:
    BinaryExpr(Expr* l, enum type o, Expr* r) : left(l), op(o), right(r) {}
    ~BinaryExpr() { delete left; delete right; }
    
    Value evaluate(map<string, Value>& variables) override {
        Value lVal = left->evaluate(variables);
        Value rVal = right->evaluate(variables);
        
        if (op == TOKEN_EQUALS_EQUALS) {
            if (lVal.isNumber() && rVal.isNumber()) return Value{lVal.asNumber() == rVal.asNumber()};
            if (lVal.isString() && rVal.isString()) return Value{lVal.asString() == rVal.asString()};
            if (lVal.isBool() && rVal.isBool()) return Value{lVal.asBool() == rVal.asBool()};
            return Value{false};
        }

        // Both sides numeric: if either side is a float, promote both to float.
        if (lVal.isNumber() && rVal.isNumber()) {
            if (lVal.isInt() && rVal.isInt()) {
                int lNum = lVal.asInt();
                int rNum = rVal.asInt();
                if (op == TOKEN_PLUS) return Value{lNum + rNum};
                if (op == TOKEN_MINUS) return Value{lNum - rNum};
                if (op == TOKEN_MULTIPLY) return Value{lNum * rNum};
                if (op == TOKEN_DIVIDE) {
                    if (rNum == 0) { cout << "[RUNTIME ERROR] Division by zero." << endl; exit(1); }
                    return Value{lNum / rNum};
                }
                if (op == TOKEN_LESS) return Value{lNum < rNum};
                if (op == TOKEN_GREATER) return Value{lNum > rNum};
                if (op == TOKEN_LESS_EQUALS) return Value{lNum <= rNum};
                if (op == TOKEN_GREATER_EQUALS) return Value{lNum >= rNum};
            } else {
                double lNum = lVal.asNumber();
                double rNum = rVal.asNumber();
                if (op == TOKEN_PLUS) return Value{lNum + rNum};
                if (op == TOKEN_MINUS) return Value{lNum - rNum};
                if (op == TOKEN_MULTIPLY) return Value{lNum * rNum};
                if (op == TOKEN_DIVIDE) {
                    if (rNum == 0.0) { cout << "[RUNTIME ERROR] Division by zero." << endl; exit(1); }
                    return Value{lNum / rNum};
                }
                if (op == TOKEN_LESS) return Value{lNum < rNum};
                if (op == TOKEN_GREATER) return Value{lNum > rNum};
                if (op == TOKEN_LESS_EQUALS) return Value{lNum <= rNum};
                if (op == TOKEN_GREATER_EQUALS) return Value{lNum >= rNum};
            }
        }

        if (lVal.isString() && rVal.isString() && op == TOKEN_PLUS) {
            return Value{lVal.asString() + rVal.asString()};
        }

        cout << "[RUNTIME ERROR] Invalid operation on types." << endl;
        exit(1);
    }
};

class AssignExpr : public Expr {
private:
    string name;
    Expr* value;
public:
    AssignExpr(string n, Expr* v) : name(n), value(v) {}
    ~AssignExpr() { delete value; }
    Value evaluate(map<string, Value>& variables) override {
        if (!variables.count(name)) {
            cout << "[RUNTIME ERROR] Undefined variable '" << name << "'" << endl;
            exit(1);
        }
        Value val = value->evaluate(variables);
        variables[name] = val;
        return val;
    }
};

class CallExpr : public Expr {
private:
    string callee;
    vector<Expr*> arguments;
public:
    CallExpr(string c, vector<Expr*> args) : callee(c), arguments(args) {}
    ~CallExpr() { for (Expr* arg : arguments) delete arg; }
    
    Value evaluate(map<string, Value>& variables) override;
};

class Stmt {
public:
    virtual ~Stmt() {}
    virtual void execute(map<string, Value>& variables) = 0;
};

Value CallExpr::evaluate(map<string, Value>& variables) {
    if (!variables.count(callee) || !variables[callee].isFunc()) {
        cout << "[RUNTIME ERROR] '" << callee << "' is not a function." << endl;
        exit(1);
    }
    Function* func = variables[callee].asFunc();
    if (arguments.size() != func->params.size()) {
        cout << "[RUNTIME ERROR] Argument count mismatch." << endl;
        exit(1);
    }
    map<string, Value> localVars = variables;
    for (size_t i = 0; i < arguments.size(); i++) {
        localVars[func->params[i]] = arguments[i]->evaluate(variables);
    }
    try {
        for (Stmt* stmt : func->body) {
            if (stmt) stmt->execute(localVars);
        }
    } catch (ReturnSignal& ret) {
        return ret.value;
    }
    // No `return` was hit -- fall through with a default value,
    // same as before this function had `return` support.
    return Value{0};
}

class ExpressionStmt : public Stmt {
private:
    Expr* expression;
public:
    ExpressionStmt(Expr* expr) : expression(expr) {}
    ~ExpressionStmt() { delete expression; }
    void execute(map<string, Value>& variables) override {
        expression->evaluate(variables);
    }
};

class BlockStmt : public Stmt {
private:
    vector<Stmt*> statements;
public:
    BlockStmt(vector<Stmt*> stmts) : statements(stmts) {}
    ~BlockStmt() { for (Stmt* s : statements) delete s; }
    void execute(map<string, Value>& variables) override {
        for (Stmt* stmt : statements) {
            if (stmt) stmt->execute(variables);
        }
    }
};

class LetStmt : public Stmt {
private:
    string varName;
    Expr* initializer;
public:
    LetStmt(string name, Expr* init) : varName(name), initializer(init) {}
    ~LetStmt() { delete initializer; }
    void execute(map<string, Value>& variables) override {
        variables[varName] = initializer->evaluate(variables);
    }
};

class QuackStmt : public Stmt {
private:
    Expr* expression;
public:
    QuackStmt(Expr* expr) : expression(expr) {}
    ~QuackStmt() { delete expression; }
    void execute(map<string, Value>& variables) override {
        expression->evaluate(variables).print();
    }
};

// `return;` has expression == nullptr and returns Value{0}.
// `return <expr>;` evaluates the expression and returns that.
// Either way, execute() throws to unwind back up to the CallExpr that
// invoked this function -- that's how the value escapes nested
// if/for blocks without every Stmt needing to know about returns.
class ReturnStmt : public Stmt {
private:
    Expr* expression;
public:
    ReturnStmt(Expr* expr) : expression(expr) {}
    ~ReturnStmt() { if (expression) delete expression; }
    void execute(map<string, Value>& variables) override {
        Value val = expression ? expression->evaluate(variables) : Value{0};
        throw ReturnSignal{val};
    }
};

class IfStmt : public Stmt {
private:
    Expr* condition;
    Stmt* thenBranch;
    Stmt* elseBranch;
public:
    IfStmt(Expr* cond, Stmt* thenB, Stmt* elseB) : condition(cond), thenBranch(thenB), elseBranch(elseB) {}
    ~IfStmt() { delete condition; delete thenBranch; if (elseBranch) delete elseBranch; }
    void execute(map<string, Value>& variables) override {
        Value condVal = condition->evaluate(variables);
        if ((condVal.isBool() && condVal.asBool()) || (condVal.isNumber() && condVal.asNumber() != 0)) {
            thenBranch->execute(variables);
        } else if (elseBranch) {
            elseBranch->execute(variables);
        }
    }
};

class ForStmt : public Stmt {
private:
    Stmt* initializer;
    Expr* condition;
    Stmt* increment;
    Stmt* body;
public:
    ForStmt(Stmt* init, Expr* cond, Stmt* incr, Stmt* b) : initializer(init), condition(cond), increment(incr), body(b) {}
    ~ForStmt() { delete initializer; delete condition; delete increment; delete body; }
    void execute(map<string, Value>& variables) override {
        if (initializer) initializer->execute(variables);
        while (true) {
            Value condVal = condition->evaluate(variables);
            if (condVal.isBool() && !condVal.asBool()) break;
            if (condVal.isInt() && condVal.asInt() == 0) break;
            body->execute(variables);
            if (increment) increment->execute(variables);
        }
    }
};

class FunStmt : public Stmt {
private:
    string name;
    vector<string> params;
    vector<Stmt*> body;
public:
    FunStmt(string n, vector<string> p, vector<Stmt*> b) : name(n), params(p), body(b) {}
    void execute(map<string, Value>& variables) override {
        Function* func = new Function{params, body};
        variables[name] = Value{func};
    }
};

class Parser {
private:
    vector<Token*> tokens;
    int current = 0;

    Token* peek() { if (current < tokens.size()) return tokens[current]; return nullptr; }
    Token* advance() { if (current < tokens.size()) return tokens[current++]; return nullptr; }
    bool match(string val) { if (peek() && peek()->VALUE == val) { advance(); return true; } return false; }
    
    Token* consume(enum type expectedType, string errorMessage) {
        Token* token = peek();
        if (token && token->TYPE == expectedType) return advance();
        cout << "[SYNTAX ERROR] " << errorMessage << endl;
        exit(1);
    }

    Token* consumeValue(string val, string errorMessage) {
        Token* token = peek();
        if (token && token->VALUE == val) return advance();
        cout << "[SYNTAX ERROR] " << errorMessage << " Got: " << (token ? token->VALUE : "NULL") << endl;
        exit(1);
    }

    Expr* parsePrimary() {
        Token* token = peek();
        if (token && token->TYPE == TOKEN_ID && current + 1 < tokens.size() && tokens[current + 1]->VALUE == "(") {
            string name = token->VALUE;
            advance();
            consumeValue("(", "Expected '('");
            vector<Expr*> args;
            if (peek() && peek()->VALUE != ")") {
                do { args.push_back(parseExpression()); } while (match(","));
            }
            consumeValue(")", "Expected ')'");
            return new CallExpr(name, args);
        }
        if (token && (token->TYPE == TOKEN_INT || token->TYPE == TOKEN_FLOAT || token->TYPE == TOKEN_BOOL ||
                      token->TYPE == TOKEN_STRING || token->TYPE == TOKEN_ID)) {
            advance();
            return new LiteralExpr(token);
        }
        cout << "[SYNTAX ERROR] Expected expression." << endl;
        exit(1);
    }

    Expr* parseTerm() {
        Expr* expr = parsePrimary();
        while (peek() && (peek()->TYPE == TOKEN_MULTIPLY || peek()->TYPE == TOKEN_DIVIDE)) {
            Token* op = advance();
            Expr* right = parsePrimary();
            expr = new BinaryExpr(expr, op->TYPE, right);
        }
        return expr;
    }

    Expr* parseAssignment() {
        Token* token = peek();
        if (token && token->TYPE == TOKEN_ID && current + 1 < tokens.size()) {
            Token* next = tokens[current + 1];
            if (next->VALUE == "=" || next->VALUE == "+=" || next->VALUE == "-=" || next->VALUE == "*=" || next->VALUE == "/=") {
                string name = token->VALUE;
                advance(); 
                Token* opToken = advance(); 
                Expr* rightSide = parseExpression();
                
                if (opToken->VALUE == "=") {
                    return new AssignExpr(name, rightSide);
                }
                
                enum type mathOp;
                if (opToken->VALUE == "+=") mathOp = TOKEN_PLUS;
                else if (opToken->VALUE == "-=") mathOp = TOKEN_MINUS;
                else if (opToken->VALUE == "*=") mathOp = TOKEN_MULTIPLY;
                else if (opToken->VALUE == "/=") mathOp = TOKEN_DIVIDE;
                
                Expr* varRef = new LiteralExpr(token);
                Expr* desugared = new BinaryExpr(varRef, mathOp, rightSide);
                return new AssignExpr(name, desugared);
            }
        }
        return parseTerm();
    }

    Expr* parseExpression() {
        Expr* expr = parseAssignment();
        while (peek() && (peek()->TYPE == TOKEN_PLUS || peek()->TYPE == TOKEN_MINUS || peek()->TYPE == TOKEN_EQUALS_EQUALS ||
                          peek()->TYPE == TOKEN_LESS || peek()->TYPE == TOKEN_GREATER ||
                          peek()->TYPE == TOKEN_LESS_EQUALS || peek()->TYPE == TOKEN_GREATER_EQUALS)) {
            Token* op = advance();
            Expr* right = parseTerm();
            expr = new BinaryExpr(expr, op->TYPE, right);
        }
        return expr;
    }

    vector<Stmt*> parseBlock() {
        consumeValue("{", "Expected '{' before block.");
        vector<Stmt*> stmts;
        while (peek() && peek()->VALUE != "}") {
            stmts.push_back(parseStatement());
        }
        consumeValue("}", "Expected '}' after block.");
        return stmts;
    }

public:
    Parser(vector<Token*> t) : tokens(t) {}

    vector<Stmt*> parse() {
        vector<Stmt*> programAST;
        while (peek() != nullptr) {
            Stmt* stmt = parseStatement();
            if (stmt != nullptr) programAST.push_back(stmt);
        }
        return programAST;
    }

    Stmt* parseStatement() {
        Token* token = peek();
        if (!token) return nullptr;

        if (token->VALUE == "let") return parseLet();
        if (token->VALUE == "quack") return parseQuack();
        if (token->VALUE == "if") return parseIf();
        if (token->VALUE == "for") return parseFor();
        if (token->VALUE == "fun") return parseFun();
        if (token->VALUE == "return") return parseReturn();
        
        Expr* expr = parseExpression();
        consume(TOKEN_SEMICOLON, "Expected ';'");
        return new ExpressionStmt(expr);
    }

    Stmt* parseLet() {
        consumeValue("let", "Expected 'let'");
        Token* varToken = consume(TOKEN_ID, "Expected variable name.");
        consume(TOKEN_EQUALS, "Expected '='.");
        Expr* initializer = parseExpression();
        consume(TOKEN_SEMICOLON, "Expected ';'.");
        return new LetStmt(varToken->VALUE, initializer);
    }

    Stmt* parseLetFor() {
        consumeValue("let", "Expected 'let'");
        Token* varToken = consume(TOKEN_ID, "Expected variable name.");
        consume(TOKEN_EQUALS, "Expected '='.");
        Expr* initializer = parseExpression();
        return new LetStmt(varToken->VALUE, initializer);
    }

    Stmt* parseQuack() {
        consumeValue("quack", "Expected 'quack'");
        consumeValue("(", "Expected '('.");
        Expr* expr = parseExpression();
        consumeValue(")", "Expected ')'.");
        consume(TOKEN_SEMICOLON, "Expected ';'.");
        return new QuackStmt(expr);
    }

    Stmt* parseIf() {
        consumeValue("if", "Expected 'if'");
        consumeValue("(", "Expected '('.");
        Expr* condition = parseExpression();
        consumeValue(")", "Expected ')'.");
        Stmt* thenBranch = new BlockStmt(parseBlock());
        Stmt* elseBranch = nullptr;
        if (match("else")) {
            elseBranch = new BlockStmt(parseBlock());
        }
        return new IfStmt(condition, thenBranch, elseBranch);
    }

    Stmt* parseFor() {
        consumeValue("for", "Expected 'for'");
        consumeValue("(", "Expected '('.");
        Stmt* init = parseLetFor();
        consume(TOKEN_SEMICOLON, "Expected ';'.");
        Expr* cond = parseExpression();
        consume(TOKEN_SEMICOLON, "Expected ';'.");
        
        Expr* incrExpr = parseExpression();
        Stmt* incr = new ExpressionStmt(incrExpr);
        
        consumeValue(")", "Expected ')'.");
        Stmt* body = new BlockStmt(parseBlock());
        return new ForStmt(init, cond, incr, body);
    }

    Stmt* parseFun() {
        consumeValue("fun", "Expected 'fun'");
        Token* nameToken = consume(TOKEN_ID, "Expected function name.");
        consumeValue("(", "Expected '('.");
        vector<string> params;
        if (peek() && peek()->VALUE != ")") {
            do {
                Token* param = consume(TOKEN_ID, "Expected parameter name.");
                params.push_back(param->VALUE);
            } while (match(","));
        }
        consumeValue(")", "Expected ')'");
        vector<Stmt*> body = parseBlock();
        return new FunStmt(nameToken->VALUE, params, body);
    }

    Stmt* parseReturn() {
        consumeValue("return", "Expected 'return'");
        Expr* expr = nullptr;
        // Allow a bare "return;" with no value.
        if (peek() && peek()->VALUE != ";") {
            expr = parseExpression();
        }
        consume(TOKEN_SEMICOLON, "Expected ';' after return statement.");
        return new ReturnStmt(expr);
    }
};

class Interpreter {
private:
    map<string, Value> variables;
public:
    void interpret(const vector<Stmt*>& programAST) {
        try {
            for (Stmt* stmt : programAST) {
                if (stmt) stmt->execute(variables);
            }
        } catch (ReturnSignal&) {
            cout << "[RUNTIME ERROR] 'return' used outside of a function." << endl;
            exit(1);
        }
    }
};

#endif