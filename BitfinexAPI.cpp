
//////////////////////////////////////////////////////////////////////////////
//                                                                          
//  BitfinexAPI.cpp
//
//
//  Bitfinex REST API C++ client
//
//
//  Copyright (C) 2017      Petr Javorik    maple@mmquant.net
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//////////////////////////////////////////////////////////////////////////////



#include "BitfinexAPI.hpp"
#include <iostream>
#include <utility>
#include <map>
#include <fstream>
#include <regex>
#include <chrono>

#include <cryptopp/hmac.h>
#include <cryptopp/osrng.h>
#include <cryptopp/hex.h>
#include <cryptopp/base64.h>



using std::cout;
using std::vector;
using std::to_string;


// CRYPTOPP_NO_GLOBAL_BYTE signals byte is at CryptoPP::byte
#if defined(CRYPTOPP_NO_GLOBAL_BYTE)
    using CryptoPP::byte;
#endif


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
getTicker(string &result, const string &symbol)
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
getStats(string &result, const string &symbol)
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
getFundingBook(string &result, const string &currency, const int &limit_bids, const int &limit_asks)
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
getOrderBook(string &result, const string &symbol, const int &limit_bids, const int &limit_asks, const bool &group)
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
getTrades(string &result, const string &symbol, const time_t &since, const int &limit_trades)
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
getLends(string &result, const string &currency, const time_t &since, const int &limit_lends)
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
deposit(string &result, const string &method, const string &walletType, const bool &renew)
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
transfer(string &result, const double &amount, const string &currency, const string &walletfrom,
         const string &walletto)
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
newOrder(string &result, const string &symbol, const double &amount,
         const double &price, const string &side, const string &type,
         const bool &is_hidden, const bool &is_postonly,
         const bool &use_all_available, const bool &ocoorder,
         const double &buy_price_oco)
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
newOrders(string &result, const vOrders &orders)
{
    
    string endPoint = "/order/new/multi/";
    string params = "{\"request\":\"/v1/order/new/multi\",\"nonce\":\"" + getTonce() + "\"";
    
    // Get pointer to last element in orders. We will not place
    // ',' character at the end of the last loop.
    auto &last = *(--orders.end());
    
    params += ",\"payload\":[";
    for (const auto &order : orders)
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


int BitfinexAPI::
cancelOrder(string &result, const long long &order_id)
{
    
    string endPoint = "/order/cancel/";
    string params = "{\"request\":\"/v1/order/cancel\",\"nonce\":\"" + getTonce() + "\"";
    
    params += ",\"order_id\":" + to_string(order_id);
    
    params += "}";
    return DoPOSTrequest(endPoint, params, result);
    
}


int BitfinexAPI::
cancelOrders(string &result, const vIds &vOrder_ids)
{
    
    string endPoint = "/order/cancel/multi/";
    string params = "{\"request\":\"/v1/order/cancel/multi\",\"nonce\":\"" + getTonce() + "\"";
    
    // Get pointer to last element in vOrders. We will not place
    // ',' character at the end of the last loop.
    auto &last = *(--vOrder_ids.end());
    
    params += ", \"order_ids\":[";
    for (const auto &order_id : vOrder_ids)
    {
        params += to_string(order_id);
        if (&order_id != &last)
        {
            params += ",";
        }
    }
    params += "]";
    
    params += "}";
    return DoPOSTrequest(endPoint, params, result);
    
}


int BitfinexAPI::
cancelAllOrders(string &result)
{
    
    string endPoint = "/order/cancel/all/";
    string params = "{\"request\":\"/v1/order/cancel/all\",\"nonce\":\"" + getTonce() + "\"";
    
    params += "}";
    return DoPOSTrequest(endPoint, params, result);
    
}


int BitfinexAPI::
replaceOrder(string &result, const long long &order_id, const string &symbol,
                 const double &amount, const double &price, const string &side,
                 const string &type, const bool &is_hidden,
                 const bool &use_remaining)
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
    
    string endPoint = "/order/cancel/replace/";
    string params = "{\"request\":\"/v1/order/cancel/replace\",\"nonce\":\"" + getTonce() + "\"";
    
    params += ",\"order_id\":" + to_string(order_id);
    params += ",\"symbol\":\"" + symbol + "\"";
    params += ",\"amount\":\"" + to_string(amount) + "\"";
    params += ",\"price\":\"" + to_string(price) + "\"";
    params += ",\"side\":\"" + side + "\"";
    params += ",\"type\":\"" + type + "\"";
    params += ",\"is_hidden\":" + bool2string(is_hidden);
    params += ",\"use_all_available\":" + bool2string(use_remaining);
    
    params += "}";
    return DoPOSTrequest(endPoint, params, result);
    
}


