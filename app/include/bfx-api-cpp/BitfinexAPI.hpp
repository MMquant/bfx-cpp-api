////////////////////////////////////////////////////////////////////////////////
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
////////////////////////////////////////////////////////////////////////////////

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

// internal jsonutils
#include "jsonutils.hpp"

// internal error
#include "error.hpp"

// internal TRequest
#include "TRequest.hpp"

// namespaces
using std::cerr;
using std::cout;
using std::endl;
using std::string;
using std::to_string;
using std::unordered_set;
using std::vector;

namespace BfxAPI
{

    class BitfinexAPI
    {

        ////////////////////////////////////////////////////////////////////////
        // Class constants
        ////////////////////////////////////////////////////////////////////////

        static constexpr auto API_URL = "https://api.bitfinex.com/v1";
        #ifndef WITHDRAWAL_CONF_FILE_PATH
        static constexpr auto WITHDRAWAL_CONF_FILE_PATH = "withdraw.conf";
        #endif

        ////////////////////////////////////////////////////////////////////////
        // Typedefs
        ////////////////////////////////////////////////////////////////////////

        // Structure for multiple new orders endpoint
        struct sOrder
        {
            string symbol;
            double amount;
            double price;
            string side;
            string type;
        };
        using vOrders = vector<sOrder>;
        using vIds = vector<long long>;

    public:

        ////////////////////////////////////////////////////////////////////////
        // Constructor - Destructor
        ////////////////////////////////////////////////////////////////////////

        explicit BitfinexAPI():BitfinexAPI("", "") {}

