/* Defines the LispVar class and singleton Lisp variables.

The singletons are the following:
- _SINGLETON_NOTHING
- _SINGLETON_NOT_SET
- _SINGLETON_NOARGS_TOKEN
*/
#include <cassert>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "escape.h"
#include "gen_builtins.h"
#include "tree.h"

#define NUMPART(a) (a.tag == NUM ? a.num : a.flt)
#define PNUMPART(a) (a->tag == NUM ? a->num : a->flt)

enum LispType {
    NUM,
    FLOAT,
    STRING,
    LIST,
    NOTHING,
    BOOL,
    BUILTIN,
    TYPE,
    EXPRESSION,
    VARIABLE,
    CLOSURE,
    __NOT_SET__,
    __NO_ARGS__,
};

[[noreturn]] void _throw_does_not_implement(LispType type,
                                            std::string notimplemented);

/* A Lisp runtime variable. */
class LispVar {
   public:
    LispType tag;
    union {
        long int num;                  // Used by NUM, NOTHING, BOOL.
        float flt;                     // Used by FLOAT.
        std::string *string;           // Used by STRING, TYPE, and VARIABLE.
        std::vector<LispVar> *vector;  // Used by LIST.
        Tree<LispVar> *tree;           // Used by EXPRESSION.
        LispBuiltin builtin;           // Used by BUILTIN.
    };

    /* Whether or not one variable equals another. */
    bool operator==(LispVar var) {
        if ((&var) == this) return true;
        if (var.tag != tag) return false;
        // If it's a singleton, tags being the same imply that the objects are
        // the same.
        if (is_singleton()) return true;
        if (is_numeric() || tag == BUILTIN) {
            return PNUMPART(this) == NUMPART(var);
        }
        if (tag == STRING) return !(*string).compare(*var.string);

        // Compare the contents.
        if (tag == LIST) {
            unsigned int var_size = var.size();
            if (var_size != size()) return false;
            for (size_t i = 0; i < var_size; i++) {
                if ((*vector)[i] != (*var.vector)[i]) return false;
            }
            return true;
        }

        if (tag == EXPRESSION) return (*tree) == var.tree;

        std::cout << tag << '\n';
        std::cout << "Reached forbidden part of the == operator for LispVar.\n";
        exit(1);
    }

    bool operator!=(LispVar var) { return !(*this == var); }

    constexpr bool is_singleton() {
        return (tag == __NOT_SET__ || tag == __NO_ARGS__ || tag == NOTHING);
    }

    bool is_numeric() { return (tag == NUM || tag == BOOL || tag == FLOAT); }

    bool is_callable() { return (tag == BUILTIN || tag == CLOSURE); }

    bool is_sized() {
        return (tag == STRING || tag == LIST || tag == EXPRESSION ||
                tag == CLOSURE);
    }

    bool is_booly() {
        return (tag == NUM || tag == NOTHING || tag == BOOL || is_sized());
    }

    bool truthiness() {
        if (tag == NUM || tag == NOTHING || tag == BOOL || tag == FLOAT)
            return num;
        if (is_sized()) return size();
        _throw_does_not_implement(tag, "truthiness");
    }

    long size() {
        if (tag == STRING) return string->size();
        if (tag == LIST) return vector->size();
        if (tag == EXPRESSION || tag == CLOSURE) return tree->size();
        _throw_does_not_implement(tag, "size");
    }

    LispVar copy() {
        LispVar output;
        output.tag = tag;
        if (tag == NUM || tag == NOTHING || tag == BOOL)
            output.num = num;
        else if (tag == FLOAT)
            output.flt = flt;
        else if (tag == BUILTIN)
            output.builtin = builtin;
        else if (tag == STRING || tag == TYPE || tag == VARIABLE) {
            auto str = new std::string;
            *str = *string;
            output.string = str;
        } else if (tag == LIST) {
            auto vec = new std::vector<LispVar>;
            *vec = *vector;
            output.vector = vec;
        } else if (tag == EXPRESSION) {
            auto new_tree = new Tree<LispVar>;
            *new_tree = *tree;
            output.tree = new_tree;
        } else
            assert(false);

        return output;
    }

    /* Cast to float. */
    float to_f() {
        assert(is_numeric());
        return PNUMPART(this);
    }

    /* Cast to long. */
    long to_l() {
        assert(is_numeric());
        return PNUMPART(this);
    }

    std::string to_repr() {
        auto str = to_str();
        if (tag == STRING) { return escape_string(str); }
        return str;
    }

    std::string _pretty_tree() {
        std::stringstream ss;
        auto size = tree->size();
        for (size_t i = 0; i < size; i++) {
            if (tree->depths[i]) ss << "│ ";
            for (size_t j = 1; j < tree->depths[i]; j++) ss << "· ";
            ss << tree->nodes[i].to_str();
            if (i != size - 1) { ss << "\n"; }
        }
        return ss.str();
    }

    bool contents_have_same_tag() {
        if (tag != LIST) {
            std::cout << "ERROR" << '\n';
            exit(1);
        }
        unsigned int size = vector->size();
        for (size_t i = 1; i < size; i++) {
            if ((*vector)[i].tag != (*vector)[i - 1].tag) return 0;
        }
        return 1;
    }

    bool contents_are_unsized() {
        if (tag != LIST) {
            std::cout << "ERROR" << '\n';
            exit(1);
        }
        unsigned int size = vector->size();
        for (size_t i = 0; i < size; i++) {
            if ((*vector)[i].is_sized()) return 0;
        }
        return 1;
    }

    std::string to_str();
    std::string get_help_str();
};

LispVar parse_and_evaluate(std::string input);

auto _SINGLETON_NOTHING = new LispVar;
auto _SINGLETON_NOT_SET = new LispVar;
auto _SINGLETON_NOARGS_TOKEN = new LispVar;

const std::map<unsigned int, const std::string> TYPENAMES{
    {NUM, "int"},
    {STRING, "string"},
    {FLOAT, "float"},
    {LIST, "list"},
    {NOTHING, "nothing"},
    {BOOL, "bool"},
    {BUILTIN, "builtin"},
    {TYPE, "type"},
    {EXPRESSION, "expression"},
    {CLOSURE, "function"}};
