//////////////////////////////////////////////////////////////////////////////
//                                                                          
//  BitfinexAPI.hpp
//
//
//  Bitfinex REST API C++ client
//
//
//  Copyright (C) 2018      Petr Javorik    maple@mmquant.net
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

#pragma once

// std
#include <chrono>
#include <fstream>
#include <iostream>
#include <iostream>
#include <map>
#include <regex>
#include <unordered_set>
#include <utility>
#include <vector>

// curl
#include <curl/curl.h>

// cryptopp
#include <cryptopp/base64.h>
#include <cryptopp/hex.h>
#include <cryptopp/hmac.h>
#include <cryptopp/osrng.h>

// internal jsonutils
#include "jsonutils.hpp"


using std::cout;
using std::string;
using std::to_string;
using std::unordered_set;
using std::vector;


// CRYPTOPP_NO_GLOBAL_BYTE signals byte is at CryptoPP::byte
#if defined(CRYPTOPP_NO_GLOBAL_BYTE)
using CryptoPP::byte;
#endif


class BitfinexAPI
{
public:
    
    //////////////////////////////////////////////////////////////////////////////
    // Enumerations
    //////////////////////////////////////////////////////////////////////////////
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
    
    //////////////////////////////////////////////////////////////////////////////
    // Typedefs
    //////////////////////////////////////////////////////////////////////////////
    
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
    
    typedef vector<long long> vIds;
    
    //////////////////////////////////////////////////////////////////////////////
    // Constructor - destructor
    //////////////////////////////////////////////////////////////////////////////
    
    explicit BitfinexAPI():BitfinexAPI("", "") {}
    
    explicit BitfinexAPI(const string &accessKey, const string &secretKey):
    _accessKey(accessKey),
    _secretKey(secretKey),
    _WDconfFilePath("doc/withdraw.conf"),
    _APIurl("https://api.bitfinex.com/v1"),
    _curl(curl_easy_init())
    {
        string temp;
        getSymbols(temp);
        jsonutils::arrayToUset(_symbols, temp);
        
        _currencies =
        {
            "BTG",
            "DSH",
            "ETC",
            "ETP",
            "EUR",
            "GBP",
            "IOT",
            "JPY",
            "LTC",
            "NEO",
            "OMG",
            "SAN",
            "USD",
            "XMR",
            "XRP",
            "ZEC"
        };
        // As found on https://bitfinex.readme.io/v1/reference#rest-auth-deposit
        _methods =
        {
            "bcash"
            "bitcoin",
            "ethereum",
            "ethereumc",
            "ethereumc",
            "litecoin",
            "mastercoin",
            "monero",
            "tetheruso",
            "zcash",
        };
        _walletNames =
        {
            "trading", "exchange", "deposit"
        };
        // New order endpoint "type" parameter
        _types =
        {
            "market",
            "limit",
            "stop",
            "trailing-stop",
            "fill-or-kill",
            "exchange market",
            "exchange limit",
            "exchange stop",
            "exchange trailing-stop",
            "exchange fill-or-kill"
        };
    }
    
    ~BitfinexAPI()
    {
        curl_easy_cleanup(_curl);
    }
    
    //////////////////////////////////////////////////////////////////////////////
    // Accessors
    //////////////////////////////////////////////////////////////////////////////
    
    string getWDconfFilePath() const { return _WDconfFilePath; }
    
    void setWDconfFilePath(const string &path) { _WDconfFilePath = path; }
    
    void setKeys(const string &accessKey, const string &secretKey)
    {
        this->_accessKey = accessKey;
        this->_secretKey = secretKey;
    }
    
    //////////////////////////////////////////////////////////////////////////////
    // Public endpoints
    //////////////////////////////////////////////////////////////////////////////
    
    int getTicker(string &result, const string &symbol)
    {
        // Is symbol valid ?
        if(!inArray(symbol, _symbols))
        {
            return badSymbol;
        }
        
        return DoGETrequest("/pubticker/" + symbol, "", result);
    };
    
    int getStats(string &result, const string &symbol)
    {
        // Is symbol valid ?
        if(!inArray(symbol, _symbols))
        {
            return badSymbol;
        }
        
        return DoGETrequest("/stats/" + symbol, "", result);
    };
    
    int getFundingBook(string &result,
                       const string &currency,
                       const int &limit_bids = 50,
                       const int &limit_asks = 50)
    {
        // Is currency valid ?
        if(!inArray(currency, _currencies))
        {
            return badCurrency;
        }
        
        string params =
        "?limit_bids=" + to_string(limit_bids) +
        "&limit_asks=" + to_string(limit_asks);
        return DoGETrequest("/lendbook/" + currency, params, result);
    };
    
