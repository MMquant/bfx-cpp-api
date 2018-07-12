////////////////////////////////////////////////////////////////////////////////
// error.hpp
//
// bfx-api-cpp error enumeration
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

enum BfxClientErrors
{
    noError = 0,
    curlERR,                // 1
    badSymbol,              // 2
    badCurrency,            // 3
    badDepositMethod,       // 4
    badWalletType,          // 5
    requiredParamsMissing,  // 6
    wireParamsMissing,      // 7
    addressParamsMissing,   // 8
    badOrderType,           // 9
    jsonStrToUSetError,     // 10
    badWDconfFilePath,      // 11
    responseParseError,     // 12
    responseSchemaError     // 13
};
