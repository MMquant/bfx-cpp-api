//////////////////////////////////////////////////////////////////////////////
//
//  example.cpp
//
//
//  Bitfinex REST API C++ client - examples
//
//
//////////////////////////////////////////////////////////////////////////////



#include <iostream>
#include "BitfinexAPI.hpp"
#include <fstream>



using std::cout;
using std::endl;
using std::ifstream;
using std::string;



int main(int argc, char *argv[])
{
    
    const char *keyFilePath = "/Path/to/your/file/with/API-key-secret";
    ifstream ifs(keyFilePath);
    if (!ifs.is_open())
    {
        cout << "Can't open file: " << argv[1] << endl;
        return -1;
    }
    else
    {
        string accessKey, secretKey;
        getline(ifs, accessKey);
        getline(ifs, secretKey);
        
        BitfinexAPI bfxAPI(accessKey, secretKey);

		/////////////////////////////////////////////////////////////////////////
		// This magic is needed to avoid crashes at a public endpoint after the authenticated.
		/////////////////////////////////////////////////////////////////////////

        string response;
        int errCode;
        
        /////////////////////////////////////////////////////////////////////////
        //  Examples
        //  Note that default values are not mandatory. See BitfinexAPI.hpp
        //  for details.
        /////////////////////////////////////////////////////////////////////////

        ///  Public endpoints ///
        
        //  errCode = bfxAPI.getTicker(response, "btcusd");
        //  errCode = bfxAPI.getStats(response, "btcusd");
        //  errCode = bfxAPI.getFundingBook(response, "USD", 50, 50);
        //  errCode = bfxAPI.getOrderBook(response, "btcusd", 50, 50, 1);
        //  errCode = bfxAPI.getTrades(response, "btcusd", 0L, 50);
        //  errCode = bfxAPI.getLends(response, "USD", 0L, 50);
        //  errCode = bfxAPI.getSymbols(response);
        //  errCode = bfxAPI.getSymbolDetails(response);
        
        /// Authenticated endpoints ///
        
        //  Account  //
        //  errCode = bfxAPI.getAccountInfo(response);
        //  errCode = bfxAPI.getSummary(response);
        //  errCode = bfxAPI.deposit(response, "bitcoin", "deposit", 1);
        //  errCode = bfxAPI.getKeyPermissions(response);
        //  errCode = bfxAPI.getMarginInfos(response);
        //  errCode = bfxAPI.getBalances(response);
        //  errCode = bfxAPI.transfer(response, 0.1, "BTC", "trading", "deposit");
        //  errCode = bfxAPI.withdraw(response); // configure withdraw.conf file before use
        //
        //  Orders  //
        //  errCode = bfxAPI.newOrder(response, "btcusd", 0.01, 983, "sell", "exchange limit", 0, 1,
        //                           0, 0, 0);
        //
        //  How to create vOrders object for newOrders() call
        //  BitfinexAPI::vOrders orders =
        //  {
        //      {"btcusd", 0.1, 950, "sell", "exchange limit"},
        //      {"btcusd", 0.1, 950, "sell", "exchange limit"},
        //      {"btcusd", 0.1, 950, "sell", "exchange limit"}
        //  };
        //  errCode = bfxAPI.newOrders(response, orders);
        //
        //  errCode = bfxAPI.cancelOrder(response, 13265453586LL);
        //
        //  How to create ids object for cancelOrders() call
        //  BitfinexAPI::vIds ids =
        //  {
        //      12324589754LL,
        //      12356754322LL,
        //      12354996754LL
        //  };
        //  errCode = bfxAPI.cancelOrders(response, ids);
        //
        //  errCode = bfxAPI.cancelAllOrders(response);
        //  errCode = bfxAPI.replaceOrder(response, 1321548521LL, "btcusd", 0.05, 1212, "sell",
        //                                "exchange limit", 0, 0);
        //  errCode = bfxAPI.getOrderStatus(response, 12113548453LL);
        //  errCode = bfxAPI.getActiveOrders(response);
        //
        //  Positions  //
        //  errCode = bfxAPI.getActivePositions(response);
        //  errCode = bfxAPI.claimPosition(response, 156321412LL, 150);
        //
        //  Historical data  //
        //  errCode = bfxAPI.getBalanceHistory(response, "USD", 0L, 0L, 500, "all");
        //  errCode = bfxAPI.getDWHistory(response, "BTC", "all", 0L , 0L, 500);
        //  errCode = bfxAPI.getPastTrades(response, "btcusd", 0L, 0L, 500, 0);
        //
        //  Margin funding  //
        //  errCode = bfxAPI.newOffer(response, "USD", 12000, 25.2, 30, "lend");
        //  errCode = bfxAPI.cancelOffer(response, 12354245628LL);
        //  errCode = bfxAPI.getOfferStatus(response, 12313541215LL);
        //  errCode = bfxAPI.getActiveCredits(response);
        //  errCode = bfxAPI.getOffers(response);
        //  errCode = bfxAPI.getTakenFunds(response);
        //  errCode = bfxAPI.getUnusedTakenFunds(response);
        //  errCode = bfxAPI.getTotalTakenFunds(response);
        //  errCode = bfxAPI.closeLoan(response, 1235845634LL);
        
        /////////////////////////////////////////////////////////////////////////
        /////////////////////////////////////////////////////////////////////////
        
        cout << "Response: " << response << endl;
        cout << "Error code: " << errCode << endl;
        
        ifs.close();
        return 0;
        
    }
}
