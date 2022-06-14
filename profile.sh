#!/bin/sh
./lisp --dump examples/sort.lisp > _sort_canon.lisp
valgrind --tool=callgrind source/lisp _sort_canon.lisp 0 0

# ./lisp --recompile "always" -c ""
# ./lisp examples/sort.lisp
# ./lisp -c "(put (> 10 20))"
# ./lisp --dump examples/sort.lisp > _sort_canon.lisp
# source/lisp _sort_canon.lisp 0 1


# python source/python/speed.py --num 1
