#include <math.h>

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <numeric>
#include <random>
#include <regex>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "./command.h"
#include "./num.h"
#include "./scoping.h"
#include "./vecex.h"

bool DEBUG_MODE;
bool SAFE_MODE;
// LispVar[1024] ARGS_BUFFER;

std::mt19937 RNG;

struct LispEarlyReturn : public std::exception {
    LispVar value;
    const char *what() const throw() {
        return "Uncaught LispEarlyReturn exception";
    }
};

struct LispBreak : public std::exception {
    const char *what() const throw() { return "Uncaught LispBreak exception"; }
};

/* Assert that a condition is truthy, or print an error message and exit. */
void _lisp_assert_or_exit(bool condition, std::string message) {
    if (!condition) {
        std::cout << message << '\n';
        exit(1);
    }
}

/* Display an error message that a type does not implement something. */
[[noreturn]] void _throw_does_not_implement(LispType type,
                                            std::string notimplemented) {
    std::cout << "[NotImplementedError] LispType '" << TYPENAMES.at(type)
              << "' does not implement `" << notimplemented << "`.";
    exit(1);
}

[[noreturn]] void _throw_could_not_cast(LispVar expected, LispVar actual) {
    std::cout << "[CastingError] Could not cast `" << actual.to_str()
              << "` to signature `" << expected.to_str() << "`.";
    exit(1);
}

[[noreturn]] void _throw_could_not_cast(LispVar expected,
                                        LispVar actual,
                                        LispVar of) {
    std::cout << "[CastingError] Could not cast `" << actual.to_str()
              << "` to signature `" << expected.to_str() << "` of `"
              << of.to_str() << "`.";
    exit(1);
}

VariableScope<LispVar> VARIABLE_SCOPE = {{}, 0};

std::string LispVar::to_str() {
    uint size;
    std::stringstream ss;

    if (this->tag == BUILTIN) {
        ss << "<Builtin '" << BUILTINS_NAMES.at(this->builtin) << "'>";
    } else if (this->tag == VARIABLE) {
        ss << "<Variable '" << *(this->string) << "'>";
    } else if (this->tag == TYPE) {
        ss << "<Type '" << TYPENAMES.at(this->type) << "'>";
    } else if (this->tag == EXPRESSION || this->tag == CLOSURE) {
        ss << this->_pretty_tree();
    } else if (this->tag == NUM || this->tag == FLOAT)
        ss << PNUMPART(this);
    else if (this->tag == NIL)
        ss << "Nil";
    else if (this->tag == __NO_ARGS__)
        ss << "<Special token '__NO_ARGS__'>";
    else if (this->tag == BOOL) {
        ss << (this->num ? "Yes" : "No");
    } else if (this->tag == STRING) {
        ss << *(this->string);
    } else if (this->tag == VECTOR) {
        size = this->vector->size();
        ss << "[";
        for (size_t i = 0; i < size; i++) {
            ss << (*this->vector)[i].to_repr();
            if (i != (size - 1)) { ss << " "; }
        }
        ss << "]";
    } else if (this->tag == LIST) {
        size = this->list->size();
        ss << "<";
        for (size_t i = 0; i < size; i++) {
            ss << (*this)[i].to_repr();
            if (i != (size - 1)) { ss << " "; }
        }
        ss << ">";
    } else {
        if (!TYPENAMES.count(this->tag)) {
            std::cout << "Error: Tag " << this->tag << " not in TYPENAMES.\n";
        } else {
            _throw_does_not_implement(this->tag, "to_str");
        }
        exit(1);
    }
    return ss.str();
}

std::string LispVar::get_help_str() {
    std::stringstream ss;

    if (this->tag == BUILTIN) {
        ss << "builtin '" << BUILTINS_NAMES.at(this->builtin)
           << "' with signature "
           << BUILTINS_TYPES[BUILTINS_NAMES.at(this->builtin)]->to_str();
    } else if (this->tag == TYPE) {
        return "a type";
    } else if (this->tag == EXPRESSION) {
        return "an expression";
    } else if (this->tag == NUM)
        return "a number";
    else if (this->tag == NIL)
        return "nothing";
    else if (this->tag == BOOL) {
        return "either Yes or No";
    } else if (this->tag == STRING) {
        return "a string of characters";
    } else if (this->tag == VECTOR) {
        return "a vector of items";
    } else {
        if (!TYPENAMES.count(this->tag)) {
            std::cout << "Error: Tag " << this->tag << " not in TYPENAMES.\n"
                      << '\n';
        } else
            _throw_does_not_implement(this->tag, "get_help_str");
        exit(1);
    }
    return ss.str();
}

LispVar call_builtin(LispVar operation, std::vector<LispVar *> args);
LispVar call_closure(LispVar operation, std::vector<LispVar *> args);
LispVar evaluate_const(std::string item);
LispVar parse_expression(std::string expression);

