//
// Created by Albert Varaksin on 15/06/2021.
//
// clang-format off

#ifndef DIAG
#    define DIAG(LEVEL, ID, MSG)
#endif

// Parse errors
DIAG(Error, notAllowedTopLevelStatement,
    "statements are not allowed at the top level")
DIAG(Error, unexpectedToken,
    "expected '{0}' got '{1}'")
DIAG(Error, moduleNotFound,
    "no such module '{0}'")
DIAG(Error, failedToLoadModule,
    "failed to load module '{0}'")
DIAG(Error, expectedDeclarationAfterAttribute,
    "expected declaration after attributes, got '{0}'")
DIAG(Error, unexpectedNestedDeclaration,
    "unexpected nested declaration '{0}'")
DIAG(Error, variadicArgumentNotLast,
    "variadic argument must be last")
DIAG(Error, unexpectedReturn,
    "return not allowed outside main module, SUB or FUNCTION")
DIAG(Error, expectedExpression,
    "expected expression, got '{0}'")

#undef DIAG
