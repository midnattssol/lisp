/* Provides runtime variable resolution in the lisp.*/
#include <forward_list>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

/* A scoped implementation of variables for a programming language running in a
CPP virtual machine.

The namespaces are implemented as a map from variable names to a forward list
of ValueAndDepth objects. The depth should be incremented every time a
function/subroutine is called.

The namespace takes one template argument, which is the type of the variable
values.

*/
template <class T>
class VariableScope {
   public:
    struct ValueAndDepth {
        T value;
        unsigned int depth;
    };

    std::map<std::string, std::forward_list<ValueAndDepth>> scopes;
    unsigned int depth;

    /* Increment the scope depth. */
    void increment() { this->depth++; }

    /* Decrement the scope depth. */
    void decrement() {
        this->depth--;
        this->clean_scope();
    }

    /* Get whether or not the variable is set.*/
    bool is_set(std::string* varname) {
        auto pos = this->scopes.find(*varname);
        return pos != this->scopes.end();
    }

    /* Get the value of the variable name. Throws runtime_error if the variable
     * has not been set. */
    T get_var(std::string* varname) {
        auto pos = this->scopes.find(*varname);

        if (pos == this->scopes.end()) {
            throw std::runtime_error("Could not resolve variable name '" +
                                     *varname + "'");
        }

        return pos->second.front().value;
    }

    /* Get the variable or a fallback value if it isn't set.*/
    T get_var_or(std::string* varname, T fallback) {
        if (this->is_set(varname)) { return this->get_var(varname); }
        return fallback;
    }

    /* Insert a new forward list if none is avaliable or push
    the value onto the front of the avaliable list. */
    void set_var(std::string* varname, T value) {
        auto pos = this->scopes.find(*varname);
        ValueAndDepth item = {value, this->depth};

        if (pos == this->scopes.end()) {
            this->scopes[*varname] = {item};
        } else {
            // Discards the value if it has already been set in this scope.
            if (this->scopes[*varname].front().depth == this->depth) {
                this->scopes[*varname].pop_front();
            }
            this->scopes[*varname].push_front(item);
        }
    }

    /* Get the total number of variables stored in the map. */
    unsigned int tally() {
        unsigned int acc = 0;

        for (auto x : this->scopes) {
            acc += std::distance(x.second.begin(), x.second.end());
        }
        return acc;
    }

   private:
    /* Remove variables which have gone out of scope. */
    void clean_scope() {
        for (auto it = this->scopes.begin(); it != this->scopes.end();) {
            auto values = &it->second;
            if (values->front().depth > this->depth) {
                // Remove definitions that have gone out of scope.
                while (!values->empty() &&
                       values->front().depth > this->depth) {
                    values->pop_front();
                }

                // Remove the forward list entirely if the variable is no longer
                // defined. Required iterator magic is based on
                // https://stackoverflow.com/a/8234813.
                if (values->empty()) {
                    this->scopes.erase(it++);
                    continue;
                }
            }
            it++;
        }
    }
};
