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
#include "IrBuilder.h"
#include "SourceFile.h"
#include "Emitter.h"

using namespace lbc;


int main(int argc, const char * argv[])
{
    const char * input = "examples/HelloWorld.bas";
    if (argc == 2) {
        input = argv[1];
    }
    try {
        // create the context
        auto ctx = make_shared<Context>();
        ctx->add(FS::current_path(), Context::GlobalPath);
        ctx->add(input, Context::Source);
        // create the parser
        auto parser = make_shared<Parser>(ctx);
		// create output emitter
		auto emitter = make_shared<Emitter>(ctx);
		// IR builder
		auto builder = make_shared<IrBuilder>();
        // process the files
        for (auto & file : ctx->get(Context::Source)) {
            auto ast = parser->parse(make_shared<SourceFile>(file));
            if (ast) {

                // analyse
                auto analyser = new SemanticAnalyser();
                ast->accept(analyser);
                delete analyser;
                
                // generate llvm code
                builder->reset();
                ast->accept(builder.get());
				if (auto module = builder->getModule()) {
					emitter->add(module);
				}
				
                // cleanup
                delete ast;
            } else {
                std::cout << "NO AST\n";
            }
        }
		// generate the output
		emitter->generate();
    } catch (Exception e) {
        cout << "Error: " << e.what() << endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

