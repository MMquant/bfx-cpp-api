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
        
        BitfinexAPI bfxAPIpubl(accessKey, secretKey); //Use this curl for requests of public endpoints
		BitfinexAPI bfxAPIauth(accessKey, secretKey); //Use this curl for requests of authenticated endpoints

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
        
        //  errCode = bfxAPIpubl.getTicker(response, "btcusd");
        //  errCode = bfxAPIpubl.getStats(response, "btcusd");
        //  errCode = bfxAPIpubl.getFundingBook(response, "USD", 50, 50);
        //  errCode = bfxAPIpubl.getOrderBook(response, "btcusd", 50, 50, 1);
        //  errCode = bfxAPIpubl.getTrades(response, "btcusd", 0L, 50);
        //  errCode = bfxAPIpubl.getLends(response, "USD", 0L, 50);
        //  errCode = bfxAPIpubl.getSymbols(response);
        //  errCode = bfxAPIpubl.getSymbolDetails(response);
        
        /// Authenticated endpoints ///
        
        //  Account  //
        //  errCode = bfxAPIauth.getAccountInfo(response);
        //  errCode = bfxAPIauth.getSummary(response);
        //  errCode = bfxAPIauth.deposit(response, "bitcoin", "deposit", 1);
        //  errCode = bfxAPIauth.getKeyPermissions(response);
        //  errCode = bfxAPIauth.getMarginInfos(response);
        //  errCode = bfxAPIauth.getBalances(response);
        //  errCode = bfxAPIauth.transfer(response, 0.1, "BTC", "trading", "deposit");
        //  errCode = bfxAPIauth.withdraw(response); // configure withdraw.conf file before use
        //
        //  Orders  //
        //  errCode = bfxAPIauth.newOrder(response, "btcusd", 0.01, 983, "sell", "exchange limit", 0, 1,
        //                           0, 0, 0);
        //
        //  How to create vOrders object for newOrders() call
        //  BitfinexAPI::vOrders orders =
        //  {
        //      {"btcusd", 0.1, 950, "sell", "exchange limit"},
        //      {"btcusd", 0.1, 950, "sell", "exchange limit"},
        //      {"btcusd", 0.1, 950, "sell", "exchange limit"}
        //  };
        //  errCode = bfxAPIauth.newOrders(response, orders);
        //
        //  errCode = bfxAPIauth.cancelOrder(response, 13265453586LL);
        //
        //  How to create ids object for cancelOrders() call
        //  BitfinexAPI::vIds ids =
        //  {
        //      12324589754LL,
        //      12356754322LL,
        //      12354996754LL
        //  };
        //  errCode = bfxAPIauth.cancelOrders(response, ids);
        //
        //  errCode = bfxAPIauth.cancelAllOrders(response);
        //  errCode = bfxAPIauth.replaceOrder(response, 1321548521LL, "btcusd", 0.05, 1212, "sell",
        //                                "exchange limit", 0, 0);
        //  errCode = bfxAPIauth.getOrderStatus(response, 12113548453LL);
        //  errCode = bfxAPIauth.getActiveOrders(response);
        //
        //  Positions  //
        //  errCode = bfxAPIauth.getActivePositions(response);
        //  errCode = bfxAPIauth.claimPosition(response, 156321412LL, 150);
        //
        //  Historical data  //
        //  errCode = bfxAPIauth.getBalanceHistory(response, "USD", 0L, 0L, 500, "all");
        //  errCode = bfxAPIauth.getDWHistory(response, "BTC", "all", 0L , 0L, 500);
        //  errCode = bfxAPIauth.getPastTrades(response, "btcusd", 0L, 0L, 500, 0);
        //
        //  Margin funding  //
        //  errCode = bfxAPIauth.newOffer(response, "USD", 12000, 25.2, 30, "lend");
        //  errCode = bfxAPIauth.cancelOffer(response, 12354245628LL);
        //  errCode = bfxAPIauth.getOfferStatus(response, 12313541215LL);
        //  errCode = bfxAPIauth.getActiveCredits(response);
        //  errCode = bfxAPIauth.getOffers(response);
        //  errCode = bfxAPIauth.getTakenFunds(response);
        //  errCode = bfxAPIauth.getUnusedTakenFunds(response);
        //  errCode = bfxAPIauth.getTotalTakenFunds(response);
        //  errCode = bfxAPIauth.closeLoan(response, 1235845634LL);
        
        /////////////////////////////////////////////////////////////////////////
        /////////////////////////////////////////////////////////////////////////
        
        cout << "Response: " << response << endl;
        cout << "Error code: " << errCode << endl;
        
        ifs.close();
        return 0;
        
    }
}