// ===| BUILTINS |===
LispVar evaluate_expression(LispVar *expression, uint index = 1);

/* Parse a Lisp expression into a tree. */
LispVar parse_expression(std::string expression) {
    auto ast = new Tree<LispVar>;
    *ast = {{*_SINGLETON_NOT_SET}, {0}};

    LispVar output;
    output.tag = EXPRESSION;
    output.tree = ast;

    const auto size = expression.size();
    const std::string incr_depth = "([{";
    const std::string decr_depth = "}])";
    const auto npos = std::string::npos;

    uint tree_length = 1;
    uint depth = 0;
    int depth_buffer;
    int d_depth;  // Depth difference

    bool last_is_empty;
    bool in_string_literal = false;
    bool escape_next = false;
    bool last_was_paren = false;
    bool last_was_paren_buffer = false;  // One token behind.
    bool line_is_comment = false;

    char character;

    std::string string_buffer;

    for (size_t i = 0; i < size; i++) {
        character = expression[i];
        d_depth = (incr_depth.find(character) != npos) -
                  (decr_depth.find(character) != npos);
        last_was_paren = last_was_paren || (d_depth > 0);
        depth += d_depth;

        // Keeps track of whether or not the parser is inside a string
        // literal to avoid splitting at bad times.
        if (in_string_literal)
            in_string_literal &= character != '"' || escape_next;
        else
            in_string_literal |= !in_string_literal && character == '"';

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
                string_buffer += "vector";
                if (last_is_empty) depth_buffer = depth;
                last_is_empty = 0;
                last_was_paren = 0;
            }

            if (!last_is_empty) {
                ast->nodes[tree_length - 1] = evaluate_const(string_buffer);
                ast->depths[tree_length - 1] = depth_buffer;

                if (last_was_paren_buffer) {
                    ast->nodes.push_back(*_SINGLETON_NOARGS_TOKEN);
                    ast->depths.push_back(ast->depths[tree_length - 1] + 1);
                    tree_length++;
                }

                string_buffer = "";
                ast->depths.push_back(depth);
                ast->nodes.push_back(*_SINGLETON_NOT_SET);
                tree_length++;
                last_was_paren_buffer = 0;
                last_was_paren = 0;
            }
            continue;
        }

        escape_next = !escape_next && (character == '\\');

        if (last_is_empty) depth_buffer = depth + !last_was_paren;

        // Add to the current token.
        string_buffer.push_back(character);
        last_was_paren = false;
    }

    if (depth) {
        throw std::runtime_error("Expression '" + expression +
                                 "' does not have balanced parentheses!");
    }

    if (ast->nodes[tree_length - 1] == *_SINGLETON_NOT_SET) {
        ast->depths.pop_back();
        ast->nodes.pop_back();
    }

    auto new_tree = new Tree<LispVar>;

    // Removes the __NO_ARGS__ tokens from the stream
    // if they aren't necessary.
    for (size_t i = 0; (i + 1) < tree_length; i++) {
        if (ast->nodes[i] == *_SINGLETON_NOARGS_TOKEN) {
            if (ast->depths[i] == ast->depths[i + 1]) continue;
        }
        new_tree->nodes.push_back(ast->nodes[i]);
        new_tree->depths.push_back(ast->depths[i]);
    }

    output.tree = new_tree;
    delete ast;
    return output;
}

/* Check if a LispVar's type at runtime is less broad than or equal to a type.
 */
bool _type_leq(LispType constant, LispVar *lesser) {
    if (constant == ANY) { return true; }
    if (constant == CALLABLE) { return lesser->is_callable(); }
    if (constant == BOOLY) { return lesser->is_booly(); }
    if (constant == TRUTHY) { return lesser->truthiness(); }
    if (constant == FALSY) { return !lesser->truthiness(); }
    if (constant == ITERABLE) { return lesser->is_sized(); }
    if (constant == INDEXABLE) {
        return lesser->tag == LIST || lesser->tag == VECTOR;
    }
    if (constant == NUMERIC) { return lesser->is_numeric(); }
    return constant == lesser->tag;
}

