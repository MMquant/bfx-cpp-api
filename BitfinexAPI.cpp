
//////////////////////////////////////////////////////////////////////////////
// BitfinexAPI.cpp
//////////////////////////////////////////////////////////////////////////////



#include "BitfinexAPI.hpp"
#include <iostream>
#include <utility>
#include <map>
#include <fstream>
#include <regex>

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
    symbols =
    {
        "btcusd", "ltcusd", "ltcbtc",
        "ethusd", "ethbtc", "etcbtc",
        "etcusd", "bfxusd", "bfxbtc",
        "rrtusd", "rrtbtc", "zecusd",
        "zecbtc", "xmrusd", "xmrbtc"
    };
    currencies =
    {
        "USD", "BTC", "ETH", "ETC", "BFX", "ZEC", "LTC"
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
    // New order endpoint "type" parameter
    types =
    {
        "market","limit", "stop", "trailing-stop", "fill-or-kill",
        "exchange market", "exchange limit", "exchange stop",
        "exchange trailing-stop", "exchange fill-or-kill"
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
transfer(string &result, double amount, string currency, string walletfrom, string walletto)
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
    
    params += ",\"amount\":\"" + to_string(amount) + "\"";
    params += ",\"currency\":\"" + currency + "\"";
    params += ",\"walletfrom\":\"" + walletfrom + "\"";
    params += ",\"walletto\":\"" + walletto + "\"";
    
    params += "}";
    return DoPOSTrequest(endPoint, params, result);
    
}


int BitfinexAPI::
withdraw(string &result)
{
    
    string endPoint = "/withdraw/";
    string params = "{\"request\":\"/v1/withdraw\",\"nonce\":\"" + getTonce() + "\"";
    
    // Add params from withdraw.conf
    int err = parseWDconfParams(params);
    if (err != 0) { return err ;};
    
    params += "}";
    return DoPOSTrequest(endPoint, params, result);
    
}


int BitfinexAPI::
newOrder(string &result, const string symbol, const double amount,
         const double price, const string side, const string type,
         const bool is_hidden, const bool is_postonly,
         const bool use_all_available, const bool ocoorder,
         const double buy_price_oco)
{
    
    // Is symbol valid ?
    if(!inArray(symbol, symbols))
    {
        return badSymbol;
    }
    // Is order type valid ?
    if(!inArray(type, types))
    {
        return badOrderType;
    }
    
    string endPoint = "/order/new/";
    string params = "{\"request\":\"/v1/order/new\",\"nonce\":\"" + getTonce() + "\"";
    
    params += ",\"symbol\":\"" + symbol + "\"";
    params += ",\"amount\":\"" + to_string(amount) + "\"";
    params += ",\"price\":\"" + to_string(price) + "\"";
    params += ",\"side\":\"" + side + "\"";
    params += ",\"type\":\"" + type + "\"";
    params += ",\"is_hidden\":" + bool2string(is_hidden);
    params += ",\"is_postonly\":" + bool2string(is_postonly);
    params += ",\"use_all_available\":" + bool2string(use_all_available);
    params += ",\"ocoorder\":" + bool2string(ocoorder);
    params += ",\"buy_price_oco\":" + bool2string(buy_price_oco);
    
    params += "}";
    return DoPOSTrequest(endPoint, params, result);
    
}


int BitfinexAPI::
newOrders(string &result, const vector<sOrders> &vOrders)
{
    
    string endPoint = "/order/new/multi/";
    string params = "{\"request\":\"/v1/order/new/multi\",\"nonce\":\"" + getTonce() + "\"";
    
    // Get pointer to last element in vOrders. We will not place
    // ',' character at the end of the last loop.
    auto &last = *(--vOrders.end());
    
    params += ",[";
    for (const auto &order : vOrders)
    {
        params += "{\"symbol\":\"" + order.symbol + "\"";
        params += ",\"amount\":\"" + to_string(order.amount) + "\"";
        params += ",\"price\":\"" + to_string(order.price) + "\"";
        params += ",\"side\":\"" + order.side + "\"";
        params += ",\"type\":\"" + order.type + "\"}";
        if (&order != &last)
        {
            params += ",";
        }
    }
    params += "]";
    
    params += "}";
    return DoPOSTrequest(endPoint, params, result);
}


//////////////////////////////////////////////////////////////////////////////
// Support private methods
//////////////////////////////////////////////////////////////////////////////


// Generates parameters for withdraw method
int BitfinexAPI::
parseWDconfParams(string &params)
{
    
    using std::map;
    using std::getline;
    using std::ifstream;
    using std::regex;
    using std::smatch;
    using std::regex_search;
    
    string line;
    ifstream inFile;
    map<string, string> mParams;
    inFile.open(WDconfFilePath);
    regex rgx("^(.*)\\b\\s*=\\s*(\"{0,1}.*\"{0,1})$");
    smatch match;
    
    // Create map with parameters
    while (getline(inFile, line)) {
        
        // Skip comments, blank lines ...
        if (isalpha(line[0]))
        {
            // ... and keys with empty values
            if (regex_search(line, match, rgx) && match[2] != "\"\"")
                mParams.emplace(match[1], match[2]);
        }
        
    }
    
    // Check parameters
    if (!mParams.count("withdraw_type") ||
        !mParams.count("walletselected") ||
        !mParams.count("amount"))
    {
        return requiredParamsMissing;
    }
    
    if (mParams["withdraw_type"] == "wire")
    {
        if (!mParams.count("account_number") ||
            !mParams.count("bank_name") ||
            !mParams.count("bank_address") ||
            !mParams.count("bank_city") ||
            !mParams.count("bank_country"))
        {
            return wireParamsMissing;
        }
    }
    else if (inArray(mParams["withdraw_type"], methods))
    {
        if(!mParams.count("address"))
        {
            return addressParamsMissing;
        }
    }
    
    // Create JSON string
    
    for (const auto &param : mParams)
    {
        params += ",\"";
        params += param.first;
        params += "\":";
        params += param.second;
    }
    
    return 0;
    
}


// bool to string cast
string BitfinexAPI::
bool2string(const bool &in)
{
    return in ? "true" : "false";
}


// Tonce
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
    using std::transform;
    
    SecByteBlock byteKey((const byte*)key.data(), key.size());
    string mac;
    digest.clear();
    
    HMAC<CryptoPP::SHA384> hmac(byteKey, byteKey.size());
    StringSource ss1(content, true, new HashFilter(hmac, new StringSink(mac)));
    StringSource ss2(mac, true, new HexEncoder(new StringSink(digest)));
    transform(digest.begin(), digest.end(), digest.begin(), ::tolower);
    
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
