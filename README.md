A Lisp.

# TODO

- [x] Q-exprs working
- [x] Global Nothing object
- [x] Command line argument parsing
    - [x] Read from file
- [x] Comparison operators
- [x] Range shorthands with :
- [x] Variables and scopes
    - [ ] Bind variables using the `VARIABLE` type instead of `STRING`.
- [x] Calling functions
    - [ ] Start writing a standard library in the lisp
    - [ ] Resolve and call functions without the `call` function.
- [x] Type matching
    - [x] BUG: 0-ary function calls resolve to <Builtin 'function name'> instead of the result of the call.
    - [ ] Union type support
    - [ ] BUG: Can't join strings right now.
- [ ] Global error stack
- [ ] Broadcasting function
- [ ] Unescaping strings

## Lambdas


Trees can be called but are still pretty messed up. This is likely due to memory issues that will have to be fixed before much other work on the language.

Store the whole tree in the variable and then just execute it when the function is called?

    (= starcall {fn vec} {
        (eval (insert vec fn 0))
    })

    (= pow {a b} {
        (eval (insert (repeat b a) * 0))
    })
