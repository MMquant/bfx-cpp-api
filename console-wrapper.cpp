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
        
        

        int code = bfxAPI.newOffer(result1, "BTC", 0.0195, 250, 2, "lend");
        
        
        
        cout << result1 << endl;
        cout << code << endl;
        ifs.close();
        return 0;
    }
}
