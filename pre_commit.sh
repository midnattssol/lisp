#!/bin/sh
set -xe

# Recompile the Lisp.
./lisp --recompile "always" --log DEBUG -c ""

# Runs all tests.
for i in tests/*.lisp; do
    ./lisp --log DEBUG $i
done

# Saves the performance statistics.
python source/python/speed.py --num 100 > performance.txt

# Saves the dependency tree for easy access.
python source/python/includetree.py source/cpp/lisp.cpp > includetree.txt

# Count lines of code in the repository.
cloc . | tail -n +6 > lines.txt
