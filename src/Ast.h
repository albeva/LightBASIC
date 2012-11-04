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
    AST_DECLARE_CLASSES()
    
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
        _LAST
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
    class AstRoot : NonCopyable
    {
    public:
        
        // virtual destructor
        virtual ~AstRoot();
        
        // visitor pattern
        virtual void accept(AstVisitor * visitor) = 0;
        
        // is of type?
        virtual bool is(Ast ast) const = 0;
        
        // dump the ast node as string
        void dump();
    };
    
    
    /**
     * base node for statements
     */
    class AstStatement : public AstRoot
    {
    public:
        
        // create
        AstStatement();
        virtual ~AstStatement();
    };
    
    
    /**
     * program root
     */
    class AstProgram : public AstRoot
    {
    public:
        
        // create
        AstProgram(const std::string & name);
        // program name
        std::string name;
        // list of declaration lists
        std::vector<std::unique_ptr<AstDeclaration>> decls;
        // symbol table
        std::shared_ptr<SymbolTable> symbolTable;
        // content node
        DECLARE_AST(Program)
    };


    //--------------------------------------------------------------------------
    // Declarations
    //--------------------------------------------------------------------------
    
    
    /**
     * Declaration
     */
    class AstDeclaration : public AstStatement
    {
    public:
        
        // create
        AstDeclaration(std::unique_ptr<AstAttributeList> attribs = nullptr);
        // attribs
        std::unique_ptr<AstAttributeList> attribs;
        // declaration symbol. Does not own the symbol!
        Symbol * symbol;
        // content node
        DECLARE_AST(Declaration)
    };
    
    
    /**
     * attribute list
     */
    class AstAttributeList : public AstRoot
    {
    public:
        
        // create
        AstAttributeList();
        // hold list of attributes
        std::vector<std::unique_ptr<AstAttribute>> attribs;
        // content node
        DECLARE_AST(AttributeList)
    };
    
    
    /**
     * attribute node
     */
    class AstAttribute : public AstRoot
    {
    public:
        
        // create
        AstAttribute(std::unique_ptr<AstIdentExpr> id = nullptr,
                     std::unique_ptr<AstAttribParamList> params = nullptr);
        // attribute id
        std::unique_ptr<AstIdentExpr> id;
        // attribute params
        std::unique_ptr<AstAttribParamList> params;
        // content node
        DECLARE_AST(Attribute)
    };
    
    
    /**
     * list of attribute parameters
     */
    class AstAttribParamList : public AstRoot
    {
    public:
        
        // create
        AstAttribParamList();
        // attribute params
        std::vector<std::unique_ptr<AstLiteralExpr>> params;
        // content node
        DECLARE_AST(AttribParamList)
    };
    
    
    /**
     * variable declaration
     */
    class AstVarDecl : public AstDeclaration
    {
    public:
        
        // create
        AstVarDecl(std::unique_ptr<AstIdentExpr>  id = nullptr,
                   std::unique_ptr<AstTypeExpr>   typeExpr = nullptr,
                   std::unique_ptr<AstExpression> expr = nullptr);
        // variable id
        std::unique_ptr<AstIdentExpr> id;
        // variable type
        std::unique_ptr<AstTypeExpr> typeExpr;
        // initalizer expression
        std::unique_ptr<AstExpression> expr;
        // content node
        DECLARE_AST(VarDecl)
    };
    
    
    /**
     * function declaration
     */
    class AstFunctionDecl : public AstDeclaration
    {
    public:
        
        // create
        AstFunctionDecl(std::unique_ptr<AstFuncSignature> signature = nullptr);
        // function signature
        std::unique_ptr<AstFuncSignature> signature;
        // content node
        DECLARE_AST(FunctionDecl)
    };
    
    
    /**
     * function signature part
     */
    class AstFuncSignature : public AstRoot
    {
    public:
        
        // create
        AstFuncSignature(std::unique_ptr<AstIdentExpr> id = nullptr,
                         std::unique_ptr<AstFuncParamList> args = nullptr,
                         std::unique_ptr<AstTypeExpr> typeExpr = nullptr,
                         bool vararg = false);
        // function id
        std::unique_ptr<AstIdentExpr> id;
        // function parameters
        std::unique_ptr<AstFuncParamList> params;
        // function type
        std::unique_ptr<AstTypeExpr> typeExpr;
        // flags
        int vararg:1;
        // content node
        DECLARE_AST(FuncSignature)
    };
    
    
    /**
     * function parameter list
     */
    class AstFuncParamList : public AstRoot
    {
    public:
        
        // create
        AstFuncParamList();
        // list of function parameters
        std::vector<std::unique_ptr<AstFuncParam>> params;
        // content node
        DECLARE_AST(FuncParamList)
    };
    
    
    /**
     * Function parameter
     */
    class AstFuncParam : public AstDeclaration
    {
    public:
        
        // create
        AstFuncParam(std::unique_ptr<AstIdentExpr> id = nullptr,
                     std::unique_ptr<AstTypeExpr> typeExpr = nullptr);
        // variable id
        std::unique_ptr<AstIdentExpr> id;
        // variable type
        std::unique_ptr<AstTypeExpr> typeExpr;
        // content node
        DECLARE_AST(FuncParam)
    };
    
    
    /**
     * function implementation
     */
    class AstFunctionStmt : public AstDeclaration
    {
    public:
        
        // create
        AstFunctionStmt(std::unique_ptr<AstFuncSignature> signature = nullptr,
                        std::unique_ptr<AstStmtList> stmts = nullptr);
        // function signature
        std::unique_ptr<AstFuncSignature> signature;
        // function body
        std::unique_ptr<AstStmtList> stmts;
        // content node
        DECLARE_AST(FunctionStmt)
    };
    
    
    //--------------------------------------------------------------------------
    // Statements
    //--------------------------------------------------------------------------
    
    
    /**
     * Root node representing whole program
     */
    class AstStmtList : public AstStatement
    {
    public:
        
        // create
        AstStmtList();
        // list of statements
        std::vector<std::unique_ptr<AstStatement>> stmts;
        // associated symbol table. Does not own!
        SymbolTable * symbolTable;
        // content node
        DECLARE_AST(StmtList)
    };
    
    
    /**
     * assigment statement
     */
    class AstAssignStmt : public AstStatement
    {
    public:
        
        // create 
        AstAssignStmt(std::unique_ptr<AstExpression> left = nullptr,
                      std::unique_ptr<AstExpression> right = nullptr);
        // assignee id
        std::unique_ptr<AstExpression> left;
        // the expression
        std::unique_ptr<AstExpression> right;
        // content node
        DECLARE_AST(AssignStmt)
    };
    
    
    /**
     * RETURN statement
     */
    class AstReturnStmt : public AstStatement
    {
    public:
        
        // create
        AstReturnStmt(std::unique_ptr<AstExpression> expr = nullptr);
        // the expression
        std::unique_ptr<AstExpression> expr;
        // content node
        DECLARE_AST(ReturnStmt)
    };
    
    
    /**
     * call statement. wrap the call expression
     */
    class AstCallStmt : public AstStatement
    {
    public:
        
        // create
        AstCallStmt(std::unique_ptr<AstCallExpr> expr = nullptr);
        // the call expression
        std::unique_ptr<AstCallExpr> expr;
        // content node
        DECLARE_AST(CallStmt)
    };
    
    
    /**
     * IF statement
     */
    class AstIfStmt : public AstStatement
    {
    public:
        
        // create
        AstIfStmt(std::unique_ptr<AstExpression> expr = nullptr,
                  std::unique_ptr<AstStatement> trueBlock = nullptr,
                  std::unique_ptr<AstStatement> falseBlock = nullptr);
        // expression
        std::unique_ptr<AstExpression> expr;
        // true branch
        std::unique_ptr<AstStatement> trueBlock;
        // else
        std::unique_ptr<AstStatement> falseBlock;
        // content node
        DECLARE_AST(IfStmt)
    };
    
    
    /**
     * FOR statement
     */
    class AstForStmt : public AstStatement
    {
    public:
        
        // create
        AstForStmt(std::unique_ptr<AstStatement> stmt = nullptr,
                   std::unique_ptr<AstExpression> end = nullptr,
                   std::unique_ptr<AstExpression> step = nullptr,
                   std::unique_ptr<AstStmtList> block = nullptr);
        // AstVarDeclStmt | AstAssignStmt
        std::unique_ptr<AstStatement> stmt;
        // id begin, end and step expressions
        std::unique_ptr<AstExpression> end, step;
        //the body
        std::unique_ptr<AstStmtList> block;
        // content node
        DECLARE_AST(ForStmt)
    };
    
    
    //--------------------------------------------------------------------------
    // Expressions
    //--------------------------------------------------------------------------
    
    /**
     * base node for expressions
     */
    class AstExpression : public AstRoot
    {
    public:
        
        // create
        AstExpression();
        // is this a constant expression ?
        virtual bool isConstant() const { return false; }
        // expression type. Does not own!
        Type * type;
        // content node
        DECLARE_AST(Expression)
    };
    
    
    /**
     * cast expression. Target type is help in the Expression
     */
    class AstCastExpr : public AstExpression
    {
    public:
        
        // create
        AstCastExpr(std::unique_ptr<AstExpression> expr = nullptr,
                    std::unique_ptr<AstTypeExpr> typeExpr = nullptr);
        // sub expression
        std::unique_ptr<AstExpression> expr;
        // type expession
        std::unique_ptr<AstTypeExpr> typeExpr;
        // content node
        DECLARE_AST(CastExpr)
    };
    
    
    /**
     * Reference expression. Take address of &i
     */
    class AstAddressOfExpr : public AstExpression
    {
    public:
        
        // create
        AstAddressOfExpr(std::unique_ptr<AstIdentExpr> id = nullptr);
        // child expression
        std::unique_ptr<AstIdentExpr> id;
        // content node
        DECLARE_AST(AddressOfExpr)
    };
    
    
    /**
     * Dereference expression. value of a pointer *i
     */
    class AstDereferenceExpr : public AstExpression
    {
    public:
        
        // create
        AstDereferenceExpr(std::unique_ptr<AstExpression> expr = nullptr);
        // child expression
        std::unique_ptr<AstExpression> expr;
        // content node
        DECLARE_AST(DereferenceExpr)
    };
    
    
    /**
     * identifier node
     */
    class AstIdentExpr : public AstExpression
    {
    public:
        
        // create
        AstIdentExpr(Token * token = nullptr);
        // the id token
        std::unique_ptr<Token> token;
        // content node
        DECLARE_AST(IdentExpr)
    };
    
    
    /**
     * base for literal expressions (std::string, number)
     */
    class AstLiteralExpr : public AstExpression
    {
    public:
        
        // create
        AstLiteralExpr(Token * token = nullptr);
        // is const expression
        virtual bool isConstant() const { return true; }
        // the value token
        std::unique_ptr<Token> token;
        // content node
        DECLARE_AST(LiteralExpr)
    };
    
    
    /**
     * Binary expression
     */
    class AstBinaryExpr : public AstExpression
    {
    public:
        
        // create
        AstBinaryExpr(Token * op = nullptr,
                      std::unique_ptr<AstExpression> lhs = nullptr,
                      std::unique_ptr<AstExpression> rhs = nullptr);
        // token.
        std::unique_ptr<Token> token;
        // lhs, rhs
        std::unique_ptr<AstExpression> lhs, rhs;
        // content node
        DECLARE_AST(BinaryExpr)
    };
    
    
    /**
     * call expression
     */
    class AstCallExpr : public AstExpression
    {
    public:
        
        // create
        AstCallExpr(std::unique_ptr<AstIdentExpr> id = nullptr,
                    std::unique_ptr<AstFuncArgList> args = nullptr);
        // callee id
        std::unique_ptr<AstIdentExpr> id;
        // parameters
        std::unique_ptr<AstFuncArgList> args;
        // content node
        DECLARE_AST(CallExpr)
    };
    
    
    /**
     * function arguments
     */
    class AstFuncArgList : public AstRoot
    {
    public:
        
        // create
        AstFuncArgList();
        // list of arguments
        std::vector<std::unique_ptr<AstExpression>> args;
        // content node
        DECLARE_AST(FuncArgList)
    };
    
    
    //--------------------------------------------------------------------------
    // Type
    //--------------------------------------------------------------------------
    
    /**
     * type declarator
     */
    class AstTypeExpr : public AstRoot
    {
    public:
        
        // create
        AstTypeExpr(Token * token = nullptr, int level = 0);
        // type id
        std::unique_ptr<Token> token;
        // dereference level
        int level;
        // content node
        DECLARE_AST(TypeExpr)
    };
    
}
