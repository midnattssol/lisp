#include <algorithm>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <numeric>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
/*
(let add3 (=> {a b c} {+ a b c}))
*/

// ===| MACROS |===

#define LASSERT_TYPE(item, expected, err)                                \
    if (item.tag != expected) {                                          \
        std::cout << std::endl << "TypeAssertionError: " << err << '\n'; \
        exit(1);                                                         \
    }

// ===| GLOBALS |===

const std::set<std::string> LISP_BUILTINS = {
    "%",      "&",      "*",      "+",       "-",         "-",         "/",
    "<",      ">",      "?",      "^",       "|",         "~",         "!=",
    "<=",     "==",     ">=",     "eq",      "gt",        "lt",        "or",
    "add",    "and",    "div",    "geq",     "get",       "len",       "leq",
    "map",    "mod",    "mul",    "neg",     "neq",       "put",       "sub",
    "xor",    "bool",   "eval",   "flip",    "fold",      "join",      "list",
    "noop",   "repr",   "type",   "range",   "slice",     "assert",    "insert",
    "repeat", "symbol", "typeof", "ternary", "typematch", "expression"};

enum LispType {
    NUM,
    STRING,
    LIST,
    NOTHING,
    BOOL,
    VECTOR,
    BUILTIN,
    TYPE,
    EXPRESSION,
    __NOT_SET__,
    __NO_ARGS__,
};

// ===| FORWARD DECLARATIONS |===

std::string escape_string(std::string const &s);
void _lisp_assert_or_exit(bool condition, std::string message);
[[noreturn]] void _throw_does_not_implement(LispType type,
                                            std::string notimplemented);

// ===| STRUCTS |===

const std::map<unsigned int, const std::string> TYPENAMES{
    {NUM, "num"},
    {STRING, "string"},
    {LIST, "list"},
    {NOTHING, "nothing"},
    {BOOL, "bool"},
    {VECTOR, "vector"},
    {BUILTIN, "builtin"},
    {TYPE, "type"},
    {EXPRESSION, "expression"},
};

/* Assert that a condition is truthy, or print an error message and exit. */
void _lisp_assert_or_exit(bool condition, std::string message) {
    if (!condition) {
        std::cout << message << '\n';
        exit(1);
    }
}
[[noreturn]] void _throw_does_not_implement(LispType type,
                                            std::string notimplemented) {
    std::cout << "LispType '" << TYPENAMES.at(type) << "' does not implement `"
              << notimplemented << "`.";
    exit(1);
}

/* A simple tree template. */
template <class T>
class Tree {
   public:
    std::vector<T> nodes;
    std::vector<unsigned int> depths;

    size_t size() { return this->nodes.size(); }

    /* Returns a new subtree from a node and its children. */
    Tree<T> subtree(unsigned int index) {
        Tree<T> result;
        auto base_depth = this->depths[index];
        auto size = this->size();

        result.nodes.push_back(nodes[index]);
        result.depths.push_back(0);

        for (size_t i = index + 1; (i < size) && (depths[i] > base_depth);
             i++) {
            result.nodes.push_back(nodes[i]);
            result.depths.push_back(depths[i] - base_depth);
        }
        return result;
    }

    bool operator==(Tree<T> *tree) {
        unsigned int size = this->size();
        for (size_t i = 0; i < size; i++) {
            if (this->depths[i] != tree->depths[i]) { return false; }
            if (this->nodes[i] != tree->nodes[i]) { return false; }
        }
        return true;
    }

    bool operator!=(Tree<T> *tree) { return !(*this == tree); }
};

/* A Lisp runtime variable. */
class LispVar {
   public:
    LispType tag;
    union {
        long int num;                  // Used by NUM, NOTHING and BOOL.
        std::string *string;           // Used by STRING, TOKEN and TYPE.
        std::vector<LispVar> *vector;  // Used by LIST, VECTOR, and SOMETHING.
        Tree<LispVar> *tree;           // Used by EXPRESSION to store code.
    };

    /* Whether or not one variable equals another. */
    bool operator==(LispVar var) {
        if ((&var) == this) return true;
        if (var.tag != this->tag) return false;
        // Must be the same object since tags are the same.
        if (this->is_singleton()) return true;
        if (this->is_numeric()) { return this->num == var.num; }

        if (this->tag == STRING || this->tag == VECTOR || this->tag == LIST) {
            // Loop through the contents.
            unsigned int size = var.size();
            if (size != this->size()) return false;
            for (size_t i = 0; i < size; i++) {
                if ((*this->vector)[i] != (*var.vector)[i]) return false;
            }
            return true;
        }

        if (this->tag == EXPRESSION) return (*this->tree) == var.tree;

        std::cout << "Reached forbidden part of the == operator for LispVar.\n";
        exit(1);
    }

    bool operator!=(LispVar var) { return !(*this == var); }

    bool is_singleton() {
        return (this->tag == __NOT_SET__ || this->tag == __NO_ARGS__ ||
                this->tag == NOTHING);
    }

