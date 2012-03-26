//
//  Ast.h
//  LightBASIC
//
//  Created by Albert Varaksin on 25/02/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//
#pragma once
#include "Ast.def.h"

namespace lbc {
    
    // forward declare Ast classes
    AST_DECLARE_CLASSES();
    
    // forward declare the visitor
    class AstVisitor;
    class Token;
    class Symbol;
    class SymbolTable;
    class Type;
    
    // ast node enum
    enum class Ast : int {
        #define ENUM_AST_CLASSES(ID) ID,
        AST_ALL_NODES(ENUM_AST_CLASSES)
    };
    
    //
    // declare Ast node. define visitor method, mempory pooling and virtual destructor
    #define DECLARE_AST(C)                          \
        virtual void accept(AstVisitor * visitor);  \
        void * operator new(size_t);                \
        void operator delete(void *);               \
        virtual bool is(Ast ast) const { return ast == Ast::C; } \
        virtual ~Ast##C();
    
    
    //--------------------------------------------------------------------------
    // Basic Nodes
    //--------------------------------------------------------------------------
    
    /**
     * This is the base node for the AST tree
     */
    struct AstRoot : NonCopyable
    {
        // visitor pattern
        virtual void accept(AstVisitor * visitor) = 0;
        
        // virtual destructor
        virtual ~AstRoot() = default;
        
        // is of type?
        virtual bool is(Ast ast) const = 0;
    };
    
    
    /**
     * base node for statements
     */
    struct AstStatement : AstRoot
    {
    };
    
    
    /**
     * program root
     */
    struct AstProgram : AstRoot {
        // create
        AstProgram(const string & name);
        // program name
        string name;
        // list of declaration lists
        vector<unique_ptr<AstDeclaration>> decls;
        // symbol table
        shared_ptr<SymbolTable> symbolTable;
        // content node
        DECLARE_AST(Program);
    };


