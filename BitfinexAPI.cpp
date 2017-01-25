#include "BitfinexAPI.hpp"
#include <iostream>
#include <cryptopp/hmac.h>
#include <cryptopp/osrng.h>
#include <cryptopp/hex.h>


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
        "zecbtc",
        "xmrusd",
        "xmrbtc"
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


int BitfinexAPI::
getTrades(string &result, string symbol, time_t since, int limit_trades)
{
    
    // Is symbol valid ?
    if(!inArray(symbol, symbols))
    {
        return badSymbol;
    }
    string endPoint = "/trades/" + symbol;
    string params =
    "?timestamp=" + to_string(since) +
    "&limit_trades=" + to_string(limit_trades);
    return DoGETrequest(endPoint, params, result);
    
}


int BitfinexAPI::
getLends(string &result, string currency, time_t since, int limit_lends)
{
    
    // Is currency valid ?
    if(!inArray(currency, currencies))
    {
        return badCurrency;
    }
    string endPoint = "/lends/" + currency;
    string params =
    "?timestamp=" + to_string(since) +
    "&limit_lends=" + to_string(limit_lends);
    return DoGETrequest(endPoint, params, result);
    
}


int BitfinexAPI::
getSymbols(string &result)
{
    
    string endPoint = "/symbols/";
    string params = "";
    return DoGETrequest(endPoint, params, result);
    
}


int BitfinexAPI::
getSymbolDetails(string &result)
{
    
    string endPoint = "/symbols_details/";
    string params = "";
    return DoGETrequest(endPoint, params, result);
    
}


//////////////////////////////////////////////////////////////////////////////
// Support private methods
//////////////////////////////////////////////////////////////////////////////


// String to HMAC-SHA384
int BitfinexAPI::
getHmacSha384(const string &key, const string &content, string &digest)
{
    
    using CryptoPP::SecByteBlock;
    using CryptoPP::StringSource;
    using CryptoPP::HexEncoder;
    using CryptoPP::StringSink;
    using CryptoPP::HMAC;
    using CryptoPP::HashFilter;
    
    SecByteBlock byteKey((const byte*)key.data(), key.size());
    string mac;
    digest.clear();
    
    HMAC<CryptoPP::SHA384> hmac(byteKey, byteKey.size());
    StringSource ss1(content, true, new HashFilter(hmac, new StringSink(mac)));
    StringSource ss2(mac, true, new HexEncoder(new StringSink(digest)));
    
    return 0;
}


// Curl write callback function. Appends fetched *content to *userp pointer.
// *userp pointer is set up by curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result) line.
// In this case *userp will point to result.
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


int BitfinexAPI::
DoPOSTrequest(const string &UrlEndPoint, const string &params, string &result)
{
    
    if(curl) {
        
        // Headers
        struct curl_slist *httpHeaders = NULL;
        httpHeaders = curl_slist_append(httpHeaders, ("X-BFX-APIKEY:" + accessKey).c_str());
        httpHeaders = curl_slist_append(httpHeaders, ("X-BFX-PAYLOAD:" + accessKey).c_str());
        
        
        
        string url = APIurl + UrlEndPoint;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, httpHeaders);
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