// warning this leaks
/* Matches a vector of LispVars against a LispVar VECTOR containing TYPEs.

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
bool _types_match(std::vector<LispVar *> args, LispVar expected) {
    std::string str_l = "vector";
    std::string str_t = "type";
    LispVar l;
    l.tag = TYPE;
    l.string = &str_l;

    LispVar t;
    t.tag = TYPE;
    t.string = &str_t;

    // Type check the type input.
    if (expected.tag != VECTOR) _throw_could_not_cast(l, expected);
    for (auto pattern : *expected.vector) {
        if (pattern.tag != VECTOR) _throw_could_not_cast(l, pattern);
        for (auto type : (*pattern.vector)) {
            if (type.tag != TYPE) _throw_could_not_cast(t, type);
        }
    }

    using Left = LispType;    // Comparison object.
    using Right = LispVar *;  // Target object.

    vecex::Token<Left> top_node = {vecex::JUST};
    std::optional<std::pair<vecex::uint_inf, vecex::uint_inf>> repeats;

    // Parse the LispVars and construct a vecex tree.
    for (auto child_0 : *expected.vector) {
        auto inner_token = new vecex::Token<Left>;
        *inner_token = {vecex::INTERSECTION, {}};
        repeats = {};

        for (auto child_1 : *child_0.vector) {
            if (child_1.type == STAR) {
                repeats = {{0, false}, {0, true}};
            } else if (child_1.type == PLUS) {
                repeats = {{1, false}, {0, true}};
            } else if (child_1.type == QMARK) {
                repeats = {{0, false}, {1, false}};
            } else {
                inner_token->tokens.push_back(child_1.type);
            }
        }

        // Wrap the token inside another one if it has been repeated.
        if (repeats.has_value()) {
            auto nums = repeats.value();
            auto wrapper_token = new vecex::Token<Left>;
            *wrapper_token = {vecex::BETWEEN, {inner_token}};
            wrapper_token->min = nums.first;
            wrapper_token->max = nums.second;
            top_node.tokens.push_back(wrapper_token);
        } else {
            top_node.tokens.push_back(inner_token);
        }
    }

    vecex::Comparer<Left, Right> matches = _type_leq;
    auto match = vecex::fullmatch(&top_node, &args, &matches);
    // std::cout << match.has_value() << '\n';
    // std::cout << expected.to_str() << '\n';
    // exit(1);
    return match.has_value();
}

LispVar call_variable(LispVar variable, std::vector<LispVar *> args) {
    if (variable.tag == BUILTIN) { return call_builtin(variable, args); }
    if (variable.tag == CLOSURE) { return call_closure(variable, args); }

    assert(false);
}

/* Call a closure on the inputs. */
LispVar call_closure(LispVar closure, std::vector<LispVar *> arguments) {
    assert(closure.tag == CLOSURE);

    // Function example: {{n} (+ n 10)}
    // The first branch is the arguments the function takes.
    // The second is an expression to execute.

    // Calling a function should first increase the scope depth.
    VARIABLE_SCOPE.increment();
    assert(VARIABLE_SCOPE.depth < 2048);

    // Then it should call (let argname arg) to bind the arguments
    // to the function values.
    auto argument_names = closure.tree->subtree(1);
    uint arity = argument_names.size() - 1;

    assert(arity == arguments.size());

    // Offset for the arity + 1 (base tree index)
    auto function_steps = closure.tree->subtree(arity + 2);
    for (size_t i = 1; i <= arity; i++) {
        auto a = argument_names.nodes[i];
        auto b = arguments[i - 1];
        VARIABLE_SCOPE.set_var(a.string, *b);
    }

    // Then, it should evaluate the function.
    auto callable = new LispVar;
    auto ptr = new Tree<LispVar>;
    *ptr = function_steps;

    callable->tree = ptr;
    callable->tag = EXPRESSION;

    LispVar result;

    try {
        result = evaluate_expression(callable, 0);
    } catch (LispEarlyReturn &value_exception) {
        result = value_exception.value;
    }

    delete callable;

    // If the result is another closure, the local variables need to be
    // evaluated before the closure is returned to the outer scope. Otherwise,
    // the name resolution would fail since the locals have gone out of scope.

    if (result.tag == CLOSURE) {
        std::set<std::string> variables_set_in_this_scope;

        for (auto item : VARIABLE_SCOPE.scopes) {
            auto item_max_depth = item.second.front().depth;
            if (item_max_depth == VARIABLE_SCOPE.depth) {
                variables_set_in_this_scope.insert(item.first);
            }
        }
        std::vector<LispVar> new_nodes;

        for (auto node : result.tree->nodes) {
            if (node.tag == VARIABLE &&
                variables_set_in_this_scope.find(*node.string) !=
                    variables_set_in_this_scope.end()) {
                new_nodes.push_back(VARIABLE_SCOPE.get_var(node.string));
            } else {
                new_nodes.push_back(node);
            }
        }

        result.tree->nodes = new_nodes;
    }

    // If it is a closure, it can get information from the surrounding
    // scope. If it is a pure function, it can't. This should be checked and
    // enforced by the Python code.

    // Finally, the scope should be exited and cleaned up.
    VARIABLE_SCOPE.decrement();
    return result;
}