    bool is_numeric() {
        return (this->tag == NUM || this->tag == NOTHING || this->tag == BOOL);
    }

    bool is_sized() {
        return (this->tag == STRING || this->tag == VECTOR ||
                this->tag == LIST || this->tag == EXPRESSION);
    }

    bool is_booly() {
        return (this->tag == NUM || this->tag == NOTHING || this->tag == BOOL ||
                this->is_sized());
    }

    bool truthiness() {
        if (this->tag == NUM or this->tag == NOTHING or this->tag == BOOL)
            return this->num;
        if (this->is_sized()) return this->size();
        _throw_does_not_implement(this->tag, "truthiness");
    }

    long size() {
        if (this->tag == STRING) return this->string->size();
        if (this->tag == VECTOR || this->tag == LIST)
            return this->vector->size();
        if (this->tag == EXPRESSION) return this->tree->size();
        _throw_does_not_implement(this->tag, "size");
    }

    std::string to_repr() {
        auto str = this->to_str();
        if (this->tag == STRING) { return escape_string(str); }
        return str;
    }

    std::string _pretty_tree() {
        std::stringstream ss;
        auto size = this->tree->size();
        for (size_t i = 0; i < size; i++) {
            if (this->tree->depths[i]) ss << "│ ";
            for (size_t j = 1; j < this->tree->depths[i]; j++) ss << "· ";
            ss << this->tree->nodes[i].to_str();
            if (i != size - 1) { ss << "\n"; }
        }
        return ss.str();
    }

    std::string to_str();
    std::string get_help_str();
};

LispVar parse_and_evaluate(std::string input);

std::map<std::string, LispVar *> BUILTINS_TYPES = {

};
bool BUILTINS_TYPES_READY = false;

LispVar _SINGLETON_NOTHING = {NOTHING, 0};
LispVar _SINGLETON_NOT_SET = {__NOT_SET__, 0};
LispVar _SINGLETON_NOARGS_TOKEN = {__NO_ARGS__, 0};

/* Accumulate a vector of numeric LispVars into an integer. */
long accumulate(std::vector<LispVar> *args,
                long base,
                std::function<long(long, long)> func) {
    for (auto arg : *args) { base = func(base, arg.num); }
    return base;
}

/* Returns a boolean corresponding to whether or not all the items are
 * ordered according to the function `func`.

Examples
--------
std::vector<int> v = {1, 2, 3};
bool result = vector_is_ordered(&v, std::lesser<long>()) // true
bool result = vector_is_ordered(&v, std::greater<long>()) // false
 . */
bool vector_is_ordered(std::vector<LispVar> *args,
                       std::function<bool(long, long)> func) {
    unsigned int arity = args->size();
    if (!arity) return true;

    bool output = true;
    long last_num = (*args)[0].num;
    for (size_t i = 1; i < arity; i++) {
        output = func(last_num, (*args)[i].num);
        if (!output) break;
        last_num = (*args)[i].num;
    }

    return output;
}

std::string LispVar::to_str() {
    unsigned int size;
    std::stringstream ss;

    // std::cout << "(DEBUG:tag:" << this->tag << ") ";

    if (this->tag == BUILTIN) {
        // std::cout << BUILTINS_TYPES.at(*(this->string))->tag << '\n';
        ss << "<Builtin '"
           << *(this->string)
           // << BUILTINS_TYPES.at(*(this->string))->to_str()
           << "'>";
    } else if (this->tag == TYPE) {
        ss << "<Type '" << *(this->string) << "'>";
    } else if (this->tag == EXPRESSION) {
        ss << this->_pretty_tree();
    } else if (this->tag == NUM)
        ss << this->num;
    else if (this->tag == NOTHING)
        ss << "Nothing";
    else if (this->tag == __NO_ARGS__)
        ss << "<Special token '__NO_ARGS__'>";
    else if (this->tag == BOOL) {
        ss << (this->num ? "True" : "False");
    } else if (this->tag == STRING) {
        ss << *(this->string);
    } else if (this->tag == LIST || this->tag == VECTOR) {
        size = this->vector->size();
        ss << "[";
        for (size_t i = 0; i < size; i++) {
            ss << (*this->vector)[i].to_repr();
            if (i != (size - 1)) { ss << " "; }
        }
        ss << "]";
    } else {
        if (!TYPENAMES.count(this->tag)) {
            std::cout << "Error: Tag " << this->tag << " not in TYPENAMES.\n";
        } else
            std::cout << "[CPPBug] The Lisp type" << TYPENAMES.at(this->tag)
                      << " does not implement `to_str`.\n";
        exit(1);
    }
    return ss.str();
}
std::string LispVar::get_help_str() {
    std::stringstream ss;

    if (this->tag == BUILTIN) {
        ss << *(this->string) << BUILTINS_TYPES[*this->string];
    } else if (this->tag == TYPE) {
        return "a type";
    } else if (this->tag == EXPRESSION) {
        return "an expression";
    } else if (this->tag == NUM)
        return "a number";
    else if (this->tag == NOTHING)
        return "nothing";
    else if (this->tag == BOOL) {
        return "either True or False";
    } else if (this->tag == STRING) {
        return "a string of characters";
    } else if (this->tag == LIST || this->tag == VECTOR) {
        return "a vector of other items";
    } else {
        if (!TYPENAMES.count(this->tag)) {
            std::cout << "Error: Tag " << this->tag << " not in TYPENAMES.\n"
                      << '\n';
        } else
            std::cout << "[CPPBug] The Lisp type" << TYPENAMES.at(this->tag)
                      << " does not implement `get_help_str`.\n";
        exit(1);
    }
    return ss.str();
}