    int getOrderBook(string &result,
                     const string &symbol,
                     const int &limit_bids = 50,
                     const int &limit_asks = 50,
                     const bool &group = 1)
    {
        // Is symbol valid ?
        if(!inArray(symbol, _symbols))
        {
            return badSymbol;
        }
        
        string params =
        "?limit_bids=" + to_string(limit_bids) +
        "&limit_asks=" + to_string(limit_asks) +
        "&group=" + to_string(group);
        return DoGETrequest("/book/" + symbol, params, result);
    };
    
    int getTrades(string &result,
                  const string &symbol,
                  const time_t &since = 0,
                  const int &limit_trades = 50)
    {
        // Is symbol valid ?
        if(!inArray(symbol, _symbols))
        {
            return badSymbol;
        }
        
        string params =
        "?timestamp=" + to_string(since) +
        "&limit_trades=" + to_string(limit_trades);
        return DoGETrequest("/trades/" + symbol, params, result);
    };
    
    int getLends(string &result,
                 const string &currency,
                 const time_t &since = 0,
                 const int &limit_lends = 50)
    {
        // Is currency valid ?
        if(!inArray(currency, _currencies))
        {
            return badCurrency;
        }
        
        string params =
        "?timestamp=" + to_string(since) +
        "&limit_lends=" + to_string(limit_lends);
        return DoGETrequest("/lends/" + currency, params, result);
    };
    
    int getSymbols(string &result)
    {
        return DoGETrequest("/symbols/", "", result);
    };
    
    int getSymbolDetails(string &result)
    {
        return DoGETrequest("/symbols_details/", "", result);
    };
    
    //////////////////////////////////////////////////////////////////////////////
    // Authenticated endpoints
    //////////////////////////////////////////////////////////////////////////////
    
    //  Account
    int getAccountInfo(string &result)
    {
        string params = "{\"request\":\"/v1/account_infos\",\"nonce\":\"" + getTonce() + "\"";
        params += "}";
        return DoPOSTrequest("/account_infos/", params, result);
    };
    
    int getAccountFees(string &result)
    {
        string params = "{\"request\":\"/v1/account_fees\",\"nonce\":\"" + getTonce() + "\"";
        params += "}";
        return DoPOSTrequest("/account_fees/", params, result);
    };
    
    int getSummary(string &result)
    {
        string params = "{\"request\":\"/v1/summary\",\"nonce\":\"" + getTonce() + "\"";
        params += "}";
        return DoPOSTrequest("/summary/", params, result);
    };
    
    int deposit(string &result,
                const string &method,
                const string &walletName,
                const bool &renew = 0)
    {
        // Is deposit method valid ?
        if(!inArray(method, _methods))
        {
            return badDepositMethod;
        }
        // Is walletType valid ?
        if(!inArray(walletName, _walletNames))
        {
            return badWalletType;
        }
        
        string params = "{\"request\":\"/v1/deposit/new\",\"nonce\":\"" + getTonce() + "\"";
        params += ",\"method\":\"" + method + "\"";
        params += ",\"wallet_name\":\"" + walletName + "\"";
        params += ",\"renew\":" + to_string(renew);
        params += "}";
        return DoPOSTrequest("/deposit/new/", params, result);
    };
    
    int getKeyPermissions(string &result)
    {
        string params = "{\"request\":\"/v1/key_info\",\"nonce\":\"" + getTonce() + "\"";
        params += "}";
        return DoPOSTrequest("/key_info/", params, result);
    };
    
    int getMarginInfos(string &result)
    {
        string params = "{\"request\":\"/v1/margin_infos\",\"nonce\":\"" + getTonce() + "\"";
        params += "}";
        return DoPOSTrequest("/margin_infos/", params, result);
    };
    
    int getBalances(string &result)
    {
        string params = "{\"request\":\"/v1/balances\",\"nonce\":\"" + getTonce() + "\"";
        params += "}";
        return DoPOSTrequest("/balances/", params, result);
    };
    