/* Perform an operation on the inputs. */
LispVar call_builtin(LispVar operation, std::vector<LispVar *> args) {
    assert(operation.tag == BUILTIN);

    LispVar output;
    auto arity = args.size();

    // Set the kind to be the type of all arguments, if they are the same.
    LispType kind = arity ? args[0]->tag : __NOT_SET__;
    for (size_t i = 1; i < arity; i++) {
        kind = args[i]->tag == kind ? kind : __NOT_SET__;
    }

    auto op = operation.builtin;
    auto name = BUILTINS_NAMES.at(op);
    if (arity == 1 && *args[0] == *_SINGLETON_NOARGS_TOKEN) args = {};

    // Typecheck the arguments.
    if (BUILTINS_TYPES_READY && SAFE_MODE) {
        if (!BUILTINS_TYPES.count(name)) {
            std::cout << "[bug] Operation '" << op
                      << "' is not typed. Exiting.\n";
            exit(1);
        }
        auto arg_types_match = _types_match(args, *BUILTINS_TYPES.at(name));
        if (!arg_types_match) {
            LispVar actual_type;
            actual_type.tag = VECTOR;
            actual_type.vector = new std::vector<LispVar>;
            for (auto arg : args) { actual_type.vector->push_back(*arg); }

            _throw_could_not_cast(
                *BUILTINS_TYPES.at(name), actual_type, operation);
        }
    }

    if (op == B_PARSE) {
        output = evaluate_const(*args[0]->string);
        return output;
    }

    // No operation.
    if (op == B_DO) { return arity ? *args[arity - 1] : *_SINGLETON_NIL; }
    if (op == B_CALL) {
        return call_variable(*args[0], {args.begin() + 1, args.end()});
    }

    if (op == B_APPLY) {
        std::vector<LispVar *> new_args;
        size_t nargs = args[1]->vector->size();
        for (size_t i = 0; i < nargs; i++) {
            new_args.push_back(&(*args[1]->vector)[i]);
        }
        return call_variable(*args[0], new_args);
    }
    if (op == B_EXIT) { exit(arity ? args[0]->num : 0); }
    if (op == B_WHILE) {
        int count = 0;
        while (evaluate_expression(args[0]).truthiness()) {
            try {
                evaluate_expression(args[1]);
            } catch (LispBreak &_) { break; }

            count++;
            if (count > 100000) {
                std::cout << "Infinite loop!" << '\n';
                exit(1);
            }
        }
        return *_SINGLETON_NIL;
    }
    // Break out of a loop.
    if (op == B_BREAK) {
        LispBreak exception;
        throw exception;
        return *_SINGLETON_NIL;
    }

    if (op == B_SEED) {
        RNG = std::mt19937(args[0]->num);
        return *_SINGLETON_NIL;
    }

    if (op == B_RETURN) {
        LispEarlyReturn early_return;
        early_return.value = *args[0];
        throw early_return;
        return *_SINGLETON_NIL;
    }

    // Generate a vector of random numbers.
    if (op == B_RAND) {
        output.vector = new std::vector<LispVar>;
        for (int i = 0; i < args[0]->num; i++) {
            // Using the mod here fixes the casting.
            long num = RNG() % (1 << 16);
            output.vector->push_back({NUM, num});
        }
        output.tag = VECTOR;
        return output;
    }

    // Bind a string to a variable value.
    if (op == B_LET) {
        VARIABLE_SCOPE.set_var(args[0]->string, *args[1]);
        return *_SINGLETON_NIL;
    }

    if ((op == B_MATCH) | (op == B_FINDALL) | (op == B_SPLIT)) {
        std::regex txt_regex;
        try {
            txt_regex = std::regex(*args[0]->string);
        } catch (std::regex_error const &) {
            std::cout << "RegexError: " << args[0]->to_repr()
                      << " is an invalid regular expression.\n";
            exit(1);
        }
        // Return whether or not an expression matches.
        if (op == B_MATCH) {
            output.tag = BOOL;
            output.num = std::regex_match(*args[1]->string, txt_regex);
            return output;
        }

        if ((op == B_SPLIT) | (op == B_FINDALL)) {
            // Get a token iterator which finds the groups.
            // -1 is for split and 0 for the first capture group.
            std::regex_token_iterator<std::string::iterator> rend;
            std::regex_token_iterator<std::string::iterator> iter(
                args[1]->string->begin(),
                args[1]->string->end(),
                txt_regex,
                (op == B_SPLIT) ? -1 : 0);

            output.vector = new std::vector<LispVar>;
            output.tag = VECTOR;

            // Push all matches into the vector.
            while (iter != rend) {
                LispVar var;
                var.string = new std::string;
                var.tag = STRING;

                *var.string = (*iter++);
                output.vector->push_back(var);
            }

            return output;
        }
    }

    // ===| Base constructors |===

    if (op == B_CHR) {
        output.string = new std::string;
        *output.string = std::string(1, args[0]->to_l());
        output.tag = STRING;
        return output;
    }

    if (op == B_INPUT) {
        output.tag = STRING;
        output.string = new std::string;
        std::getline(std::cin, *output.string);
        return output;
    }

    if (op == B_ORD) {
        output.tag = NUM;
        output.num = (*args[0]->string)[0];
        return output;
    }

    if (op == B_INT) return {NUM, args[0]->to_l()};
    if (op == B_FLOAT) {
        output.tag = FLOAT;
        output.flt = args[0]->to_f();
        return output;
    }
    if (op == B_BOOL) return {BOOL, args[0]->truthiness()};
    if (op == B_TYPE) {
        output.type = TYPENAMES_REV.at(*args[0]->string);
        output.tag = TYPE;
        return output;
    }
    if (op == B_VECTOR) {
        output.vector = new std::vector<LispVar>;
        output.tag = VECTOR;
        for (auto arg : args) { output.vector->push_back(*arg); }
        return output;
    }
    if (op == B_LIST) {
        output.list = new std::list<LispVar>;
        output.tag = LIST;
        for (auto arg : args) output.list->push_back(*arg);
        return output;
    }
    if (op == B_CLOSURE) {
        output = *args[0];
        output.tag = CLOSURE;
        return output;
    }

    // ===| Help functions |===
    if (op == B_HELP) {
        output.string = new std::string;
        *output.string = args[0]->get_help_str();
        output.tag = STRING;
        return output;
    }

    if (op == B_COPY) return args[0]->copy();

    if (op == B_LEN) return {NUM, args[0]->size()};
    if (op == B_NOT) return {BOOL, !args[0]->truthiness()};

    // Get the type of {0}.
    if (op == B_TYPEOF) {
        output.type = args[0]->tag;
        output.tag = TYPE;
        return output;
    }

    // Matches the type {0} against the variables {1}.
    if (op == B_TYPEMATCH) {
        std::vector<LispVar *> v;
        for (auto arg : *args[1]->vector) { v.push_back(&arg); }
        return {BOOL, _types_match(v, *args[0])};
    }

    // Assertions.
    if (op == B_ASSERT) {
        bool success = args[0]->truthiness();
        _lisp_assert_or_exit(
            success,
            "Assertion" + ((arity == 2) ? " `" + *args[1]->string + "`" : "") +
                " failed (evaluated " + args[0]->to_str() + ").");
        return *_SINGLETON_NIL;
    }

    // Write a string to `cout`.
    if (op == B_PUT) {
        for (size_t i = 0; i < arity; i++) std::cout << args[i]->to_str();
        return *_SINGLETON_NIL;
    }

    // Get the string representation of {0}.
    if (op == B_REPR) {
        output.string = new std::string;
        output.tag = STRING;

        *output.string = (args[0]->tag == STRING)
                             ? escape_string(args[0]->to_str())
                             : args[0]->to_str();
        return output;
    }

    // Joins the contents of Q-Expressions.
    if (op == B_JOIN) {
        if (kind == VECTOR) {
            output.vector = new std::vector<LispVar>;
            output.tag = VECTOR;
            for (auto arg : args)
                for (auto i : *arg->vector) output.vector->push_back(i);
            return output;
        }
        if (kind == STRING) {
            output.string = new std::string;
            output.tag = STRING;
            for (auto arg : args) (*output.string) += (*arg->string);
            return output;
        }
    }

    // Push {1} into {0}.
    if (op == B_PUSH) {
        args[0]->vector->push_back(*args[1]);
        return *_SINGLETON_NIL;
    }

    // Pop the last element of {0} in place and return it.
    if (op == B_POP) {
        auto item = (*args[0]->vector)[args[0]->vector->size() - 1];
        args[0]->vector->pop_back();
        return item;
    }

    // Insert {0} at index {1} in {2}.
    if (op == B_INSERT) {
        output.vector = new std::vector<LispVar>;
        output.tag = VECTOR;

        auto size = args[2]->size();
        auto index = args[1]->num;

        // Allows for negative indices.
        index = (index < 0) ? size + index + 1 : index;
        _lisp_assert_or_exit(
            index <= size,
            "OutOfBoundsError: Second argument of `insert` must be "
            "less than or equal to the size of the first argument.");

        for (int i = 0; i < size; i++) {
            if (i == index) output.vector->push_back(*args[0]);
            output.vector->push_back((*args[2])[i]);
        }
        if (index == size) output.vector->push_back(*args[0]);
        return output;
    }

    if (op == B_LINSERT) {
        output.list = new std::list<LispVar>;
        output.tag = LIST;

        auto size = args[2]->size();
        auto index = args[1]->num;

        // Allows for negative indices.
        index = (index < 0) ? size + index + 1 : index;
        _lisp_assert_or_exit(
            index <= size,
            "OutOfBoundsError: Second argument of `insert` must be "
            "less than or equal to the size of the first argument.");

        for (int i = 0; i < size; i++) {
            if (i == index) output.list->push_back(*args[0]);
            output.list->push_back((*args[2])[i]);
        }
        if (index == size) output.list->push_back(*args[0]);
        return output;
    }

    if (op == B_FIND) {
        uint size = args[1]->size();
        for (uint i = 0; i < size; i++) {
            if (*args[0] == (*args[1])[i]) {
                output.num = i;
                output.tag = NUM;
                return output;
            }
        }
        return *_SINGLETON_NIL;
    }

    // Repeat {1} {0} times.
    if (op == B_REPEAT) {
        output.vector = new std::vector<LispVar>;
        for (int i = 0; i < args[0]->num; i++)
            output.vector->push_back(*args[1]);
        output.tag = VECTOR;

        return output;
    }

    // Get the {0}th element of {1}.
    if (op == B_GET) {
        auto size = args[1]->size();
        auto index = args[0]->num;

        // Allows for negative indices.
        index = (index < 0) ? size + index : index;
        _lisp_assert_or_exit(
            index < size,
            "OutOfBoundsError: {0} for `get` must be less than the "
            "size of {1}.");
        return (*args[1])[index];
    }

    // Map a function across one or more iterable, getting an iterable back.
    if (op == B_MAP) {
        output.vector = new std::vector<LispVar>;
        output.tag = VECTOR;

        if (arity > 1) {
            // Checks that the sizes of the vectors are all equal.
            auto _size = new uint;
            *_size = args[1]->size();
            for (size_t i = 2; i < arity; i++) {
                _lisp_assert_or_exit(
                    args[i]->size() == *_size,
                    "[SizeError] The sizes of vectors used as arguments to "
                    "`map` must all be of equal length.");
            }

            // Evaluate the function over the members.
            auto _vector = new std::vector<LispVar *>;
            for (size_t i = 0; i < *_size; i++) {
                for (size_t j = 1; j < arity; j++) {
                    _vector->push_back(&(*args[j]->vector)[i]);
                }
                (*output.vector).push_back(call_variable(*args[0], *_vector));
                _vector->clear();
            }
            delete _vector;
            delete _size;
        }

        return output;
    }

    // Accumulate a function across an iterable, getting an iterable back.
    if (op == B_ACCUMULATE) {
        output.vector = new std::vector<LispVar>;
        output.tag = VECTOR;
        uint size = args[1]->size();
        if (!size) return output;

        LispVar left = (arity != 3) ? (*args[1])[0] : *args[2];
        if (arity != 3) (*output.vector).push_back(left);

        for (size_t i = (arity != 3); i < size; i++) {
            LispVar right = ((*args[1])[i]);
            left = call_variable(*args[0], {&left, &right});
            (*output.vector).push_back(left);
        }
        return output;
    }

    if (op == B_FOLD) {
        LispVar accumulator;
        uint vec_size = args[1]->vector->size();
        uint i = 0;

        if (arity == 3) {
            accumulator = *args[2];
        } else {
            _lisp_assert_or_exit(vec_size,
                                 "FoldError: An empty list cannot be folded "
                                 "without an accumulator.\n");
            accumulator = (*args[1]->vector)[0];
            i = 1;
        }

        for (; i < vec_size; i++) {
            auto item = (*args[1]->vector)[i];
            accumulator = call_variable(*args[0], {&accumulator, &item});
        }

        return accumulator;
    }

    // Generates a list using a slice-like syntax.
    if (op == B_RANGE) {
        int start = 0;
        int stop = -1;
        int step = 1;

        // Allows any arity between 1 and 3.
        if (arity == 1)
            stop = args[0]->num;
        else {
            start = args[0]->num;
            stop = args[1]->num;
        }
        if (arity >= 3) step = args[2]->num;

        output.vector = new std::vector<LispVar>;
        output.tag = VECTOR;

        if (stop > start and step < 0) return output;
        if (stop < start and step > 0) return output;

        for (int i = start; (step > 0) ? (i < stop) : (i > stop); i += step) {
            (*output.vector).push_back({NUM, i});
        }
        return output;
    }
    if (op == B_SLICE) {
        long int start = 0;
        long int stop = -1;
        long int step = 1;
        auto size = args[0]->size();

        // Allows any arity between 2 and 4.
        if (arity >= 2) start = args[1]->num;
        if (arity >= 3) stop = args[2]->num;
        if (arity >= 4) step = args[3]->num;

        // Allows for negative indices.
        start = (start < 0) ? size + start : start;
        stop = (stop < 0) ? size + stop : stop;

        // Cut off too large indices.
        stop = std::min(stop, size - 1);
        start = std::min(start, size - 1);

        if (args[0]->tag == VECTOR) {
            output.tag = VECTOR;
            output.vector = new std::vector<LispVar>;

            if (stop > start && step < 0) return output;
            if (stop < start && step > 0) return output;

            for (int i = start; (step > 0) ? (i <= stop) : (i >= stop);
                 i += step) {
                (*output.vector).push_back((*args[0]->vector)[i]);
            }

            return output;
        }

        if (args[0]->tag == STRING) {
            output.tag = STRING;
            output.string = new std::string;

            std::ostringstream buf;

            if (stop > start && step < 0) return output;
            if (stop < start && step > 0) return output;

            for (int i = start; (step > 0) ? (i <= stop) : (i >= stop);
                 i += step) {
                buf << (*args[0]->string)[i];
            }

            *output.string = buf.str();
            return output;
        }

        std::cout << "Slice not implemented for this type\n";
        exit(1);
    }

    if (op == B_EVAL_EXPR) { return evaluate_expression(args[0], 1); }

    // Flow control
    if (op == B_TERNARY) { return args[0]->truthiness() ? *args[1] : *args[2]; }

    // Numeric functions
    if (op == B_ADD) {
        bool can_be_int = true;
        for (auto arg : args) {
            can_be_int &= arg->tag != FLOAT;
            if (!can_be_int) break;
        }
        if (can_be_int) {
            output.tag = NUM;
            output.num = accumulate_l(&args, 0, std::plus<long>());
            return output;
        } else {
            output.tag = FLOAT;
            output.flt = accumulate_f(&args, 0, std::plus<float>());
            return output;
        }
    }
    if (op == B_MUL) {
        bool can_be_int = true;
        for (auto arg : args) {
            can_be_int &= arg->tag != FLOAT;
            if (!can_be_int) break;
        }
        if (can_be_int) {
            output.tag = NUM;
            output.num = accumulate_l(&args, 1, std::multiplies<long>());
            return output;
        } else {
            output.tag = FLOAT;
            output.flt = accumulate_f(&args, 1, std::multiplies<float>());
            return output;
        }
    }
    if (op == B_AND) {
        output.tag = NUM;
        output.num = accumulate_l(&args, ~0, std::bit_and<long>());
        return output;
    }
    if (op == B_OR) {
        output.tag = NUM;
        output.num = accumulate_l(&args, 0, std::bit_or<long>());
        return output;
    }
    if (op == B_XOR) {
        output.tag = NUM;
        output.num = accumulate_l(&args, 0, std::bit_xor<long>());
        return output;
    }
    if (op == B_EQ) {
        // All arguments should be equal.
        output.num = true;
        output.tag = BOOL;
        if (!arity) return output;
        auto first = *args[0];
        for (size_t i = 1; i < arity; i++) {
            output.num = first == *args[i];
            if (!output.num) break;
        }
        return output;
    }
    if (op == B_NEQ) {
        // All arguments should be different.
        output.num = true;
        output.tag = BOOL;
        if (!arity) return output;
        auto first = *args[0];
        for (size_t i = 1; i < arity; i++) {
            output.num = first != *args[i];
            if (!output.num) break;
        }
        return output;
    }
    if (op == B_GT) {
        // All arguments should be strictly decreasing.
        output.tag = BOOL;
        output.num = vector_is_ordered(&args, std::greater<float>());
        return output;
    }
    if (op == B_LT) {
        // All arguments should be strictly increasing.
        output.tag = BOOL;
        output.num = vector_is_ordered(&args, std::less<float>());
        return output;
    }
    if (op == B_GEQ) {
        // All arguments should be non-strictly decreasing.
        output.tag = BOOL;
        output.num = vector_is_ordered(&args, std::greater_equal<float>());
        return output;
    }
    if (op == B_LEQ) {
        // All arguments should be non-strictly increasing.
        output.tag = BOOL;
        output.num = vector_is_ordered(&args, std::less_equal<float>());
        return output;
    }

    // 2-ary numeric functions
    if (op == B_SUB) {
        bool can_be_int = true;
        for (auto arg : args) {
            can_be_int &= arg->tag != FLOAT;
            if (!can_be_int) break;
        }
        if (can_be_int) {
            return {NUM, args[0]->num - args[1]->num};
        } else {
            output.tag = FLOAT;
            output.flt = PNUMPART(args[0]) - PNUMPART(args[1]);
            return output;
        }
    }
    if (op == B_DIV) {
        bool can_be_int = true;
        for (auto arg : args) {
            can_be_int &= arg->tag != FLOAT;
            if (!can_be_int) break;
        }
        if (can_be_int) {
            return {NUM, args[0]->num / args[1]->num};
        } else {
            output.tag = FLOAT;
            output.flt = PNUMPART(args[0]) / PNUMPART(args[1]);
            return output;
        }
    }
    if (op == B_MOD) {
        output.tag = NUM;
        output.num = args[0]->num % args[1]->num;
        return output;
    }

    // 1-ary numeric functions
    if (op == B_NEG) {
        output.tag = args[0]->tag;
        if (output.tag == NUM)
            output.num = -args[0]->num;
        else
            output.flt = -args[0]->flt;
        return output;
    }

    if (op == B_FLIP) return {NUM, ~args[0]->num};

    // Results haven't been set.
    std::cout << "ExpressionEvaluationError: Could not evaluate ("
              << BUILTINS_NAMES.at(op);
    for (size_t i = 0; i < arity; i++) std::cout << " " << args[i]->to_str();
    std::cout << ")\n";
    exit(1);
}

