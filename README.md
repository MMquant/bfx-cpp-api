![bfx-cpp-api logo](doc/logo/bfx-cpp-api_logo.png)

***

### Build status

| [Linux][lin-link] |
| :---------------: |
| ![lin-badge]      |

[lin-badge]: https://circleci.com/gh/MMquant/bfx-cpp-api/tree/master.svg?style=svg "CircleCI build status"
[lin-link]:  https://circleci.com/gh/MMquant/bfx-cpp-api "CircleCI build status"

### Notice

***Master*** branch contains new version of the client. For old version checkout ***legacy*** branch.

### Synopsis

This header-only library contains class for interfacing Bitfinex REST API v1. Current version supports response JSON
schema validation.

### Dependencies

*bfx-cpp-api* depends on following external libraries/packages

* cmake - [https://cmake.org/download/](https://cmake.org/download/)
* libcrypto++ - [https://www.cryptopp.com/](https://www.cryptopp.com/)
* libcrypto++-dev - [https://www.cryptopp.com/](https://www.cryptopp.com/)
* curl - [https://curl.haxx.se/download.html](https://curl.haxx.se/download.html)
* libcurl4-gnutls-dev or libcurl4-openssl-dev - [https://curl.haxx.se/download.html](https://curl.haxx.se/download.html)

### How to Build'n'Run `src/example.cpp`

1. Install dependencies (via apt, homebrew etc.).
2. Clone or download bfx-api-cpp repository.
3. Peek into self-documented `<your_project_dir>/src/example.cpp`.
4. Create `build` directory `$ mkdir <your_project_dir>/build`.
5. Build `example` binary `$ cd <your_project_dir>/build && cmake .. && make`.
6. Run `example` binary `$ ./example`.

### Usage

	// Create API client for both authenticated and unauthenticated requests
	BfxAPI::BitfinexAPI bfxAPI("accessKey", "secretKey");
	
	// Create API client for just unauthenticated requests
	BfxAPI::BitfinexAPI bfxAPI();
	
	// Fetch data
	bfxAPI.getTicker("btcusd");
	
	// Check for errors
	if (!bfxAPI.hasApiError())
	{
	    // Get response in string
	    cout << bfxAPI.strResponse() << endl;
	}
	else
	{
	    // Inspect errors
	    cout << bfxAPI.getBfxApiStatusCode() << endl;
	    cout << bfxAPI.getCurlStatusCode() << endl;
	}

See self-explanatory `src/example.cpp` for general usage and more requests.

### Change Log

- 2018-07-24 CMakeLists.txt added. Installation instructions changed.
- 2018-07-11 Schema validation logic complete. Client currently validates public requests only.

### Known issues

You will not be able to compile *bfx-cpp-api* with GCC<7.2 due to this [bug](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66297).

### Author

Petr Javorik [www.mmquant.net](www.mmquant.net) [maple@mmquant.net](maple@mmquant.net)
