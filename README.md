A Lisp.

# TODO

## Features

- [ ] Make errors look more similar.
- [ ] Tracebacks to error location in code.
- [ ] Static type inference in Python
- [ ] Optional typing of closures
- [ ] Global error stack
- [x] Imports
    - [ ] Possibility to define own namespaces
    - [ ] Precompiling Lisp import functions
- [x] Returning closures from functions
    - [x] Optional function argument evaluation in inner closures (the reason why this doesn't work now is that the inner expression doesn't evaluate the argument until it's called, when the original argument has gone out of scope)
    - [ ] Note that closures returning closures with variables which are set both in the inner and outer loop don't work since the return checker replaces all occurrences - this can be fixed with preprocessing though.
    - [ ] Preprocess variables into numbers instead of strings to find them quicker.

## Bugs

- [ ] BUG: `join` can't be called on strings.

## Functions
- [ ] hash function
- [ ] Broadcasting function
