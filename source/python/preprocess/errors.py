#!/usr/bin/env python3.10
"""Provides pretty error messages."""
import dataclasses as dc
import typing as t

import regex as re
from colorama import init
from termcolor import colored

# init()

ANSI_REGEX = re.compile(r"\x1b\[\d+m")


def _range_avg(x):
    return (x.start + x.stop) // 2


def _get_underline(underlined, n):
    out = ""
    middle = _range_avg(underlined)

    for i in range(len(n)):
        if i + 1 == middle:
            out += "┬"
            continue
        if i + 1 in underlined:
            out += "─"
            continue
        out += " "
    return out


@dc.dataclass
class SourceError(BaseException):
    """A source error."""

    code: str
    begin: int
    stop: int
    error_code: int
    desc: str
    under_msg: t.Optional[str] = None
    helpmsg: t.Optional[str] = None
    ansi: bool = True

    def __str__(self):
        """Display the error."""
        # Count the newlines up to the error.
        code_up_to_error = self.code[: self.begin]
        err_line_no = sum(i == "\n" for i in code_up_to_error)

        no_ctx = 2

        # Find the lines that should be in the output.
        split_code = self.code.splitlines(keepends=False)

        start = max(0, err_line_no - no_ctx)
        end = err_line_no + no_ctx
        error_and_context = split_code[start : end + 1]

        # Add lines to the output.
        code2errorline = "\n".join(split_code[:err_line_no])
        len2errorline = len(code2errorline)

        underlined = range(self.begin - len2errorline, self.stop - len2errorline)

        # Right-adjust the numbers so they don't look weird around
        # powers of 10 where the number of digits are changing.
        output = colored(f"🗙 error[{self.error_code}]: {self.desc}\n", "red")
        right_adjust = len(repr(end + 1))

        for i, item in enumerate(error_and_context, start):
            output += f"{repr(i + 1).rjust(right_adjust)} │ {item}\n"

            # Add underlining.
            if i == err_line_no:
                dots = right_adjust * " "
                left = f"{dots} · "
                underline = _get_underline(underlined, item)
                output += left + colored(underline + "\n", "green")

                if self.under_msg is not None:
                    underline = (
                        " " * (_range_avg(underlined) - 1) + f"╰──── {self.under_msg}"
                    )
                    output += left + colored(underline + "\n", "green")
                    output += left + "\n"

        output = output.removesuffix("\n")

        if self.helpmsg is not None:
            output += colored("\nhelp:", "magenta") + f" {self.helpmsg}"

        if not self.ansi:
            output = re.sub(ANSI_REGEX, "", output)

        return output


if __name__ == "__main__":
    err = "Lorem ipsum dolor sit amet, \nconsectetur adipisicing elit, sed do eiusmod\ntempor incididunt ut labore et dolore magna aliqua.\nUt enim ad minim veniam, quis nostrud exercitation ullamco laboris\nnisi ut aliquip ex ea commodo consequat. \nDuis aute irure dolor in reprehenderit in voluptate velit\nesse cillum dolore eu fugiatnulla pariatur. \nExcepteur sint occaecat cupidatat non proident, sunt in culpa \nqui officia deserunt mollit anim id est laborum."

    e = SourceError(
        err,
        err.index("Duis aute"),
        err.index("Duis aute") + len("Duis aute"),
        "E33",
        "Incorrect type",
        under_msg="Expected type 'T' but recieved 'String'",
        ansi=True,
    )
    out = repr(e)
    out.replace("(", colored("(", "grey"))
    out.replace(")", colored(")", "grey"))
    print(e)