    //--------------------------------------------------------------------------
    // Declarations
    //--------------------------------------------------------------------------
    
    
    /**
     * Declaration
     */
    struct AstDeclaration : AstStatement
    {
        // create
        AstDeclaration(AstAttributeList * attribs = nullptr);
        // attribs
        unique_ptr<AstAttributeList> attribs;
        // declaration symbol. Does not own the symbol!
        Symbol * symbol;
        // content node
        DECLARE_AST(Declaration);
    };
    
    
    /**
     * attribute list
     */
    struct AstAttributeList : AstRoot
    {
        // create
        AstAttributeList();
        // hold list of attributes
        vector<unique_ptr<AstAttribute>> attribs;
        // content node
        DECLARE_AST(AttributeList);
    };
    
    
    /**
     * attribute node
     */
    struct AstAttribute : AstRoot
    {
        // create
        AstAttribute(AstIdentExpr * id = nullptr, AstAttribParamList * params = nullptr);
        // attribute id
        unique_ptr<AstIdentExpr> id;
        // attribute params
        unique_ptr<AstAttribParamList> params;
        // content node
        DECLARE_AST(Attribute);
    };
    
    
    /**
     * list of attribute parameters
     */
    struct AstAttribParamList : AstRoot
    {
        // create
        AstAttribParamList();
        // attribute params
        vector<unique_ptr<AstLiteralExpr>> params;
        // content node
        DECLARE_AST(AttribParamList);
    };
    
    
    /**
     * variable declaration
     */
    struct AstVarDecl : AstDeclaration
    {
        // create
        AstVarDecl(AstIdentExpr * id = nullptr, AstTypeExpr * typeExpr = nullptr, AstExpression * expr = nullptr);
        // variable id
        unique_ptr<AstIdentExpr> id;
        // variable type
        unique_ptr<AstTypeExpr> typeExpr;
        // initalizer expression
        unique_ptr<AstExpression> expr;
        // content node
        DECLARE_AST(VarDecl);
    };
    
    
    /**
     * function declaration
     */
    struct AstFunctionDecl : AstDeclaration
    {
        // create
        AstFunctionDecl(AstFuncSignature * signature = nullptr);
        // function signature
        unique_ptr<AstFuncSignature> signature;
        // content node
        DECLARE_AST(FunctionDecl);
    };
    
    
    /**
     * function signature part
     */
    struct AstFuncSignature : AstRoot
    {
        // create
        AstFuncSignature(AstIdentExpr * id = nullptr, AstFuncParamList * args = nullptr, AstTypeExpr * typeExpr = nullptr, bool vararg = false);
        // function id
        unique_ptr<AstIdentExpr> id;
        // function parameters
        unique_ptr<AstFuncParamList> params;
        // function type
        unique_ptr<AstTypeExpr> typeExpr;
        // flags
        int vararg:1;
        // content node
        DECLARE_AST(FuncSignature);
    };
    
    
    /**
     * function parameter list
     */
    struct AstFuncParamList : AstRoot
    {
        // create
        AstFuncParamList();
        // list of function parameters
        vector<unique_ptr<AstFuncParam>> params;
        // content node
        DECLARE_AST(FuncParamList);
    };
    
    
    /**
     * Function parameter
     */
    struct AstFuncParam : AstDeclaration
    {
        // create
        AstFuncParam(AstIdentExpr * id = nullptr, AstTypeExpr * typeExpr = nullptr);
        // variable id
        unique_ptr<AstIdentExpr> id;
        // variable type
        unique_ptr<AstTypeExpr> typeExpr;
        // content node
        DECLARE_AST(FuncParam);
    };
    
    
    /**
     * function implementation
     */
    struct AstFunctionStmt : AstDeclaration
    {
        // create
        AstFunctionStmt(AstFuncSignature * signature = nullptr, AstStmtList * stmts = nullptr);
        // function signature
        unique_ptr<AstFuncSignature> signature;
        // function body
        unique_ptr<AstStmtList> stmts;
        // content node
        DECLARE_AST(FunctionStmt);
    };
    
    
    //--------------------------------------------------------------------------
    // Statements
    //--------------------------------------------------------------------------
    
    
    /**
     * Root node representing whole program
     */
    struct AstStmtList : AstStatement
    {
        // create
        AstStmtList();
        // list of statements
        vector<unique_ptr<AstStatement>> stmts;
        // associated symbol table. Does not own!
        SymbolTable * symbolTable;
        // content node
        DECLARE_AST(StmtList);
    };
    
    
    /**
     * assigment statement
     */
    struct AstAssignStmt : AstStatement
    {
        // create 
        AstAssignStmt(AstExpression * left = nullptr, AstExpression * right = nullptr);
        // assignee id
        unique_ptr<AstExpression> left;
        // the expression
        unique_ptr<AstExpression> right;
        // content node
        DECLARE_AST(AssignStmt);
    };
    
    
    /**
     * RETURN statement
     */
    struct AstReturnStmt : AstStatement
    {
        // create
        AstReturnStmt(AstExpression * expr = nullptr);
        // the expression
        unique_ptr<AstExpression> expr;
        // content node
        DECLARE_AST(ReturnStmt);
    };
    
    
    /**
     * call statement. wrap the call expression
     */
    struct AstCallStmt : AstStatement
    {
        // create
        AstCallStmt(AstCallExpr * expr = nullptr);
        // the call expression
        unique_ptr<AstCallExpr> expr;
        // content node
        DECLARE_AST(CallStmt);
    };
    
    
    /**
     * If statement
     */
    struct AstIfStmt : AstStatement
    {
        // create
        AstIfStmt(AstExpression * expr = nullptr, AstStatement * trueBlock = nullptr, AstStatement * falseBlock = nullptr);
        // expression
        unique_ptr<AstExpression> expr;
        // true branch
        unique_ptr<AstStatement> trueBlock;
        // else
        unique_ptr<AstStatement> falseBlock;
        // content node
        DECLARE_AST(IfStmt);
    };
    
    
    
