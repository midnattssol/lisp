"""Render Mako templates automagically."""
import logging
import pathlib as p
import typing as t

import cson
import regex as re
import utils
from mako.template import Template


def _load_data(filename: p.Path) -> t.Any:
    return cson.loads(utils.cat(BASEPATH.parent / "data" / filename))


BASEPATH = p.Path(__file__).parent.resolve()

SHORTHANDS = _load_data("shorthands.cson")
PREFIX_EQUALS = _load_data("prefix_equals.cson")
SIGNATURES = _load_data("builtins.cson")
TYPES = _load_data("types.cson")

SIGNATURES = dict(sorted(SIGNATURES.items(), key=lambda x: (len(x[0]), x[0])))

UNIQUE_SIGNATURES = {i[0] for i in SIGNATURES.values()}
UNIQUE_SIGNATURES = sorted(UNIQUE_SIGNATURES, key=len)


def _escape(item: str) -> str:
    item = item.replace("\\", "\\\\")
    item = item.replace('"', '\\"')
    return item


def _regex_joined(items: t.Iterable[str], joiner: str) -> str:
    result = joiner.join(map(re.escape, items))
    return _escape(result)


def _parse_signature(item: str) -> str:
    finder = r"\(map type \[([^]]*)\]\)"
    args = re.findall(finder, item)
    args = [list(filter(None, [a.strip() for a in i.split('"')])) for i in args]

    out = ""
    for i in args:
        out += "["
        for a in i:
            out += a
            out += " "
        out = out.rstrip()
        out += "] "

    return out.rstrip()


def _builtin2enum(item: str) -> str:
    return f"B_{item.upper()}"


def _render_file(filename: p.Path) -> p.Path:
    template = Template(utils.cat(filename))
    result = template.render(
        filename=filename,
        SIGNATURES=SIGNATURES,
        UNIQUE_SIGNATURES=UNIQUE_SIGNATURES,
        SHORTHANDS=SHORTHANDS,
        PREFIX_EQUALS=PREFIX_EQUALS,
        TYPES=TYPES,
        _escape=_escape,
        _builtin2enum=_builtin2enum,
        _parse_signature=_parse_signature,
        _regex_joined=_regex_joined,
    )

    return result


def _update_gitignore(targets: t.List[p.Path]) -> None:
    gitignore_path = BASEPATH / ".." / ".." / ".gitignore"
    if not gitignore_path.exists():
        return

    content = utils.cat(gitignore_path)
    auto_identifier = "# Items below this have been automatically generated.\n"
    index = content.find(auto_identifier) + len(auto_identifier)
    content = content[:index]

    base = str((BASEPATH / ".." / "..").resolve())

    for target in targets:
        content += str(target.resolve()).removeprefix(base + "/") + "\n"

    with gitignore_path.open("w", encoding="utf-8") as file:
        file.write(content)


def _walk_recursive(directory: p.Path):
    for file in directory.iterdir():
        if file.is_dir():
            yield from _walk_recursive(file)
        else:
            yield file


def render_all() -> None:
    """Render all Mako files."""
    to_ignore = []
    filecount = 0
    basedir = BASEPATH / ".." / ".."
    line_count = 0
    logging.debug(
        f"Rendering all Mako files in the directory '{basedir.absolute()!r}'."
    )

    for file in _walk_recursive(basedir):
        if file.name.endswith(".mako"):
            logging.debug(f"Rendering {file.name!r}.")

            code = _render_file(file)
            line_count += code.count("\n") + 1
            target = file.parent / file.name.removesuffix(".mako")

            with target.open("w", encoding="utf-8") as file:
                file.write(code)

            if target.suffix != ".md":
                to_ignore.append(target)
            filecount += 1

    logging.debug(f"Generated {line_count} lines of code across {filecount} files.")
    _update_gitignore(to_ignore)


if __name__ == "__main__":
    render_all()
