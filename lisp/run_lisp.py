#!/usr/bin/env python3.10
"""Runs the lisp."""
import argparse
import hashlib
import pathlib as p
import subprocess
import sys
import typing as t

from preprocessor import make_canon
from utils import has_changed

BASEPATH = p.Path(__file__).parent


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
        "--force",
        action="store_const",
        const=True,
        default=False,
        help="force recompilation",
    )
    args = parser.parse_args()
    with open(args.origin, "r", encoding="utf-8") as file:
        file_contents = file.read()

    file_contents = f"(noop {file_contents})"
    canon = make_canon(file_contents)

    if args.dump:
        print(canon)
        exit(0)

    temp_path = p.Path(
        "/tmp/lisp/" + hashlib.md5(canon.encode("utf-8")).hexdigest() + ".lisp"
    )
    if not temp_path.parent.exists():
        temp_path.parent.mkdir()

    with open(temp_path, "w", encoding="utf-8") as file:
        file.write(canon)

    origin = BASEPATH / "lisp.cpp"
    digest = has_changed(origin, BASEPATH / ".source.hash")
    if digest or args.force:
        if args.debug:
            print("[DEBUG] The executable has changed and needs to be recompiled.")

        subprocess.run(["g++", "-O1", "-o", str(BASEPATH / "lisp"), origin])

        if digest:
            with open(BASEPATH / ".source.hash", "w", encoding="utf-8") as file:
                file.write(digest)

    subprocess.run([str(BASEPATH / "lisp"), str(temp_path), str(int(args.debug))])


if __name__ == "__main__":
    main(sys.argv)
