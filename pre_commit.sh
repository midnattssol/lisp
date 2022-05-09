#!/bin/sh
set -xe

# Recompile the Lisp.
python lisp/python/run_lisp.py --recompile "always" -c ""

# Runs all tests.
for i in tests/*.lisp; do
    python lisp/python/run_lisp.py $i
done

# Saves the performance statistics.
python lisp/python/speed.py --num 100 > performance.txt

# Saves the dependency tree for easy access.
python lisp/python/includetree.py lisp/cpp/lisp.cpp > includetree.txt
