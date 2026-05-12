#!/usr/bin/env python3
"""Re-align every LAYOUT_elora_hlc layer block in keymap.c so that, across all
layers, every arg at a given grid column starts at the same screen column,
and every thumb-row key sits in the same column as its physical neighbor on
the main rows. See agents.md and the layer ASCII art for the geometry.

Pure whitespace pass: no token is changed.

Usage:
  python3 format_keymap.py                  # rewrite keymap.c in place
  python3 format_keymap.py --check          # exit 1 if file is not canonical
  python3 format_keymap.py --file PATH ...  # operate on a custom path
"""
import argparse
import re
import sys
from collections import defaultdict
from pathlib import Path

DEFAULT_PATH = Path(__file__).resolve().parent.parent / "keymap.c"

ROW_SIZES = [12, 12, 12, 16, 10, 10]
INDENT = "      "                 # 6 spaces
INNER_THUMB_SPLIT = "   "         # 3 spaces between halves of inner-thumb cluster


def col_key(row_idx, arg_idx):
    """Map a (row, arg) pair to the shared-width column group it belongs to."""
    if row_idx in (0, 1, 2):
        return ("main", arg_idx)
    if row_idx == 3:                                    # 16-arg row
        if arg_idx <= 5:
            return ("main", arg_idx)                    # left main 0..5
        if arg_idx <= 9:
            return ("inner_thumb", arg_idx - 6)         # mid-row thumb cluster
        return ("main", arg_idx - 4)                    # right main 6..11
    if row_idx == 4:                                    # thumb row — physical alignment
        return {0: ("main", 3), 1: ("main", 4), 2: ("main", 5),
                3: ("inner_thumb", 0), 4: ("inner_thumb", 1),
                5: ("inner_thumb", 2), 6: ("inner_thumb", 3),
                7: ("main", 6), 8: ("main", 7), 9: ("main", 8)}[arg_idx]
    if row_idx == 5:
        return ("enc", arg_idx)
    raise ValueError(f"unknown row {row_idx}")


def tokenize(line):
    s = line.strip()
    if s.endswith(","):
        s = s[:-1]
    tokens, cur, depth = [], "", 0
    for ch in s:
        if ch == "," and depth == 0:
            tokens.append(cur.strip())
            cur = ""
            continue
        if ch == "(":
            depth += 1
        elif ch == ")":
            depth -= 1
        cur += ch
    if cur.strip():
        tokens.append(cur.strip())
    return tokens


def render_line(row_idx, args, widths, gap_str, indent, is_last_line, gap_after):
    out = indent
    n = len(args)
    for j, tok in enumerate(args):
        is_last_arg = (j == n - 1)
        token = tok if (is_last_line and is_last_arg) else tok + ","
        if is_last_arg:
            out += token
        else:
            out += token.ljust(widths[col_key(row_idx, j)])
            if gap_after is not None and j == gap_after:
                out += gap_str
    return out.rstrip()


