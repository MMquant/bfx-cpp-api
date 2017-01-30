// console-wrapper.cpp
// Defines the entry point for the console application
//


#include <iostream>
#include "BitfinexAPI.hpp"
#include <fstream>


using std::cout;
using std::endl;
using std::ifstream;
using std::string;


int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        cout << "usage: console-wrapper.cpp path/to/keyFile" << endl
        << "first line is accessKey and second line is secretKey" << endl;
        return -1;
    }
    ifstream ifs(argv[1]);
    if (!ifs.is_open())
    {
        cout << "Can't open file: " << argv[1] << endl;
        return -1;
    }
    else
    {
        string accessKey, secretKey;
        getline(ifs, accessKey);
        getline(ifs, secretKey);
        BitfinexAPI bfxAPI(accessKey, secretKey);
        string result1, result2;
        
        
//        vector<BitfinexAPI::sOrders> vOrders =
//        {
//            {"btcusd", 0.02, 1000, "sell", "exchange limit"},
//            {"btcusd", 0.03, 1001, "sell", "exchange limit"},
//            {"btcusd", 0.04, 1002, "sell", "exchange limit"},
//        };
//        int code = bfxAPI.newOrders(result1, vOrders);
        

//        bfxAPI.newOrder(result1, "btcusd", 0.012, 1000, "sell", "exchange limit");
        bfxAPI.activePositions(result1);
        
        
        cout << result1 << endl;
        ifs.close();
        return 0;
    }
}
