#pragma once



#include <iostream>
#include <curl/curl.h>



using std::string;



class BitfinexAPI
{
public:
    
    // Enumerations
    enum bfxERR{ curlERR = -10, }; // negative return values for errors
    
    // Constructor - destructor
    explicit BitfinexAPI(const string &accessKey, const string &secretKey);
    ~BitfinexAPI();
    
    // Public endpoints
    int getTicker(string &result, string symbol);
    
private:
    
    // BitfinexAPI object cannot be copied
    BitfinexAPI(const BitfinexAPI&);
    BitfinexAPI &operator=(const BitfinexAPI&);
    
    // Static private variables
    static const string apiUrl;
    static const string symbols[];
    
    // Private variables
    string accessKey, secretKey;
    CURL *curl;
    CURLcode res;
    
    // Support private methods
    string getHmacSha384(const string &key, const string &content);
    int DoGETrequest(const string &UrlEndPoint, const string &params, string &result);
    int DoPOSTrequest(const string &UrlEndPoint, const string &params, string &result);
    
};











