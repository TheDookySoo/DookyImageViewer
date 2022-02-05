#include "StringUtils.h"

namespace Dooky {
    void LowerString(std::string& str) {
        std::transform(
            str.begin(),
            str.end(),
            str.begin(),
            [](unsigned char c) {
                return std::tolower(c);
            }
        );
    }
}