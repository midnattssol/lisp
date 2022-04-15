#!/bin/sh
# Quit on errors in any subprocess.
set -e
set -o pipefail

# Compile the source if it has changed.
source_file="lisp.cpp"
cpp_hash=$(sha256sum $source_file | awk '{print $1}')

if [[ $(cat .source.hash) == $cpp_hash ]]; then
    echo "[DEBUG] Source remained the same (no recompilation needed)."
else
    echo "[DEBUG] Source has changed since the last compilation."
    echo "[DEBUG] Recompiling...."

    (g++ -O3 -o lisp $source_file) > /dev/null 2>&1
    echo $cpp_hash > .source.hash
fi
