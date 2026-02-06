#!/usr/bin/env python3
import argparse
import os
import sys

def main():
    parser = argparse.ArgumentParser(description="Convert binary file to C header")
    parser.add_argument("input", help="Input binary file")
    parser.add_argument("-o", "--output", help="Output header file (default: stdout)")
    parser.add_argument("-n", "--name", help="Array variable name (default: derived from filename)")
    parser.add_argument("-W", "--width", type=int, default=0, help="Define a WIDTH macro")
    parser.add_argument("-H", "--height", type=int, default=0, help="Define a HEIGHT macro")
    parser.add_argument("-c", "--cols", type=int, default=11, help="Hex values per line (default: 11)")
    args = parser.parse_args()

    with open(args.input, "rb") as f:
        data = f.read()

    if not data:
        print("Error: input file is empty", file=sys.stderr)
        sys.exit(1)

    name = args.name or (
        os.path.splitext(os.path.basename(args.input))[0]
        .upper()
        .replace("-", "_")
        .replace(" ", "_")
    )

    lines = []
    lines.append("#pragma once")
    if args.height:
        lines.append(f"#define {name}_HEIGHT {args.height}")
    if args.width:
        lines.append(f"#define {name}_WIDTH {args.width}")
    lines.append("")
    lines.append(f"// array size is {len(data)}")
    lines.append(f"static const byte {name}[] PROGMEM  = {{")

    for i in range(0, len(data), args.cols):
        chunk = data[i : i + args.cols]
        hex_vals = ", ".join(f"0x{b:02x}" for b in chunk)
        if i + args.cols < len(data):
            hex_vals += ", "
        lines.append("  " + hex_vals)

    lines.append("};")
    lines.append("")

    output = "\n".join(lines)

    if args.output:
        with open(args.output, "w") as f:
            f.write(output)
    else:
        print(output)


if __name__ == "__main__":
    main()
