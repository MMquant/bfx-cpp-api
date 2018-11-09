////////////////////////////////////////////////////////////////////////////////
//
//  test.cpp
//
//
//  Bitfinex REST API C++ client - Available unauthenticated requests test
//
////////////////////////////////////////////////////////////////////////////////

// std
#include <fstream>
#include <iostream>

// BitfinexAPI
#include "bfx-api-cpp/BitfinexAPI.hpp"


// namespaces
using std::cerr;
using std::cout;
using std::endl;
using std::ifstream;
using std::string;

void check(BfxAPI::BitfinexAPI &bfxAPI) {
  if (bfxAPI.hasApiError()) {
    cout << "❌" << endl << endl;
    cout << "BfxApiStatusCode: ";
    cout << bfxAPI.getBfxApiStatusCode() << " - ";
    cout << "CurlStatusCode: ";
    cout << bfxAPI.getCurlStatusCode() << endl;
    cout << "Response: " << bfxAPI.strResponse() << endl << endl;
  } else {
    cout << "✅" << endl << endl;
  }
}

int main(int argc, char *argv[]) {
  // Create bfxAPI without API keys
  BfxAPI::BitfinexAPI bfxAPI;

  ////////////////////////////////////////////////////////////////////////////
  ///  Available unauthenticated requests
  ////////////////////////////////////////////////////////////////////////////
  cout << "Starting available unauthenticated requests test" << endl << endl;

  cout << "- getTicker(\"btcusd\"): ";
  bfxAPI.getTicker("btcusd"); check(bfxAPI);

  cout << "- getStats(\"btcusd\"): ";
  bfxAPI.getStats("btcusd"); check(bfxAPI);

  cout << "- getFundingBook(\"USD\", 50, 50): ";
  bfxAPI.getFundingBook("USD", 50, 50); check(bfxAPI);

  cout << "- getOrderBook(\"btcusd\", 50, 50, true): ";
  bfxAPI.getOrderBook("btcusd", 50, 50, true); check(bfxAPI);

  cout << "- getTrades(\"btcusd\", 0L, 50): ";
  bfxAPI.getTrades("btcusd", 0L, 50); check(bfxAPI);

  cout << "- getLends(\"USD\", 0L, 50)): ";
  bfxAPI.getLends("USD", 0L, 50); check(bfxAPI);

  cout << "- getSymbols(): ";
  bfxAPI.getSymbols(); check(bfxAPI);

  cout << "- getSymbolsDetails(): ";
  bfxAPI.getSymbolsDetails(); check(bfxAPI);

  return 0;
}
