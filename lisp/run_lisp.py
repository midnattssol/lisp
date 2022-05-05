#!/usr/bin/env python3.10
"""Runs the lisp."""
import argparse
import hashlib
import pathlib as p
import subprocess
import sys
import typing as t
import functools as ft

from preprocessor import make_canon
import utils

BASEPATH = p.Path(__file__).parent


def _recompile_if_necessary(args):
    if args.recompile == "never":
        return

    # Regenerate programmatical headers.
    subprocess.run(["python", str(BASEPATH / "generative.py")])

    # Get hashes of files.
    origin = BASEPATH / "lisp.cpp"
    files2hash = [origin, *(i for i in BASEPATH.iterdir() if i.suffix == ".h")]
    digest = ft.reduce(
        lambda a, b: a ^ b, (int(utils.hash_file(f), base=16) for f in files2hash)
    )
    digest = hex(digest).removeprefix("0x")
    files_changed = digest != utils.cat(BASEPATH / ".source.hash")

    # Perform the recompilation.
    if args.recompile == "always" or files_changed:
        if args.debug:
            print("[DEBUG] The executable has changed and needs to be recompiled.")

        subprocess.run(["g++", "-O1", "-o", str(BASEPATH / "lisp"), origin])
        with open(BASEPATH / ".source.hash", "w", encoding="utf-8") as file:
            file.write(digest)


def main(argv: t.List[str]) -> None:
    """Run the lisp.cpp file with the inputs."""
    parser = argparse.ArgumentParser(description="Run the lisp.")
    parser.add_argument(
        "origin",
        metavar="O",
        type=p.Path,
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
        "--recompile",
        choices=["never", "change", "always"],
        default="never",
        help="when to recompile the executable",
    )
    args = parser.parse_args()
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
    subprocess.run([str(BASEPATH / "lisp"), str(temp_path), str(int(args.debug))])


if __name__ == "__main__":
    main(sys.argv)
