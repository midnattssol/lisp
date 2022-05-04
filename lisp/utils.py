#!/usr/bin/env python3.10
"""Utilities."""
import hashlib
import json
import pathlib as p
import typing as t

import regex as re


def has_changed(path: p.Path, hash_path: p.Path) -> t.Union[bool, str]:
    """Return False if the hash is the same or the new hash otherwise."""
    digest = hashlib.md5(cat(path).encode("utf-8")).hexdigest()
    if digest == cat(hash_path):
        return False
    return digest


def cat(path: p.Path):
    """Cat a file."""
    with open(path, "r", encoding="utf-8") as file:
        return file.read()


def json_from_path(path: p.Path):
    """Load JSON from a path."""
    with open(path, "r", encoding="utf-8") as file:
        return json.load(file)


def fullmatch(pattern, string, *args, **kwargs):
    """Return a bool corresponding to whether or not a pattern fully matches."""
    item = re.match(pattern, string, *args, **kwargs)
    return item and len(item.group()) == len(string)


def composed_with(outer):
    """Decorate a function to compose it with another one."""

    def inner_0(function):
        def inner_1(*args, **kwargs):
            return outer(function(*args, **kwargs))

        return inner_1

    return inner_0


def bool2sign(b):
    """Get a sign from a bool-like."""
    return 1 if b else -1