int BitfinexAPI::
getOrderStatus(string &result, const long long &order_id)
{
    
    string endPoint = "/order/status/";
    string params = "{\"request\":\"/v1/order/status\",\"nonce\":\"" + getTonce() + "\"";
    
    params += ",\"order_id\":" + to_string(order_id);
    
    params += "}";
    return DoPOSTrequest(endPoint, params, result);
    
}


int BitfinexAPI::
getActiveOrders(string &result)
{
    
    string endPoint = "/orders/";
    string params = "{\"request\":\"/v1/orders\",\"nonce\":\"" + getTonce() + "\"";
    
    params += "}";
    return DoPOSTrequest(endPoint, params, result);
    
}


int BitfinexAPI::
getActivePositions(string &result)
{
    
    string endPoint = "/positions/";
    string params = "{\"request\":\"/v1/positions\",\"nonce\":\"" + getTonce() + "\"";
    
    params += "}";
    return DoPOSTrequest(endPoint, params, result);
    
}


int BitfinexAPI::
claimPosition(string &result, long long &position_id, const double &amount)
{
    
    string endPoint = "/position/claim/";
    string params = "{\"request\":\"/v1/position/claim\",\"nonce\":\"" + getTonce() + "\"";
    
    params += ",\"position_id\":" + to_string(position_id);
    params += ",\"amount\":\"" + to_string(amount) + "\"";
    
    params += "}";
    return DoPOSTrequest(endPoint, params, result);
    
}


int BitfinexAPI::
getBalanceHistory(string &result, const string &currency, const time_t &since,
                      const time_t &until, const int &limit, const string &walletType)
{
    
    // Is currency valid ?
    if(!inArray(currency, currencies))
    {
        return badCurrency;
    }
    // Is wallet type valid ?
    // Modified condition which accepts "all" value for all wallets balances together.
    if(!inArray(walletType, walletTypes) && walletType != "all")
    {
        return badWalletType;
    }
    
    string endPoint = "/history/";
    string params = "{\"request\":\"/v1/history\",\"nonce\":\"" + getTonce() + "\"";
    
    params += ",\"currency\":\"" + currency + "\"";
    params += ",\"since\":\"" + to_string(since) + "\"";
    params += ",\"until\":\"" + (!until ? getTonce() : to_string(until)) + "\"";
    params += ",\"limit\":" + to_string(limit);
    if (walletType != "all")
    {
        params += ",\"wallet\":\"" + walletType + "\"";
    }
    
    params += "}";
    return DoPOSTrequest(endPoint, params, result);
    
}


int BitfinexAPI::
getDWHistory(string &result, const string &currency, const string &method,
                 const time_t &since, const time_t &until, const int &limit)
{
    
    // Is currency valid ?
    if(!inArray(currency, currencies))
    {
        return badCurrency;
    }
    // Is deposit method valid ?
    if(!inArray(method, methods) && method != "wire" && method != "all")
    {
        return badDepositMethod;
    }
    
    string endPoint = "/history/movements/";
    string params = "{\"request\":\"/v1/history/movements\",\"nonce\":\"" + getTonce() + "\"";
    
    params += ",\"currency\":\"" + currency + "\"";
    if (method != "all")
    {
        params += ",\"method\":\"" + method + "\"";
    }
    params += ",\"since\":\"" + to_string(since) + "\"";
    params += ",\"until\":\"" + (!until ? getTonce() : to_string(until)) + "\"";
    params += ",\"limit\":" + to_string(limit);
    
    params += "}";
    return DoPOSTrequest(endPoint, params, result);
    
}


int BitfinexAPI::
getPastTrades(string &result, const string &symbol, const time_t &timestamp,
                  const time_t &until, const int &limit_trades, const bool reverse)
{

    // Is symbol valid ?
    if(!inArray(symbol, symbols))
    {
        return badSymbol;
    }
    
    string endPoint = "/mytrades/";
    string params = "{\"request\":\"/v1/mytrades\",\"nonce\":\"" + getTonce() + "\"";
    
    params += ",\"symbol\":\"" + symbol + "\"";
    params += ",\"timestamp\":\"" + to_string(timestamp) + "\"";
    params += ",\"until\":\"" + (!until ? getTonce() : to_string(until)) + "\"";
    params += ",\"limit_trades\":" + to_string(limit_trades);
    params += ",\"reverse\":" + to_string(reverse);
    
    params += "}";
    return DoPOSTrequest(endPoint, params, result);
    
}