        explicit BitfinexAPI(const string &accessKey, const string &secretKey):
        WDconfFilePath_(WITHDRAWAL_CONF_FILE_PATH),
        Request(API_URL),
        bfxApiStatusCode_(noError)
        {
            // Internal TRequest set Keys
            Request.setAccessKey(accessKey);
            Request.setSecretKey(secretKey);

            // populate _symbols directly from Bitfinex getSymbols endpoint
            jsonutils::jsonStrToUset(symbols_, getSymbols().strResponse());

            currencies_ =
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

            schemaValidator_ = jsonutils::BfxSchemaValidator(symbols_, currencies_);

            // As found on
            // https://bitfinex.readme.io/v1/reference#rest-auth-deposit
            methods_ =
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

            walletNames_ =
            {
                "trading", "exchange", "deposit"
            };

            // New order endpoint "type" parameter
            types_ =
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

        // BitfinexAPI object cannot be
        // copied
        BitfinexAPI(const BitfinexAPI&) = delete;
        BitfinexAPI& operator = (const BitfinexAPI&) = delete;
        // moved
        BitfinexAPI(BitfinexAPI&&) = delete;
        BitfinexAPI& operator = (BitfinexAPI&&) = delete;

        ~BitfinexAPI() { }

        ////////////////////////////////////////////////////////////////////////
        // Accessors
        ////////////////////////////////////////////////////////////////////////

        // Getters
        const string getWDconfFilePath() const noexcept
        { return WDconfFilePath_; }

        const BfxClientErrors& getBfxApiStatusCode() const noexcept
        { return bfxApiStatusCode_; }

        const CURLcode getCurlStatusCode() const noexcept
        { return Request.getLastStatusCode(); }

        const string strResponse() const noexcept
        { return Request.getLastResponse(); }

        bool hasApiError()
        {
            return (checkErrors() != noError || Request.hasError());
        }

        // Setters
        constexpr void setWDconfFilePath(const string &path) noexcept
        { WDconfFilePath_ = path; }

        constexpr void setKeys(const string &accessKey, const string &secretKey) noexcept
        {
            Request.setAccessKey(accessKey);
            Request.setSecretKey(secretKey);
        }

        ////////////////////////////////////////////////////////////////////////
        // Public endpoints
        ////////////////////////////////////////////////////////////////////////

        BitfinexAPI& getTicker(const string &symbol)
        {
            if (!inArray(symbol, symbols_))
                bfxApiStatusCode_ = badSymbol;
            else
                Request.get("/pubticker/" + symbol);

            return *this;
        };

        BitfinexAPI& getStats(const string &symbol)
        {
            if (!inArray(symbol, symbols_))
                bfxApiStatusCode_ = badSymbol;
            else
                Request.get("/stats/" + symbol);

            return *this;
        };

        BitfinexAPI& getFundingBook(const string &currency,
                                    const unsigned &limit_bids = 50,
                                    const unsigned &limit_asks = 50)
        {
            if (!inArray(currency, currencies_))
                bfxApiStatusCode_ = badCurrency;
            else
            {
                map<string, string> params;
                params["limit_bids"] = to_string(limit_bids);
                params["limit_asks"] = to_string(limit_asks);
                Request.get("/lendbook/" + currency, params);
            }

            return *this;
        };

        BitfinexAPI& getOrderBook(const string &symbol,
                                  const unsigned &limit_bids = 50,
                                  const unsigned &limit_asks = 50,
                                  const bool &group = true)
        {
            if (!inArray(symbol, symbols_))
                bfxApiStatusCode_ = badSymbol;
            else
            {
                map<string, string> params;
                params["limit_bids"] = to_string(limit_bids);
                params["limit_asks"] = to_string(limit_asks);
                params["group"]      = to_string(group);
                Request.get("/book/" + symbol, params);
            }

            return *this;
        };

        BitfinexAPI& getTrades(const string &symbol,
                               const time_t &since = 0,
                               const unsigned &limit_trades = 50)
        {
            if (!inArray(symbol, symbols_))
                bfxApiStatusCode_ = badSymbol;
            else
            {
                map<string, string> params;
                params["timestamp"]    = to_string(since);
                params["limit_trades"] = to_string(limit_trades);
                Request.get("/trades/" + symbol, params);
            }

            return *this;
        };

        BitfinexAPI& getLends(const string &currency,
                              const time_t &since = 0,
                              const unsigned &limit_lends = 50)
        {
            if (!inArray(currency, currencies_))
                bfxApiStatusCode_ = badCurrency;
            else
            {
                map<string, string> params;
                params["timestamp"]   = to_string(since);
                params["limit_lends"] = to_string(limit_lends);
                Request.get("/lends/" + currency, params);
            }

            return *this;
        };

        BitfinexAPI& getSymbols()
        {
            Request.get("/symbols/");

            return *this;
        };

        BitfinexAPI& getSymbolsDetails()
        {
            Request.get("/symbols_details/");

            return *this;
        };

        ////////////////////////////////////////////////////////////////////////
        // Authenticated endpoints
        ////////////////////////////////////////////////////////////////////////

        //  Account
        BitfinexAPI& getAccountInfo()
        {
            string params = "{\"request\":\"/v1/account_infos\",\"nonce\":\"" +
            getTonce() + "\"";
            params += "}";
            Request.post("/account_infos/", params);

            return *this;
        };

        BitfinexAPI& getAccountFees()
        {
            string params = "{\"request\":\"/v1/account_fees\",\"nonce\":\"" +
            getTonce() + "\"";
            params += "}";
            Request.post("/account_fees/", params);

            return *this;
        };

        BitfinexAPI& getSummary()
        {
            string params = "{\"request\":\"/v1/summary\",\"nonce\":\"" +
            getTonce() + "\"";
            params += "}";
            Request.post("/summary/", params);

            return *this;
        };

        BitfinexAPI& deposit(const string &method,
                             const string &walletName,
                             const bool &renew = false)
        {
            if (!inArray(method, methods_))
            { bfxApiStatusCode_ = badDepositMethod; return *this; }

            if (!inArray(walletName, walletNames_))
            { bfxApiStatusCode_ = badWalletType; return *this; }

            string params = "{\"request\":\"/v1/deposit/new\",\"nonce\":\"" +
            getTonce() + "\"";
            params += ",\"method\":\"" + method + "\"";
            params += ",\"wallet_name\":\"" + walletName + "\"";
            params += ",\"renew\":" + to_string(renew);
            params += "}";
            Request.post("/deposit/new/", params);

            return *this;
        };

        BitfinexAPI& getKeyPermissions()
        {
            string params = "{\"request\":\"/v1/key_info\",\"nonce\":\"" +
            getTonce() + "\"";
            params += "}";
            Request.post("/key_info/", params);

            return *this;
        };

        BitfinexAPI& getMarginInfos()
        {
            string params = "{\"request\":\"/v1/margin_infos\",\"nonce\":\"" +
            getTonce() + "\"";
            params += "}";
            Request.post("/margin_infos/", params);

            return *this;
        };

        BitfinexAPI& getBalances()
        {
            string params = "{\"request\":\"/v1/balances\",\"nonce\":\"" +
            getTonce() + "\"";
            params += "}";
            Request.post("/balances/", params);

            return *this;
        };

        BitfinexAPI& transfer(const double &amount,
                              const string &currency,
                              const string &walletfrom,
                              const string &walletto)
        {
            if (!inArray(currency, currencies_))
            { bfxApiStatusCode_ = badCurrency; return *this; }

            if (!inArray(walletfrom, walletNames_) ||
                !inArray(walletto, walletNames_))
            { bfxApiStatusCode_ = badWalletType; return *this; }

            string params = "{\"request\":\"/v1/transfer\",\"nonce\":\"" +
            getTonce() + "\"";
            params += ",\"amount\":\"" + to_string(amount) + "\"";
            params += ",\"currency\":\"" + currency + "\"";
            params += ",\"walletfrom\":\"" + walletfrom + "\"";
            params += ",\"walletto\":\"" + walletto + "\"";
            params += "}";
            Request.post("/transfer/", params);

            return *this;
        };

        // configure withdraw.conf file before use
        BitfinexAPI& withdraw()
        {
            string params = "{\"request\":\"/v1/withdraw\",\"nonce\":\"" +
            getTonce() + "\"";

            // Add params from withdraw.conf
            BfxClientErrors code(parseWDconfParams(params));
            if (code != noError)
                bfxApiStatusCode_ = code;
            else
            {
                params += "}";
                Request.post("/withdraw/", params);
            }

            return *this;
        };

        //  Orders
        BitfinexAPI& newOrder(const string &symbol,
                              const double &amount,
                              const double &price,
                              const string &side,
                              const string &type,
                              const bool &is_hidden = false,
                              const bool &is_postonly = false,
                              const bool &use_all_available = false,
                              const bool &ocoorder = false,
                              const double &buy_price_oco = 0)
        {
            if (!inArray(symbol, symbols_))
            { bfxApiStatusCode_ = badSymbol; return *this; };

            if (!inArray(type, types_))
            { bfxApiStatusCode_ = badOrderType; return *this; };

            string params = "{\"request\":\"/v1/order/new\",\"nonce\":\"" +
            getTonce() + "\"";
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

            Request.post("/order/new/", params);
            return *this;
        };

        BitfinexAPI& newOrders(const vOrders &orders)
        {
            string params = "{\"request\":\"/v1/order/new/multi\",\"nonce\":\""
            + getTonce() + "\"";

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
                    params += ",";
            }
            params += "]}";
            Request.post("/order/new/multi/", params);

            return *this;
        };

