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
std::mt19937 RNG;

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
    std::cout << "[UnimplementedError] LispType '" << TYPENAMES.at(type)
              << "' does not implement `" << notimplemented << "`.";
    exit(1);
}

[[noreturn]] void _throw_could_not_cast(LispVar expected, LispVar actual) {
    std::cout << "[CastingError] Could not cast `" << actual.to_str()
              << "` to signature `" << expected.to_str() << "`.";
    exit(1);
}

VariableScope<LispVar> VARIABLE_SCOPE = {{}, 0};

std::string LispVar::to_str() {
    uint size;
    std::stringstream ss;

    if (this->tag == BUILTIN) {
        ss << "<Builtin '" << *(this->string) << "'>";
    } else if (this->tag == VARIABLE) {
        ss << "<Variable '" << *(this->string) << "'>";
    } else if (this->tag == TYPE) {
        ss << "<Type '" << *(this->string) << "'>";
    } else if (this->tag == EXPRESSION) {
        ss << this->_pretty_tree();
    } else if (this->tag == CLOSURE) {
        ss << this->_pretty_tree();
    } else if (this->tag == NUM || this->tag == FLOAT)
        ss << PNUMPART(this);
    else if (this->tag == NOTHING)
        ss << "Nothing";
    else if (this->tag == __NO_ARGS__)
        ss << "<Special token '__NO_ARGS__'>";
    else if (this->tag == BOOL) {
        ss << (this->num ? "True" : "False");
    } else if (this->tag == STRING) {
        ss << *(this->string);
    } else if (this->tag == LIST) {
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
        ss << "builtin '" << *(this->string) << "' "
           << BUILTINS_TYPES[*this->string]->to_str();
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
    } else if (this->tag == LIST) {
        return "a list of other items";
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

LispVar call_builtin(LispVar operation,
                     std::vector<LispVar> args,
                     bool safe = true);
LispVar call_closure(LispVar operation, std::vector<LispVar> args);
LispVar evaluate_const(std::string item);
LispVar parse_expression(std::string expression);

// ===| BUILTINS |===
LispVar evaluate_expression(LispVar *expression, uint index);

/* Parse a Lisp expression into a tree. */
LispVar parse_expression(std::string expression) {
    // expression = exec("python ./preprocessor.py " +
    // _escape_exec(expression));

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

        // Keeps track of whether or not the parser is inside a string
        // literal to avoid splitting at bad times.
        in_string_literal ^= character == '"' && !last_was_backslash;
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
bool _type_leq(std::string *string, LispVar *lesser) {
    if (!string->compare("any")) { return true; }
    if (!string->compare("callable")) { return lesser->is_callable(); }
    if (!string->compare("booly")) { return lesser->is_booly(); }
    if (!string->compare("truthy")) { return lesser->truthiness(); }
    if (!string->compare("falsy")) { return !lesser->truthiness(); }
    if (!string->compare("iterable")) { return lesser->is_sized(); }
    if (!string->compare("numeric")) { return lesser->is_numeric(); }
    return *string == TYPENAMES.at(lesser->tag);
}

// warning this leaks
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
    auto str_l = "list";
    auto str_t = "type";

    // Type check the type input.
    if (expected.tag != LIST) _throw_could_not_cast({TYPE, *str_l}, expected);
    for (auto pattern : *expected.vector) {
        if (pattern.tag != LIST) _throw_could_not_cast({TYPE, *str_l}, pattern);
        for (auto type : (*pattern.vector)) {
            if (type.tag != TYPE) _throw_could_not_cast({TYPE, *str_t}, type);
        }
    }

    using Left = std::string;  // Comparison object.
    using Right = LispVar;     // Target object.

    vecex::Token<Left> top_node = {vecex::JUST};
    std::optional<std::pair<vecex::uint_inf, vecex::uint_inf>> repeats;

    // Parse the LispVars and construct a vecex tree.
    for (auto child_0 : *expected.vector) {
        auto inner_token = new vecex::Token<Left>;
        *inner_token = {vecex::INTERSECTION, {}};
        repeats = {};

        for (auto child_1 : *child_0.vector) {
            // std::cout << *child_1.string << '\n';
            if (!child_1.string->compare("*")) {
                repeats = {{0, false}, {0, true}};
            } else if (!child_1.string->compare("+")) {
                repeats = {{0, false}, {0, true}};
            } else if (!child_1.string->compare("?")) {
                repeats = {{0, false}, {1, false}};
            } else {
                inner_token->tokens.push_back(*child_1.string);
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

    vecex::Comparer<Left, Right> matches = [&](Left a, Right b) {
        return _type_leq(&a, &b);
    };
    auto match = vecex::fullmatch(&top_node, &args, &matches);
    return match.has_value();
}

LispVar call_variable(LispVar variable, std::vector<LispVar> args) {
    if (variable.tag == BUILTIN) {
        return call_builtin(variable, args);
    } else if (variable.tag == CLOSURE) {
        return call_closure(variable, args);
    }

    assert(false);
}

/* Call a closure on the inputs. */
LispVar call_closure(LispVar closure, std::vector<LispVar> arguments) {
    assert(closure.tag == CLOSURE);

    // Function example: {{n} (+ n 10)}
    // The first branch is the arguments the function takes.
    // The second is an expression to execute.

    // Calling a function should first increase the scope depth.
    VARIABLE_SCOPE.increment();
    assert(VARIABLE_SCOPE.depth < 5);

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
        VARIABLE_SCOPE.set_var(a.string, b);
    }

    // Then, it should evaluate the function.
    auto callable = new LispVar;
    auto ptr = new Tree<LispVar>;
    *ptr = function_steps;

    callable->tree = ptr;
    callable->tag = EXPRESSION;

    auto result = evaluate_expression(callable, 0);
    delete callable;

    // If it is a closure, it can get information from the surrounding
    // scope. If it is a pure function, it can't. This should be checked and
    // enforced by the Python code.

    // Finally, the scope should be exited and cleaned up.
    VARIABLE_SCOPE.decrement();
    return result;
}

/* Perform an operation on the inputs. */
LispVar call_builtin(LispVar operation, std::vector<LispVar> args, bool safe) {
    assert(operation.tag == BUILTIN);

    LispVar output;
    auto arity = args.size();

    // Set the kind to be the type of all arguments, if they are the same.
    LispType kind = arity ? args[0].tag : __NOT_SET__;
    for (size_t i = 1; i < arity; i++) {
        kind = args[i].tag == kind ? kind : __NOT_SET__;
    }

    auto op = *operation.string;
    if (arity == 1 && args[0] == *_SINGLETON_NOARGS_TOKEN) args = {};

    // Typecheck the arguments.
    if (BUILTINS_TYPES_READY && safe) {
        if (!BUILTINS_TYPES.count(op)) {
            std::cout << "[bug] Operation '" << op
                      << "' is not typed. Exiting.\n";
            exit(1);
        }
        auto arg_types_match = _types_match(args, *BUILTINS_TYPES.at(op));
        if (!arg_types_match) {
            LispVar actual_type;
            actual_type.tag = LIST;
            actual_type.vector = &args;
            _throw_could_not_cast(*BUILTINS_TYPES.at(op), actual_type);
        }
    }

    // No operation.
    if (!op.compare("noop")) { return *_SINGLETON_NOTHING; }
    if (!op.compare("do")) { return args[arity - 1]; }

    // Generate a list of random numbers.
    if (!op.compare("rand")) {
        output.vector = new std::vector<LispVar>;
        for (int i = 0; i < args[0].num; i++) {
            // Using the mod here fixes the casting.
            long num = RNG() % (1 << 16);
            output.vector->push_back({NUM, num});
        }
        output.tag = LIST;
        return output;
    }

    // Bind a string to a variable value.
    if (!op.compare("let")) {
        VARIABLE_SCOPE.set_var(args[0].string, args[1]);
        return *_SINGLETON_NOTHING;
    }

    // Do regular expression matching.
    if (!op.compare("match")) {
        std::regex txt_regex;
        try {
            txt_regex = std::regex(*args[0].string);
        } catch (std::regex_error const &) {
            std::cout << "RegexError: " << args[0].to_repr()
                      << " is an invalid regular expression.\n";
            exit(1);
        }

        output.tag = BOOL;
        output.num = std::regex_match(*args[1].string, txt_regex);
        return output;
    }

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
    if (!op.compare("closure")) {
        output = args[0];
        output.tag = CLOSURE;
        return output;
    }

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
        bool success = args[0].truthiness();
        _lisp_assert_or_exit(
            success,
            "Assertion" + ((arity == 2) ? " `" + *args[1].string + "`" : "") +
                " failed (evaluated " + args[0].to_str() + ").");
        return *_SINGLETON_NOTHING;
    }

    // Write a string to `cout`.
    if (!op.compare("put")) {
        std::cout << args[0].to_str();
        return *_SINGLETON_NOTHING;
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

    // Insert {0} at index {1} in {2}.
    if (!op.compare("insert")) {
        output.vector = new std::vector<LispVar>;
        output.tag = LIST;

        auto size = args[2].size();
        auto index = args[1].num;

        // Allows for negative indices.
        index = (index < 0) ? size + index + 1 : index;
        _lisp_assert_or_exit(
            index <= size,
            "OutOfBoundsError: Second argument of `insert` must be "
            "less than or equal to the size of the first argument.");

        for (int i = 0; i < size; i++) {
            if (i == index) (*output.vector).push_back(args[0]);
            (*output.vector).push_back((*args[2].vector)[i]);
        }
        if (index == size) (*output.vector).push_back(args[0]);
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
            auto _size = new uint;
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
                (*output.vector).push_back(call_variable(args[0], *_vector));
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
            accumulator = call_variable(args[0], {accumulator, arg});
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

        if (stop > start && step < 0) return output;
        if (stop < start && step > 0) return output;

        for (int i = start; (step > 0) ? (i < stop) : (i > stop); i += step) {
            (*output.vector).push_back((*args[0].vector)[i]);
        }

        return output;
    }

    if (!op.compare("eval")) {
        return call_variable(
            (*args[0].vector)[0],
            {(*args[0].vector).begin() + 1, (*args[0].vector).end()});
    }
    // Flow control
    if (!op.compare("ternary")) {
        return args[0].truthiness() ? args[1] : args[2];
    }

    // Numeric functions
    if (!op.compare("add")) {
        if (kind == NUM) {
            output.tag = NUM;
            output.num = accumulate_l(&args, 0, std::plus<long>());
            return output;
        } else {
            output.tag = FLOAT;
            output.flt = accumulate_f(&args, 0, std::plus<float>());
        }
    }
    if (!op.compare("mul")) {
        if (kind == NUM) {
            output.tag = NUM;
            output.num = accumulate_l(&args, 1, std::multiplies<long>());
            return output;
        } else {
            output.tag = FLOAT;
            output.flt = accumulate_f(&args, 1, std::multiplies<float>());
            return output;
        }
    }
    if (!op.compare("and")) {
        output.tag = NUM;
        output.num = accumulate_l(&args, ~0, std::bit_and<long>());
        return output;
    }
    if (!op.compare("or")) {
        output.tag = NUM;
        output.num = accumulate_l(&args, 0, std::bit_or<long>());
        return output;
    }
    if (!op.compare("xor")) {
        output.tag = NUM;
        output.num = accumulate_l(&args, 0, std::bit_xor<long>());
        return output;
    }
    if (!op.compare("eq")) {
        // All arguments should be equal.
        output.num = true;
        output.tag = BOOL;
        if (!arity) return output;
        auto first = (args)[0];
        for (size_t i = 1; i < arity; i++) {
            output.num = first == (args)[i];
            if (!output.num) break;
        }
        return output;
    }
    if (!op.compare("neq")) {
        // All arguments should be different.
        output.num = true;
        output.tag = BOOL;
        if (!arity) return output;
        auto first = (args)[0];
        for (size_t i = 1; i < arity; i++) {
            output.num = first != (args)[i];
            if (!output.num) break;
        }
        return output;
    }
    if (!op.compare("gt")) {
        // All arguments should be strictly decreasing.
        output.tag = BOOL;
        output.num = vector_is_ordered(&args, std::greater<float>());
        return output;
    }
    if (!op.compare("lt")) {
        // All arguments should be strictly increasing.
        output.tag = BOOL;
        output.num = vector_is_ordered(&args, std::less<float>());
        return output;
    }
    if (!op.compare("geq")) {
        // All arguments should be non-strictly decreasing.
        output.tag = BOOL;
        output.num = vector_is_ordered(&args, std::greater_equal<float>());
        return output;
    }
    if (!op.compare("leq")) {
        // All arguments should be non-strictly increasing.
        output.tag = BOOL;
        output.num = vector_is_ordered(&args, std::less_equal<float>());
        return output;
    }

    // 2-ary numeric functions
    if (!op.compare("sub")) {
        if (args[0].tag == NUM && args[1].tag == NUM) {
            return {NUM, args[0].num - args[1].num};
        } else {
            output.tag = FLOAT;
            output.flt = NUMPART(args[0]) - NUMPART(args[1]);
            return output;
        }
    }
    if (!op.compare("div")) {
        output.tag = FLOAT;
        output.flt = NUMPART(args[0]) / NUMPART(args[1]);
        return output;
    }
    if (!op.compare("mod")) {
        output.tag = FLOAT;
        output.flt = std::fmod(NUMPART(args[0]), NUMPART(args[1]));
        return output;
    }

    // 1-ary numeric functions
    if (!op.compare("neg")) {
        output.tag = args[0].tag;
        if (output.tag == NUM)
            output.num = -args[0].num;
        else
            output.flt = -args[0].flt;
        return output;
    }

    if (!op.compare("flip")) return {NUM, ~args[0].num};

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

    if (is_function or !item.compare("expression")) {
        output.string = new std::string;
        *output.string = item;
        output.tag = BUILTIN;
        return output;
    }

    if (!item.compare("True")) { return {BOOL, 1}; }
    if (!item.compare("False")) { return {BOOL, 0}; }
    if (!item.compare("Nothing")) { return *_SINGLETON_NOTHING; }

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
    if (!item.string->compare("expression")) {
        LispVar result;
        Tree<LispVar> *subtree = new Tree<LispVar>;
        *subtree = expression->tree->subtree(index);

        result.tag = EXPRESSION;
        result.tree = subtree;
        return result;
    }

    // Allow binding to variables.
    // This is very scuffed right now and obviously WIP.
    if (!item.string->compare("let")) {
        LispVar result = evaluate_expression(expression, index + 2);
        VARIABLE_SCOPE.set_var(expression->tree->nodes[index + 1].string,
                               result);

        return result;
    }

    std::vector<LispVar> arguments;
    auto size = expression->tree->nodes.size();
    auto original_depth = expression->tree->depths[index];

    for (size_t i = index + 1;
         (original_depth != expression->tree->depths[i]) and i < size;
         i++) {
        if ((original_depth + 1) == expression->tree->depths[i]) {
            auto inner = evaluate_expression(expression, i);

            arguments.push_back(inner);
        }
    }
    if (arguments.empty()) { return item; }
    return call_variable(item, arguments);
}

LispVar parse_and_evaluate(std::string input) {
    auto tree = parse_expression(input);
    return evaluate_expression(&tree, 0);
}

void print_debug(std::string msg) {
    if (DEBUG_MODE) { std::cout << "[DEBUG] " << msg; }
}

int main(int argc, char const *argv[]) {
    if (argc != 3) {
        std::cout << "Error: Expected exactly 2 command-line arguments "
                     "(filename and debug mode)."
                  << '\n';
        exit(1);
    }

    auto since_epoch = std::chrono::system_clock::now().time_since_epoch();

    DEBUG_MODE = std::stoi(argv[2]);
    RNG = std::mt19937(since_epoch.count());

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
