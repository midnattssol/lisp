# Builtins

This page has documentation about the builtins of the Lisp.

## `do`
_Signature: `[+] -> any`_

Return the last of its arguments.

## `eq`
_Signature: `[*] -> bool`_

Return `True` if all elements are equal, or `False` otherwise.

## `gt`
_Signature: `[* numeric] -> bool`_

Return `True` if all arguments are ordered in strictly decreasing order, or `False` otherwise.

## `lt`
_Signature: `[* numeric] -> bool`_

Return `True` if all arguments are ordered in strictly increasing order, or `False` otherwise.

## `or`
_Signature: `[* numeric] -> numeric`_

Return the bitwise `or` of all arguments, defaulting to 0.

## `add`
_Signature: `[* numeric] -> numeric`_

Add all arguments together, defaulting to 0.

## `and`
_Signature: `[* numeric] -> numeric`_

Return the bitwise `and` of all arguments, defaulting to `(flip 0)`.

## `div`
_Signature: `[numeric] [numeric] -> numeric`_

Divide $0 by $1.

## `geq`
_Signature: `[* numeric] -> bool`_

Return `True` if all arguments are ordered in non-strictly decreasing order, or `False` otherwise.

## `get`
_Signature: `[int] [indexable] -> any`_

Get the item in $1 with index equal to $0.

## `int`
_Signature: `[numeric] -> int`_

Cast the argument to an integer.

## `len`
_Signature: `[iterable] -> int`_

Get the number of elements in the iterable.

## `leq`
_Signature: `[* numeric] -> bool`_

Return `True` if all arguments are ordered in non-strictly increasing order, or `False` otherwise.

## `let`
_Signature: `[string] [any] -> any`_

Bind $1 to the variable name $0.

## `map`
_Signature: `[callable] [vector *] -> vector`_

Return a new vector with the callable applied to each element in the original vector.

## `mod`
_Signature: `[int] [int] -> int`_

Return $0 modulo $1.

## `mul`
_Signature: `[* numeric] -> numeric`_

Multiply all arguments together, defaulting to 1.

## `neg`
_Signature: `[numeric] -> numeric`_

Return a negative version of the number.

## `neq`
_Signature: `[*] -> bool`_

Return `True` if all elements are different, or `False` otherwise.

## `pop`
_Signature: `[vector] -> any`_

Pop the last element of $0 in-place.

## `put`
_Signature: `[*] -> nothing`_

Print the arguments to `stdout`.

## `sub`
_Signature: `[numeric] [numeric] -> numeric`_

Subtract $1 from $0.

## `xor`
_Signature: `[* numeric] -> numeric`_

Return the bitwise `xor` of all arguments, defaulting to 0.

## `bool`
_Signature: `[booly] -> bool`_

Cast the input to a boolean.

## `call`
_Signature: `[callable] [any *] -> any`_

Call $0 on some arguments.

## `copy`
_Signature: `[any] -> any`_

Return a copy of $0.

## `exit`
_Signature: `[int] -> nothing`_

Exit the program with $0 as the error code.

## `flip`
_Signature: `[int] -> int`_

Flip all the bits in the number.

## `fold`
_Signature: `[callable] [vector] [?] -> any`_

Fold a vector together using a callable and an optional accumulator.

### Examples

    (fold + [10 20 30 40]) ; Equivalent to (+ 40 (+ 30 (+ 10 20)))

## `help`
_Signature: `[any] -> string`_

Get a help string about the argument.

## `join`
_Signature: `[* vector] -> vector`_

Join any number of vectors together.

## `list`
_Signature: `[*] -> list`_

Construct a linked list containing the arguments.

## `noop`
_Signature: `[*] -> nothing`_

Do nothing with the arguments.

## `push`
_Signature: `[vector] [any] -> nothing`_

Push $1 at the end of $0 in-place.

## `rand`
_Signature: `[int] -> vector`_

Get a vector of random integers of length equal to the argument. The numbers are generated using the MT19937 implementation of the Mersenne Twister.

## `repr`
_Signature: `[any] -> string`_

Get a string representation of the argument.

## `seed`
_Signature: `[int] -> nothing`_

Seed the Mersenne Twister used by `rand`.

## `type`
_Signature: `[string] -> type`_

Construct a type object from a typename.

## `float`
_Signature: `[numeric] -> float`_

Cast the argument to a float.

## `match`
_Signature: `[string] [string] -> bool`_

Return whether or not the regular expression $0 fully matches the string $1.

## `range`
_Signature: `[int ?] [int ?] [int ? truthy] -> vector`_

Return every $2 numbers from $0 to $1.

## `slice`
_Signature: `[vector] [int] [int ?] [int ?] -> vector`_

Return the elements between $1 and $2 inclusive in the vector, with a step size equal to the $3.

## `while`
_Signature: `[expression] [expression] -> nothing`_

As long as the result of evaluating $0 is `True`, $1 is evaluated.

## `assert`
_Signature: `[booly] [string ?] -> nothing`_

Check if the argument is truthy, showing an optional error message before quitting.

## `insert`
_Signature: `[any] [int] [iterable] -> vector`_

Return a copy of $2 with $0 inserted at the index $1.

## `repeat`
_Signature: `[int] [any] -> vector`_

Return a vector of length equal to $0 by repeating $1.

## `typeof`
_Signature: `[any] -> type`_

Get a type object describing the type of the argument.

## `vector`
_Signature: `[*] -> vector`_

Construct a vector containing the arguments.

## `closure`
_Signature: `[expression] -> closure`_

Construct a closure from an expression containing an argument vector and an expression to evaluate.

## `linsert`
_Signature: `[any] [int] [iterable] -> list`_

Return a copy of $2 with $0 inserted at the index $1.

## `ternary`
_Signature: `[booly] [any] [any] -> any`_

Return $1 if $0 is truthy, otherwise $2.

## `eval_expr`
_Signature: `[expression] -> any`_

Evaluate an expression.

## `typematch`
_Signature: `[vector] [vector] -> bool`_

Return whether or not the type of $1 matches $0 as a type descriptor.

## `expression`
_Signature: `[*] -> expression`_

Construct an expression from the arguments.