        BitfinexAPI& cancelOrder(const long long &order_id)
        {
            string params = "{\"request\":\"/v1/order/cancel\",\"nonce\":\"" +
            getTonce() + "\"";
            params += ",\"order_id\":" + to_string(order_id);
            params += "}";
            Request.post("/order/cancel/", params);

            return *this;
        };

        BitfinexAPI& cancelOrders(const vIds &vOrderIds)
        {
            string params = "{\"request\":\"/v1/order/cancel/multi\",\"nonce\":\""
            + getTonce() + "\"";

            // Get pointer to last element in vOrders. We will not place
            // ',' character at the end of the last loop.
            auto &last = *(--vOrderIds.cend());

            params += ", \"order_ids\":[";
            for (const auto &order_id : vOrderIds)
            {
                params += to_string(order_id);
                if (&order_id != &last)
                    params += ",";
            }
            params += "]}";
            Request.post("/order/cancel/multi/", params);

            return *this;
        };

        BitfinexAPI& cancelAllOrders()
        {
            string params = "{\"request\":\"/v1/order/cancel/all\",\"nonce\":\""
            + getTonce() + "\"";
            params += "}";
            Request.post("/order/cancel/all/", params);

            return *this;
        };

        BitfinexAPI& replaceOrder(const long long &order_id,
                                  const string &symbol,
                                  const double &amount,
                                  const double &price,
                                  const string &side,
                                  const string &type,
                                  const bool &is_hidden = false,
                                  const bool &use_remaining = false)
        {
            if (!inArray(symbol, symbols_))
            { bfxApiStatusCode_ = badSymbol; return *this; };

            if (!inArray(type, types_))
            { bfxApiStatusCode_ = badOrderType; return *this; };

            string params = "{\"request\":\"/v1/order/cancel/replace\",\"nonce\":\""
            + getTonce() + "\"";
            params += ",\"order_id\":" + to_string(order_id);
            params += ",\"symbol\":\"" + symbol + "\"";
            params += ",\"amount\":\"" + to_string(amount) + "\"";
            params += ",\"price\":\"" + to_string(price) + "\"";
            params += ",\"side\":\"" + side + "\"";
            params += ",\"type\":\"" + type + "\"";
            params += ",\"is_hidden\":" + bool2string(is_hidden);
            params += ",\"use_all_available\":" + bool2string(use_remaining);
            params += "}";
            Request.post("/order/cancel/replace/", params);

            return *this;
        };