/* Evaluate a constant. */
LispVar evaluate_const(std::string item) {
    LispVar output;
    bool is_function = LISP_BUILTINS.count(item);

    if (is_function) {
        output.builtin = BUILTINS_NUMS.at(item);
        output.tag = BUILTIN;
        return output;
    }

    if (!item.compare("expression")) {
        output.string = new std::string;
        *output.string = item;
        output.tag = BUILTIN;
        return output;
    }

    if (!item.compare("Yes")) { return {BOOL, 1}; }
    if (!item.compare("No")) { return {BOOL, 0}; }
    if (!item.compare("Nil")) { return *_SINGLETON_NIL; }

    if (item.size() >= 2 && item[0] == '"' && item[item.size() - 1] == '"') {
        output.string = new std::string;
        *output.string = unescape_string(item);
        output.tag = STRING;
        return output;
    }

    try {
        if (item.find('.') != std::string::npos) {
            // Floats
            output.tag = FLOAT;
            output.flt = std::stold(item);
            return output;
        } else {
            // Integers
            return {NUM, std::stoi(item)};
        }
    } catch (const std::invalid_argument &e) {
        output.string = new std::string;
        *output.string = item;
        output.tag = VARIABLE;
        return output;
    }
}

/* Evaluates a tree.

Function calls are defined as nodes which are not leaves.
These are computed recursively by evaluating the leaves (constants).

Note that the variable resolution takes place when they are called as leaf nodes
in this context.
*/
LispVar evaluate_expression(LispVar *expression, uint index) {
    auto item = expression->tree->nodes[index];

    // Resolves variables.
    if (item.tag == VARIABLE) { item = VARIABLE_SCOPE.get_var(item.string); }
    bool is_function = item.tag == BUILTIN || item.tag == CLOSURE;

    if (!is_function) return item;

    // Capture expressions literally.
    if (item.builtin == B_EXPRESSION) {
        LispVar result;
        Tree<LispVar> *subtree = new Tree<LispVar>;
        *subtree = expression->tree->subtree(index);

        result.tag = EXPRESSION;
        result.tree = subtree;
        return result;
    }

    // Allow binding to variables.
    // This is very scuffed right now and obviously WIP.
    if (item.builtin == B_LET) {
        LispVar result = evaluate_expression(expression, index + 2);
        VARIABLE_SCOPE.set_var(expression->tree->nodes[index + 1].string,
                               result);

        return result;
    }

    std::vector<LispVar *> arguments;
    auto size = expression->tree->nodes.size();
    auto original_depth = expression->tree->depths[index];

    for (size_t i = index + 1;
         (original_depth != expression->tree->depths[i]) & (i < size);
         i++) {
        if ((original_depth + 1) == expression->tree->depths[i]) {
            auto inner = new LispVar;
            *inner = evaluate_expression(expression, i);
            arguments.push_back(inner);
        }
    }
    auto result = arguments.empty() ? item : call_variable(item, arguments);
    arguments.clear();
    return result;
}

