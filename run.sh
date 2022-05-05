#!/bin/sh
# python lisp/run_lisp.py --debug example.lisp
# python lisp/run_lisp.py --dump test.lisp
python lisp/run_lisp.py --recompile "never" example_rand.lisp
# python lisp/run_lisp.py --recompile "change" example_rand.lisp
# python lisp/run_lisp.py --recompile "always" example_rand.lisp
