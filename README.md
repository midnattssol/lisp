A Lisp.

# TODO

- [x] Command line argument parsing
    - [x] Read from file
- [x] Calling functions
    - [x] Resolve and call functions without the `call` function.
- [x] Type matching
- [x] Unescaping strings

## Features

- [ ] Union type support
- [ ] Static type inference in Python
- [ ] Optional typing of closures
- [ ] Floating point numbers
- [ ] Global error stack
- [ ] Imports
    - [ ] Possibility to define own namespaces
    - [ ] Precompiling Lisp import functions

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
- [ ] random function (optional seed value)
- [ ] hash function
- [ ] Broadcasting function
