#!/bin/sh
# Quit on errors in any subprocess.
set -e
set -o pipefail

sh ./build.sh
./lisp example.lisp
