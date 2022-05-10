#!/usr/bin/env python3.10
"""Show the include tree of a file."""
import argparse
import pathlib as p

import regex as re
import tree
import utils

RE_INCLUDE = re.compile(r'#\s*include\s*"([^"]*)"')


def find_recursive(deptree, index=0):
    """Insert nodes recursively into the dependency tree."""
    contents = utils.cat(deptree.nodes[index])

    for i in re.findall(RE_INCLUDE, contents):
        path = deptree.nodes[index].parent / i
        deptree.insert(index, path)

    for child in list(deptree.direct_child_indices_of(index))[::-1]:
        find_recursive(deptree, child)


def main() -> None:
    """Show the include tree of a file."""
    parser = argparse.ArgumentParser(description="Show the include tree of a C++ file.")
    parser.add_argument(
        "filename",
        metavar="F",
        type=p.Path,
    )

    args = parser.parse_args()
    dependency_tree = tree.Tree([args.filename], [0])
    find_recursive(dependency_tree)
    dependency_tree.nodes = [
        i.relative_to(args.filename.parent) for i in dependency_tree.nodes
    ]
    dependency_tree.nodes = list(map(str, dependency_tree.nodes))
    print(dependency_tree)


if __name__ == "__main__":
    main()
