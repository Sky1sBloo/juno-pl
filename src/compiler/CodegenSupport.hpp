#pragma once

#include <cctype>
#include <cstddef>
#include <string>

namespace JunoPL {

/** Folds a set of lambdas into a single visitor for std::visit. */
template <class... Ts> struct Overload : Ts... {
    using Ts::operator()...;
};

/**
True when a literal is written as <Constant number="..."/> rather than
text. Requires the whole token to be consumed so a string such as "123abc"
stays text, and rejects inf/nan/hex which Vizzy cannot parse back.
 */
inline bool isNumericLiteral(const std::string &s) {
    if (s.empty())
        return false;

    std::size_t i = (s[0] == '+' || s[0] == '-') ? 1 : 0;
    if (i >= s.size())
        return false;
    bool digit = false;
    for (; i < s.size(); ++i) {
        const char c = s[i];
        if (std::isdigit(static_cast<unsigned char>(c))) {
            digit = true;
            continue;
        }
        if (c == '.' || c == 'e' || c == 'E' || c == '+' || c == '-')
            continue;
        return false;
    }
    if (!digit)
        return false;

    std::size_t used = 0;
    try {
        // The parsed value is deliberately ignored: this only validates it.
        (void)std::stod(s, &used);
    } catch (...) {
        return false;
    }
    return used == s.size();
}

/**
Text of the negated numeric literal, or "" when the value is not a number.
Used where Vizzy expects the amount itself, e.g. "x -= 2" becomes
ChangeVariable(x, -2).
 */
inline std::string negateNumericLiteral(const std::string &s) {
    if (!isNumericLiteral(s))
        return "";
    double d = -std::stod(s);
    if (d == 0.0)
        return "0";
    std::string out = std::to_string(d);
    if (out.find('.') != std::string::npos) {
        out.erase(out.find_last_not_of('0') + 1, std::string::npos);
        if (out.back() == '.')
            out.pop_back();
    }
    return out;
}

} // namespace JunoPL
