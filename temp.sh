# ./lisp "std/itertools.lisp" --log DEBUG --recompile "change"

# ./lisp "examples/arguments.lisp" -a 10 20 11
./lisp "examples/golf/fizz_buzz.lisp"

# set -xe
# for i in examples/golf/*.lisp; do
#     echo "Running $i..."
#     ./lisp $i
# done
