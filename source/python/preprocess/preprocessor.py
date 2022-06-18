#!/usr/bin/env python3.10
"""Contains utilities for preprocessing the Lisp program.

{...} -> (expression ...)
[...] -> (list ...)
a:b:c -> (range a b c)
(= a 10) -> (= 'a' 10)
"""
import argparse
import dataclasses as dc
import pathlib as p
import string
import typing as t

import regex as re
import utils

from .errors import SourceError
from .macro import Arity, BadMacroArity, Macro
from .numbers import NUMBER_REGEX, SIGNED_LONG_RANGE, get_numeric_or_none

# ===| Globals |===

BASEPATH = p.Path(__file__).parent.parent
SHORTHANDS = utils.cson_from_path(BASEPATH.parent / "data" / "shorthands.cson")
MODIFY_IN_PLACE = utils.cson_from_path(BASEPATH.parent / "data" / "prefix_equals.cson")
BOOLS = {"True": "Yes", "False": "No", "On": "Yes", "Off": "No", "Nothing": "Nil"}


class LispImportWithoutFileException(BaseException):
    """Raised if an import is attempted without a known file."""


def postparse(expr: t.Any) -> str:
    """Run after parsing."""
    if isinstance(expr, int):
        if expr not in SIGNED_LONG_RANGE:
            print(f"[ERROR] Number '{expr}' not in {SIGNED_LONG_RANGE}.")
            exit(1)
    return str(expr)


@dc.dataclass
class Token:
    index: int
    data: str

    def __bool__(self):
        return bool(self.data)

    def __str__(self):
        return self.data

    def range(self):
        return range(self.index, self.index + len(self.data))


def tokens_of(expr: str, not_as_str=False, include_comments=False) -> t.List[str]:
    """Tokenize an expression."""
    tokens = [Token(0, "")]
    in_string_literal = False
    depth = 0
    line_is_comment = False
    last_was_escape = False

    for i, char in enumerate(expr):
        line_is_comment |= char == ";" and not in_string_literal
        line_is_comment &= char != "\n"

        if line_is_comment:
            if include_comments and not tokens[-1].data.startswith(";"):
                tokens.append(Token(i, char))
            else:
                tokens[-1].data += char
            continue

        if include_comments and tokens[-1].data.startswith(";"):
            tokens.append(Token(i, ""))

        d_depth = (char in "([{") - (char in "}])")
        depth += d_depth
        if in_string_literal:
            in_string_literal &= char != '"' or last_was_escape
        else:
            in_string_literal |= not in_string_literal and char == '"'

        if not (in_string_literal or depth) and re.match(r"\s", char):
            if tokens[-1].data:
                tokens.append(Token(i, ""))
            continue

        if last_was_escape and char == "\\":
            last_was_escape = False
        else:
            last_was_escape = char == "\\"

        if not tokens[-1]:
            tokens[-1].index = i
        tokens[-1].data += char

    tokens = [
        t for t in tokens if any(map(lambda x: x not in string.whitespace, t.data))
    ]

    if not not_as_str:
        tokens = [t.data for t in tokens]

    return tokens


