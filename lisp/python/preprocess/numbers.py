#!/usr/bin/env python3.10
"""Utilities for parsing numbers."""
import string
import typing as t

import regex as re
import utils

# ===| Globals |===

SIGNED_LONG_RANGE = range(-2_147_483_648, 2_147_483_648)

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

FLOAT_REGEX = re.compile("(" + _BASE10_NUM[1:-1] + r"\.(\d*)" + r")|(\.\d+)")


def get_numeric_or_none(expr: str) -> t.Optional[t.Union[int, float]]:
    """Get a numeric value if the expression is numeric. Otherwise, return None."""
    if utils.fullmatch(FLOAT_REGEX, expr):
        return float(expr)

    # Canonicalizes numbers of other bases.
    for base_str, base in BASES.items():
        if match := re.match(r"-?" + base_str, expr):
            match_length = len(match.group())
            if match_length >= 2:
                result = int(expr[match_length:], base)
                result *= utils.bool2sign(expr[0] != "-")
                return result

    # Canonicalizes numbers in scientific notation.
    if utils.fullmatch(r"[+-]?\d+e[+-]?\d+", expr, re.IGNORECASE):
        num, exp = map(int, re.split(r"[eE]", expr))
        return num * 10**exp

    if utils.fullmatch(NUMBER_REGEX, expr):
        return int(expr)

    return None