LispVar evaluate_builtin(LispVar op,
                         std::vector<LispVar> args,
                         bool safe = true);
LispVar evaluate_const(std::string item);
LispVar parse_expression(std::string expression);

void _fill_out_lisp_builtin_types() {
    auto star_numeric = new LispVar;
    auto star = new LispVar;
    auto one = new LispVar;
    auto list = new LispVar;
    auto string = new LispVar;
    auto star_list = new LispVar;
    auto one_numeric = new LispVar;
    auto two_numeric = new LispVar;
    auto range_type = new LispVar;
    auto insert_type = new LispVar;
    auto assert_type = new LispVar;
    auto bool_type = new LispVar;
    auto slice_type = new LispVar;
    auto repeat_type = new LispVar;
    auto get_type = new LispVar;
    auto typematch_type = new LispVar;
    auto map_type = new LispVar;
    auto fold_type = new LispVar;
    auto ternary_type = new LispVar;

    // Declares the necessary types.
    *star_numeric = parse_and_evaluate("[(map type ['*' 'numeric'])]");
    *star = parse_and_evaluate("[(map type ['*'])]");
    *one = parse_and_evaluate("[(map type ['any'])]");
    *list = parse_and_evaluate("[(map type ['list'])]");
    *string = parse_and_evaluate("[(map type ['string'])]");
    *star_list = parse_and_evaluate("[(map type ['*' 'list'])]");
    *one_numeric = parse_and_evaluate("[(map type ['numeric'])]");
    *two_numeric =
        parse_and_evaluate("[(map type ['numeric']) (map type ['numeric'])]");
    *range_type = parse_and_evaluate(
        "[(map type ['numeric' '?']) (map type ['numeric' '?']) (map type "
        "['numeric' '?' 'truthy'])]");
    *insert_type = parse_and_evaluate(
        "[(map type ['list']) (map type ['numeric']) (map type ['any'])]");
    *assert_type =
        parse_and_evaluate("[(map type ['booly']) (map type ['string' '?'])]");
    *bool_type = parse_and_evaluate("[(map type ['booly'])]");
    *slice_type = parse_and_evaluate(
        "[(map type ['list']) (map type ['numeric']) (map type ['numeric' "
        "'?']) (map type "
        "['numeric' '?' 'truthy'])]");
    *repeat_type =
        parse_and_evaluate("[(map type ['numeric']) (map type ['any'])]");
    *get_type =
        parse_and_evaluate("[(map type ['numeric']) (map type ['list'])]");
    *typematch_type =
        parse_and_evaluate("[(map type ['list']) (map type ['list'])]");
    *map_type =
        parse_and_evaluate("[(map type ['builtin']) (map type ['list' '*'])]");
    *fold_type = parse_and_evaluate(
        "[(map type ['builtin']) (map type ['any']) (map type ['list'])]");
    *ternary_type = parse_and_evaluate(
        "[(map type ['booly']) (map type ['any']) (map type ['any'])]");

    // General types.
    BUILTINS_TYPES["*"] = BUILTINS_TYPES["mul"] = BUILTINS_TYPES["+"] =
        BUILTINS_TYPES["add"] = BUILTINS_TYPES["&"] = BUILTINS_TYPES["and"] =
            BUILTINS_TYPES["|"] = BUILTINS_TYPES["or"] = BUILTINS_TYPES[">"] =
                BUILTINS_TYPES["gt"] = BUILTINS_TYPES[">="] =
                    BUILTINS_TYPES["ge"] = BUILTINS_TYPES["<"] =
                        BUILTINS_TYPES["lt"] = BUILTINS_TYPES["<="] =
                            BUILTINS_TYPES["le"] = BUILTINS_TYPES["=="] =
                                BUILTINS_TYPES["eq"] = BUILTINS_TYPES["!="] =
                                    BUILTINS_TYPES["neq"] =
                                        BUILTINS_TYPES["^"] =
                                            BUILTINS_TYPES["xor"] =
                                                star_numeric;

    BUILTINS_TYPES["%"] = BUILTINS_TYPES["mod"] = BUILTINS_TYPES["-"] =
        BUILTINS_TYPES["sub"] = BUILTINS_TYPES["/"] = BUILTINS_TYPES["div"] =
            two_numeric;

    BUILTINS_TYPES["typeof"] = BUILTINS_TYPES["symbol"] =
        BUILTINS_TYPES["repr"] = BUILTINS_TYPES["put"] =
            BUILTINS_TYPES["expression"] = one;

    BUILTINS_TYPES["neg"] = BUILTINS_TYPES["~"] = BUILTINS_TYPES["flip"] =
        one_numeric;

    BUILTINS_TYPES["noop"] = BUILTINS_TYPES["list"] = star;
    BUILTINS_TYPES["len"] = BUILTINS_TYPES["eval"] = list;
    BUILTINS_TYPES["join"] = star_list;
    BUILTINS_TYPES["type"] = string;

    // Custom types.
    BUILTINS_TYPES["assert"] = assert_type;
    BUILTINS_TYPES["bool"] = bool_type;
    BUILTINS_TYPES["fold"] = fold_type;
    BUILTINS_TYPES["get"] = get_type;
    BUILTINS_TYPES["insert"] = insert_type;
    BUILTINS_TYPES["map"] = map_type;
    BUILTINS_TYPES["range"] = range_type;
    BUILTINS_TYPES["repeat"] = repeat_type;
    BUILTINS_TYPES["slice"] = slice_type;
    BUILTINS_TYPES["ternary"] = BUILTINS_TYPES["?"] = ternary_type;
    BUILTINS_TYPES["typematch"] = typematch_type;

    BUILTINS_TYPES_READY = true;

    if (BUILTINS_TYPES.size() != LISP_BUILTINS.size()) {
        throw std::runtime_error("Not all builtins have been typed.");
    }
}

