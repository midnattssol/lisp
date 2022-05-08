#!/usr/bin/env python3.10
"""Generate and sorts 400 random numbers with insertion sort."""
import argparse
import random
import subprocess
import timeit


def insert_sort(_list):
    """Run an insertion sort equivalent in implementation to that which is in the Lisp."""

    def inserted_into_list(item, _list):
        p = len(_list)
        i = p - 1

        while i >= 0:
            if _list[i] > item:
                p = i
            i -= 1

        new = _list.copy()
        new.insert(p, item)
        return new

    out = []
    for i in _list:
        out = inserted_into_list(i, out)

    return out


def main():
    """Carry out the testing."""
    parser = argparse.ArgumentParser(
        description="Generate and sort 400 random numbers with insertion sort in Python and Lisp."
    )
    parser.add_argument("--num", default=4, type=int, help="number of iterations to do")
    args = parser.parse_args()

    DEV_NULL = open("/dev/null")

    def _sort_n(n, x):
        for _ in range(x):
            insert_sort([random.random() for _ in range(n)])

    py_result = timeit.timeit(lambda: _sort_n(512, 4), number=args.num)

    lisp_result = timeit.timeit(
        lambda: subprocess.run(
            ["python", "lisp/python/run_lisp.py", "examples/sort.lisp"],
            stdout=DEV_NULL,
        ),
        number=args.num,
    )

    print(f"Did {args.num} iteration(s) of insertion sort over 4 512-element lists.")
    print(f"Python:      {py_result / args.num:.5f}s.")
    print(f"Lisp:        {lisp_result / args.num:.5f}s.")
    print(f"Python was {lisp_result / py_result:.2f} times faster than the Lisp.")


if __name__ == "__main__":
    main()
