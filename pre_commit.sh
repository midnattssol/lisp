#!/bin/sh
set -xe

# Recompile the Lisp.
python lisp/python/run_lisp.py --recompile "always" -c ""

# Runs all tests.
for i in tests/*.lisp; do
    python lisp/python/run_lisp.py $i
done

# Saves the performance statistics.
python speed.py --num 10 > performance.txt