    //--------------------------------------------------------------------------
    // Expressions
    //--------------------------------------------------------------------------
    
    /**
     * base node for expressions
     */
    struct AstExpression : AstRoot
    {
        // create
        AstExpression();
        // is this a constant expression ?
        virtual bool isConstant() const { return false; }
        // expression type. Does not own!
        Type * type;
        // content node
        DECLARE_AST(Expression);
    };
    
    
    /**
     * cast expression. Target type is help in the Expression
     */
    struct AstCastExpr : AstExpression
    {
        // create
        AstCastExpr(AstExpression * expr = nullptr, AstTypeExpr * typeExpr = nullptr);
        // sub expression
        unique_ptr<AstExpression> expr;
        // type expession
        unique_ptr<AstTypeExpr> typeExpr;
        // content node
        DECLARE_AST(CastExpr);
    };
    
    
    /**
     * Reference expression. Take address of &i
     */
    struct AstAddressOfExpr : AstExpression
    {
        // create
        AstAddressOfExpr(AstIdentExpr * id = nullptr);
        // child expression
        unique_ptr<AstIdentExpr> id;
        // content node
        DECLARE_AST(AddressOfExpr);
    };
    
    
    /**
     * Dereference expression. value of a pointer *i
     */
    struct AstDereferenceExpr : AstExpression
    {
        // create
        AstDereferenceExpr(AstExpression * expr = nullptr);
        // child expression
        unique_ptr<AstExpression> expr;
        // content node
        DECLARE_AST(DereferenceExpr);
    };
    
    
    /**
     * identifier node
     */
    struct AstIdentExpr : AstExpression
    {
        // create
        AstIdentExpr(Token * token = nullptr);
        // the id token
        unique_ptr<Token> token;
        // content node
        DECLARE_AST(IdentExpr);
    };
    
    
    /**
     * base for literal expressions (string, number)
     */
    struct AstLiteralExpr : AstExpression
    {
        // create
        AstLiteralExpr(Token * token = nullptr);
        // is const expression
        virtual bool isConstant() const { return true; }
        // the value token
        unique_ptr<Token> token;
        // content node
        DECLARE_AST(LiteralExpr);
    };
    
    
    /**
     * Binary expression
     */
    struct AstBinaryExpr : AstExpression
    {
        // create
        AstBinaryExpr(Token * op = nullptr, AstExpression * lhs = nullptr, AstExpression * rhs = nullptr);
        // token.
        unique_ptr<Token> token;
        // lhs, rhs
        unique_ptr<AstExpression> lhs, rhs;
        // content node
        DECLARE_AST(BinaryExpr);
    };
    
    
    /**
     * call expression
     */
    struct AstCallExpr : AstExpression
    {
        // create
        AstCallExpr(AstIdentExpr * id = nullptr, AstFuncArgList * args = nullptr);
        // callee id
        unique_ptr<AstIdentExpr> id;
        // parameters
        unique_ptr<AstFuncArgList> args;
        // content node
        DECLARE_AST(CallExpr);
    };
    
    
    /**
     * function arguments
     */
    struct AstFuncArgList : AstRoot
    {
        // create
        AstFuncArgList();
        // list of arguments
        vector<unique_ptr<AstExpression>> args;
        // content node
        DECLARE_AST(FuncArgList);
    };
    
    
    //--------------------------------------------------------------------------
    // Type
    //--------------------------------------------------------------------------
    
    /**
     * type declarator
     */
    struct AstTypeExpr : AstRoot
    {
        // create
        AstTypeExpr(Token * token = nullptr, int level = 0);
        // type id
        unique_ptr<Token> token;
        // dereference level
        int level;
        // content node
        DECLARE_AST(TypeExpr);
    };
    
}