    int transfer(string &result,
                 const double &amount,
                 const string &currency,
                 const string &walletfrom,
                 const string &walletto)
    {
        // Is currency valid ?
        if(!inArray(currency, _currencies))
        {
            return badCurrency;
        }
        // Is walletType valid ?
        if(!inArray(walletfrom, _walletNames) || !inArray(walletto, _walletNames))
        {
            return badWalletType;
        }
        
        string params = "{\"request\":\"/v1/transfer\",\"nonce\":\"" + getTonce() + "\"";
        params += ",\"amount\":\"" + to_string(amount) + "\"";
        params += ",\"currency\":\"" + currency + "\"";
        params += ",\"walletfrom\":\"" + walletfrom + "\"";
        params += ",\"walletto\":\"" + walletto + "\"";
        params += "}";
        return DoPOSTrequest("/transfer/", params, result);
    };
    
    int withdraw(string &result) // configure withdraw.conf file before use
    {
        string params = "{\"request\":\"/v1/withdraw\",\"nonce\":\"" + getTonce() + "\"";
        
        // Add params from withdraw.conf
        int err = parseWDconfParams(params);
        if (err != 0) { return err; };
        
        params += "}";
        return DoPOSTrequest("/withdraw/", params, result);
    };
    
    //  Orders
    int newOrder(string &result,
                 const string &symbol,
                 const double &amount,
                 const double &price,
                 const string &side,
                 const string &type,
                 const bool &is_hidden = 0,
                 const bool &is_postonly = 0,
                 const bool &use_all_available = 0,
                 const bool &ocoorder = 0,
                 const double &buy_price_oco = 0)
    {
        // Is symbol valid ?
        if(!inArray(symbol, _symbols))
        {
            return badSymbol;
        }
        // Is order type valid ?
        if(!inArray(type, _types))
        {
            return badOrderType;
        }
        
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
        return DoPOSTrequest("/order/new/", params, result);
    };
    
    int newOrders(string &result, const vOrders &orders)
    {
        string params = "{\"request\":\"/v1/order/new/multi\",\"nonce\":\"" + getTonce() + "\"";
        
        // Get pointer to last element in orders. We will not place
        // ',' character at the end of the last loop.
        auto &last = *(--orders.cend());
        
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
        return DoPOSTrequest("/order/new/multi/", params, result);
    };
    
    int cancelOrder(string &result, const long long &order_id)
    {
        string params = "{\"request\":\"/v1/order/cancel\",\"nonce\":\"" + getTonce() + "\"";
        params += ",\"order_id\":" + to_string(order_id);
        params += "}";
        return DoPOSTrequest("/order/cancel/", params, result);
    };
    
    int cancelOrders(string &result, const vIds &vOrder_ids)
    {
        string params = "{\"request\":\"/v1/order/cancel/multi\",\"nonce\":\"" + getTonce() + "\"";
        
        // Get pointer to last element in vOrders. We will not place
        // ',' character at the end of the last loop.
        auto &last = *(--vOrder_ids.cend());
        
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
        return DoPOSTrequest("/order/cancel/multi/", params, result);
    };
    
    int cancelAllOrders(string &result)
    {
        string params = "{\"request\":\"/v1/order/cancel/all\",\"nonce\":\"" + getTonce() + "\"";
        params += "}";
        return DoPOSTrequest("/order/cancel/all/", params, result);
    };
    
    int replaceOrder(string &result,
                     const long long &order_id,
                     const string &symbol,
                     const double &amount,
                     const double &price,
                     const string &side,
                     const string &type,
                     const bool &is_hidden = 0,
                     const bool &use_remaining = 0)
    {
        // Is symbol valid ?
        if(!inArray(symbol, _symbols))
        {
            return badSymbol;
        }
        // Is order type valid ?
        if(!inArray(type, _types))
        {
            return badOrderType;
        }
        
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
        return DoPOSTrequest("/order/cancel/replace/", params, result);
    };
    
    int getOrderStatus(string &result, const long long &order_id)
    {
        string params = "{\"request\":\"/v1/order/status\",\"nonce\":\"" + getTonce() + "\"";
        params += ",\"order_id\":" + to_string(order_id);
        params += "}";
        return DoPOSTrequest("/order/status/", params, result);
    };
    
    int getActiveOrders(string &result)
    {
        string params = "{\"request\":\"/v1/orders\",\"nonce\":\"" + getTonce() + "\"";
        params += "}";
        return DoPOSTrequest("/orders/", params, result);
    };
    
    int getOrdersHistory(string &result, const int &limit = 50)
    {
        string params = "{\"request\":\"/v1/orders/hist\",\"nonce\":\"" + getTonce() + "\"";
        params += ",\"limit\":" + to_string(limit);
        params += "}";
        return DoPOSTrequest("/orders/hist/", params, result);
    };
    
    
    //  Positions
    int getActivePositions(string &result)
    {
        string params = "{\"request\":\"/v1/positions\",\"nonce\":\"" + getTonce() + "\"";
        params += "}";
        return DoPOSTrequest("/positions/", params, result);
    };
    
