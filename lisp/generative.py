#!/usr/bin/env python3.10
"""Generate some code."""
import cson
import more_itertools as mit


def get_code():
    """Get the code."""
    with open("builtins.cson", "r", encoding="utf-8") as file:
        content = file.read()

    signatures = cson.loads(content)

    code = (
        "/* This code was automatically generated. */\n"
        "#include <set>\n"
        "#include <map>\n"
        '#include "./lispvar.h"\n'
        "std::map<std::string, LispVar *> BUILTINS_TYPES = {};\n"
        "bool BUILTINS_TYPES_READY = false;\n"
        "void _fill_out_lisp_builtin_types() {\n"
    )
    code += (
        "*_SINGLETON_NOTHING = {NOTHING, 0};\n"
        "*_SINGLETON_NOT_SET = {__NOT_SET__, 0};\n"
        "*_SINGLETON_NOARGS_TOKEN = {__NO_ARGS__, 0};\n"
    )

    for i, (builtins, signature) in enumerate(signatures):
        varname = f"type_{i}"
        code += f"auto {varname} = new LispVar;\n"
        code += f'*{varname} = parse_and_evaluate("{signature}");\n'

        for builtin in builtins:
            code += f'BUILTINS_TYPES["{builtin}"] = {varname};\n'

    code += "BUILTINS_TYPES_READY = true;\n}\n"

    code += (
        "const std::set<std::string> LISP_BUILTINS = {"
        + ",\n".join(
            sorted(
                mit.collapse((f'"{i}"' for i in b) for b, s in signatures),
                key=lambda x: (len(x), x),
            )
        )
        + "};\n"
    )
    return code


def main():
    """Write the code."""
    with open("gen.h", "w", encoding="utf-8") as file:
        file.write(get_code())


if __name__ == "__main__":
    main()
