/* Implements regular expression-like operations for vectors of any type. */
#include <cassert>
#include <climits>
#include <functional>
#include <iostream>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

#include "repr.h"

namespace vecex {
template <typename T>
using CaptureMapper = std::map<std::string, std::vector<std::vector<T>>>;

template <typename A, typename B>
using Comparer = std::function<bool(A, B)>;

enum Tag {
    JUST,          // Consume the tokens in order.
    DOT,           // Consume any 1 token.
    BETWEEN,       // Consume between min and max tokens.
    INTERSECTION,  // Consume any 1 token that matches all subtokens.
    UNION,         // Consume 1 token which matches one or more subtokens.
};

/* An unsigned integer which may be infinite. */
struct uint_inf {
    uint num;
    bool is_inf = false;

    uint num_or(uint val) { return is_inf ? val : num; }
    bool operator==(uint val) { return !is_inf & (num == val); }
    bool operator==(uint_inf val) {
        if (is_inf) return val.is_inf;
        return num == val.num;
    }
    bool operator!=(uint val) { return !(*this == val); }
    bool operator!=(uint_inf val) { return !(*this == val); }
    uint num_or_max() { return this->num_or(UINT_MAX); }
    std::string str() { return is_inf ? "inf" : std::to_string(num); }
};

uint_inf U_INFINITY = {0, true};

/* A vector expression match. */
template <typename T>
class Match {
   public:
    std::optional<uint> count;  // Number of items matched.
    bool exhausted;             // Whether or not the expression was exhausted.

    bool is_good() { return count.has_value() && exhausted; }
    uint value() { return count.value(); }
    uint value_or(uint a) { return count.value_or(a); }
};

/* A vector expression token. */
template <typename T>
class Token {
   public:
    Tag tag;
    std::vector<std::variant<T, Token*>> tokens;
    std::optional<std::string> groupname;

    // Only used by the sized versions.
    uint_inf min = {0, false};
    uint_inf max = U_INFINITY;

    bool is_capturing() { return groupname.has_value(); }

    bool is_valid() {
        switch (tag) {
            case DOT:
                return tokens.empty();
            case UNION: {
                for (auto a : tokens)
                    if (a.index()) return false;
                return true;
            }
            default:
                return true;
        }
    }

    Token<T> normal() { return {JUST, tokens}; }

    /* If the group is capturing, add the captures to the CaptureMapper. */
    template <typename A>
    void update_captures(CaptureMapper<A>* map,
                         std::vector<A>* items,
                         uint start,
                         uint end) {
        if (!is_capturing()) return;

        auto name = groupname.value();
        if (map->find(name) == map->end()) (*map)[name] = {};
        (*map)[name].push_back({items->begin() + start, items->begin() + end});
    }

