//
// Created by Albert Varaksin on 15/06/2021.
//

#ifndef DIAG
#define DIAG(LEVEL, ID, MSG)
#endif

// Parse errors
DIAG(Error, notAllowedTopLevelStatement, "statements are not allowed at the top level")
DIAG(Error, unexpectedToken, "Expected '{0}' got '{1}'")

#undef DIAG
