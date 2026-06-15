#!/usr/bin/env python3
# Linux replacement for bin2hex.c — byte-for-byte identical output format.
# Usage: bin2hex.py <input.spv> [+]<output.c> <array_name>
#   '+' prefix on the output path = append mode (matches bin2hex.c).
import sys

def main():
    if len(sys.argv) != 4:
        print("Usage: %s <input_file> [+]<output_file> <output_array_name>" % sys.argv[0])
        return -1
    in_path, out_arg, name = sys.argv[1], sys.argv[2], sys.argv[3]
    data = open(in_path, "rb").read()
    n = len(data)
    mode = "ab" if out_arg.startswith("+") else "wb"
    out_path = out_arg[1:] if out_arg.startswith("+") else out_arg
    line_length = 16
    buf = bytearray()
    buf += ("const unsigned char %s[%d] = {\n\t" % (name, n)).encode()
    for i, b in enumerate(data):
        buf += ("0x%.2X" % b).encode()
        if i != n - 1:
            if (i + 1) % line_length:
                buf += b", "
            else:
                buf += b",\n\t"
    buf += b"\n};\n"
    with open(out_path, mode) as f:
        f.write(buf)
    return 0

if __name__ == "__main__":
    sys.exit(main())