        BitfinexAPI& getOrderStatus(const long long &order_id)
        {
            string params = "{\"request\":\"/v1/order/status\",\"nonce\":\"" +
            getTonce() + "\"";
            params += ",\"order_id\":" + to_string(order_id);
            params += "}";
            Request.post("/order/status/", params);

            return *this;
        };

        BitfinexAPI& getActiveOrders()
        {
            string params = "{\"request\":\"/v1/orders\",\"nonce\":\"" +
            getTonce() + "\"";
            params += "}";
            Request.post("/orders/", params);

            return *this;
        };

        BitfinexAPI& getOrdersHistory(const unsigned &limit = 50)
        {
            string params = "{\"request\":\"/v1/orders/hist\",\"nonce\":\"" +
            getTonce() + "\"";
            params += ",\"limit\":" + to_string(limit);
            params += "}";
            Request.post("/orders/hist/", params);

            return *this;
        };


        //  Positions
        BitfinexAPI& getActivePositions()
        {
            string params = "{\"request\":\"/v1/positions\",\"nonce\":\"" +
            getTonce() + "\"";
            params += "}";
            Request.post("/positions/", params);

            return *this;
        };

        BitfinexAPI& claimPosition(long long &position_id,
                                   const double &amount)
        {
            string params = "{\"request\":\"/v1/position/claim\",\"nonce\":\"" +
            getTonce() + "\"";
            params += ",\"position_id\":" + to_string(position_id);
            params += ",\"amount\":\"" + to_string(amount) + "\"";
            params += "}";
            Request.post("/position/claim/", params);

            return *this;
        };


        //  Historical data
        BitfinexAPI& getBalanceHistory(const string &currency,
                                       const time_t &since = 0,
                                       const time_t &until = 0,
                                       const unsigned &limit = 500,
                                       const string &walletType = "all")
        {
            // Is currency valid ?
            if (!inArray(currency, currencies_))
            { bfxApiStatusCode_ = badCurrency; return *this; };

            // Is wallet type valid ?
            // Modified condition which accepts "all" value for all wallets
            // balances together.If "all" specified then there is simply no
            // wallet parameter in POST request.
            if (!inArray(walletType, walletNames_) || walletType != "all")
            { bfxApiStatusCode_ = badWalletType; return *this; };

            string params = "{\"request\":\"/v1/history\",\"nonce\":\"" +
            getTonce() + "\"";
            params += ",\"currency\":\"" + currency + "\"";
            params += ",\"since\":\"" + to_string(since) + "\"";
            params += ",\"until\":\"" + (!until ? getTonce() : to_string(until))
            + "\"";
            params += ",\"limit\":" + to_string(limit);
            if (walletType != "all")
                params += ",\"wallet\":\"" + walletType + "\"";
            params += "}";
            Request.post("/history/", params);

            return *this;
        };

