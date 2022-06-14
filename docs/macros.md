# Macros

This page has documentation about the macros of the Lisp.

## `&&`

Logical and. Casts all items to bools before `and`ing them together.

## `||`

Logical or. Casts all items to bools before `or`ing them together.

## `reverse!`

Reverse a collection.

## `head!`

Get the first element in a collection.

## `tail!`

Get the last element in a collection.

## `push!`

Push an element into the end of the collection.

## `pop!`

Get the element at the end of the collection.

## `if!`

If the first expression evaluates to `Yes`, calculate the second expression. If not, calculate the third expression, which defaults to `Nil`.

## `unless!`

As `if!` but with the order of the last two elements reversed.

## `putl!`

Write something to stdout followed by a newline.

## `=>`, `def`

Bind a variable to a closure.

## `while!`

Carry out the second expression as long as the first expression evaluates to Yes, or until the loop is broken out of with the `break` function.

## `loop!`

Carry out the second expression forever, or until the loop is broken out of with the `break` function.

## `include!`

Import another Lisp module from the local folder.

## `use!`

Import another Lisp module from the standard library.

## `++`

Add 1 to a variable in place.

## `--`

Subtract 1 from a variable in place.
