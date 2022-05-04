#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "escape.h"
#include "tree.h"

enum LispType {
    NUM,
    STRING,
    LIST,
    NOTHING,
    BOOL,
    BUILTIN,
    TYPE,
    EXPRESSION,
    VARIABLE,
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
        long int num;                  // Used by NUM, NOTHING and BOOL.
        std::string *string;           // Used by STRING, TOKEN and TYPE.
        std::vector<LispVar> *vector;  // Used by LIST, VECTOR, and SOMETHING.
        Tree<LispVar> *tree;           // Used by EXPRESSION to store code.
    };

    /* Whether or not one variable equals another. */
    bool operator==(LispVar var) {
        if ((&var) == this) return true;
        if (var.tag != this->tag) return false;
        // If it's a singleton, tags being the same imply that the objects are
        // the same.
        if (this->is_singleton()) return true;
        if (this->is_numeric()) { return this->num == var.num; }
        if (this->tag == STRING or this->tag == BUILTIN)
            return !(*this->string).compare(*var.string);

        // Compare the contents.
        if (this->tag == LIST) {
            unsigned int size = var.size();
            if (size != this->size()) return false;
            for (size_t i = 0; i < size; i++) {
                if ((*this->vector)[i] != (*var.vector)[i]) return false;
            }
            return true;
        }

        if (this->tag == EXPRESSION) return (*this->tree) == var.tree;

        std::cout << this->tag << '\n';
        std::cout << "Reached forbidden part of the == operator for LispVar.\n";
        exit(1);
    }

    bool operator!=(LispVar var) { return !(*this == var); }

    constexpr bool is_singleton() {
        return (this->tag == __NOT_SET__ || this->tag == __NO_ARGS__ ||
                this->tag == NOTHING);
    }

    bool is_numeric() {
        return (this->tag == NUM || this->tag == NOTHING || this->tag == BOOL);
    }

    bool is_sized() {
        return (this->tag == STRING || this->tag == LIST ||
                this->tag == EXPRESSION);
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
        if (this->tag == LIST) return this->vector->size();
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

    bool contents_have_same_tag() {
        if (this->tag != LIST) {
            std::cout << "ERROR" << '\n';
            exit(1);
        }
        unsigned int size = this->vector->size();
        for (size_t i = 1; i < size; i++) {
            if ((*this->vector)[i].tag != (*this->vector)[i - 1].tag) return 0;
        }
        return 1;
    }

    bool contents_are_unsized() {
        if (this->tag != LIST) {
            std::cout << "ERROR" << '\n';
            exit(1);
        }
        unsigned int size = this->vector->size();
        for (size_t i = 0; i < size; i++) {
            if ((*this->vector)[i].is_sized()) return 0;
        }
        return 1;
    }

    std::string to_str();
    std::string get_help_str();
};
LispVar parse_and_evaluate(std::string input);
