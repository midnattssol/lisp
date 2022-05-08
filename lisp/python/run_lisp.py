#!/usr/bin/env python3.10
"""Runs the lisp."""
import argparse
import functools as ft
import hashlib
import pathlib as p
import subprocess
import sys
import typing as t

import utils
from preprocess import make_canon

BASEPATH = p.Path(__file__).parent
GCPP_FLAGS = ["-O1", "-fconcepts-ts"]


def _run(*args, **kwargs):
    result = subprocess.run(*args, **kwargs)
    if result.returncode:
        exit(result.returncode)


def _recompile_if_necessary(args):
    if args.recompile == "never":
        return

    # Regenerate programmatical headers.
    _run(["python", str(BASEPATH / "generate_code.py")])

    # Get hashes of files.
    origin = BASEPATH.parent / "cpp" / "lisp.cpp"
    files2hash = [
        origin,
        *(i for i in (BASEPATH.parent / "cpp").iterdir() if i.suffix == ".h"),
    ]
    digest = ft.reduce(
        lambda a, b: a ^ b, (int(utils.hash_file(f), base=16) for f in files2hash)
    )
    digest = hex(digest).removeprefix("0x")
    hash_path = BASEPATH.parent / "cpp" / ".source.hash"
    files_changed = digest != utils.cat(hash_path)

    # Perform the recompilation.
    if args.recompile == "always" or files_changed:
        if args.debug:
            print("[DEBUG] The executable has changed and needs to be recompiled.")

        _run(["g++", *GCPP_FLAGS, "-o", str(BASEPATH.parent / "lisp"), origin])

        with open(hash_path, "w", encoding="utf-8") as file:
            file.write(digest)


def main(argv: t.List[str]) -> None:
    """Run the lisp.cpp file with the inputs."""
    parser = argparse.ArgumentParser(description="Run the lisp.")
    parser.add_argument(
        "origin",
        metavar="O",
        type=p.Path,
        nargs="?",
        help="the lisp file to run",
    )
    parser.add_argument(
        "--debug",
        action="store_const",
        const=True,
        default=False,
        help="activate debug mode",
    )
    parser.add_argument(
        "--dump",
        action="store_const",
        const=True,
        default=False,
        help="dump canonized output",
    )
    parser.add_argument(
        "--unsafe",
        action="store_const",
        const=True,
        default=False,
        help="run unsafely",
    )
    parser.add_argument(
        "--recompile",
        choices=["never", "change", "always"],
        default="never",
        help="when to recompile the executable",
    )
    parser.add_argument(
        "-c",
        "--code",
        help="code to execute instead of running from a file",
    )

    args = parser.parse_args()

    if args.origin is None:
        if args.code is None:
            print("Error: Either the origin or --code options must be in use.")
            exit(1)
        file_contents = args.code

    else:
        with open(args.origin, "r", encoding="utf-8") as file:
            file_contents = file.read()

    file_contents = f"(noop {file_contents})"
    canon = make_canon(file_contents)

    if args.dump:
        print(canon)
        exit(0)

    # Makes the code canon.
    temp_path = p.Path(
        "/tmp/lisp/" + hashlib.md5(canon.encode("utf-8")).hexdigest() + ".lisp"
    )
    if not temp_path.parent.exists():
        temp_path.parent.mkdir()

    with open(temp_path, "w", encoding="utf-8") as file:
        file.write(canon)

    _recompile_if_necessary(args)
    _run(
        [
            str(BASEPATH.parent / "lisp"),
            str(temp_path),
            str(int(args.debug)),
            str(int(args.unsafe)),
        ]
    )


if __name__ == "__main__":
    main(sys.argv)
