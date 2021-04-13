#include <sstream>  // std::stringstream

#include "Utils/Helper.h"

namespace Helper
{
    std::vector<std::string> Split(const std::string& s, char delim)
    {
        std::vector<std::string> result;
        std::string token;
        std::stringstream ss{s};

        // NOTE: It seems that getline trims also trim all successive delim
        while (std::getline(ss, token, delim))
        {
            // Trim extra space
            if (token.empty()) { continue; }
            result.push_back(token);
        }
        return result;
    }

}