#include <array>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

/* Escapes a bash call. */
std::string _escape_exec(std::string const &s) {
    std::string out;
    out += '"';
    for (auto c : s) {
        if (c != '\\' and c != '"')
            out += c;
        else {
            out += '\\';
            out += c;
        }
    }
    out += '"';
    return out;
}

/* Executes a command. */
std::string exec(std::string cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) { throw std::runtime_error("popen() failed!"); }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}