        BitfinexAPI& getWithdrawalHistory(const string &currency,
                                          const string &method = "all",
                                          const time_t &since = 0,
                                          const time_t &until = 0,
                                          const unsigned &limit = 500)
        {
            if (!inArray(currency, currencies_))
            { bfxApiStatusCode_ = badCurrency; return *this; };

            if (!inArray(method, methods_) && method != "wire" && method != "all")
            { bfxApiStatusCode_ = badDepositMethod; return *this; };

            string params = "{\"request\":\"/v1/history/movements\",\"nonce\":\""
            + getTonce() + "\"";
            params += ",\"currency\":\"" + currency + "\"";
            if (method != "all")
                params += ",\"method\":\"" + method + "\"";
            params += ",\"since\":\"" + to_string(since) + "\"";
            params += ",\"until\":\"" +
            (!until ? getTonce() : to_string(until)) + "\"";
            params += ",\"limit\":" + to_string(limit);
            params += "}";
            Request.post("/history/movements/", params);

            return *this;
        };

        BitfinexAPI& getPastTrades(const string &symbol,
                                   const time_t &timestamp,
                                   const time_t &until = 0,
                                   const unsigned &limit_trades = 500,
                                   const bool reverse = false)
        {
            if (!inArray(symbol, symbols_))
                bfxApiStatusCode_ = badSymbol;
            else
            {
                string params = "{\"request\":\"/v1/mytrades\",\"nonce\":\"" +
                getTonce() + "\"";
                params += ",\"symbol\":\"" + symbol + "\"";
                params += ",\"timestamp\":\"" + to_string(timestamp) + "\"";
                params += ",\"until\":\"" +
                (!until ? getTonce() : to_string(until)) + "\"";
                params += ",\"limit_trades\":" + to_string(limit_trades);
                params += ",\"reverse\":" + to_string(reverse);
                params += "}";
                Request.post("/mytrades/", params);
            }

            return *this;
        };

        //  Margin funding
        BitfinexAPI& newOffer(const string &currency,
                              const double &amount,
                              const float &rate,
                              const unsigned &period,
                              const string &direction)
        {
            if(!inArray(currency, currencies_))
                bfxApiStatusCode_ = badCurrency;
            else
            {
                string params = "{\"request\":\"/v1/offer/new\",\"nonce\":\"" +
                getTonce() + "\"";
                params += ",\"currency\":\"" + currency + "\"";
                params += ",\"amount\":\"" + to_string(amount) + "\"";
                params += ",\"rate\":\"" + to_string(rate) + "\"";
                params += ",\"period\":" + to_string(period);
                params += ",\"direction\":\"" + direction + "\"";
                params += "}";
                Request.post("/offer/new/", params);
            }

            return  *this;
        };

        BitfinexAPI& cancelOffer(const long long &offer_id)
        {
            string params = "{\"request\":\"/v1/offer/cancel\",\"nonce\":\"" +
            getTonce() + "\"";
            params += ",\"offer_id\":" + to_string(offer_id);
            params += "}";
            Request.post("/offer/cancel/", params);

            return *this;
        };

        BitfinexAPI& getOfferStatus(const long long &offer_id)
        {
            string params = "{\"request\":\"/v1/offer/status\",\"nonce\":\"" +
            getTonce() + "\"";
            params += ",\"offer_id\":" + to_string(offer_id);
            params += "}";
            Request.post("/offer/status/", params);

            return *this;
        };

        BitfinexAPI& getActiveCredits()
        {
            string params = "{\"request\":\"/v1/credits\",\"nonce\":\"" +
            getTonce() + "\"";
            params += "}";
            Request.post("/credits/", params);

            return *this;
        };

        BitfinexAPI& getOffers()
        {
            string params = "{\"request\":\"/v1/offers\",\"nonce\":\"" +
            getTonce() + "\"";
            params += "}";
            Request.post("/offers/", params);

            return *this;
        };

        BitfinexAPI& getOffersHistory(const unsigned &limit)
        {
            string params = "{\"request\":\"/v1/offers/hist\",\"nonce\":\"" +
            getTonce() + "\"";
            params += ",\"limit\":" + to_string(limit);
            params += "}";
            Request.post("/offers/hist/", params);

            return *this;
        };