[[noreturn]] void _throw_could_not_cast(LispVar expected, LispVar actual) {
    std::cout << "[casting error] Could not cast `" << actual.to_str()
              << "` to signature `" << expected.to_str() << "`.";
    exit(1);
}

// ===| BUILTINS |===

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

/* Parse a Lisp expression into a tree. */
LispVar parse_expression(std::string expression) {
    auto ast = new Tree<LispVar>;
    *ast = {{_SINGLETON_NOT_SET}, {0}};

    LispVar output;
    output.tag = EXPRESSION;
    output.tree = ast;

    const auto size = expression.size();
    const std::string incr_depth = "([{";
    const std::string decr_depth = "}])";
    const auto npos = std::string::npos;

    unsigned int tree_length = 1;
    unsigned int depth = 0;
    int depth_buffer;
    int d_depth;  // Depth difference

    bool last_is_empty;
    bool in_string_literal = false;
    bool last_was_backslash = false;
    bool last_was_paren = false;
    bool last_was_paren_buffer = false;  // One token behind.
    bool line_is_comment = false;

    char character;

    std::string string_buffer;

    // toggle the call thing on and off

    for (size_t i = 0; i < size; i++) {
        character = expression[i];
        d_depth = (incr_depth.find(character) != npos) -
                  (decr_depth.find(character) != npos);
        last_was_paren = last_was_paren || (d_depth > 0);
        depth += d_depth;

        last_was_paren_buffer = last_was_paren || last_was_paren_buffer;

        // Toggle line comment on ;, continuing until a newline.
        line_is_comment = (line_is_comment && character != '\n') |
                          (!in_string_literal && character == ';');

        last_is_empty = string_buffer.empty();

        // Finish the current token on whitespace or parentheses outside
        // Q-expressions.
        if (!in_string_literal &&
            (line_is_comment || isspace(character) || d_depth)) {
            if (character == '{') {
                string_buffer += "expression";
                if (last_is_empty) depth_buffer = depth;
                last_is_empty = 0;
                last_was_paren = 0;
            }

            if (character == '[') {
                string_buffer += "list";
                if (last_is_empty) depth_buffer = depth;
                last_is_empty = 0;
                last_was_paren = 0;
            }

            if (!last_is_empty) {
                ast->nodes[tree_length - 1] = evaluate_const(string_buffer);
                ast->depths[tree_length - 1] = depth_buffer;

                if (last_was_paren_buffer) {
                    ast->nodes.push_back(_SINGLETON_NOARGS_TOKEN);
                    ast->depths.push_back(ast->depths[tree_length - 1] + 1);
                    tree_length++;
                }

                string_buffer = "";
                ast->depths.push_back(depth);
                ast->nodes.push_back(_SINGLETON_NOT_SET);
                tree_length++;
                last_was_paren_buffer = 0;
                last_was_paren = 0;
            }
            continue;
        }

        // Keeps track of whether or not the parser is inside a string
        // literal to avoid splitting at bad times.
        in_string_literal ^= character == '\'' && !last_was_backslash;
        last_was_backslash = character == '\\';

        if (last_is_empty) depth_buffer = depth + !last_was_paren;

        // Add to the current token.
        string_buffer.push_back(character);
        last_was_paren = false;
    }

    if (depth) {
        throw std::runtime_error("Expression '" + expression +
                                 "' does not have balanced parentheses!");
    }

    if (ast->nodes[tree_length - 1] == _SINGLETON_NOT_SET) {
        ast->depths.pop_back();
        ast->nodes.pop_back();
    }

    auto new_tree = new Tree<LispVar>;

    // Removes the __NO_ARGS__ tokens from the stream
    // if they aren't necessary.
    for (size_t i = 0; (i + 1) < tree_length; i++) {
        if (ast->nodes[i] == _SINGLETON_NOARGS_TOKEN) {
            if (ast->depths[i] == ast->depths[i + 1]) continue;
        }
        new_tree->nodes.push_back(ast->nodes[i]);
        new_tree->depths.push_back(ast->depths[i]);
    }

    output.tree = new_tree;
    delete ast;
    return output;
}

