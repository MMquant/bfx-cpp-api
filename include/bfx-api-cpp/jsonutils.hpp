////////////////////////////////////////////////////////////////////////////////
//
// JSON utility routines for BitfinexAPI
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

// rapidjson
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/schema.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

// internal error
#include "error.hpp"

// std
#include <iostream>
#include <string>
#include <unordered_set>
#include <unordered_map>

// namespaces
using std::cerr;
using std::cout;
using std::endl;
using std::string;
using std::unordered_set;
using std::unordered_map;
namespace rj = rapidjson;


namespace jsonutils
{
    
    ////////////////////////////////////////////////////////////////////////////
    // Classes
    ////////////////////////////////////////////////////////////////////////////
    
    /// Helper class resolving remote schema for schema $ref operator
    class MyRemoteSchemaDocumentProvider: public rj::IRemoteSchemaDocumentProvider
    {
        
        #ifndef JSON_DEFINITIONS_FILE_PATH
        static constexpr auto JSON_DEFINITIONS_FILE_PATH = "definitions.json";
        #endif
        
    public:
        
        MyRemoteSchemaDocumentProvider()
        {
            FILE *pFileIn = fopen(JSON_DEFINITIONS_FILE_PATH, "r"); // non-Windows use "r"
            char readBuffer[65536];
            rj::FileReadStream fReadStream(pFileIn, readBuffer, sizeof(readBuffer));
            rj::Document d;
            d.ParseStream(fReadStream);
            fclose(pFileIn);
            remoteSchemaDoc_ = new rj::SchemaDocument(d);
        };
        
        //        ~MyRemoteSchemaDocumentProvider()
        //        {
        //            delete remoteSchemaDoc_;
        //        };
        
    private:
        
        rj::SchemaDocument *remoteSchemaDoc_;
        
        virtual const rj::SchemaDocument*
        GetRemoteDocument(const char* uri, rj::SizeType length)
        {
            // Resolve the URI and return a pointer to that schema
            return remoteSchemaDoc_;
        }
    };
    
    class BfxSchemaValidator
    {
    public:
        
