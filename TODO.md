# TODO

## Features

- [ ] Make errors look more similar.
- [ ] Tracebacks to error location in code.
- [ ] Static type inference in Python
- [ ] Optional typing of closures
- [ ] Global error stack
- [x] Imports
    - [ ] Namespaces
    - [ ] Precompiling headers
        - Just dump all the variables into some file with some protocol

- [x] Returning closures from functions
    - [x] Optional function argument evaluation in inner closures (the reason why this doesn't work now is that the inner expression doesn't evaluate the argument until it's called, when the original argument has gone out of scope)
    - [ ] Note that closures returning closures with variables which are set both in the inner and outer loop don't work since the return checker replaces all occurrences - this can be fixed with preprocessing though.
    - [ ] Preprocess variables into numbers instead of strings to find them quicker.
- [x] #() function syntax

## Bugs

- [ ] Typematching has broken again, but there's probably an earlier version where it worked because the tests fail now but didn't earlier.

## Functions
- [ ] Hash function
- [ ] Broadcasting function