def format_text(text, src_for_errors="<input>"):
    """Return (new_text, n_blocks). Raises ValueError on a malformed block."""
    pat = re.compile(
        r"(    \[_(?P<name>\w+)\] = LAYOUT_elora_hlc\(\n)(?P<body>.*?)(\n    \),)",
        re.DOTALL,
    )
    matches = list(pat.finditer(text))
    if not matches:
        raise ValueError(f"{src_for_errors}: no LAYOUT_elora_hlc blocks found")

    parsed = {}
    for m in matches:
        name = m.group("name")
        body = m.group("body")
        arg_lines = [ln for ln in body.split("\n") if ln.strip()]
        if len(arg_lines) != 6:
            raise ValueError(
                f"{src_for_errors}: layer {name} has {len(arg_lines)} arg lines, expected 6"
            )
        rows = []
        for i, ln in enumerate(arg_lines):
            toks = tokenize(ln)
            if len(toks) != ROW_SIZES[i]:
                raise ValueError(
                    f"{src_for_errors}: layer {name} row {i}: "
                    f"got {len(toks)} args, expected {ROW_SIZES[i]}\n  line: {ln!r}"
                )
            rows.append(toks)
        parsed[name] = rows

    maxlen = defaultdict(int)
    for rows in parsed.values():
        for row_idx, args in enumerate(rows):
            for arg_idx, tok in enumerate(args):
                k = col_key(row_idx, arg_idx)
                if len(tok) > maxlen[k]:
                    maxlen[k] = len(tok)
    widths = {k: v + 2 for k, v in maxlen.items()}

    inner_cluster_width = (
        widths[("inner_thumb", 0)] + widths[("inner_thumb", 1)]
        + len(INNER_THUMB_SPLIT)
        + widths[("inner_thumb", 2)] + widths[("inner_thumb", 3)]
    )
    main_gap = " " * inner_cluster_width

    # Dynamic: thumb arg 0 (MO(_META)) starts where main col 3 starts.
    thumb_indent_col = (
        len(INDENT) + widths[("main", 0)] + widths[("main", 1)] + widths[("main", 2)]
    )
    thumb_indent = " " * thumb_indent_col

    thumb_arg_8_col = thumb_indent_col
    for i in range(8):
        thumb_arg_8_col += widths[col_key(4, i)]
        if i == 4:
            thumb_arg_8_col += len(INNER_THUMB_SPLIT)

    enc_pos_after_arg4 = len(INDENT)
    for i in range(5):
        enc_pos_after_arg4 += widths[("enc", i)]
    encoder_gap_len = max(1, thumb_arg_8_col - enc_pos_after_arg4)
    encoder_gap = " " * encoder_gap_len

    new_text = text
    for m in reversed(matches):
        name = m.group("name")
        prefix = m.group(1)
        suffix = m.group(4)
        rows = parsed[name]
        rendered = []
        for i, args in enumerate(rows):
            if i in (0, 1, 2):
                g, ga, ind = main_gap, 5, INDENT
            elif i == 3:
                g, ga, ind = INNER_THUMB_SPLIT, 7, INDENT
            elif i == 4:
                g, ga, ind = INNER_THUMB_SPLIT, 4, thumb_indent
            else:
                g, ga, ind = encoder_gap, 4, INDENT
            rendered.append(
                render_line(i, args, widths, g, ind, is_last_line=(i == 5), gap_after=ga)
            )
        rendered.insert(5, "")              # blank line before encoder row
        new_body = "\n".join(rendered)
        replacement = prefix + new_body + suffix
        new_text = new_text[: m.start()] + replacement + new_text[m.end():]

    return new_text, len(matches)


def main(argv):
    ap = argparse.ArgumentParser(description=__doc__.split("\n", 1)[0])
    ap.add_argument("--check", action="store_true",
                    help="exit 1 if the file is not in canonical form (no write)")
    ap.add_argument("--file", type=Path, default=DEFAULT_PATH,
                    help=f"path to keymap.c (default: {DEFAULT_PATH})")
    args = ap.parse_args(argv)

    if not args.file.exists():
        print(f"ERROR: {args.file} not found", file=sys.stderr)
        return 2

    text = args.file.read_text()
    try:
        new_text, n_blocks = format_text(text, src_for_errors=str(args.file))
    except ValueError as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 2

    if args.check:
        if text != new_text:
            print(f"✗ {args.file} is not in canonical column-aligned form.")
            print(f"  Run: python3 {Path(__file__).resolve()}")
            return 1
        print(f"✓ {args.file} is canonical ({n_blocks} layer blocks).")
        return 0

    if text == new_text:
        print(f"✓ {args.file} already canonical ({n_blocks} layer blocks).")
        return 0
    args.file.write_text(new_text)
    print(f"✓ Rewrote {args.file} ({n_blocks} layer blocks).")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
