
//////////////////////////////////////////////////////////////////////////////
// BitfinexAPI.hpp
//////////////////////////////////////////////////////////////////////////////

#pragma once



#include <iostream>
#include <curl/curl.h>
#include <vector>



using std::string;
using std::vector;



class BitfinexAPI
{
public:
    
    // Enumerations
    enum bfxERR // positive values for curl internal error codes
    {
        curlERR = -50,
        badSymbol = -40,
        badCurrency = -39,
        badDepositMethod = -38,
        badWalletType = -37,
        requiredParamsMissing = -36,
        wireParamsMissing = -35,
        addressParamsMissing = -34,
        badOrderType = -33
    };
    
    // Structure for multiple new orders endpoint
    struct sOrder
    {
        string symbol;
        double amount;
        double price;
        string side;
        string type;
    };
    typedef vector<sOrder> vOrders;
    
    // Constructor - destructor
    explicit BitfinexAPI(const string &accessKey, const string &secretKey);
    ~BitfinexAPI();
    
    // Accessors
    string getWDconfFilePath() const { return WDconfFilePath; }
    void setWDconfFilePath(const string &path) { WDconfFilePath = path; }
    
    //  Public endpoints
    int getTicker(string &result, const string &symbol);
    int getStats(string &result, const string &symbol);
    int getFundingBook(string &result, const string &currency, const int &limit_bids = 50,
                       const int &limit_asks = 50);
    int getOrderBook(string &result, const string &symbol, const int &limit_bids = 50,
                     const int &limit_asks = 50, const bool &group = 1);
    int getTrades(string &result, const string &symbol, const time_t &since = 0,
                  const int &limit_trades = 50);
    int getLends(string &result, const string &currency, const time_t &since = 0,
                 const int &limit_lends = 50);
    int getSymbols(string &result);
    int getSymbolDetails(string &result);
    
    /// Authenticated endpoints ///
    //  Account
    int getAccountInfo(string &result);
    int getSummary(string &result);
    int deposit(string &result, const string &method, const string &walletType,
                const bool &renew = 0);
    int getKeyPermissions(string &result);
    int getMarginInfos(string &result);
    int getBalances(string &result);
    int transfer(string &result, const double &amount, const string &currency,
                 const string &walletfrom, const string &walletto);
    int withdraw(string &result); // configure withdraw.conf file before use
    //  Orders
    int newOrder(string &result, const string &symbol, const double &amount,
                 const double &price, const string &side, const string &type,
                 const bool &is_hidden = 0, const bool &is_postonly = 0,
                 const bool &use_all_available = 0, const bool &ocoorder = 0,
                 const double &buy_price_oco = 0);
    int newOrders(string &result, const vOrders &orders);
    int cancelOrder(string &result, const long long &order_id);
    int cancelOrders(string &result, const vector<long long> &vOrder_ids);
    int cancelAllOrders(string &result);
    int replaceOrder(string &result, const long long &order_id, const string &symbol,
                     const double &amount, const double &price, const string &side,
                     const string &type, const bool &is_hidden = 0,
                     const bool &use_remaining = 0);
    int getOrderStatus(string &result, const long long &order_id);
    int getActiveOrders(string &result);
    //  Positions
    int getActivePositions(string &result);
    int claimPosition(string &result, long &position_id, const double &amount);
    //  Historical data
    int getBalanceHistory(string &result, const string &currency, const time_t &since = 0,
                          const time_t &until = 0, const int &limit = 500,
                          const string &walletType = "all");
    int getDWHistory(string &result, const string &currency, const string &method,
                     const time_t &since = 0 , const time_t &until = 0,
                     const int &limit = 500);
    int getPastTrades(string &result, const string &symbol, const time_t &timestamp,
                      const time_t &until = 0, const int &limit_trades = 500,
                      const bool reverse = 0);
    //  Margin funding
    int newOffer(string &result, const string &currency, const double &amount,
                 const float &rate, const int &period, const string &direction);
    int cancelOffer(string &result, const long long &offer_id);
    int getOfferStatus(string &result, const long long &offer_id);
    int getActiveCredits(string &result);
    int getOffers(string &result);
    int getTakenFunds(string &result);
    int getUnusedTakenFunds(string &result);
    int getTotalTakenFunds(string &result);
    int closeLoan(string &result, const long long &offer_id);
    
private:
    
    // BitfinexAPI object cannot be copied
    BitfinexAPI(const BitfinexAPI&);
    BitfinexAPI &operator=(const BitfinexAPI&);
    
    // Private variables
    vector<string> symbols; // possible symbol pairs
    vector<string> currencies; // possible currencies
    vector<string> methods; // possible deposit methods
    vector<string> walletTypes; // possible walletTypes
    vector<string> types; // possible Types (see new order endpoint)
    string WDconfFilePath;
    string APIurl;
    string accessKey, secretKey;
    CURL *curl;
    CURLcode res;
    
    // Utility private methods
    int parseWDconfParams(string &params);
    static string bool2string(const bool &in);
    static string getTonce();
    static int getBase64(const string &content, string &base64);
    static int getHmacSha384(const string &key, const string &content, string &digest);
    static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);
    static bool inArray(const string &value, const vector<string> &symbols);
    int DoGETrequest(const string &UrlEndPoint, const string &params, string &result);
    int DoPOSTrequest(const string &UrlEndPoint, const string &params, string &result);
    
};