        BfxSchemaValidator() {}
        BfxSchemaValidator(unordered_set<string> &symbols,
                           unordered_set<string> &currencies)
        {
            // Mapping is needed because rapidjson implementation of $ref
            // keyword in json schema doesn't support json schema names which
            // contain special characters thus direct mapping of endpoint such
            // "/symbols/" to "/symbols/" schema name is not possible.
            // See https://github.com/Tencent/rapidjson/issues/1311
            
            ////////////////////////////////////////////////////////////////////
            // Public endpoints
            ////////////////////////////////////////////////////////////////////
            
            // Create map for endpoints which use symbols in URL
            for (const auto &symbol : symbols)
            {
                // Map /pubticker/[symbol] endpoints to "pubticker" schema
                apiEndPointToSchemaMap_.emplace("/pubticker/" + symbol,
                                                "pubticker");
                // Map /stats/[symbol] endpoints to "stats" schema
                apiEndPointToSchemaMap_.emplace("/stats/" + symbol,
                                                "stats");
                // Map /book/[symbol] endpoints to "book" schema
                apiEndPointToSchemaMap_.emplace("/book/" + symbol,
                                                "book");
                // Map /trades/[symbol] endpoints to "trades" schema
                apiEndPointToSchemaMap_.emplace("/trades/" + symbol,
                                                "trades");
            }
            
            // Create map for endpoints which use currencies in URL
            for (const auto &currency : currencies)
            {
                // Map /lendbook/[currency] endpoints to "lendbook" schema
                apiEndPointToSchemaMap_.emplace("/lendbook/" + currency,
                                                "lendbook");
                // Map /lends/[currency] endpoints to "lends" schema
                apiEndPointToSchemaMap_.emplace("/lends/" + currency,
                                                "lends");
            }
            
            // Create map for static endpoints
            apiEndPointToSchemaMap_.emplace("/symbols/", "symbols");
            apiEndPointToSchemaMap_.emplace("/symbols_details/",
                                            "symbols_details");
            
            ////////////////////////////////////////////////////////////////////
            // Authenticated endpoints
            ////////////////////////////////////////////////////////////////////
            
            // Create map for static endpoints
            apiEndPointToSchemaMap_.emplace("/account_infos/", "account_infos");
            apiEndPointToSchemaMap_.emplace("/account_fees/", "account_fees");
            apiEndPointToSchemaMap_.emplace("/summary/", "summary");
            apiEndPointToSchemaMap_.emplace("/deposit/new/", "deposit_new");
            apiEndPointToSchemaMap_.emplace("/key_info/", "key_info");
            apiEndPointToSchemaMap_.emplace("/margin_infos/", "margin_infos");
            apiEndPointToSchemaMap_.emplace("/balances/", "balances");
            apiEndPointToSchemaMap_.emplace("/transfer/", "transfer");
            apiEndPointToSchemaMap_.emplace("/withdraw/", "withdraw");
            apiEndPointToSchemaMap_.emplace("/order/new/", "order_new");
            apiEndPointToSchemaMap_.emplace("/order/new/multi/", "order_new_multi");
            apiEndPointToSchemaMap_.emplace("/order/cancel/", "order_cancel");
            apiEndPointToSchemaMap_.emplace("/order/cancel/multi/",
                                            "order_cancel_multi");
            apiEndPointToSchemaMap_.emplace("/order/cancel/all/",
                                            "order_cancel_all");
            apiEndPointToSchemaMap_.emplace("/order/cancel/replace/",
                                            "order_cancel_replace");
            apiEndPointToSchemaMap_.emplace("/order/status/", "order_status");
            apiEndPointToSchemaMap_.emplace("/orders/", "orders");
            apiEndPointToSchemaMap_.emplace("/orders/hist/", "orders_hist");
            apiEndPointToSchemaMap_.emplace("/positions/", "positions");
            apiEndPointToSchemaMap_.emplace("/position/claim/", "position_claim");
            apiEndPointToSchemaMap_.emplace("/history/", "history");
            apiEndPointToSchemaMap_.emplace("/history/movements/",
                                            "history_movements");
            apiEndPointToSchemaMap_.emplace("/mytrades/", "mytrades");
            apiEndPointToSchemaMap_.emplace("/offer/new/", "offer_new");
            apiEndPointToSchemaMap_.emplace("/offer/cancel/", "offer_cancel");
            apiEndPointToSchemaMap_.emplace("/offer/status/", "offer_status");
            apiEndPointToSchemaMap_.emplace("/credits/", "credits");
            apiEndPointToSchemaMap_.emplace("/offers/", "offers");
            apiEndPointToSchemaMap_.emplace("/offers/hist/", "offers_hist");
            apiEndPointToSchemaMap_.emplace("/mytrades_funding/",
                                            "mytrades_funding");
            apiEndPointToSchemaMap_.emplace("/taken_funds/", "taken_funds");
            apiEndPointToSchemaMap_.emplace("/unused_taken_funds/",
                                            "unused_taken_funds");
            apiEndPointToSchemaMap_.emplace("/total_taken_funds/",
                                            "total_taken_funds");
            apiEndPointToSchemaMap_.emplace("/funding/close/", "funding_close");
            apiEndPointToSchemaMap_.emplace("/position/close/", "position_close");
            
        }
        
        auto validateSchema(const string &apiEndPoint, const string &inputJson)
        {
            const auto schemaName = getApiEndPointSchemaName(apiEndPoint);
            
            // Create rapidjson schema document
            rj::Document sd;
            string schema =
            "{ \"$ref\": \"definitions.json#/" + schemaName + "\" }";
            sd.Parse(schema.c_str());
            rj::SchemaDocument schemaDocument(sd, 0, 0, &provider_);
            
            // Create rapidjson document and check for parse errors
            rj::Document d;
            if (d.Parse(inputJson.c_str()).HasParseError())
            {
                cerr << "Invalid json - response:" << endl;
                cerr << inputJson << endl;
                cerr << "API endpoint: " << apiEndPoint << endl;
                return BfxClientErrors::responseParseError;
            }
            
            // Create rapidjson validator and check for schema errors
            rj::SchemaValidator validator(schemaDocument);
            if (!d.Accept(validator))
            {
                // Input JSON is invalid according to the schema
                // Output diagnostic information
                rj::StringBuffer sb;
                validator.GetInvalidSchemaPointer().StringifyUriFragment(sb);
                cerr << "Invalid schema: " << sb.GetString() << endl;
                cerr << "Invalid keyword: " << validator.GetInvalidSchemaKeyword() << endl;
                sb.Clear();
                validator.GetInvalidDocumentPointer().StringifyUriFragment(sb);
                cerr << "Invalid document: " << sb.GetString() << endl;
                cerr << "Invalid response: " << inputJson << endl;
                cerr << "Invalid API endpoint: " << apiEndPoint << endl;
                return BfxClientErrors::responseSchemaError;
            }
            
            return BfxClientErrors::noError;
        }
        
