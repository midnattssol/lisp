#!/usr/bin/env python3.10
"""Extracts some information from the C++ code."""
import regex as re

with open("lisp/lisp.cpp", "r", encoding="utf-8") as file:
    CODE = file.read()


def get_builtins() -> str:
    """Get all builtins from the code."""
    results = re.findall(r'op.compare\("([^()]*)"\)', CODE)
    print("|".join(map(re.escape, sorted(results, key=lambda x: (len(x), x)))))
    strings = map(lambda x: f'"{x}"', sorted(results, key=lambda x: (len(x), x)))

    lines = [""]
    for string in strings:
        if len(lines[-1] + string) > 70:
            lines.append("")
        lines[-1] += string + ", "

    return "\n".join(lines).removesuffix(", ")


def get_types() -> str:
    """Get all LispTypes from the code."""
    result = re.findall(r"enum\s+LispType\s+{([^}]*)}", CODE)[0]
    result = re.findall(r"\w+", result)
    result = map(str.lower, result)
    result = dict(map(lambda x: x[::-1], enumerate(result)))
    result = {
        k: v for k, v in result.items() if not (k.startswith("__") & k.endswith("__"))
    }
    result = ",\n".join("    {" + f'{v}, "{k}"' + "}" for k, v in result.items())
    result = "{\n" + result + "\n}"
    result = f"const std::map<unsigned int, std::string> TYPENAME_FROM_INT {result};"
    return result


if __name__ == "__main__":
    # t = get_types()
    # print(t)
    t = get_builtins()
    # print(t)