/* Matches a vector of LispVars against a LispVar LIST containing TYPEs.

Allowed sub-sub-values of `expected`
====================================
- truthy: (bool arg)
- falsy: (! (bool arg))
- iterable: Arg is String or Q-expression.
- numeric: Arg is Num, Bool, or Nothing.
- any: Any type. Omitting this changes nothing - it's only used for
readability.
- (typename): The specific type.
- *: Matches argument repeatedly.
- +: Matches argument at least once.

*/
bool _types_match(std::vector<LispVar> args, LispVar expected) {
    if (expected.tag != LIST) {
        auto temp_str = "[['type' '*'] '*']";
        _throw_could_not_cast({TYPE, *temp_str}, expected);
    }

    LispVar pattern;
    LispVar arg;

    bool increase_expected_pointer;
    unsigned int expected_pointer = 0;
    unsigned int num_matched_in_repeat = 0;

    const auto expected_size = expected.vector->size();
    const auto args_size = args.size();

    auto expected_size_minus_stars = expected_size;

    bool has_star = 0;
    bool has_add = 0;
    bool has_qmrk = 0;
    bool matched = 0;
    bool matched_has_been_set = 0;

    for (size_t i = 0; i < args_size; i++) {
        // Check if continuing will go past the size of the expected vector.
        if (expected_pointer >= expected_size) { return false; }

        pattern = (*expected.vector)[expected_pointer];
        if (pattern.tag != LIST) {
            auto temp_str = "[type *]";
            _throw_could_not_cast({TYPE, *temp_str}, expected);
        }

        arg = args[i];
        matched = true;
        increase_expected_pointer = true;

        for (auto type : (*pattern.vector)) {
            if (type.tag != TYPE) {
                auto temp_str = "type";
                _throw_could_not_cast({TYPE, *temp_str}, type);
            }

            if (!type.string->compare("any")) {
            } else if (!type.string->compare("booly")) {
                matched &= arg.is_booly();
            } else if (!type.string->compare("truthy")) {
                matched &= arg.truthiness();
            } else if (!type.string->compare("falsy")) {
                matched &= !arg.truthiness();
            } else if (!type.string->compare("iterable")) {
                matched &= arg.is_sized();
            } else if (!type.string->compare("numeric")) {
                matched &= arg.is_numeric();
            } else if (!type.string->compare("*")) {
                has_star = 1;
                expected_size_minus_stars -= !num_matched_in_repeat;
            } else if (!type.string->compare("+")) {
                has_add = 1;
                expected_size_minus_stars -= !num_matched_in_repeat;
            } else if (!type.string->compare("?")) {
                has_qmrk = 1;
                expected_size_minus_stars -= !num_matched_in_repeat;
            } else {
                // Iterate over all allowed types.
                for (size_t i = 0; i < TYPENAMES.size(); i++) {
                    if (!type.string->compare(TYPENAMES.at(i))) {
                        matched &= arg.tag == i;
                        matched_has_been_set = true;
                        break;
                    }
                }

                if (!matched_has_been_set) {
                    // The type is malformed, so an error is thrown.
                    std::cout << "Could not typecheck with malformed type "
                              << type.to_repr() << ".\n";
                    exit(1);
                }
            }
        }

        if (has_add || has_star || has_qmrk) {
            // Increases or sets the counter to 0.
            num_matched_in_repeat += matched;
            num_matched_in_repeat *= matched;
            // Removes the star and add for the next matching attempt.
            has_star &= matched;
            has_add &= matched;
            has_qmrk &= matched;
            // Increase the pointer only if it didn't match.
            increase_expected_pointer = !matched;

            if (has_add && !matched && !num_matched_in_repeat) {
                // The match failed due to getting 0 matches on a +
                // argument.
                std::cout << "/* message */" << '\n';
                return false;
            }

            if (has_qmrk && (num_matched_in_repeat > 1)) {
                // The match failed due to getting > 1 matches on a ?
                // argument.
                std::cout << "/* message */" << '\n';
                continue;
                // return false;
            }

        } else if (!matched) {
            // The match failed by not matching the type exactly and not
            // having a repeat flag set.
            return false;
        }

        expected_pointer += increase_expected_pointer;
    }

    // Iterate through the remaining arguments to find the stars.
    // This is done to avoid bugs where lower-arity calls don't match
    // higher-arity conditional calls.
    bool _flag;
    for (size_t i = expected_pointer + 1; i < expected_size; i++) {
        pattern = (*expected.vector)[expected_pointer];
        if (pattern.tag != LIST) {
            auto temp_str = "[type *]";
            _throw_could_not_cast({TYPE, *temp_str}, expected);
        }

        for (auto type : (*pattern.vector)) {
            _flag = (!type.string->compare("*") || !type.string->compare("+") ||
                     !type.string->compare("?"));
            expected_size_minus_stars -= _flag;
            if (_flag) { continue; }
        }
    }

    return (expected_pointer >= expected_size_minus_stars &&
            expected_pointer <= expected_size);
}

