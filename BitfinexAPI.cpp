
//////////////////////////////////////////////////////////////////////////////
// BitfinexAPI.cpp
//////////////////////////////////////////////////////////////////////////////



#include "BitfinexAPI.hpp"
#include <iostream>
#include <utility>
#include <map>
#include <fstream>

#include <cryptopp/hmac.h>
#include <cryptopp/osrng.h>
#include <cryptopp/hex.h>
#include <cryptopp/base64.h>



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
    WDconfFilePath = "withdraw.conf";
    symbols = // to do: initialize symbols values from API symbols call.
    {
        "btcusd", "ltcusd", "ltcbtc",
        "ethusd", "ethbtc", "etcbtc",
        "etcusd", "bfxusd", "bfxbtc",
        "rrtusd", "rrtbtc", "zecusd",
        "zecbtc", "xmrusd", "xmrbtc"
    };
    currencies =
    {
        "usd", "btc", "eth", "etc", "bfx", "zec", "ltc"
    };
    methods =
    {
        "bitcoin", "litecoin", "ethereum",
        "mastercoin", "ethereumc", "zcash",
        "monero"
    };
    walletTypes =
    {
        "trading", "exchange", "deposit"
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
    "&group=" + to_string(group);
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
// Authenticated endpoints
//////////////////////////////////////////////////////////////////////////////


int BitfinexAPI::
getAccountInfo(string &result)
{
    
    string endPoint = "/account_infos/";
    string params = "{\"request\":\"/v1/account_infos\",\"nonce\":\"" + getTonce() + "\"";
    params += "}";
    
    return DoPOSTrequest(endPoint, params, result);
    
}


int BitfinexAPI::
getSummary(string &result)
{
    
    string endPoint = "/summary/";
    string params = "{\"request\":\"/v1/summary\",\"nonce\":\"" + getTonce() + "\"";
    params += "}";
    return DoPOSTrequest(endPoint, params, result);
    
}


int BitfinexAPI::
deposit(string &result, string method, string walletType, bool renew)
{
    
    // Is deposit method valid ?
    if(!inArray(method, methods))
    {
        return badDepositMethod;
    }
    // Is walletType valid ?
    if(!inArray(walletType, walletTypes))
    {
        return badWalletType;
    }
    
    string endPoint = "/deposit/new/";
    string params = "{\"request\":\"/v1/deposit/new\",\"nonce\":\"" + getTonce() + "\"";
    
    params += ",\"method\":\"" + method + "\"";
    params += ",\"wallet_name\":\"" + walletType + "\"";
    params += ",\"renew\":" + to_string(renew);
    
    params += "}";
    return DoPOSTrequest(endPoint, params, result);
    
}


int BitfinexAPI::
getKeyPermissions(string &result)
{
    
    string endPoint = "/key_info/";
    string params = "{\"request\":\"/v1/key_info\",\"nonce\":\"" + getTonce() + "\"";
    params += "}";
    return DoPOSTrequest(endPoint, params, result);
    
}


int BitfinexAPI::
getMarginInfos(string &result)
{
    
    string endPoint = "/margin_infos/";
    string params = "{\"request\":\"/v1/margin_infos\",\"nonce\":\"" + getTonce() + "\"";
    params += "}";
    return DoPOSTrequest(endPoint, params, result);
    
}


int BitfinexAPI::
getBalances(string &result)
{
    
    string endPoint = "/balances/";
    string params = "{\"request\":\"/v1/balances\",\"nonce\":\"" + getTonce() + "\"";
    params += "}";
    return DoPOSTrequest(endPoint, params, result);
    
}


int BitfinexAPI::
transfer(string &result, int amount, string currency, string walletfrom, string walletto)
{
    
    // Is currency valid ?
    if(!inArray(currency, currencies))
    {
        return badCurrency;
    }
    // Is walletType valid ?
    if(!inArray(walletfrom, walletTypes) || !inArray(walletto, walletTypes))
    {
        return badWalletType;
    }
    
    
    string endPoint = "/transfer/";
    string params = "{\"request\":\"/v1/transfer\",\"nonce\":\"" + getTonce() + "\"";
    
    params += ",\"amount\":" + to_string(amount);
    params += ",\"currency\":\"" + currency + "\"";
    params += ",\"walletfrom\":\"" + walletfrom + "\"";
    params += ",\"walletto\":\"" + walletto + "\"";
    
    params += "}";
    return DoPOSTrequest(endPoint, params, result);
    
}


//////////////////////////////////////////////////////////////////////////////
// Support private methods
//////////////////////////////////////////////////////////////////////////////


// Generates parameters for withdraw method
string BitfinexAPI::
parseWDconfParams()
{
 
    using std::map;
    using std::getline;
    using std::ifstream;
    
    string params;
    string line;
    ifstream inFile;
    map<string, string> mParams;
    inFile.open(WDconfFilePath);
    while (getline(inFile, line)) {
        
        // Here be dragons
        
    }
    
    return params;
    
}


string BitfinexAPI::
getTonce()
{

    std::stringstream tonce;
    
    struct timeval start;
    gettimeofday(&start, NULL);
    tonce << start.tv_sec * 1000000LL + start.tv_usec;

    return tonce.str();
    
}


// String to Base64
int BitfinexAPI::
getBase64(const string &content, string &encoded)
{
    
    using CryptoPP::StringSource;
    using CryptoPP::Base64Encoder;
    using CryptoPP::StringSink;
    
    byte buffer[1024] = {};
    
    for (int i = 0; i < content.length(); i++)
    {
        buffer[i] = content[i];
    };
    
    StringSource ss(buffer, content.length(), true, new Base64Encoder( new StringSink(encoded), false));
    
    return 0;
    
}


// String to HMAC-SHA384 hex digest
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
    std::transform(digest.begin(), digest.end(), digest.begin(), ::tolower);
    
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
        
        string url = APIurl + UrlEndPoint;
        string payload;
        string signature;
        getBase64(params, payload);
        getHmacSha384(secretKey, payload, signature);
        
        // Headers
        struct curl_slist *httpHeaders = NULL;
        httpHeaders = curl_slist_append(httpHeaders, ("X-BFX-APIKEY:" + accessKey).c_str());
        httpHeaders = curl_slist_append(httpHeaders, ("X-BFX-PAYLOAD:" + payload).c_str());
        httpHeaders = curl_slist_append(httpHeaders, ("X-BFX-SIGNATURE:" + signature).c_str());
        
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L); // debug option
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "\n");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, httpHeaders);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
        res = curl_easy_perform(curl);
        curl_slist_free_all(httpHeaders);
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