@dc.dataclass
class Preprocessor:
    """Preprocess Lisp code."""

    contexts: t.List[t.Optional[p.Path]] = dc.field(default_factory=lambda: [None])
    included: t.Set[p.Path] = dc.field(default_factory=set)
    # Ranges corresponding to the tokens
    _traceback: t.List[range] = dc.field(default_factory=list)
    _code: str = None

    def make_canon(self, expr: str) -> str:
        """Make an expression canon."""
        expr = f"(do {expr})"
        self._code = expr
        assert expr.count("(") == expr.count(")")
        return self._make_canon(expr, stack=True)

    def __post_init__(self):
        self.macros = self._get_macros()

    def _import(self, filename: str, std=False):
        # TODO: proper error message here
        assert filename.startswith('"') and filename.endswith('"')
        filename = filename[1:-1]

        if std:
            imported_path = BASEPATH.parent.parent / "std" / (filename + ".lisp")
            if imported_path in self.included:
                return "do"
            self.included.add(imported_path)
        else:
            imported_path = self.contexts[-1].parent / (filename + ".lisp")

            if self.contexts[-1] is None:
                raise LispImportWithoutFileException

        code = utils.cat(imported_path)
        code = f"(do {code})"

        # Make upcoming imports come from this path.
        self.contexts.append(imported_path)

        # The indexing is necessary since the macro expects the result not to have parentheses.
        code = self._make_canon(code)
        self.contexts.pop()
        return code[1:-1]

    def _get_macros(self):
        def switch_fn(args):
            name = args.pop(0)
            code = "\n(if! {condition} {yes} {no})"
            base_code = code

            while args:
                item = args.pop(0)
                tokenized = tokens_of(item[1:-1])

                is_well_formed = tokenized[0] == "case"
                is_well_formed &= len(tokenized) == 3

                if not args:
                    is_well_formed = tokenized[0] == "otherwise"
                    is_well_formed &= len(tokenized) == 2

                assert is_well_formed

                if tokenized[0] == "otherwise":
                    code = code.replace("(if! {condition} {yes} {no})", tokenized[1])
                else:
                    code = code.format(
                        condition=f"(== {tokenized[1]} {name})",
                        yes=tokenized[2],
                        no="%s",
                    )
                    code %= base_code if args else "Nil"

            return code[2:-1]

        def dfn(args):
            if len(args) == 1:
                args.insert(0, "{_}")
            assert len(args) == 2
            res = f"closure (expression {args[0]} {args[1]})"
            return res

        def over_fn(args):
            ftokens = tokens_of(args[0][1:-1])
            assert len(ftokens) in range(2, 5) and ftokens[0] == "vector"
            ftokens.pop(0)
            if len(ftokens) == 2:
                ftokens.append("__index__")
            if len(ftokens) == 3:
                ftokens.append("__length__")

            res = (
                self._make_canon(f"(= {ftokens[2]} 0)"),
                self._make_canon(f"(= {ftokens[3]} (# {ftokens[0]}))"),
                self._make_canon(
                    f" (while! (< {ftokens[2]} {ftokens[3]}) (do (= item (@ {ftokens[2]} {ftokens[0]})) {' '.join(args[1:])} (++ {ftokens[2]})))"
                ),
            )
            # print(self._make_canon("(noop" + res + ")") == "(noop" + res + ")")
            return " ".join(res)[1:-1]

        macros = [
            Macro(
                "if!",
                func=lambda args: f"eval_expr (? {args[0]} (expression {args[1]}) (expression {args[2] if len(args) >= 3 else 'Nil'}))",
                arity=Arity(2, 3),
            ),
            Macro(
                "unless!",
                func=lambda args: f"if! {args[0]} {args[2] if len(args) >= 3 else 'Nil'} {args[1]}",
                arity=Arity(2, 3),
            ),
            Macro(
                "putl!",
                arity=Arity.any(),
                func=lambda args: f'put {" ".join(args)} "\\n"',
            ),
            Macro(
                "head!",
                arity=1,
                func=lambda args: f'@ 0 {" ".join(args)}',
            ),
            Macro(
                "tail!",
                arity=1,
                func=lambda args: f'@ -1 {" ".join(args)}',
            ),
            Macro(
                "&&",
                arity=Arity.any(),
                func=lambda args: f'bool (& {" ".join(f"(bool {i})" for i in args)})',
            ),
            Macro(
                "pipe!",
                arity=Arity.min(2),
                func=lambda args: f'{"pipe! " + " ".join(args[2:]) if len(args) > 2 else ""} ({args[1]} {args[0]})',
            ),
            Macro(
                "||",
                arity=Arity.any(),
                func=lambda args: f'bool (| {" ".join(f"(bool {i})" for i in args)})',
            ),
            Macro(
                "push!",
                arity=2,
                func=lambda args: f"insert {args[0]} -1 {args[1]}",
            ),
            Macro(
                "pop!",
                arity=1,
                func=lambda args: f'slice {" ".join(args)} 0 -2',
            ),
            Macro(
                ["=>", "def!"],
                arity=Arity(2, 3),
                func=lambda args: f"let {args[0]} ({dfn(args[1:])})",
            ),
            Macro(
                "while!",
                arity=Arity.min(2),
                func=lambda args: f"while (expression {args[0]}) (expression (do {' '.join(args[1:])}))",
            ),
            Macro(
                "loop!",
                arity=Arity.min(1),
                func=lambda args: f"while! Yes {' '.join(args)}",
            ),
            Macro(
                "rev!",
                arity=Arity(1),
                func=lambda args: f"slice {args[0]} -1 0 -1",
            ),
            Macro(
                "for!",
                arity=Arity.min(2),
                func=over_fn,
            ),
            Macro(
                "where!",
                arity=Arity.min(3),
                func=lambda args: f"for! {args[0]} (if! {args[1]} {' '.join(args[2:])})",
            ),
            Macro("include!", arity=1, func=lambda args: self._import(args[0])),
            Macro("switch", arity=Arity.min(1), func=switch_fn),
            Macro("use!", arity=1, func=lambda args: self._import(args[0], std=True)),
            Macro(["++"], arity=1, fmt="let {args[0]} (+ {args[0]} 1)"),
            Macro(["--"], arity=1, fmt="let {args[0]} (- {args[0]} 1)"),
            Macro(["λ", "lambda!", "->"], Arity(1, 2), func=dfn),
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

    def _make_range_canon(self, expr: str) -> str:
        """Make a shorthand range written with colon separation canon."""
        result = expr.split(":")
        while len(result) < 3:
            result.append("")

        result[0] = "0" if not result[0] else result[0]  # Start
        result[2] = "1" if not result[2] else result[2]  # Step

        # Stop
        if not result[1]:
            first = int(self._make_canon(result[0]))
            result[1] = "-1" if first > 0 else "0"

        return self._make_canon("(range " + " ".join(result) + ")")

    def _make_function_canon(self, expr: str) -> str:
        """Recursively canonize child tokens and expand macros."""
        expr = expr[1:-1]
        tokens = tokens_of(expr, not_as_str=True)
        tokens = [Token(i.index + 1 + self._last_token_start(), i.data) for i in tokens]
        canon_tokens = []

        for i in tokens:
            self._traceback.append(i.range())
            canon_tokens.append(self._make_canon(i.data))
            self._traceback.pop()

        self._traceback.append(tokens[0].range())

        for macro in self.macros:
            if canon_tokens[0] in macro.names:
                try:
                    result = macro.format_or_throw(canon_tokens)
                except BadMacroArity as a:
                    error = SourceError(
                        self._code,
                        self._traceback[-1].start,
                        self._traceback[-1].stop,
                        "E0",
                        "Incorrect macro arity",
                        under_msg=repr(a),
                        ansi=True,
                    )
                    print(f"\n{error}\n")
                    exit(1)

                result = f"({result})"
                canon_tokens = tokens_of(self._make_canon(result)[1:-1])
                break

        self._traceback.pop()

        return "(" + " ".join(canon_tokens) + ")"

    def _last_token_start(self):
        return self._traceback[-1].start if self._traceback else 0

    def _make_canon(self, expr: str, stack=False) -> str:
        if stack:
            last = self._last_token_start()
            self._traceback.append(range(last, last + len(expr)))

        expr = self.__make_canon(expr)
        expr = postparse(expr)

        if stack:
            self._traceback.pop()

        return expr

    def __make_canon(self, expr: str) -> str:
        expr = expr.strip()
        if expr in SHORTHANDS:
            return SHORTHANDS[expr]

        if len(expr) >= 2:
            # Canonicalizes shorthand brackets.
            if expr.startswith("#[") and expr[-1] == "]":
                expr = "(-> (" + expr[2:-1] + "))"
            if expr[0] == "{" and expr[-1] == "}":
                expr = "(expression " + expr[1:-1] + ")"
            if expr[0] == "[" and expr[-1] == "]":
                expr = "(vector " + expr[1:-1] + ")"
            if expr.startswith("l[") and expr[-1] == "]":
                expr = "(list " + expr[2:-1] + ")"
            if expr[0] == "(" and expr[-1] == ")":
                expr = self._make_function_canon(expr)
                # This makes the C++ code confused if it isn't fixed,
                # since the indent gets messy.
                if expr.startswith("(("):
                    expr = f"(call {expr[1:-1]})"
                return expr

        expr, *_ = expr.split(";")

        # Canonicalizes ranges.
        if not expr.startswith('"') and 1 <= expr.count(":") <= 3:
            result = expr.split(":")
            # assert not any(set("({[]})") & set(i) for i in result)
            if any(result):
                return self._make_range_canon(expr)
            else:
                raise ValueError(expr)

        numeric = get_numeric_or_none(expr)
        if numeric is not None:
            return numeric

        if expr in BOOLS:
            return BOOLS[expr]

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
    canonizer = Preprocessor()
    print(canonizer._make_canon(args.code))


if __name__ == "__main__":
    main()