LispVar parse_and_evaluate(std::string input) {
    auto tree = parse_expression(input);
    return evaluate_expression(&tree, 0);
}

void print_debug(std::string msg) {
    if (DEBUG_MODE) { std::cout << "[DEBUG] " << msg; }
}

int main(int argc, char const *argv[]) {
    if (argc < 4) {
        std::cout << "Error: Expected at least 3 command-line arguments "
                     "(filename, debug mode, and safe mode)."
                  << '\n';
        exit(1);
    }

    auto since_epoch = std::chrono::system_clock::now().time_since_epoch();

    DEBUG_MODE = std::stoi(argv[2]);
    SAFE_MODE = std::stoi(argv[3]);
    RNG = std::mt19937(since_epoch.count());

    // Set argv to the command-line arguments.
    LispVar argv_lisp_var;

    argv_lisp_var.vector = new std::vector<LispVar>;
    argv_lisp_var.tag = VECTOR;

    // Add the filename.
    LispVar filename;

    filename.string = new std::string;
    *filename.string = argv[0];
    filename.tag = STRING;

    argv_lisp_var.vector->push_back(filename);

    for (int i = 4; i < argc; i++) {
        LispVar argument;

        argument.string = new std::string;
        *argument.string = argv[i];
        argument.tag = STRING;

        argv_lisp_var.vector->push_back(argument);
    }

    std::string varname = "argv";
    VARIABLE_SCOPE.set_var(&varname, argv_lisp_var);

    _fill_out_lisp_builtin_types();

    std::ifstream t(argv[1]);
    std::stringstream buffer;
    buffer << t.rdbuf();
    auto expr = buffer.str();

    auto tree = parse_expression(expr);
    print_debug("Dumping AST below:\n");
    print_debug(tree.to_str() + ":\n");
    print_debug("Done\n");
    print_debug("Evaluating AST at node 0.\n");
    evaluate_expression(&tree, 0);

    std::cout << '\n';
    return 0;
}
