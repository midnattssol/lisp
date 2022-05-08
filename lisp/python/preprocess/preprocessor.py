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
import utils
from .macro import Macro, Arity, BadMacroArity
from .numbers import SIGNED_LONG_RANGE, NUMBER_REGEX, get_numeric_or_none

# ===| Globals |===

BASEPATH = p.Path(__file__).parent.parent
SHORTHANDS = utils.cson_from_path(BASEPATH.parent / "data" / "shorthands.cson")
MODIFY_IN_PLACE = utils.cson_from_path(BASEPATH.parent / "data" / "prefix_equals.cson")


def _get_macros():
    def _dfn(args):
        if len(args) == 1:
            args.insert(0, "{_}")
        assert len(args) == 2
        res = f"closure (expression {args[0]} {args[1]})"
        return res

    macros = [
        Macro(
            ["??", "call_ncol!"],
            "eval_expr (? {args[0]} (expression {args[1]}) (expression Nothing))",
            arity=2,
        ),
        Macro(
            "if!",
            "eval_expr (? {args[0]} (expression {args[1]}) (expression {args[2]}))",
            arity=3,
        ),
        Macro(
            ["=>", "def!"],
            arity=Arity(2, 3),
            func=lambda args: f"let {args[0]} ({_dfn(args[1:])})",
        ),
        Macro(
            "while!", arity=2, fmt="while (expression {args[0]}) (expression {args[1]})"
        ),
        Macro(["++"], arity=1, fmt="let {args[0]} (+ {args[0]} 1)"),
        Macro(["--"], arity=1, fmt="let {args[0]} (- {args[0]} 1)"),
        Macro(["λ", "lambda!"], Arity(1, 2), func=_dfn),
    ]

    # Add the in-place operators as macros.
    func0 = (
        lambda op: lambda args: f"let {args[0]} ({op} {' '.join(args[1:])} {args[0]})"
    )
    func1 = (
        lambda op: lambda args: f"let {args[0]} ({op} {args[0]} {' '.join(args[1:])})"
    )

    for name, items in MODIFY_IN_PLACE.items():
        if MODIFY_IN_PLACE[name].get("rev"):
            macros.append(Macro(name, func=func0(items["op"]), arity=Arity.min(1)))
        else:
            macros.append(Macro(name, func=func1(items["op"]), arity=Arity.min(1)))

    return macros


MACROS = _get_macros()

# ===| Functions |===


def tokens_of(expr: str) -> t.List[str]:
    """Tokenize an expression."""
    tokens = [""]
    in_string_literal = False
    depth = 0
    line_is_comment = False
    last_was_escape = False

    for i, char in enumerate(expr):
        line_is_comment |= char == ";" and not in_string_literal
        line_is_comment &= char != "\n"

        if line_is_comment:
            continue

        d_depth = (char in "([{") - (char in "}])")
        depth += d_depth
        if in_string_literal:
            in_string_literal &= char != '"' or last_was_escape
        else:
            in_string_literal |= not in_string_literal and char == '"'

        if not (in_string_literal or depth) and re.match(r"\s", char):
            if tokens[-1]:
                tokens.append("")
            continue

        if last_was_escape and char == "\\":
            last_was_escape = False
        else:
            last_was_escape = char == "\\"

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
    """Make a shorthand range written with colon separation canon."""
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


def _make_function_canon(expr: str) -> str:
    """Recursively canonize child tokens and expand macros."""
    expr = expr[1:-1]
    canon_tokens = list(map(make_canon, tokens_of(expr)))

    for macro in MACROS:
        if canon_tokens[0] in macro.names:
            try:
                result = macro.format_or_throw(canon_tokens)
            except BadMacroArity as a:
                print(repr(a))
                exit(1)

            result = f"({result})"
            canon_tokens = tokens_of(make_canon(result)[1:-1])
            break

    return "(" + " ".join(canon_tokens) + ")"


@utils.counted
@utils.composed_with(postparse)
def make_canon(expr: str) -> str:
    """Make an expression canon."""
    expr = expr.strip()

    if expr in SHORTHANDS:
        return SHORTHANDS[expr]

    if len(expr) >= 2:
        # Canonicalizes shorthand brackets.
        if expr[0] == "{" and expr[-1] == "}":
            expr = "(expression " + expr[1:-1] + ")"
        if expr[0] == "[" and expr[-1] == "]":
            expr = "(list " + expr[1:-1] + ")"
        if expr[0] == "(" and expr[-1] == ")":
            expr = _make_function_canon(expr)
            if expr.startswith("((") and expr.endswith("))"):
                expr = f"(call {expr[1:-1]})"
            return expr

    # Canonicalizes ranges.
    if 1 <= expr.count(":") <= 3:
        result = expr.split(":")
        if any(result) and all(
            not i or utils.fullmatch(NUMBER_REGEX, i) for i in result
        ):
            return _make_range_canon(expr)

    numeric = get_numeric_or_none(expr)
    if numeric is not None:
        return numeric

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
