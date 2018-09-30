////////////////////////////////////////////////////////////////////////////////
//  TRequest.hpp
//
//
//  Bitfinex REST API C++ client
//
//
//  Copyright (C) 2018      Blas Vicco    blasvicco@gmail.com
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

#include <map>
#include <string>

// curl
#include <curl/curl.h>

// cryptopp
#include <cryptopp/base64.h>
#include <cryptopp/hex.h>
#include <cryptopp/hmac.h>
#include <cryptopp/osrng.h>

using std::string;
using std::map;

// CRYPTOPP_NO_GLOBAL_BYTE signals byte is at CryptoPP::byte
#ifdef CRYPTOPP_NO_GLOBAL_BYTE
using CryptoPP::byte;
#endif

namespace BfxAPI {

  class TRequest {

    ////////////////////////////////////////////////////////////////////////
    // Class constants
    ////////////////////////////////////////////////////////////////////////
    static constexpr auto CURL_TIMEOUT = 30L;
    static constexpr auto CURL_DEBUG_VERBOSE = 0L;

    public:
      ////////////////////////////////////////////////////////////////////////
      // Constructor / Destructor
      ////////////////////////////////////////////////////////////////////////
      TRequest(string inEndpoint) {
        endpoint = inEndpoint;
        curlGET = curl_easy_init();
        curlPOST = curl_easy_init();
      };

      ~TRequest() {
        curl_easy_cleanup(curlGET);
        curl_easy_cleanup(curlPOST);
      };

      ////////////////////////////////////////////////////////////////////////
      // Public methods
      ////////////////////////////////////////////////////////////////////////

      string get(string inPath, map<string, string> params = {}) {
        response = "";
        if (curlGET) {
          path = inPath;
          string url = endpoint + path + "?" + parseParams(params);

          setupHeader();
          curl_easy_setopt(curlPOST, CURLOPT_HTTPHEADER, curlHeader);
          curl_easy_setopt(curlGET, CURLOPT_TIMEOUT, CURL_TIMEOUT);
          curl_easy_setopt(curlGET, CURLOPT_URL, url.c_str());
          curl_easy_setopt(curlGET, CURLOPT_VERBOSE, CURL_DEBUG_VERBOSE);
          curl_easy_setopt(curlGET, CURLOPT_WRITEDATA, &response);
          curl_easy_setopt(curlGET, CURLOPT_WRITEFUNCTION, writeCallback);

          curlStatusCode = curl_easy_perform(curlGET);
          // libcurl internal error handling
          if (curlStatusCode != CURLE_OK) {
            cerr << "libcurl error in Request.get():" << endl;
            cerr << "CURLcode: " << curlStatusCode << endl;
          }
        } else {
          cerr << "curl not properly initialized curlGET = nullptr";
        }
        return response;
      };

      string post(string inPath, string json = "") {
        response = "";
        if (curlPOST) {
          path = inPath;
          string url = endpoint + path;
          string payload;
          getBase64(json, payload);

          if (accessKey != "") {
            header["X-BFX-APIKEY"] = accessKey;
          }

          if (secretKey != "") {
            header["X-BFX-SIGNATURE"] = getSignature(payload);
          }

          if (payload != "") {
           header["X-BFX-PAYLOAD"] = payload;
          }

          setupHeader();
          curl_easy_setopt(curlPOST, CURLOPT_HTTPHEADER, curlHeader);
          curl_easy_setopt(curlPOST, CURLOPT_POST, 1);
          curl_easy_setopt(curlPOST, CURLOPT_POSTFIELDS, "\n");
          curl_easy_setopt(curlPOST, CURLOPT_TIMEOUT, CURL_TIMEOUT);
          curl_easy_setopt(curlPOST, CURLOPT_URL, url.c_str());
          curl_easy_setopt(curlPOST, CURLOPT_VERBOSE, CURL_DEBUG_VERBOSE);
          curl_easy_setopt(curlPOST, CURLOPT_WRITEDATA, &response);
          curl_easy_setopt(curlPOST, CURLOPT_WRITEFUNCTION, writeCallback);

          curlStatusCode = curl_easy_perform(curlPOST);
          clearHeader();
          // libcurl internal error handling
          if (curlStatusCode != CURLE_OK) {
            cerr << "libcurl error in Request.post():" << endl;
            cerr << "CURLcode: " << curlStatusCode << endl;
          }
        } else {
          cerr << "curl not properly initialized curlPOST = nullptr";
        }

        return response;
      };

      string parseParams(map<string, string> params) {
        string pp = "";
        map<string, string>::iterator it;
        for (it = params.begin(); it != params.end(); it++) {
          pp += it->first + "=" + it->second + "&";
        }
        return pp;
      };

      string getSignature(string payload) {
        string signature;
        getHmacSha384(secretKey, payload, signature);
        return signature;
      }

      const CURLcode getLastStatusCode() const noexcept {
        return curlStatusCode;
      }

      const string getLastResponse() const noexcept {
        return response;
      }

      const string getLastPath() const noexcept {
        return path;
      }

      const bool hasError() const noexcept {
        return curlStatusCode != CURLE_OK;
      }

      void setSecretKey(string inSecretKey) {
        secretKey = inSecretKey;
      }

      void setAccessKey(string inAccessKey) {
        accessKey = inAccessKey;
      }

      void setHeader(map<string, string> inHeader) {
        header = inHeader;
      }

    private:
      ////////////////////////////////////////////////////////////////////////
      // Private properties
      ////////////////////////////////////////////////////////////////////////
      string endpoint = "";
      string path = "";
      string secretKey = "";
      string accessKey = "";
      string response = "";
      map<string, string> header;
      struct curl_slist *curlHeader = nullptr;

      // Curl properties
      CURL *curlGET;
      CURL *curlPOST;
      CURLcode curlStatusCode;

      ////////////////////////////////////////////////////////////////////////
      // Private methods
      ////////////////////////////////////////////////////////////////////////

      // Curl write callback function. Appends fetched *content to *userp
      // pointer. *userp pointer is set up by
      // curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result) line.
      // In this case *userp points to result.
      static size_t writeCallback(
        void *data,
        size_t size,
        size_t nmemb,
        void *userp) noexcept
      {
        (static_cast <string*>(userp))
          ->append(static_cast <char*>(data), size * nmemb);
        return size * nmemb;
      };

      static void getBase64(const string &content, string &encoded) {
        using CryptoPP::Base64Encoder;
        using CryptoPP::StringSink;
        using CryptoPP::StringSource;

        byte buffer[1024] = {};

        for (int i = 0; i < content.length(); ++i)
            buffer[i] = content[i];

        StringSource ss(
          buffer,
          content.length(),
          true,
          new Base64Encoder(new StringSink(encoded), false)
        );
      };

      static void getHmacSha384(
        const string &key,
        const string &content,
        string &digest)
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
          StringSource ss1(
            content, true,
            new HashFilter(hmac, new StringSink(mac))
          );
          StringSource ss2(mac, true, new HexEncoder(new StringSink(digest)));
          transform(digest.cbegin(), digest.cend(), digest.begin(), ::tolower);
      };

      void setupHeader() {
        curlHeader = nullptr;
        map<string, string>::iterator it;
        for (it = header.begin(); it != header.end(); it++) {
          curlHeader = curl_slist_append(
            curlHeader,
            (it->first + ":" + it->second).c_str()
          );
        }
      };

      void clearHeader() {
        curlHeader = nullptr;
        header.erase("X-BFX-APIKEY");
        header.erase("X-BFX-SIGNATURE");
        header.erase("X-BFX-PAYLOAD");
      }

  };

}
