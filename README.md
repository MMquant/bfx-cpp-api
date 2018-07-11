# bfx-cpp-api

_C++ Bitfinex REST API client_

***

### Notice

***Master*** branch contains new version of the client. For old version checkout ***legacy*** branch.

### Synopsis

This header-only library contains class for interfacing Bitfinex REST API v1. Current version supports response JSON
schema validation.

### Installation

1. Copy content of `include/` directory into your project's `include/` directory and
add `#include "bfx-api-cpp/BitfinexAPI.hpp"` in your `.cpp` file.

2. Copy `doc/` directory with configuration files into your project.

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

See self-explanatory `src/example.cpp` for general usage and more requests.

### Dependencies

*bfx-cpp-api* depends on following external libraries

* libcryptopp - [https://www.cryptopp.com/](https://www.cryptopp.com/)
* libcurl - [https://curl.haxx.se/download.html](https://curl.haxx.se/download.html)

### ToDo

- [ ] Unit tests
- [x] JSON Scheme validation

### Change Log

- 2018-07-11 Schema validation logic complete. Client currently validates public requests only.

### Author

Petr Javorik [www.mmquant.net](www.mmquant.net) [maple@mmquant.net](maple@mmquant.net)