    int claimPosition(string &result,
                      long long &position_id,
                      const double &amount)
    {
        string params = "{\"request\":\"/v1/position/claim\",\"nonce\":\"" + getTonce() + "\"";
        params += ",\"position_id\":" + to_string(position_id);
        params += ",\"amount\":\"" + to_string(amount) + "\"";
        params += "}";
        return DoPOSTrequest("/position/claim/", params, result);
    };
    
    //  Historical data
    int getBalanceHistory(string &result,
                          const string &currency,
                          const time_t &since = 0,
                          const time_t &until = 0,
                          const int &limit = 500,
                          const string &walletType = "all")
    {
        // Is currency valid ?
        if(!inArray(currency, _currencies))
        {
            return badCurrency;
        }
        // Is wallet type valid ?
        // Modified condition which accepts "all" value for all wallets balances together.
        if(!inArray(walletType, _walletNames) && walletType != "all")
        {
            return badWalletType;
        }
        
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
        return DoPOSTrequest("/history/", params, result);
    };
    
    int getWithdrawalHistory(string &result,
                     const string &currency,
                     const string &method = "all",
                     const time_t &since = 0,
                     const time_t &until = 0,
                     const int &limit = 500)
    {
        // Is currency valid ?
        if(!inArray(currency, _currencies))
        {
            return badCurrency;
        }
        // Is deposit method valid ?
        if(!inArray(method, _methods) && method != "wire" && method != "all")
        {
            return badDepositMethod;
        }
        
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
        return DoPOSTrequest("/history/movements/", params, result);
    };
    
    int getPastTrades(string &result,
                      const string &symbol,
                      const time_t &timestamp,
                      const time_t &until = 0,
                      const int &limit_trades = 500,
                      const bool reverse = 0)
    {
        // Is symbol valid ?
        if(!inArray(symbol, _symbols))
        {
            return badSymbol;
        }
        
        string params = "{\"request\":\"/v1/mytrades\",\"nonce\":\"" + getTonce() + "\"";
        params += ",\"symbol\":\"" + symbol + "\"";
        params += ",\"timestamp\":\"" + to_string(timestamp) + "\"";
        params += ",\"until\":\"" + (!until ? getTonce() : to_string(until)) + "\"";
        params += ",\"limit_trades\":" + to_string(limit_trades);
        params += ",\"reverse\":" + to_string(reverse);
        params += "}";
        return DoPOSTrequest("/mytrades/", params, result);
    };
    
    //  Margin funding
    int newOffer(string &result,
                 const string &currency,
                 const double &amount,
                 const float &rate,
                 const int &period,
                 const string &direction)
    {
        // Is currency valid ?
        if(!inArray(currency, _currencies))
        {
            return badCurrency;
        }
        
        string params = "{\"request\":\"/v1/offer/new\",\"nonce\":\"" + getTonce() + "\"";
        params += ",\"currency\":\"" + currency + "\"";
        params += ",\"amount\":\"" + to_string(amount) + "\"";
        params += ",\"rate\":\"" + to_string(rate) + "\"";
        params += ",\"period\":" + to_string(period);
        params += ",\"direction\":\"" + direction + "\"";
        params += "}";
        return DoPOSTrequest("/offer/new/", params, result);
    };
    
    int cancelOffer(string &result, const long long &offer_id)
    {
        string params = "{\"request\":\"/v1/offer/cancel\",\"nonce\":\"" + getTonce() + "\"";
        params += ",\"offer_id\":" + to_string(offer_id);
        params += "}";
        return DoPOSTrequest("/offer/cancel/", params, result);
    };
    
    int getOfferStatus(string &result, const long long &offer_id)
    {
        string params = "{\"request\":\"/v1/offer/status\",\"nonce\":\"" + getTonce() + "\"";
        params += ",\"offer_id\":" + to_string(offer_id);
        params += "}";
        return DoPOSTrequest("/offer/status/", params, result);
    };
    
    int getActiveCredits(string &result)
    {
        string params = "{\"request\":\"/v1/credits\",\"nonce\":\"" + getTonce() + "\"";
        params += "}";
        return DoPOSTrequest("/credits/", params, result);
    };
    
    int getOffers(string &result)
    {
        string params = "{\"request\":\"/v1/offers\",\"nonce\":\"" + getTonce() + "\"";
        params += "}";
        return DoPOSTrequest("/offers/", params, result);
    };
    
