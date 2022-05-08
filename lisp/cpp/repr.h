/* String representations of common objects and collections of objects. */
#include <iostream>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

std::string repr(float in);
std::string repr(std::optional<auto> in);
std::string repr(std::vector<auto>* v);
std::string repr(std::vector<auto> v);
std::string repr(std::map<auto, auto>* v);
std::string repr(std::map<auto, auto> v);
std::string repr(std::string* s);
std::string repr(std::string s);

std::string repr(float v) {
    std::stringstream out;
    out << v;
    return out.str();
}

std::string repr(std::optional<auto> v) {
    std::stringstream out;
    out << v.has_value() ? ("Some(" + repr(v.value()) + ")") : "Nothing";
    return out.str();
}

std::string repr(std::vector<auto>* v) {
    std::stringstream out;
    out << "[";
    for (auto i : *v) {
        out << repr(i);
        out << ", ";
    }
    auto result = out.str();
    if (v->size()) result = result.substr(0, result.size() - 2);
    return result + "]";
}

std::string repr(std::vector<auto> v) { return repr(&v); }

std::string repr(std::map<auto, auto>* v) {
    std::stringstream out;
    out << "{";
    for (auto i : *v) {
        out << repr(i.first);
        out << ": ";
        out << repr(i.second);
        out << ", ";
    }
    auto result = out.str();
    if (v->size()) result = result.substr(0, result.size() - 2);
    return result + "}";
}
std::string repr(std::map<auto, auto> v) { return repr(&v); }

std::string repr(std::string* v) { return "'" + *v + "'"; }
std::string repr(std::string v) { return repr(&v); }
