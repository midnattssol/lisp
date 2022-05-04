#include <string>

/* Escapes a string.

Based on https://stackoverflow.com/a/2417875. */
std::string escape_string(std::string const &s) {
    std::string out;
    out += '\'';
    for (auto c : s) {
        if (' ' <= c and c <= '~' and c != '\\' and c != '\'')
            out += c;
        else {
            out += '\\';
            switch (c) {
                case '\'':
                    out += '\'';
                    break;
                case '\\':
                    out += '\\';
                    break;
                case '\t':
                    out += 't';
                    break;
                case '\r':
                    out += 'r';
                    break;
                case '\n':
                    out += 'n';
                    break;
                default:
                    char const *const hexdig = "0123456789ABCDEF";
                    out += 'x';
                    out += hexdig[c >> 4];
                    out += hexdig[c & 0xF];
            }
        }
    }
    out += '\'';
    return out;
}
