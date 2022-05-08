/* Utilities to escape and unescape strings. */
#include <string>

/* Escapes a string.

Based on https://stackoverflow.com/a/2417875. */
std::string escape_string(std::string const &s) {
    std::string out;
    out += '"';
    for (auto c : s) {
        if (' ' <= c and c <= '~' and c != '\\' and c != '"')
            out += c;
        else {
            out += '\\';
            switch (c) {
                case '"':
                    out += '"';
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
    out += '"';
    return out;
}


std::string unescape_string(std::string const &s) {
    std::string out;
    bool escape_next = false;
    unsigned int size = s.size();

    for (size_t i = 1; i < (size - 1); i++) {
        char c = s[i];
        if (!escape_next) {
            escape_next = c == '\\';
            if (!escape_next) {
                out += c;
            }
        }
        else {
            switch (c) {
                case '"':
                    out += '"';
                    break;
                case '\\':
                    out += '\\';
                    break;
                case 't':
                    out += '\t';
                    break;
                case 'r':
                    out += '\r';
                    break;
                case 'n':
                    out += '\n';
                    break;
            }
            escape_next = false;
        }

    }
    return out;
}
