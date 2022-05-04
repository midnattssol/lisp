#include <vector>
#include <functional>
#include "./gen.h"

/* Accumulate a vector of numeric LispVars into an integer. */
long accumulate(std::vector<LispVar> *args,
                long base,
                std::function<long(long, long)> func) {
    for (auto arg : *args) base = func(base, arg.num);
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
