#!/usr/bin/env python3.10
"""Contains utilities for preprocessing the Lisp program.

{...} -> (expression ...)
[...] -> (list ...)
a:b:c -> (range a b c)
(= a 10) -> (= 'a' 10)
"""
import argparse
import pathlib as p
import string
import typing as t

import regex as re
from utils import bool2sign, composed_with, fullmatch, json_from_path

# ===| Globals |===

BASEPATH = p.Path(__file__).parent
SIGNED_LONG_RANGE = range(-2_147_483_648, 2_147_483_648)

SHORTHANDS = json_from_path(BASEPATH / "shorthands.json")
prefix_equals = json_from_path(BASEPATH / "prefix_equals.json")

_BASE_N_NUM_REGEX = "({prefix}([{nums}]+_)*[{nums}]+)"
_BASE10_NUM = _BASE_N_NUM_REGEX.format(prefix="", nums=r"\d")

BASES = {"0b": 2, "0t": 3, "0x": 16}
NUMBER_REGEX = re.compile(
    r"[-+]?("
    + "|".join(
        _BASE_N_NUM_REGEX.format(
            prefix=prefix,
            nums=string.digits[:base] + string.ascii_lowercase[: max(base - 10, 0)],
        )
        for prefix, base in BASES.items()
    )
    + "|"
    + _BASE10_NUM
    + r"|"
    # Exponents
    + "(" + _BASE10_NUM + "e" + _BASE10_NUM + ")" + ")",
    re.VERBOSE | re.IGNORECASE,
)

# ===| Functions |===


def tokens_of(expr: str) -> t.List[str]:
    """Tokenize an expression."""
    tokens = [""]
    in_string_literal = False
    depth = 0
    line_is_comment = False

    for i, char in enumerate(expr):
        line_is_comment |= char == ";" and not in_string_literal
        line_is_comment &= char != "\n"

        if line_is_comment:
            continue

        d_depth = (char in "([{") - (char in "}])")
        depth += d_depth
        in_string_literal ^= char == "'"

        if not (in_string_literal or depth) and re.match(r"\s", char):
            if tokens[-1]:
                tokens.append("")
            continue

        tokens[-1] += char

    tokens = [t for t in tokens if any(map(lambda x: x not in string.whitespace, t))]
    return tokens


def postparse(expr: t.Any) -> str:
    """Run after parsing."""
    if isinstance(expr, int):
        if expr not in SIGNED_LONG_RANGE:
            print(f"[ERROR] Number '{expr}' not in {SIGNED_LONG_RANGE}")
            exit(1)
    return str(expr)


def _make_range_canon(expr: str) -> str:
    """Make a range canon."""
    result = expr.split(":")
    while len(result) < 3:
        result.append("")

    result[0] = "0" if not result[0] else result[0]  # Start
    result[2] = "1" if not result[2] else result[2]  # Step

    # Stop
    if not result[1]:
        first = int(make_canon(result[0]))
        result[1] = "-1" if first > 0 else "0"

    return make_canon("(range " + " ".join(result) + ")")


@composed_with(postparse)
def make_canon(expr: str) -> str:
    """Make an expression canon."""
    expr = expr.strip()

    if expr in SHORTHANDS:
        return SHORTHANDS[expr]

    if expr.startswith(";"):
        expr = "\n".join(expr.splitlines()[1:])
        return make_canon(expr)

    if len(expr) >= 2:
        # Canonicalizes shorthand brackets.
        if expr[0] == "{" and expr[-1] == "}":
            return make_canon("(expression " + expr[1:-1] + ")")

        if expr[0] == "[" and expr[-1] == "]":
            return make_canon("(list " + expr[1:-1] + ")")

        # Recursively calls the function on child tokens.
        if expr[0] == "(" and expr[-1] == ")":
            expr = expr[1:-1]
            canon_tokens = list(map(make_canon, tokens_of(expr)))

            if canon_tokens[0] in prefix_equals:
                if prefix_equals[canon_tokens[0]].get("rev"):
                    canon_tokens = [
                        "let",
                        canon_tokens[1],
                        f"({prefix_equals[canon_tokens[0]]['op']} "
                        + " ".join(canon_tokens[2:])
                        + " "
                        + canon_tokens[1]
                        + ")",
                    ]
                else:
                    canon_tokens = [
                        "let",
                        canon_tokens[1],
                        f"({prefix_equals[canon_tokens[0]]['op']} {canon_tokens[1]} "
                        + " ".join(canon_tokens[2:])
                        + ")",
                    ]

                canon_tokens = list(map(make_canon, canon_tokens))

            if canon_tokens[0] in {"=>", "λ"}:
                # No arguments have been provided. Fall back on _.
                if len(canon_tokens) == 3:
                    canon_tokens.insert(2, "{_}")

                canon_tokens = [
                    "let",
                    canon_tokens[1],
                    "(closure {" + " ".join(canon_tokens[2:]) + "})",
                ]
                canon_tokens = list(map(make_canon, canon_tokens))

            return "(" + " ".join(canon_tokens) + ")"

    # Canonicalizes ranges.
    if 1 <= expr.count(":") <= 3:
        result = expr.split(":")
        if any(result) and all(not i or fullmatch(NUMBER_REGEX, i) for i in result):
            return _make_range_canon(expr)

    # Canonicalizes numbers of other bases.
    for base_str, base in BASES.items():
        if match := re.match(r"-?" + base_str, expr):
            match_length = len(match.group())
            if match_length >= 2:
                result = int(expr[match_length:], base)
                result *= bool2sign(expr[0] != "-")
                return result

    # Canonicalizes numbers in scientific notation.
    if fullmatch(r"[+-]?\d+e[+-]?\d+", expr, re.IGNORECASE):
        num, exp = map(int, re.split(r"[eE]", expr))
        return num * 10**exp

    return expr


def main():
    """Preprocess some Lisp code."""
    parser = argparse.ArgumentParser(description="Preprocess some Lisp code.")
    parser.add_argument(
        "code",
        metavar="O",
        help="the code to parse",
    )
    args = parser.parse_args()
    print(make_canon(args.code))


if __name__ == "__main__":
    main()
