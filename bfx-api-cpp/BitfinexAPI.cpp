#include "BitfinexAPI.hpp"
#include <iostream>



using std::cout;


//////////////////////////////////////////////////////////////////////////////
// Support functions
//////////////////////////////////////////////////////////////////////////////

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

//////////////////////////////////////////////////////////////////////////////
// Constructor - destructor
//////////////////////////////////////////////////////////////////////////////

BitfinexAPI::
BitfinexAPI(const string &accessKey, const string &secretKey):
accessKey(accessKey), secretKey(secretKey), curl(NULL)
{
    curl = curl_easy_init();
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
    string endPoint = "/pubticker/" + symbol;
    string params = "";
    return DoGETrequest(endPoint, params, result);
    
}

//////////////////////////////////////////////////////////////////////////////
// Support private methods
//////////////////////////////////////////////////////////////////////////////

int BitfinexAPI::
DoGETrequest(const string &UrlEndPoint, const string &params, string &result)
{
    
    if(curl) {
        
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.bitfinex.com/v1/pubticker/btcusd");
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
        
        return curlERR;
        
    }
}