/* Perform an operation on the inputs. */
LispVar evaluate_builtin(LispVar operation,
                         std::vector<LispVar> args,
                         bool safe) {
    LispVar output;
    LispType kind = __NOT_SET__;
    auto arity = args.size();
    auto op = *operation.string;
    bool all_same = true;

    if (arity == 1 && args[0] == _SINGLETON_NOARGS_TOKEN) args = {};

    if (arity) {
        kind = args[0].tag;
        for (size_t i = 1; (i < arity) and all_same; i++) {
            all_same &= args[i].tag == kind;
        }
    } else
        all_same = false;

    // Typecheck the arguments.
    if (BUILTINS_TYPES_READY && safe) {
        auto arg_types_match = _types_match(args, *BUILTINS_TYPES.at(op));
        if (!arg_types_match) {
            LispVar actual_type;
            actual_type.tag = LIST;
            actual_type.vector = &args;
            _throw_could_not_cast(*BUILTINS_TYPES.at(op), actual_type);
        }
    }

    // No operation.
    if (!op.compare("noop")) { return _SINGLETON_NOTHING; }

    // ===| Base constructors |===
    if (!op.compare("bool")) return {BOOL, args[0].truthiness()};
    if (!op.compare("type")) {
        output.string = new std::string;
        *output.string = *args[0].string;
        output.tag = TYPE;
        return output;
    }
    if (!op.compare("list")) {
        output.vector = new std::vector<LispVar>;
        *output.vector = args;
        output.tag = LIST;
        return output;
    }
    if (!op.compare("symbol")) return args[0];

    // ===| Help functions |===
    if (!op.compare("help")) {
        output.string = new std::string;
        *output.string = args[0].get_help_str();
        output.tag = STRING;
        return output;
    }

    if (!op.compare("len")) return {NUM, args[0].size()};

    // Get the type of {0}.
    if (!op.compare("typeof")) {
        output.string = new std::string;
        *output.string = TYPENAMES.at(args[0].tag);
        output.tag = TYPE;
        return output;
    }

    // Matches the type {0} against the variables {1}.
    if (!op.compare("typematch")) {
        return {BOOL, _types_match(*args[1].vector, args[0])};
    }

    // Assertions.
    if (!op.compare("assert")) {
        auto temp_string = "bool";
        bool success =
            evaluate_builtin(LispVar{BUILTIN, *temp_string}, {args[0]}).num;
        _lisp_assert_or_exit(
            success, ((arity == 2) ? *args[1].string : "AssertionError"));
        return _SINGLETON_NOTHING;
    }

    // Write a string to `cout`.
    if (!op.compare("put")) {
        std::cout << args[0].to_str();
        return _SINGLETON_NOTHING;
    }

    // Get the string representation of {0}.
    if (!op.compare("repr")) {
        output.string = new std::string;
        output.tag = STRING;

        *output.string = (args[0].tag == STRING)
                             ? escape_string(args[0].to_str())
                             : args[0].to_str();
        return output;
    }

    // Joins the contents of Q-Expressions.
    if (!op.compare("join")) {
        if (kind == LIST) {
            output.vector = new std::vector<LispVar>;
            output.tag = LIST;
            for (auto arg : args)
                for (auto i : *arg.vector) output.vector->push_back(i);
            return output;
        }
        if (kind == STRING) {
            output.string = new std::string;
            output.tag = STRING;
            for (auto arg : args) (*output.string) += (*arg.string);
            return output;
        }
    }

    // Insert {1} at index {2} in {0}.
    if (!op.compare("insert")) {
        output.vector = new std::vector<LispVar>;
        output.tag = LIST;

        auto size = args[0].size();
        auto index = args[2].num;

        // Allows for negative indices.
        index = (index < 0) ? size + index : index;
        _lisp_assert_or_exit(
            index <= size,
            "OutOfBoundsError: Second argument of `insert` must be "
            "less than or equal to the size of the first argument.");

        for (int i = 0; i < size; i++) {
            (*output.vector).push_back((*args[0].vector)[i]);
            if (i == index) (*output.vector).push_back(args[1]);
        }
        if (index == size) (*output.vector).push_back(args[1]);

        *output.vector = {(*args[0].vector)[args[0].vector->size() - 1]};
        return output;
    }

    // Repeat {1} {0} times.
    if (!op.compare("repeat")) {
        output.vector = new std::vector<LispVar>;
        for (int i = 0; i < args[0].num; i++)
            (*output.vector).push_back(args[1]);
        output.tag = LIST;

        return output;
    }

    // Get the {0}th element of {1}.
    if (!op.compare("get")) {
        auto size = args[1].size();
        auto index = args[0].num;

        // Allows for negative indices.
        index = (index < 0) ? size + index : index;
        _lisp_assert_or_exit(
            index < size,
            "OutOfBoundsError: {0} for `get` must be less than the "
            "size of {1}.");
        return (*args[1].vector)[index];
    }

    // Map a function across one or more iterable, getting an iterable back.
    if (!op.compare("map")) {
        output.vector = new std::vector<LispVar>;
        output.tag = LIST;

        if (arity > 1) {
            // Checks that the sizes of the lists are all equal.
            auto _size = new unsigned int;
            *_size = args[1].size();
            for (size_t i = 2; i < arity; i++) {
                _lisp_assert_or_exit(
                    args[i].size() == *_size,
                    "[SizeError] The sizes of lists used as arguments to "
                    "`map` must all be of equal length.");
            }

            // Evaluate the function over the members.
            auto _vector = new std::vector<LispVar>;
            for (size_t i = 0; i < *_size; i++) {
                for (size_t j = 1; j < arity; j++) {
                    _vector->push_back((*args[j].vector)[i]);
                }
                (*output.vector).push_back(evaluate_builtin(args[0], *_vector));
                _vector->clear();
            }
            delete _vector;
            delete _size;
        }

        return output;
    }

    if (!op.compare("fold")) {
        auto accumulator = args[1];
        for (auto arg : *args[2].vector) {
            accumulator = evaluate_builtin(args[0], {accumulator, arg});
        }
        return accumulator;
    }

    // Generates a list using a slice-like syntax.
    if (!op.compare("range")) {
        int start = 0;
        int stop = -1;
        int step = 1;

        // Allows any arity between 1 and 3.
        if (arity == 1)
            stop = args[0].num;
        else {
            start = args[0].num;
            stop = args[1].num;
        }
        if (arity >= 3) step = args[2].num;

        output.vector = new std::vector<LispVar>;
        output.tag = LIST;

        if (stop > start and step < 0) return output;
        if (stop < start and step > 0) return output;

        for (int i = start; (step > 0) ? (i < stop) : (i > stop); i += step) {
            (*output.vector).push_back({NUM, i});
        }
        return output;
    }
    if (!op.compare("slice")) {
        long int start = 0;
        long int stop = -1;
        long int step = 1;
        auto size = args[0].size();

        // Allows any arity between 2 and 4.
        if (arity >= 2) start = args[1].num;
        if (arity >= 3) stop = args[2].num;
        if (arity >= 4) step = args[3].num;

        // Allows for negative indices.
        start = (start < 0) ? size + start : start;
        stop = (stop < 0) ? size + stop : stop;

        // Cut off too large indices.
        stop = std::min(stop, size);
        start = std::min(start, size);

        output.tag = LIST;
        output.vector = new std::vector<LispVar>;

        if (stop > start and step < 0) return output;
        if (stop < start and step > 0) return output;

        for (int i = start; (step > 0) ? (i < stop) : (i > stop); i += step) {
            (*output.vector).push_back((*args[0].vector)[i]);
        }

        return output;
    }

    if (!op.compare("eval")) {
        return evaluate_builtin(
            (*args[0].vector)[0],
            {(*args[0].vector).begin() + 1, (*args[0].vector).end()});
    }

    // Flow control
    if (!op.compare("?") || !op.compare("ternary")) {
        auto temp_str = "bool";
        auto result = evaluate_builtin({BUILTIN, *temp_str}, {args[0]}).num
                          ? args[1]
                          : args[2];
        return result;
    }

    // Numeric functions
    if (!op.compare("+") || !op.compare("add")) {
        output.tag = NUM;
        output.num = accumulate(&args, 0, std::plus<long>());
        return output;
    }
    if (!op.compare("*") || !op.compare("mul")) {
        output.tag = NUM;
        output.num = accumulate(&args, 1, std::multiplies<long>());
        return output;
    }
    if (!op.compare("&") || !op.compare("and")) {
        output.tag = NUM;
        output.num = accumulate(&args, ~0, std::bit_and<long>());
        return output;
    }
    if (!op.compare("|") || !op.compare("or")) {
        output.tag = NUM;
        output.num = accumulate(&args, 0, std::bit_or<long>());
        return output;
    }
    if (!op.compare("^") || !op.compare("xor")) {
        output.tag = NUM;
        output.num = accumulate(&args, 0, std::bit_xor<long>());
        return output;
    }
    if (!op.compare("==") || !op.compare("eq")) {
        // All arguments should be equal.
        output.num = true;
        output.tag = BOOL;
        if (!arity) return output;
        auto last = (args)[0];
        for (size_t i = 1; i < arity; i++) {
            output.num = last == (args)[i];
            if (!output.num) break;
            last = (args)[i];
        }
        return output;
    }
    if (!op.compare("!=") || !op.compare("neq")) {
        // All arguments should be different.
        output.num = true;
        output.tag = BOOL;
        if (!arity) return output;
        auto last = (args)[0];
        for (size_t i = 1; i < arity; i++) {
            output.num = last != (args)[i];
            if (!output.num) break;
            last = (args)[i];
        }
        return output;
    }
    if (!op.compare(">") || !op.compare("gt")) {
        // All arguments should be strictly decreasing.
        output.num = vector_is_ordered(&args, std::greater<long>());
        output.tag = BOOL;
        return output;
    }
    if (!op.compare("<") || !op.compare("lt")) {
        // All arguments should be strictly increasing.
        output.num = vector_is_ordered(&args, std::less<long>());
        output.tag = BOOL;
        return output;
    }
    if (!op.compare(">=") || !op.compare("geq")) {
        // All arguments should be non-strictly decreasing.
        output.num = vector_is_ordered(&args, std::greater_equal<long>());
        output.tag = BOOL;
        return output;
    }
    if (!op.compare("<=") || !op.compare("leq")) {
        // All arguments should be non-strictly increasing.
        output.num = vector_is_ordered(&args, std::less_equal<long>());
        output.tag = BOOL;
        return output;
    }

    // 2-ary numeric functions
    if (!op.compare("-") || !op.compare("sub"))
        return {NUM, args[0].num - args[1].num};
    if (!op.compare("/") || !op.compare("div"))
        return {NUM, args[0].num / args[1].num};
    if (!op.compare("%") || !op.compare("mod"))
        return {NUM, args[0].num % args[1].num};

    // 1-ary functions
    if (!op.compare("neg")) return {NUM, -args[0].num};
    if (!op.compare("~") || !op.compare("flip")) return {NUM, ~args[0].num};

    // Results haven't been set.
    std::cout << "ExpressionEvaluationError: Could not evaluate (" << op;
    auto size = args.size();
    for (size_t i = 0; i < size; i++) std::cout << " " << args[i].to_str();
    std::cout << ")\n";
    exit(1);
}