        // There is ambiguity in the "symbol" parameter value for this call.
        // It should be "currency" not "symbol".
        // Typical values for "symbol" are trading pairs such as "btcusd",
        // "btcltc" ...
        // Typical values for "currency" are "btc", "ltc" ...
        BitfinexAPI& getPastFundingTrades(const string &currency,
                                          const time_t &until = 0,
                                          const unsigned &limit_trades = 50)
        {
            // Is currency valid ?
            if(!inArray(currency, currencies_))
                bfxApiStatusCode_ = badCurrency;
            else
            {
                string params = "{\"request\":\"/v1/mytrades_funding\",\"nonce\":\""
                + getTonce() + "\"";
                // param inconsistency in BFX API, "symbol" should be currency
                params += ",\"symbol\":\"" + currency + "\"";
                params += ",\"until\":" + to_string(until);
                params += ",\"limit_trades\":" + to_string(limit_trades);
                params += "}";
                Request.post("/mytrades_funding/", params);
            }

            return *this;
        };

        BitfinexAPI& getTakenFunds()
        {
            string params = "{\"request\":\"/v1/taken_funds\",\"nonce\":\"" +
            getTonce() + "\"";
            params += "}";
            Request.post("/taken_funds/", params);

            return *this;
        };

        BitfinexAPI& getUnusedTakenFunds()
        {
            string params = "{\"request\":\"/v1/unused_taken_funds\",\"nonce\":\""
            + getTonce() + "\"";
            params += "}";
            Request.post("/unused_taken_funds/", params);

            return *this;
        };

        BitfinexAPI& getTotalTakenFunds()
        {
            string params = "{\"request\":\"/v1/total_taken_funds\",\"nonce\":\""
            + getTonce() + "\"";
            params += "}";
            Request.post("/total_taken_funds/", params);

            return *this;
        };

        BitfinexAPI& closeLoan(const long long &offer_id)
        {
            string params = "{\"request\":\"/v1/funding/close\",\"nonce\":\"" +
            getTonce() + "\"";
            params += ",\"swap_id\":" + to_string(offer_id);
            params += "}";
            Request.post("/funding/close/", params);

            return *this;
        };

        BitfinexAPI& closePosition(const long long &position_id)
        {
            string params = "{\"request\":\"/v1/position/close\",\"nonce\":\"" +
            getTonce() + "\"";
            params += ",\"position_id\":" + to_string(position_id);
            params += "}";
            Request.post("/position/close/", params);

            return *this;
        };

    private:

        ////////////////////////////////////////////////////////////////////////
        // Private attributes
        ////////////////////////////////////////////////////////////////////////

        // containers with supported parameters
        unordered_set<string> symbols_; // valid symbol pairs
        unordered_set<string> currencies_; // valid currencies
        unordered_set<string> methods_; // valid deposit methods
        unordered_set<string> walletNames_; // valid walletTypes
        unordered_set<string> types_; // valid Types (see new order endpoint)
        // BitfinexAPI settings
        string WDconfFilePath_;
        // internal jsonutils instances
        jsonutils::BfxSchemaValidator schemaValidator_;
        // internal TRequest instance
        TRequest Request;
        // dynamic and status variables
        BfxClientErrors bfxApiStatusCode_;

        ////////////////////////////////////////////////////////////////////////
        // Utility private methods
        ////////////////////////////////////////////////////////////////////////

        BfxClientErrors parseWDconfParams(string &params)
        {
            using std::getline;
            using std::ifstream;
            using std::map;
            using std::regex;
            using std::regex_search;
            using std::smatch;

            string line;
            map<string, string> mParams;
            ifstream inFile(WDconfFilePath_, ifstream::in);
            if (!inFile.is_open())
            {
                return badWDconfFilePath;
            }
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
            else if (inArray(mParams["withdraw_type"], methods_))
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

            return noError;
        };

        BfxClientErrors checkErrors() {
            bfxApiStatusCode_ = Request.hasError()
                ? curlERR
                : schemaValidator_.validateSchema(
                    Request.getLastPath(),
                    Request.getLastResponse()
                );
            return bfxApiStatusCode_;
        }

        ////////////////////////////////////////////////////////////////////////
        // Utility private static methods
        ////////////////////////////////////////////////////////////////////////

        const static string bool2string(const bool &in) noexcept
        { return in ? "true" : "false"; };

        static string getTonce() noexcept
        {
            using namespace std::chrono;

            milliseconds ms =
            duration_cast<milliseconds>(system_clock::now().time_since_epoch());

            return to_string(ms.count());
        };

        static bool inArray(const string &value,
                            const unordered_set<string> &inputSet) noexcept
        { return (inputSet.find(value) != inputSet.cend()); };
    };
}
