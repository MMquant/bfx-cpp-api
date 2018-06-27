//////////////////////////////////////////////////////////////////////////////
//
//  example.cpp
//
//
//  Bitfinex REST API C++ client - examples
//
//////////////////////////////////////////////////////////////////////////////

// std
#include <fstream>
#include <iostream>

// BitfinexAPI
#include "BitfinexAPI.hpp"


// namespaces
using std::cout;
using std::endl;
using std::ifstream;
using std::string;


int main(int argc, char *argv[])
{
    /////////////////////////////////////////////////////////////////////////
    //  Examples
    //  Note that default values are not mandatory. See BitfinexAPI.hpp
    //  for details.
    /////////////////////////////////////////////////////////////////////////
    
    /////////////////////////////////////////////////////////////////////////
    //    GET REQUESTS - unauthenticated endpoints
    /////////////////////////////////////////////////////////////////////////
    
    BfxAPI::BitfinexAPI bfxAPI;
    
    bfxAPI.getTicker("btcusd");
    if (!bfxAPI.hasApiError())
        cout << bfxAPI.strResponse() << endl;
    else
    {
        // see bfxERR enum in BitfinexAPI.hpp::BitfinexAPI
        cout << bfxAPI.getBfxApiStatusCode() << endl;
        // see https://curl.haxx.se/libcurl/c/libcurl-errors.html
        cout << bfxAPI.getCurlStatusCode() << endl;
    }
    
    bfxAPI.getStats("btcusd");
    if (!bfxAPI.hasApiError())
        cout << bfxAPI.strResponse() << endl;
    else
    {
        // see bfxERR enum in BitfinexAPI.hpp::BitfinexAPI
        cout << bfxAPI.getBfxApiStatusCode() << endl;
        // see https://curl.haxx.se/libcurl/c/libcurl-errors.html
        cout << bfxAPI.getCurlStatusCode() << endl;
    }
    
    return 0;
    
}