    private:
        
        MyRemoteSchemaDocumentProvider provider_;
        unordered_map<string, string> apiEndPointToSchemaMap_;
        
        const string& getApiEndPointSchemaName(const string& apiEndpoint) noexcept
        {
            return apiEndPointToSchemaMap_[apiEndpoint];
        }
        
    };
    
    /// SAX events helper struct for jsonStrToUset() routine
    struct jsonStrToUsetHandler:
    public rj::BaseReaderHandler<rj::UTF8<>, jsonStrToUsetHandler>
    {
        // Constructor
        jsonStrToUsetHandler():state_(State::kExpectArrayStart) {}
        
        // SAX events handlers
        bool StartArray() noexcept
        {
            switch (state_)
            {
                case State::kExpectArrayStart:
                    state_ = State::kExpectValueOrArrayEnd;
                    return true;
                default:
                    return false;
            }
        }
        
        bool String(const char *str, rj::SizeType length, bool)
        {
            switch (state_)
            {
                case State::kExpectValueOrArrayEnd:
                    handlerUSet_.emplace(str);
                    return true;
                default:
                    return false;
            }
        }
        
        bool EndArray(rj::SizeType) noexcept
        {
            return state_ == State::kExpectValueOrArrayEnd;
        }
        
        bool Default() { return false; } // All other events are invalid.
        
        // Handler attributes
        std::unordered_set<std::string> handlerUSet_; // output uSet
        enum class State // valid states
        {
            kExpectArrayStart,
            kExpectValueOrArrayEnd,
        } state_;
    };
    
    ////////////////////////////////////////////////////////////////////////////
    // Routines
    ////////////////////////////////////////////////////////////////////////////
    
    BfxClientErrors jsonStrToUset(unordered_set<string> &uSet, const string &inputJson)
    {
        // Create schema $ref resolver
        rj::Document sd;
        string schema = "{ \"$ref\": \"doc/definitions.json#/flatJsonSchema\" }";
        sd.Parse(schema.c_str());
        MyRemoteSchemaDocumentProvider provider;
        rj::SchemaDocument schemaDoc(sd, 0, 0, &provider);
        
        // Create SAX events handler which contains parsed uSet after successful
        // parsing
        jsonStrToUsetHandler handler;
        
        // Create schema validator
        rj::GenericSchemaValidator<rj::SchemaDocument, jsonStrToUsetHandler>
        validator(schemaDoc, handler);
        
        // Create reader
        rj::Reader reader;
        
        // Create input JSON StringStream
        rj::StringStream ss(inputJson.c_str());
        
        // Parse and validate
        if (!reader.Parse(ss, validator))
        {
            // Input JSON is invalid according to the schema
            // Output diagnostic information
            cerr << "Error(offset " << reader.GetErrorOffset() << "): ";
            cerr << GetParseError_En(reader.GetParseErrorCode()) << endl;
            
            if (!validator.IsValid())
            {
                rj::StringBuffer sb;
                validator.GetInvalidSchemaPointer().StringifyUriFragment(sb);
                cerr << "Invalid schema: " << sb.GetString() << endl;
                cerr << "Invalid keyword: " << validator.GetInvalidSchemaKeyword() << endl;
                sb.Clear();
                validator.GetInvalidDocumentPointer().StringifyUriFragment(sb);
                cerr << "Invalid document: " << sb.GetString() << endl;
                cerr << "Invalid response: " << inputJson << endl;
            }
            return BfxClientErrors::jsonStrToUSetError;
        }
        else
        {
            uSet.swap(handler.handlerUSet_);
            return BfxClientErrors::noError;
        }
    }
}
