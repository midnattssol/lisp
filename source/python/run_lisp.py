#!/usr/bin/env python3.10
"""Runs the lisp."""
import argparse
import functools as ft
import hashlib
import logging
import pathlib as p
import subprocess
import sys
import typing as t

import gen_code
import preprocess
import utils

BASEPATH = p.Path(__file__).parent
GCPP_FLAGS = ["-O1", "-fconcepts-ts"]


def _run(*args, **kwargs):
    result = subprocess.run(*args, **kwargs)
    if result.returncode:
        exit(result.returncode)


def _recompile_if_necessary(args):
    if args.recompile == "never":
        logging.debug(f"Skipping recompilation (--recompile={args.recompile!r}).")
        return

    logging.debug(f"Recompiling code (--recompile={args.recompile!r}).")

    # Regenerate programmatical headers.
    gen_code.render_all()

    # Get hashes of files.
    origin = BASEPATH.parent / "cpp" / "lisp.cpp"
    files_changed = False

    if args.recompile != "always":
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
        logging.debug(f"Hashed files in directory (digest: {digest}).")

    # Perform the recompilation.
    if args.recompile == "always" or files_changed:
        logging.debug("Recompiling the executable with g++.")
        _run(["g++", *GCPP_FLAGS, "-o", str(BASEPATH.parent / "lisp"), origin])

    # Write the hash again.
    if files_changed:
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
        "--log",
        default="NEVER",
        choices=["DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL", "NEVER"],
        help="logging level",
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
    parser.add_argument(
        "-a",
        "--args",
        nargs="*",
        help="arguments to pass to the Lisp program",
    )

    args = parser.parse_args()
    processor = preprocess.Preprocessor()

    if args.log != "NEVER":
        logging.basicConfig(level=getattr(logging, args.log.upper()))

    if args.origin is None:
        if args.code is None:
            logging.error(
                "Either an origin file or code passed with -c must be provided."
            )
            exit(1)

        file_contents = args.code

    else:
        if not args.origin.exists():
            logging.error(f"File '{args.origin}' does not exist.")
            exit(1)

        with open(args.origin, "r", encoding="utf-8") as file:
            file_contents = file.read()

        processor.contexts[-1] = args.origin

    canon = processor.make_canon(file_contents)

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

    executable_path = BASEPATH.parent / "lisp"

    if not executable_path.exists():
        logging.error(
            f"Could not find executable (searched {str(executable_path.resolve())!r})."
        )
        if args.recompile != "always":
            logging.error("Try rerunning with the `--recompile always` option.")
        exit(1)

    logging.info("Running executable.")

    try:
        _run(
            [
                str(executable_path),
                str(temp_path),
                str(0),
                str(int(not args.unsafe)),
                *(args.args if args.args is not None else []),
            ]
        )
    except KeyboardInterrupt:
        pass


if __name__ == "__main__":
    main(sys.argv)
