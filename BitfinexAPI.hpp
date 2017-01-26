
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
        badCurrency = -39
    };
    
    // Constructor - destructor
    explicit BitfinexAPI(const string &accessKey, const string &secretKey);
    ~BitfinexAPI();
    
    // Public endpoints
    int getTicker(string &result, string symbol);
    int getStats(string &result, string symbol);
    int getFundingBook(string &result, string currency, int limit_bids = 50,
                       int limit_asks = 50);
    int getOrderBook(string &result, string symbol, int limit_bids = 50,
                     int limit_asks = 50, bool group = 1);
    int getTrades(string &result, string symbol, time_t since = 0,
                  int limit_trades = 50);
    int getLends(string &result, string currency, time_t since = 0,
                 int limit_lends = 50);
    int getSymbols(string &result);
    int getSymbolDetails(string &result);
    
    // Authenticated endpoints
    int getAccountInfo(string &result);
    
    
    
private:
    
    // BitfinexAPI object cannot be copied
    BitfinexAPI(const BitfinexAPI&);
    BitfinexAPI &operator=(const BitfinexAPI&);
    
    // Private variables
    string APIurl;
    vector<string> symbols; // possible symbol pairs
    vector<string> currencies; // possible currencies
    string accessKey, secretKey;
    CURL *curl;
    CURLcode res;
    
    // Support private methods
    static string getTonce();
    static int getBase64(const string &content, string &base64);
    static int getHmacSha384(const string &key, const string &content, string &digest);
    static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);
    static bool inArray(const string &value, const vector<string> &symbols);
    int DoGETrequest(const string &UrlEndPoint, const string &params, string &result);
    int DoPOSTrequest(const string &UrlEndPoint, const string &params, string &result);
    
};