    int getOffersHistory(string &result, const int &limit)
    {
        string params = "{\"request\":\"/v1/offers/hist\",\"nonce\":\"" + getTonce() + "\"";
        params += ",\"limit\":" + to_string(limit);
        params += "}";
        return DoPOSTrequest("/offers/hist/", params, result);
    };
    
    // There is ambiguity in the "symbol" parameter value for this call.
    // It should be "currency" not "symbol".
    // Typical values for "symbol" are trading pairs such as "btcusd", "btcltc" ...
    // Typical values for "currency" are "btc", "ltc" ...
    int getPastFundingTrades(string &result,
                             const string &currency,
                             const time_t &until = 0,
                             const int &limit_trades = 50)
    {
        // Is currency valid ?
        if(!inArray(currency, _currencies))
        {
            return badCurrency;
        }
        
        string params = "{\"request\":\"/v1/mytrades_funding\",\"nonce\":\"" + getTonce() + "\"";
        // param inconsistency in BFX API, symbol should be currency
        params += ",\"symbol\":\"" + currency + "\"";
        params += ",\"until\":" + to_string(until);
        params += ",\"limit_trades\":" + to_string(limit_trades);
        params += "}";
        return DoPOSTrequest("/mytrades_funding/", params, result);
    };
    
    int getTakenFunds(string &result)
    {
        string params = "{\"request\":\"/v1/taken_funds\",\"nonce\":\"" + getTonce() + "\"";
        params += "}";
        return DoPOSTrequest("/taken_funds/", params, result);
    };
    
    int getUnusedTakenFunds(string &result)
    {
        string params = "{\"request\":\"/v1/unused_taken_funds\",\"nonce\":\"" + getTonce() + "\"";
        params += "}";
        return DoPOSTrequest("/unused_taken_funds/", params, result);
    };
    
    int getTotalTakenFunds(string &result)
    {
        string params = "{\"request\":\"/v1/total_taken_funds\",\"nonce\":\"" + getTonce() + "\"";
        params += "}";
        return DoPOSTrequest("/total_taken_funds/", params, result);
    };
    
    int closeLoan(string &result, const long long &offer_id)
    {
        string params = "{\"request\":\"/v1/funding/close\",\"nonce\":\"" + getTonce() + "\"";
        params += ",\"swap_id\":" + to_string(offer_id);
        params += "}";
        return DoPOSTrequest("/funding/close/", params, result);
    };
    
    int closePosition(string &result, const long long &position_id)
    {
        string params = "{\"request\":\"/v1/position/close\",\"nonce\":\"" + getTonce() + "\"";
        params += ",\"position_id\":" + to_string(position_id);
        params += "}";
        return DoPOSTrequest("/position/close/", params, result);
    };
    
protected:
    
    // BitfinexAPI object cannot be copied
    BitfinexAPI(const BitfinexAPI&);
    BitfinexAPI& operator = (const BitfinexAPI&);
    
    //////////////////////////////////////////////////////////////////////////////
    // Private attributes
    //////////////////////////////////////////////////////////////////////////////
    
    unordered_set<string> _symbols; // possible symbol pairs
    unordered_set<string> _currencies; // possible currencies
    unordered_set<string> _methods; // possible deposit methods
    unordered_set<string> _walletNames; // possible walletTypes
    unordered_set<string> _types; // possible Types (see new order endpoint)
    string _WDconfFilePath;
    string _APIurl;
    string _accessKey, _secretKey;
    CURL *_curl;
    CURLcode _res;
    
    //////////////////////////////////////////////////////////////////////////////
    // Utility private methods
    //////////////////////////////////////////////////////////////////////////////
    
