#!/usr/bin/env python3
"""RLE (Run Length Encoding) Format:
  0x01..0x7F  = run of N zero-bytes (black)
  0x81..0xFF  = run of (N-0x80) 0xFF-bytes (white)
  0x00 <byte> = one literal byte
  0x80        = eof(end-of-frame) marker
"""

import argparse
import struct
import sys

FRAME_SIZE = 4050  # 240 * 135 / 8

def rle_encode_frame(frame: bytes) -> bytearray:
    out = bytearray()
    i = 0
    while i < len(frame):
        b = frame[i]
        if b == 0x00 or b == 0xFF:
            count = 1
            while i + count < len(frame) and frame[i + count] == b and count < 127:
                count += 1
            if b == 0x00:
                out.append(count)
            else:
                out.append(0x80 | count)
            i += count
        else:
            out.append(0x00)
            out.append(b)
            i += 1
    out.append(0x80) # eof
    return out


def encode_single(raw, output):
    encoded = rle_encode_frame(raw)
    ratio = len(encoded) / len(raw) * 100
    print(f"Raw: {len(raw):,} bytes")
    print(f"Compressed: {len(encoded):,} bytes ({ratio:.1f}%)")
    with open(output, "wb") as f:
        f.write(encoded)
    print(f"Written to {output}")


def encode_video(raw, output):
    num_frames = len(raw) // FRAME_SIZE
    if len(raw) % FRAME_SIZE != 0:
        print(f"Warning: file size {len(raw)} not divisible by {FRAME_SIZE}", file=sys.stderr)

    print(f"Encoding {num_frames} frames...")

    encoded_frames = []
    for i in range(num_frames):
        frame = raw[i * FRAME_SIZE : (i + 1) * FRAME_SIZE]
        encoded_frames.append(rle_encode_frame(frame))

    offset = 0
    offsets = []
    for ef in encoded_frames:
        offsets.append(offset)
        offset += len(ef)

    total_data = sum(len(ef) for ef in encoded_frames)
    total_size = 4 + num_frames * 4 + total_data
    ratio = total_size / len(raw) * 100

    print(f"Raw: {len(raw):,} bytes")
    print(f"Compressed: {total_size:,} bytes ({ratio:.1f}%)")
    print(f"Ratio: {len(raw) / total_size:.1f}x")

    with open(output, "wb") as f:
        f.write(struct.pack("<I", num_frames))
        for o in offsets:
            f.write(struct.pack("<I", o))
        for ef in encoded_frames:
            f.write(ef)

    print(f"Written to {output}")


def main():
    parser = argparse.ArgumentParser(description="RLE-encoder")
    parser.add_argument("input", help="Raw binary file")
    parser.add_argument("-o", "--output", required=True, help="Output .rle file")
    parser.add_argument("--single", action="store_true",
                        help="Encode as single image (no frame index)")
    args = parser.parse_args()

    with open(args.input, "rb") as f:
        raw = f.read()
        
    if args.single:
        encode_single(raw, args.output)
    else:
        encode_video(raw, args.output)


if __name__ == "__main__":
    main()
