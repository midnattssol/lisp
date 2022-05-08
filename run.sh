#!/bin/sh
set -xe

for i in examples/*.lisp; do
    python lisp/python/run_lisp.py --recompile "change" $i
done
