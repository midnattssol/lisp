A Lisp.

# TODO

- [x] Command line argument parsing
    - [x] Read from file
- [x] EQ/NEQ/GT/GEQ/LEQ/LT operators
- [x] Type matching
    - [ ] Union type support
    - [ ] BUG: 0-ary function calls resolve to <Builtin 'function name'> instead of the result of the call.
    - [ ] BUG: Can't join strings right now.
- [ ] Variables and scopes
- [ ] Global Nothing object
- [ ] Global error stack
- [ ] Lambdas using ->
    - [ ] Start writing a standard library in the lisp
- [x] Q-exprs working
- [ ] Type matching in the operation evaluation.

## Lambdas

Store the whole tree in the variable and then just execute it when the function is called?
