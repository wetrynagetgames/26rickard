#!/usr/bin/env python3
"""
    Generates a clang module map for a given directory
"""

import argparse
import pathlib
import yaml
import sys


def write_file_if_not_same(file_path, content):
    try:
        with open(file_path, 'r') as f:
            if f.read() == content:
                return
    except FileNotFoundError:
        pass

    with open(file_path, 'w') as f:
        f.write(content)


def main():
    parser = argparse.ArgumentParser(
                 epilog=__doc__,
                 formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('directory', help='source directory to generate module map for')
    parser.add_argument('generated_files', nargs='+', help='extra files to include in the module map')
    parser.add_argument('-m', '--module-map', required=True, help='output module map file')
    parser.add_argument('-v', '--vfs-map', required=True, help='output VFS map file')
    args = parser.parse_args()

    root = pathlib.Path(args.directory)
    if not root.is_dir():
        print(f"Error: {args.directory} is not a directory", file=sys.stderr)
        return 1
    pathlib.Path(args.module_map).parent.mkdir(parents=True, exist_ok=True)

    header_files = [f for f in root.rglob('**/*.h') if f.is_file()]
    module_name = root.name

    module_map = f"module {module_name} {{\n"
    for header_file in header_files:
        module_map += f"    header \"{header_file.relative_to(root)}\"\n"
    for generated_file in args.generated_files:
        module_map += f"    header \"{generated_file}\"\n"
    module_map += "    requires cplusplus\n"
    module_map += "    export *\n"
    module_map += "}\n"

    vfs_map = {
        "version": 0,
        "use-external-names": False,
        "roots": [
            {
                "name": f"{root}/module.modulemap",
                "type": "file",
                "external-contents": f"{args.module_map}"
            }
        ]
    }

    write_file_if_not_same(args.module_map, module_map)
    write_file_if_not_same(args.vfs_map, yaml.dump(vfs_map))


if __name__ == '__main__':
    sys.exit(main())
