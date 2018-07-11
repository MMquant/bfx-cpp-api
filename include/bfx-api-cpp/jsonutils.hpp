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
    public:
        
        MyRemoteSchemaDocumentProvider()
        {
            FILE *pFileIn = fopen("doc/definitions.json", "r"); // non-Windows use "r"
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
            // keyword in json schema doesn't support json keys which contain
            // special characters.
            // See https://github.com/Tencent/rapidjson/issues/1311
            
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
                return bfxERR::responseParseError;
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
                return bfxERR::responseSchemaError;
            }
            
            return bfxERR::noError;
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
    
    bfxERR jsonStrToUset(unordered_set<string> &uSet, const string &inputJson)
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
        
        /// DEBUG
//        string mockJson = "{\"mid\":\"6581.55\",\"bid\":\"6581.5\",\"ask\":\"6581.6\",\"last_price\":\"6581.5\",\"low\":\"6333.2\",\"high\":\"6681.2\",\"volume\":\"28766.900098530004\",\"timestamp\":\"1530620498.4127066\"}";
        /// DEBUG
        
        // Create input JSON StringStream
        rj::StringStream ss(inputJson.c_str());
//        rj::StringStream ss(mockJson.c_str()); // DEBUG
        
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
            return bfxERR::jsonStrToUSetError;
        }
        else
        {
            uSet.swap(handler.handlerUSet_);
            return bfxERR::noError;
        }
    }
}
