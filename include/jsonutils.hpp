// JSON Utility routines for BitfinexAPI

#pragma once


#include <unordered_set>
#include <sstream>


using std::unordered_set;
using std::string;
using std::stringstream;


namespace jsonutils
{
    void arrayToUset(unordered_set<string> &uSet, string &array)
    {
        stringstream ss(array);
        
        while(ss.good())
        {
            string substr;
            getline(ss, substr, ',');
            substr.erase(std::remove_if(substr.begin(),
                                        substr.end(),
                                        [](auto const &c) -> bool {
                                            return !std::isalnum(c);}),
                         substr.end());
            uSet.emplace(substr);
        }
    }
}
