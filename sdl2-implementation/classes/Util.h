#pragma once

#include <string>

namespace Util {

void replace_all(
    std::string& s,
    std::string const& toReplace,
    std::string const& replaceWith
);

}
