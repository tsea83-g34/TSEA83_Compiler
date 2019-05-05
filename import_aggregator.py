#!/usr/bin/env python3
import sys
import re

INCLUDE_KEYWORD = "#include"

def get_local_path(path):
    s = "/".join(path.split("/")[:-1])
    return s + "/" if s != "" else ""

def get_imports(lines):
    imports = {}
    for i, line in enumerate(lines):
        if line.startswith(INCLUDE_KEYWORD):
            imports.update({i : line[8:].strip()})

    return imports

def aggregate_imports(text, relative_path = ""):
    lines = text.split("\n")
    imports = get_imports(lines)

    # print()
    # print("Rel path: ", relative_path)
    # print("Imports:  ", imports)

    if not imports:
        return text

    inserts = {}

    for i, path in imports.items():
        output = ""

        with open(relative_path + path) as f:
            text = f.read()
            output = aggregate_imports(text, relative_path + get_local_path(path))

        inserts.update({i : output})
    
    for i, insert in inserts.items():
        lines[i] = insert

    return "\n".join(lines)

if __name__ == "__main__":
    output = ""

    with open(sys.argv[1]) as f:
        text = f.read()
        output = aggregate_imports(text)

    with open("agg.cm", "w+") as f:
        f.write(output + "\n")