    std::string str() {
        auto n_tokens = tokens.size();
        if (tag != DOT && !n_tokens) return "";

        std::stringstream result;
        std::string sep = " ";
        if (tag == UNION) sep = " | ";
        if (tag == INTERSECTION) sep = " & ";

        result << "(";
        if (is_capturing()) result << repr(groupname.value());

        if (tag == DOT) {
            result << ".";
        } else
            for (auto i : tokens) {
                result << (i.index() ? std::get<1>(i)->str()
                                     : repr(std::get<0>(i)));
                result << sep;
            }
        // Removes the last separator.
        std::string sresult = result.str();
        if (tokens.size())
            sresult = sresult.substr(0, sresult.size() - sep.size());
        sresult += ")";

        if (tag == BETWEEN) {
            if (min == 0 && max == U_INFINITY) {
                sresult += "*";
            } else if (min == 0 && max == 1) {
                sresult += "?";
            } else if (min == 1 && max == U_INFINITY) {
                sresult += "+";
            } else {
                sresult += "{";

                if (min != 0) sresult += min.str();
                sresult += ",";
                if (max != U_INFINITY) sresult += max.str();
                sresult += "}";
            }
        }

        return sresult;
    }
};

/* Match a vector expression token against a vector, returning
   its captures if it is successful, or `Nothing` otherwise.

   The template type `A` is the type of the token, and the type `T` is the type
   of the items.

   One example use case is having some complex nested numeric conditions (`>`,
   `<` for example) that a vector of `int` needs to be compared against.

   One way to implement this would be having `A` as the type `bool(int)`, and
   creating the closures along with the tokens. The `matches` function here
   could be function application of the first argument on the second.

   Another way to implement this would be having `A` as the type `std::string`.
   For example, the condition needing to be greater than 10 could be written as
   `">10"`. Here, the `matches` function could be getting the operator from the
   first string character, and then applying it on the rest of the string and
   the second argument.
 */
template <typename A, typename T>
std::optional<CaptureMapper<T>> fullmatch(Token<A>* token,
                                          std::vector<T>* items,
                                          Comparer<A, T>* matches) {
    CaptureMapper<T> captures = {};

    std::function<Match<T>(Token<A>*, uint)> inner;
    inner = [&items, &captures, &matches, &inner](Token<A>* token,
                                                  uint old_index) -> Match<T> {
        assert(token->is_valid());
        uint i_items = old_index;
        uint i_tokens = 0;
        uint num_items = items->size();
        uint num_tokens = token->tokens.size();

        if (token->tag == JUST) {
            for (; i_tokens < num_tokens; i_tokens++) {
                auto subtoken = token->tokens[i_tokens];
                bool is_literal = std::holds_alternative<A>(subtoken);
                if (is_literal) {
                    if (i_items >= num_items) break;  // Items are exhausted.
                    auto expected = std::get<A>(subtoken);
                    auto actual = (*items)[i_items];

                    if (!(*matches)(expected, actual)) break;  // No match.
                    i_items++;

                } else {
                    // Recursively match the subtoken.
                    auto result = inner(std::get<1>(subtoken), i_items);
                    if (!result.is_good()) break;
                    i_items += result.value();
                }
            }
            bool exhausted = i_tokens == num_tokens;
            if (exhausted)
                token->update_captures(&captures, items, old_index, i_items);
            return {i_items - old_index, exhausted};
        }

        if (token->tag == DOT) {
            if (i_items >= num_items) return {0, false};
            token->update_captures(&captures, items, old_index, old_index + 1);
            return {1, true};
        }

        if (token->tag == UNION) {
            if (i_items >= num_items) return {};
            for (auto item : token->tokens) {
                auto expected = std::get<A>(item);
                auto actual = (*items)[i_items];
                if ((*matches)(expected, actual)) {
                    token->update_captures(
                        &captures, items, old_index, old_index + 1);
                    return {1, true};
                }
            }
            return {0, false};
        }

        if (token->tag == INTERSECTION) {
            if (i_items >= num_items) return {};
            for (auto item : token->tokens) {
                auto expected = std::get<A>(item);
                auto actual = (*items)[i_items];
                if (!(*matches)(expected, actual)) { return {0, false}; }
            }
            token->update_captures(&captures, items, old_index, old_index + 1);
            return {1, true};
        }

        if (token->tag == BETWEEN) {
            Match<T> result;
            uint max_items = std::min(
                num_items,
                token->max.is_inf ? UINT_MAX : i_items + token->max.num);

            auto norm = token->normal();

            while (i_items < max_items) {
                result = inner(&norm, i_items);
                if (!result.is_good()) break;
                i_items += result.value();
            }

            uint counter = i_items - old_index;

            bool exhausted = counter >= token->min.num_or(0) &&
                             counter <= token->max.num_or_max();

            if (exhausted) {
                token->update_captures(&captures, items, old_index, i_items);
            }
            return {counter, exhausted};
        }

        // Catch fallthrough.
        assert(false);
        return {0, false};
    };

    auto result = inner(token, 0);
    if (result.is_good() && result.value() == items->size()) return captures;
    return {};
}
}  // namespace vecex
