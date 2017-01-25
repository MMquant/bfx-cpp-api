#include "BitfinexAPI.hpp"
#include <iostream>



using std::cout;
using std::vector;
using std::to_string;


//////////////////////////////////////////////////////////////////////////////
// Constructor - destructor
//////////////////////////////////////////////////////////////////////////////

BitfinexAPI::
BitfinexAPI(const string &accessKey, const string &secretKey):
accessKey(accessKey), secretKey(secretKey), curl(NULL)
{
    curl = curl_easy_init();
    APIurl = "https://api.bitfinex.com/v1";
    symbols = // to do: initialize symbols values from API symbols call.
    {
        "btcusd",
        "ltcusd",
        "ltcbtc",
        "ethusd",
        "ethbtc",
        "etcbtc",
        "etcusd",
        "bfxusd",
        "bfxbtc",
        "rrtusd",
        "rrtbtc",
        "zecusd",
        "zecbtc"
    };
    currencies =
    {
        "usd",
        "btc",
        "eth",
        "etc",
        "bfx",
        "zec",
        "ltc"
    };
}

BitfinexAPI::
~BitfinexAPI()
{
    curl_easy_cleanup(curl);
}

//////////////////////////////////////////////////////////////////////////////
// Public endpoints
//////////////////////////////////////////////////////////////////////////////

int BitfinexAPI::
getTicker(string &result, string symbol)
{
    
    // Is symbol valid ?
    if(!inArray(symbol, symbols))
    {
        return badSymbol;
    }
    string endPoint = "/pubticker/" + symbol;
    string params = "";
    return DoGETrequest(endPoint, params, result);
    
}

int BitfinexAPI::
getStats(string &result, string symbol)
{
    
    // Is symbol valid ?
    if(!inArray(symbol, symbols))
    {
        return badSymbol;
    }
    string endPoint = "/stats/" + symbol;
    string params = "";
    return DoGETrequest(endPoint, params, result);
    
}

int BitfinexAPI::
getFundingBook(string &result, string currency, int limit_bids, int limit_asks)
{
    
    // Is currency valid ?
    if(!inArray(currency, currencies))
    {
        return badCurrency;
    }
    string endPoint = "/lendbook/" + currency;
    string params =
    "?limit_bids=" + to_string(limit_bids) +
    "&limit_asks=" + to_string(limit_asks);
    return DoGETrequest(endPoint, params, result);
    
}

int BitfinexAPI::
getOrderBook(string &result, string symbol, int limit_bids, int limit_asks, bool group)
{
    
    // Is symbol valid ?
    if(!inArray(symbol, symbols))
    {
        return badSymbol;
    }
    string endPoint = "/book/" + symbol;
    string params =
    "?limit_bids=" + to_string(limit_bids) +
    "&limit_asks=" + to_string(limit_asks) +
    "&group=" + to_string(limit_asks);
    return DoGETrequest(endPoint, params, result);
    
}

//////////////////////////////////////////////////////////////////////////////
// Support private methods
//////////////////////////////////////////////////////////////////////////////

// Curl write callback function. Appends fetched *content to *userp pointer.
// *userp pointer is set up by curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result) line
size_t BitfinexAPI::
WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Search in vector of strings
bool BitfinexAPI::
inArray(const string &value, const vector<string> &symbols)
{
    return std::find(symbols.begin(), symbols.end(), value) != symbols.end();
}


int BitfinexAPI::
DoGETrequest(const string &UrlEndPoint, const string &params, string &result)
{
    
    if(curl) {
        string url = APIurl + UrlEndPoint + params;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
        res = curl_easy_perform(curl);
        
        // libcurl internal error handling
        if (res != CURLE_OK)
        {
            cout << "Libcurl error in DoGETRequest(), code:\n";
            return res;
        }
        return res;
        
    }
    else
    {
        // curl not properly initialized curl = NULL
        return curlERR;
        
    }
}
