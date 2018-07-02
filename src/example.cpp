////////////////////////////////////////////////////////////////////////////////
//
//  example.cpp
//
//
//  Bitfinex REST API C++ client - examples
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


int main(int argc, char *argv[])
{
    // Create bfxAPI without API keys
    BfxAPI::BitfinexAPI bfxAPI;
    // Create bfxAPI with API keys
    // BfxAPI::BitfinexAPI bfxAPI("accessKey", "secretKey");
    
    // Load API keys from file
    ifstream ifs("doc/key-secret", ifstream::in);
    if (!ifs.is_open())
    {
        cerr << "Can't open 'key-secret' file. " << endl;
        return 1;
    }
    getline(ifs, bfxAPI.getAccessKey());
    getline(ifs, bfxAPI.getSecretKey());
    ifs.close();
    
    // Fetch API
    cout << "Request with error checking: " << endl;
    bfxAPI.getTicker("btcusd");
    if (!bfxAPI.hasApiError())
        cerr << bfxAPI.strResponse() << endl;
    else
    {
        // see bfxERR enum in BitfinexAPI.hpp::BitfinexAPI
        cout << bfxAPI.getBfxApiStatusCode() << endl;
        // see https://curl.haxx.se/libcurl/c/libcurl-errors.html
        cout << bfxAPI.getCurlStatusCode() << endl;
    }
    
    cout << "Request without error checking: " << endl;
    cout << bfxAPI.getSummary().strResponse() << endl;
    
    ////////////////////////////////////////////////////////////////////////////
    ///  Available unauthenticated requests
    ////////////////////////////////////////////////////////////////////////////
    
    //  bfxAPI.getTicker("btcusd");
    //  bfxAPI.getStats("btcusd");
    //  bfxAPI.getFundingBook("USD", 50, 50);
    //  bfxAPI.getOrderBook("btcusd", 50, 50, 1);
    //  bfxAPI.getTrades("btcusd", 0L, 50);
    //  bfxAPI.getLends("USD", 0L, 50);
    //  bfxAPI.getSymbols();
    //  bfxAPI.getSymbolDetails();
    
    ////////////////////////////////////////////////////////////////////////////
    ///  Available authenticated requests
    ////////////////////////////////////////////////////////////////////////////
    
    ///  Account  ///
    //  bfxAPI.getAccountInfo();
    //  bfxAPI.getAccountFees();
    //  bfxAPI.getSummary();
    //  bfxAPI.deposit("bitcoin", "deposit", 1);
    //  bfxAPI.getKeyPermissions();
    //  bfxAPI.getMarginInfos();
    //  bfxAPI.getBalances();
    //  bfxAPI.transfer(0.1, "BTC", "trading", "deposit");
    //  bfxAPI.withdraw(); // configure withdraw.conf file before use
    
    ///  Orders  ///
    //  bfxAPI.newOrder("btcusd", 0.01, 983, "sell", "exchange limit", 0, 1,
    //                           0, 0, 0);
    //
    //  How to create vOrders object for newOrders() call
    //  BitfinexAPI::vOrders orders =
    //  {
    //      {"btcusd", 0.1, 950, "sell", "exchange limit"},
    //      {"btcusd", 0.1, 950, "sell", "exchange limit"},
    //      {"btcusd", 0.1, 950, "sell", "exchange limit"}
    //  };
    //  bfxAPI.newOrders(orders);
    //
    //  bfxAPI.cancelOrder(13265453586LL);
    //
    //  How to create ids object for cancelOrders() call
    //  BitfinexAPI::vIds ids =
    //  {
    //      12324589754LL,
    //      12356754322LL,
    //      12354996754LL
    //  };
    //  bfxAPI.cancelOrders(ids);
    //
    //  bfxAPI.cancelAllOrders();
    //  bfxAPI.replaceOrder(1321548521LL, "btcusd", 0.05, 1212, "sell",
    //                                "exchange limit", 0, 0);
    //  bfxAPI.getOrderStatus(12113548453LL);
    //  bfxAPI.getActiveOrders();
    //  bfxAPI.getOrdersHistory(10);
    
    ///  Positions  ///
    //  bfxAPI.getActivePositions();
    //  bfxAPI.claimPosition(156321412LL, 150);
    
    ///  Historical data  ///
    //  bfxAPI.getBalanceHistory("USD", 0L, 0L, 500, "all");
    //  bfxAPI.getWithdrawalHistory("BTC", "all", 0L , 0L, 500);
    //  bfxAPI.getPastTrades("btcusd", 0L, 0L, 500, 0);
    
    ///  Margin funding  ///
    //  bfxAPI.newOffer("USD", 12000, 25.2, 30, "lend");
    //  bfxAPI.cancelOffer(12354245628LL);
    //  bfxAPI.getOfferStatus(12313541215LL);
    //  bfxAPI.getActiveCredits();
    //  bfxAPI.getOffers();
    //  bfxAPI.getOffersHistory(50);
    //  bfxAPI.getPastFundingTrades("BTC", 0, 50);
    //  bfxAPI.getTakenFunds();
    //  bfxAPI.getUnusedTakenFunds();
    //  bfxAPI.getTotalTakenFunds();
    //  bfxAPI.closeLoan(1235845634LL);
    //  bfxAPI.closePosition(1235845634LL);
    
    return 0;
}
