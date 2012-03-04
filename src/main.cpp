//
//  main.cpp
//  LightBASIC
//
//  Created by Albert Varaksin on 25/02/2012.
//  Copyright (c) 2012 LightBASIC development team . All rights reserved.
//

#include <iostream>
#include "Context.h"
#include "Parser.h"
#include "Ast.h"
#include "PrinterVisitor.h"
#include "SemanticAnalyser.h"
#include "CodeGen.h"
#include "SourceFile.h"

using namespace lbc;


int main(int argc, const char * argv[])
{
	try {
		// create the context
		auto ctx = make_shared<Context>();
		ctx->add(FS::current_path(), Context::GlobalPath);
		ctx->add("examples/HelloWorld.bas", Context::Source);
		// create the parser
		auto parser = make_shared<Parser>(ctx);
        // process the files
        for (auto & file : ctx->get(Context::Source)) {
            auto ast = parser->parse(make_shared<SourceFile>(file));
            if (ast) {
                // print the ast
                auto printer = new PrinterVisitor();
                ast->accept(printer);

                // analyse
                auto analyser = new SemanticAnalyser();
                ast->accept(analyser);
                
                // generate llvm code
                auto codegen = new CodeGen();
                ast->accept(codegen);
                
                // cleanup
                delete ast;
            } else {
                std::cout << "NO AST\n";
            }
        }
		
	} catch (Exception e) {
		cout << "Error: " << e.what() << endl;
		return EXIT_FAILURE;
	}
    return EXIT_SUCCESS;
}