    int parseWDconfParams(string &params)
    {
        using std::getline;
        using std::ifstream;
        using std::map;
        using std::regex;
        using std::regex_search;
        using std::smatch;
        
        string line;
        ifstream inFile;
        map<string, string> mParams;
        inFile.open(_WDconfFilePath);
        regex rgx("^(.*)\\b\\s*=\\s*(\"{0,1}.*\"{0,1})$");
        smatch match;
        
        // Create map with parameters
        while (getline(inFile, line))
        {
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
        else if (inArray(mParams["withdraw_type"], _methods))
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
    };
    
    int DoGETrequest(const string &UrlEndPoint, const string &params, string &result)
    {
        if(_curl)
        {
            string url = _APIurl + UrlEndPoint + params;
            
            _curl = curl_easy_init();
            curl_easy_setopt(_curl, CURLOPT_TIMEOUT, 30L);
            curl_easy_setopt(_curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(_curl, CURLOPT_WRITEDATA, &result);
            curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            
            _res = curl_easy_perform(_curl);
            
            // libcurl internal error handling
            if (_res != CURLE_OK)
            {
                cout << "Libcurl error in DoGETrequest(), code:\n";
                return _res;
            }
            return _res;
        }
        else
        {
            // curl not properly initialized curl = nullptr
            return curlERR;
        }
    };
    
    int DoPOSTrequest(const string &UrlEndPoint, const string &params, string &result)
    {
        if(_curl)
        {
            string url = _APIurl + UrlEndPoint;
            string payload;
            string signature;
            getBase64(params, payload);
            getHmacSha384(_secretKey, payload, signature);
            
            // Headers
            struct curl_slist *httpHeaders = nullptr;
            httpHeaders = curl_slist_append(httpHeaders,
                                            ("X-BFX-APIKEY:" + _accessKey).c_str());
            httpHeaders = curl_slist_append(httpHeaders,
                                            ("X-BFX-PAYLOAD:" + payload).c_str());
            httpHeaders = curl_slist_append(httpHeaders,
                                            ("X-BFX-SIGNATURE:" + signature).c_str());
            
            _curl = curl_easy_init();
            curl_easy_setopt(_curl, CURLOPT_HTTPHEADER, httpHeaders);
            curl_easy_setopt(_curl, CURLOPT_POST, 1);
            curl_easy_setopt(_curl, CURLOPT_POSTFIELDS, "\n");
            curl_easy_setopt(_curl, CURLOPT_TIMEOUT, 30L);
            curl_easy_setopt(_curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(_curl, CURLOPT_VERBOSE, 0L); // debug option
            curl_easy_setopt(_curl, CURLOPT_WRITEDATA, &result);
            curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            
            _res = curl_easy_perform(_curl);
            
            // libcurl internal error handling
            if (_res != CURLE_OK)
            {
                cout << "Libcurl error in DoPOSTrequest(), code:\n";
                return _res;
            }
            return _res;
            
        }
        else
        {
            // curl not properly initialized curl = NULL
            return curlERR;
        }
    };
    
    //////////////////////////////////////////////////////////////////////////////
    // Utility private static methods
    //////////////////////////////////////////////////////////////////////////////
    
    static string bool2string(const bool &in) { return in ? "true" : "false"; };
    
    static string getTonce()
    {
        using namespace std::chrono;
        
        milliseconds ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
        
        return to_string(ms.count());
    };
    
    static int getBase64(const string &content, string &encoded)
    {
        using CryptoPP::Base64Encoder;
        using CryptoPP::StringSink;
        using CryptoPP::StringSource;
        
        byte buffer[1024] = {};
        
        for (int i = 0; i < content.length(); ++i)
        {
            buffer[i] = content[i];
        };
        
        StringSource ss(buffer,
                        content.length(),
                        true,
                        new Base64Encoder(new StringSink(encoded), false));
        
        return 0;
    };
    
    static int getHmacSha384(const string &key, const string &content, string &digest)
    {
        using CryptoPP::HashFilter;
        using CryptoPP::HexEncoder;
        using CryptoPP::HMAC;
        using CryptoPP::SecByteBlock;
        using CryptoPP::StringSink;
        using CryptoPP::StringSource;
        using CryptoPP::SHA384;
        using std::transform;
        
        SecByteBlock byteKey((const byte*)key.data(), key.size());
        string mac;
        digest.clear();
        
        HMAC<SHA384> hmac(byteKey, byteKey.size());
        StringSource ss1(content, true, new HashFilter(hmac, new StringSink(mac)));
        StringSource ss2(mac, true, new HexEncoder(new StringSink(digest)));
        transform(digest.cbegin(), digest.cend(), digest.begin(), ::tolower);
        
        return 0;
    };
    
    // Curl write callback function. Appends fetched *content to *userp pointer.
    // *userp pointer is set up by curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result) line.
    // In this case *userp will point to result.
    static size_t WriteCallback(void *response, size_t size, size_t nmemb, void *userp)
    {
        (static_cast<string*>(userp))->append(static_cast<char*>(response));
        return size * nmemb;
    };
    
    static bool inArray(const string &value, const unordered_set<string> &inputSet)
    {
        return (inputSet.find(value) != inputSet.cend()) ? true : false;
    };
};
