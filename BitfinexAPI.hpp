
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
    struct sOrders
    {
        string symbol;
        double amount;
        double price;
        string side;
        string type;
    };
    
    // Constructor - destructor
    explicit BitfinexAPI(const string &accessKey, const string &secretKey);
    ~BitfinexAPI();
    
    // Accessors
    string getWDconfFilePath() const { return WDconfFilePath; }
    void setWDconfFilePath(const string &path) { WDconfFilePath = path; }
    vector<sOrders> getNewOrders() const { return vOrders; }
    void setNewOrders(const vector<sOrders> &vOrdersIn) { vOrders = vOrdersIn; }
    
    // Public endpoints
    int getTicker(string &result, const string symbol);
    int getStats(string &result, const string symbol);
    int getFundingBook(string &result, const string currency, const int limit_bids = 50,
                       const int limit_asks = 50);
    int getOrderBook(string &result, const string symbol,const int limit_bids = 50,
                     const int limit_asks = 50, const bool group = 1);
    int getTrades(string &result, const string symbol, const time_t since = 0,
                  const int limit_trades = 50);
    int getLends(string &result, const string currency, const time_t since = 0,
                 const int limit_lends = 50);
    int getSymbols(string &result);
    int getSymbolDetails(string &result);
    
    // Authenticated endpoints
    int getAccountInfo(string &result);
    int getSummary(string &result);
    int deposit(string &result, const string method, const string walletType,
                const bool renew = 0);
    int getKeyPermissions(string &result);
    int getMarginInfos(string &result);
    int getBalances(string &result);
    int transfer(string &result, const double amount, const string currency,
                 const string walletfrom, const string walletto);
    int withdraw(string &result); // configure withdraw.conf file before use
    int newOrder(string &result, const string symbol, const double amount,
                 const double price, const string side, const string type,
                 const bool is_hidden = 0, const bool is_postonly = 0,
                 const bool use_all_available = 0, const bool ocoorder = 0,
                 const double buy_price_oco = 0);
    int newOrders(string &result, const vector<struct sOrders> &vOrders);
    
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
    vector<sOrders> vOrders; // vector with parameters for newOrders method
    string WDconfFilePath;
    string APIurl;
    string accessKey, secretKey;
    CURL *curl;
    CURLcode res;
    
    // Support private methods
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
