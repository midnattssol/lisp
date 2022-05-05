# Grammar

## Function calls

    (+ 10 20) ; -> 30
    (== 10 20) ; -> False

Enclose a built-in function or a closure in parentheses followed by its arguments to call it.

### Assignments

    ; Bind a to 10.
    (let a 10) ; -> 10

    ; Increase a by 11.
    ; Equivalent to (= a (+ a 11)).
    (+= a 11) ; -> 21


The most basic version of an assignment is `let` or its shorthand `=`, which simply binds a variable name to some value and returns it.

The `=` assignment can also be prefixed into the in-place shorthands `*=`, `-=`, `+=`, `/=`, `^=`, `&=`, and `|=`.

## Lists

    [0 1 3] ; -> (list 0 1 3)

Enclosing 0 or more arguments in right-angled brackets expands into a `(list ...)`, a container for 0 or more items.

## Expressions

    {0 1 (* 2 3)} ; -> (expression 0 1 (* 2 3))

Enclosing 0 or more arguments in curly brackets expands into an `(expression ...)`.

Expressions are not evaluated, but instead frozen as they are. They can be called as closures or modified in manners similar to lists.

## Closures

    ; Sum the arguments a and b,
    ; adding 10 to the result.
    (=> sum_and_add_10 {a b}
        (starcall * (repeat b a))
    )

    ; Print something with a newline.
    (=> putl
        (noop
            (put _)
            (put "\n")
        )
    )


The syntax `(=> name args function)` can be used to create lambdas. If `args` are not provided, they default to `{_}`.

It is equivalent to `(let name (closure {{args_expression} (function)}))`.
