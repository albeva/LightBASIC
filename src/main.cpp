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
    try {
        // create the context
        auto ctx = make_shared<Context>();
        ctx->add(FS::current_path(), Context::GlobalPath);
        if (argc > 1) {
            for (int i = 1; i < argc; i++) {
                if (string(argv[i]) == "-v") {
                    ctx->verbose(true);
                } else if (string(argv[i]) == "-o") {
                    if (i + 1 >= argc) {
                        std::cout << "output name missing\n";
                        return EXIT_FAILURE;
                    }
                    ctx->output(argv[i + 1]);
                    i++;
                } else {
                    ctx->add(argv[i], Context::Source);
                }
            }
        }
        // create the parser
        auto parser = make_shared<Parser>(ctx);
        // semantic analyser
        auto semantic = make_shared<SemanticAnalyser>();
        // IR builder
        auto builder = make_shared<IrBuilder>();
        // create output emitter
        auto emitter = make_shared<Emitter>(ctx);
        // process the files
        for (auto & file : ctx->get(Context::Source)) {
            auto ast = parser->parse(make_shared<SourceFile>(file));
            if (ast) {
//                // print
//                ast->accept(new PrinterVisitor());
                
                // analyse
                ast->accept(semantic.get());
                
                // generate llvm code
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