int BitfinexAPI::
newOffer(string &result, const string &currency, const double &amount,
             const float &rate, const int &period, const string &direction)
{
 
    // Is currency valid ?
    if(!inArray(currency, currencies))
    {
        return badCurrency;
    }
    
    string endPoint = "/offer/new/";
    string params = "{\"request\":\"/v1/offer/new\",\"nonce\":\"" + getTonce() + "\"";
    
    params += ",\"currency\":\"" + currency + "\"";
    params += ",\"amount\":\"" + to_string(amount) + "\"";
    params += ",\"rate\":\"" + to_string(rate) + "\"";
    params += ",\"period\":" + to_string(period);
    params += ",\"direction\":\"" + direction + "\"";
    
    params += "}";
    return DoPOSTrequest(endPoint, params, result);
    
}


int BitfinexAPI::
cancelOffer(string &result, const long long &offer_id)
{

    string endPoint = "/offer/cancel/";
    string params = "{\"request\":\"/v1/offer/cancel\",\"nonce\":\"" + getTonce() + "\"";
    
    params += ",\"offer_id\":" + to_string(offer_id);
    
    params += "}";
    return DoPOSTrequest(endPoint, params, result);
    
}


int BitfinexAPI::
getOfferStatus(string &result, const long long &offer_id)
{
    
    string endPoint = "/offer/status/";
    string params = "{\"request\":\"/v1/offer/status\",\"nonce\":\"" + getTonce() + "\"";
    
    params += ",\"offer_id\":" + to_string(offer_id);
    
    params += "}";
    return DoPOSTrequest(endPoint, params, result);
    
}


int BitfinexAPI::
getActiveCredits(string &result)
{
    
    string endPoint = "/credits/";
    string params = "{\"request\":\"/v1/credits\",\"nonce\":\"" + getTonce() + "\"";
    
    params += "}";
    return DoPOSTrequest(endPoint, params, result);
    
}


int BitfinexAPI::
getOffers(string &result)
{
    
    string endPoint = "/offers/";
    string params = "{\"request\":\"/v1/offers\",\"nonce\":\"" + getTonce() + "\"";
    
    params += "}";
    return DoPOSTrequest(endPoint, params, result);
    
}


int BitfinexAPI::
getTakenFunds(string &result)
{
    
    string endPoint = "/taken_funds/";
    string params = "{\"request\":\"/v1/taken_funds\",\"nonce\":\"" + getTonce() + "\"";
    
    params += "}";
    return DoPOSTrequest(endPoint, params, result);
    
}


int BitfinexAPI::
getUnusedTakenFunds(string &result)
{
    
    string endPoint = "/unused_taken_funds/";
    string params = "{\"request\":\"/v1/unused_taken_funds\",\"nonce\":\"" + getTonce() + "\"";
    
    params += "}";
    return DoPOSTrequest(endPoint, params, result);
    
}


int BitfinexAPI::
getTotalTakenFunds(string &result)
{
    
    string endPoint = "/total_taken_funds/";
    string params = "{\"request\":\"/v1/total_taken_funds\",\"nonce\":\"" + getTonce() + "\"";
    
    params += "}";
    return DoPOSTrequest(endPoint, params, result);
    
}


int BitfinexAPI::
closeLoan(string &result, const long long &swap_id)
{
    
    string endPoint = "/funding/close/";
    string params = "{\"request\":\"/v1/funding/close\",\"nonce\":\"" + getTonce() + "\"";
    
    params += ",\"swap_id\":" + to_string(swap_id);
    
    params += "}";
    return DoPOSTrequest(endPoint, params, result);
    
}



//////////////////////////////////////////////////////////////////////////////
// Utility private methods
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

    using namespace std::chrono;
    
    milliseconds ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch());

    return to_string(ms.count());
    
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
            cout << "Libcurl error in DoGETrequest(), code:\n";
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
            cout << "Libcurl error in DoPOSTrequest(), code:\n";
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