/* Evaluate a constant. */
LispVar evaluate_const(std::string item) {
    LispVar output;
    bool is_function = LISP_BUILTINS.count(item);

    if (is_function) {
        output.string = new std::string;
        *output.string = item;
        output.tag = BUILTIN;

        return output;
    }

    if (!item.compare("True")) { return LispVar{BOOL, 1}; }
    if (!item.compare("False")) { return LispVar{BOOL, 0}; }
    if (!item.compare("Nothing")) { return LispVar{NOTHING, 0}; }

    if ((item[0] == '\'' and item[item.size() - 1] == '\'')) {
        output.string = new std::string;
        // Removes the quotes surrounding the string literal.
        *output.string = {item.begin() + 1, item.end() - 1};
        output.tag = STRING;
        return output;
    }

    try {
        return LispVar{NUM, std::stoi(item)};
    } catch (const std::invalid_argument &e) {
        std::cout << "ConstantParsingError: could not parse constant '" << item
                  << "'.\n";
        exit(1);
    }
}

/* Evaluates a tree. */
LispVar evaluate_tree(LispVar *expression, unsigned int index) {
    auto item = expression->tree->nodes[index];
    bool is_function = item.tag == BUILTIN;

    if (!is_function) return item;

    // Capture expressions literally.
    if (!item.string->compare("expression")) {
        LispVar result;
        Tree<LispVar> *subtree = new Tree<LispVar>;
        *subtree = expression->tree->subtree(index);

        result.tag = EXPRESSION;
        result.tree = subtree;
        return result;
    }

    std::vector<LispVar> children;
    auto size = expression->tree->nodes.size();
    auto original_depth = expression->tree->depths[index];

    for (size_t i = index + 1;
         (original_depth != expression->tree->depths[i]) and i < size;
         i++) {
        if ((original_depth + 1) == expression->tree->depths[i]) {
            children.push_back(evaluate_tree(expression, i));
        }
    }

    if (children.empty()) { return item; }
    return evaluate_builtin(item, children);
}

LispVar parse_and_evaluate(std::string input) {
    auto tree = parse_expression(input);
    return evaluate_tree(&tree, 0);
}

int main(int argc, char const *argv[]) {
    if (argc != 2) {
        std::cout
            << "Error: Expected exactly 1 command-line argument (filename)."
            << '\n';
        exit(1);
    }

    // _fill_out_lisp_builtin_types();

    std::ifstream t(argv[1]);
    std::stringstream buffer;
    buffer << t.rdbuf();
    auto expr = "(noop " + buffer.str() + ")";
    auto tree = parse_expression(expr);
    std::cout << "[DEBUG] Dumping AST below:" << '\n';
    std::cout << tree.to_str() << '\n';
    std::cout << "[DEBUG] Done." << '\n';
    std::cout << "[DEBUG] Evaluating AST at node 0." << '\n';
    evaluate_tree(&tree, 0);
    return 0;
}
