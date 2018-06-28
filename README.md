# bfx-cpp-api

_C++ Bitfinex REST API client_

***

### Synopsis

This header-only library contains class for interfacing Bitfinex REST API v1.

### Installation

Just copy content of `include/` directory into your project's `include/` directory and
add `#include "BitfinexAPI.hpp"` to your `.cpp` file.

### Usage

	// Create API client for both authenticated and unauthenticated requests
	BfxAPI::BitfinexAPI bfxAPI("accessKey", "secretKey");
	
	// Create API client for just unauthenticated requests
	BfxAPI::BitfinexAPI bfxAPI();
	
	// Fetch data
	bfxAPI.getTicker("btcusd");
	
   	// Check for errors
  	if (!bfxAPI.hasApiError())
   		// Get response in string
   		cout << bfxAPI.strResponse() << endl;
    else
    {
        // Inspect errors
        cout << bfxAPI.getBfxApiStatusCode() << endl;
        cout << bfxAPI.getCurlStatusCode() << endl;
    }

See self-explanatory `example.cpp` for general usage and more requests.

### Dependencies

*bfx-cpp-api* depends on following external libraries

* libcryptopp - [https://www.cryptopp.com/](https://www.cryptopp.com/)
* libcurl - [https://curl.haxx.se/download.html](https://curl.haxx.se/download.html)

### ToDo

- [ ] Integrating RapidJSON
- [ ] Unit tests
- [ ] JSON Scheme validation

### Author

Petr Javorik [www.mmquant.net](www.mmquant.net) [maple@mmquant.net](maple@mmquant.net)
