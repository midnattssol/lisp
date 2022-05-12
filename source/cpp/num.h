#include <functional>
#include <vector>

#include "./gen.h"

/* Accumulate a vector of numeric LispVars into a long. */
long accumulate_l(std::vector<LispVar *> *args,
                  long base,
                  std::function<long(long, long)> func) {
    for (auto arg : *args) base = func(base, arg->to_l());
    return base;
}

/* Accumulate a vector of numeric LispVars into a float. */
float accumulate_f(std::vector<LispVar *> *args,
                   float base,
                   std::function<float(float, float)> func) {
    for (auto arg : *args) base = func(base, arg->to_f());
    return base;
}

/* Returns a boolean corresponding to whether or not all the items are
 * ordered according to the function `func`.

Examples
--------
std::vector<int> v = {1, 2, 3};
bool result = vector_is_ordered(&v, std::lesser<float>()) // true
bool result = vector_is_ordered(&v, std::greater<float>()) // false
 . */
bool vector_is_ordered(std::vector<LispVar *> *args,
                       std::function<bool(float, float)> func) {
    unsigned int arity = args->size();
    if (!arity) return true;

    bool output = true;
    float last_num = (*args)[0]->to_f();
    for (size_t i = 1; i < arity; i++) {
        output = func(last_num, (*args)[i]->to_f());
        if (!output) break;
        last_num = (*args)[i]->to_f();
    }

    return output;
}
