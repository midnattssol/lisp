#!/usr/bin/env python3.10
""""""
import dataclasses as dc
import itertools as it

import more_itertools as mit
import regex as re
from preprocess import tokens_of


@dc.dataclass
class Token:
    value: str
    indent: int = 0
    is_function: bool = False
    is_first: bool = False
    is_last: bool = False

    def __iter__(self):
        return iter(self.value)

    def __getitem__(self, k):
        return self.value[k]


allowed_options = {"line_length": int}

BRACKETS = {
    "{": "}",
    "[": "]",
    "(": ")",
    "#[": "]",
}


def deconstruct(code):
    no_comments, *comment = code.split(";", maxsplit=1)
    comment = comment[0] if comment else ""
    no_comments = no_comments.strip()
    for k, v in BRACKETS.items():
        if no_comments.startswith(k) and no_comments.endswith(v):
            output = [
                k,
                *map(
                    deconstruct,
                    tokens_of(no_comments.removeprefix(k).removesuffix(v)),
                ),
                v,
            ]
            if comment:
                output.append(";" + comment)
            return output

    return code


def indent(code, chars):
    return code.replace("\n", "\n" + chars)


def reconstruct(code, line_length=30):
    def inner(code):
        if isinstance(code, str):
            return code

        tokens = [code[0], *map(inner, code[1:-1]), code[-1]]
        tokens = list(mit.collapse(tokens))

        projected_line_length = 0
        for i, token in enumerate(tokens):
            projected_line_length += i > 2

            char = None
            for char in token:
                projected_line_length += 1
                if char == "\n":
                    break

            if char == "\n":
                break

        too_long = projected_line_length > line_length

        joiner = "\n" if too_long else " "
        output = tokens[0] + joiner.join(tokens[1:-1])

        if too_long:
            output = indent(output, "    ")
            output += "\n"

        output += tokens[-1]

        return output

    result = inner(code)
    return result


def regex_fix(code, line_length):
    lines = code.splitlines()

    i = 0
    while i < len(lines):
        line = lines[i]
        deconstructed = deconstruct(line)

        if line.strip().startswith(";") and not line.startswith(";"):
            line = line.lstrip()
            lines[i] = line

        if len(line) > line_length:
            # Attempt to remove comments to shorten the line.
            if deconstructed[-1].startswith(";") and len(deconstructed) > 1:
                comment = deconstructed[-1]
                line = line.removesuffix(comment)
                lines[i] = line
                lines.insert(i, comment)
                i -= 1
                continue

            # Attempt to split long comments into shorter comments to shorten the line.
            if line.startswith(";"):
                post = line.removeprefix(";").strip() + " "
                words = re.findall("\S+\s+", post)

                adj_line_length = line_length - len("; ")

                if len(words) > 1:
                    w = [[]]
                    l = 0

                    iterable = iter(words)

                    try:
                        while l < adj_line_length:
                            w[-1].append(next(iterable))
                            l += len(w[-1][-1])
                        w.append([w[-1].pop(), *iterable])
                    except StopIteration:
                        pass

                    lines.pop(i)

                    comments = map("".join, w)
                    comments = list(comments)
                    for comment in comments[::-1]:
                        lines.insert(i, "; " + comment.rstrip())

                    continue

        i += 1

    code = "\n".join(lines)
    code = reconstruct(deconstruct(code))

    return code


# works without but breaks with comments
a = regex_fix(
    """
; Hello world!
(=> fib (do (if! (leq _ 1) (return 1)) (+ (fib (-- _)) (fib (-- _)))))
""",
    50,
)

print(a)
exit()

print(
    reconstruct(
        deconstruct(
            "(do\n    thing that\n    (do something else that requires a lot of arguments and is very long)\n)"
        )
    )
)


a = deconstruct(
    "(hello world) ; This is a comment that makes this specific line incredibly long\n"
)

print(a)


exit()
# Good:
# (do
#   thing
#   thing
# )

# (do thing thing)

# Bad:
# (do thing
#   thing
# )


class Linter:
    def lint_code(**options):
        for option, validator in allowed_options.items():
            if isinstance(validator, type):
                assert isinstance(
                    option, validator
                ), f"{option!r} is not of class {validator.__qualname__!r}"
            else:
                assert validator(option), f"{option!r} does not satisfy {validator}"


Linter.lint_code(line_length=10)
Linter.lint_code(line_length="hi")
