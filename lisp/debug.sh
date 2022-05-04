#!/bin/sh
g++ -O1 lisp.cpp -o lisp
chmod +x lisp
gdb lisp

# Run r ../example.lisp 0
