A Lisp.

# TODO

- [x] Command line argument parsing
    - [x] Read from file
- [x] Calling functions
    - [x] Resolve and call functions without the `call` function.
- [x] Type matching
- [x] Unescaping strings
- [ ] Recompilation also calls generative.py

## Features

- [ ] Union type support
- [ ] Static type inference in Python
- [ ] Optional typing of closures
- [x] Floating point numbers
- [ ] Global error stack
- [ ] Imports
    - [ ] Possibility to define own namespaces
    - [ ] Precompiling Lisp import functions
- [ ] Returning closures from functions
    - [ ] Optional function argument evaluation in inner closures (the reason why this doesn't work now is that the inner expression doesn't evaluate the argument until it's called, when the original argument has gone out of scope)

## Bugs

- [ ] BUG: `join` can't be called on strings.
- [ ] BUG. `assert` signature doesn't permit leaving out the ?-marked string.
- [ ] BUG. same as above for `slice`

## Functions
- [ ] ternary_then (`?> _ fn0 fn1`), calls one of two closures depending on first arguments truthiness
- [ ] none_coalesce (`??`)
    - [ ] `??=`
- [ ] none-coalescing function shorthand with [op]? (f. ex. `@?`)
- [ ] fold based on first value if no default given
- [x] Random function
    - [ ] Seed function
- [ ] hash function
- [ ] Broadcasting function
