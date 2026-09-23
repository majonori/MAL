#!/usr/bin/env python3
"""Regenerate the ten test cases of T793310 and data.zip.

Each test file starts with T (number of queries) and then T queries. The answers
come from the reference solution itself (``main.cpp`` linked with
``interactive_lib.cpp``), so the data always matches what a contestant using MAL
produces. The script also prints how long each file takes, which is how the
slowest point is kept in the 300-600 ms range.

Usage: python3 examples/T793310/generate_data.py   (CXX=g++ by default)
"""

import os
import pathlib
import subprocess
import sys
import time
import zipfile

HERE = pathlib.Path(__file__).resolve().parent
CXX = os.environ.get("CXX", "g++")


def digits(length, seed):
    """A deterministic decimal string of the given length (no leading zero)."""
    out = []
    value = seed
    for _ in range(length):
        value = (value * 1103515245 + 12345) % 2147483648
        out.append(str((value >> 8) % 10))
    if out[0] == "0":
        out[0] = "7"
    return "".join(out)


def mul(size, seed):
    return ("*", f"{digits(size, seed)} {digits(size, seed + 1)}")


DATA = [
    # 1: all seven operations, tiny operands
    [
        ("+", "123456789012345678901234567890 98765432109876543210"),
        ("-", "1000000000000000000000000000000 1"),
        ("*", f"{digits(30, 3)} {digits(30, 4)}"),
        ("/", "-98765432109876543210 123456789"),
        ("exp", "1.5"),
        ("log", "2"),
        ("nroot", "1000000000000000000000000000000 3"),
    ],
    # 2: ten 1000-digit multiplications
    [mul(1000, 10 + 2 * i) for i in range(10)],
    # 3: five 20000-digit multiplications
    [mul(20000, 200 + 2 * i) for i in range(5)],
    # 4: integer k-th roots of 20000-digit numbers
    [("nroot", f"{digits(20000, 400 + i)} {2 + i}") for i in range(4)],
    # 5: exp / log, with one log of a million-digit integer
    [
        ("log", digits(1000000, 500)),
        ("log", "123456789012345678901234567890"),
        ("exp", "1.5"),
        ("exp", "-3.25"),
        ("log", "2"),
    ],
    # 6: additions and subtractions of 25000-digit numbers
    [
        ("+" if i % 2 else "-", f"{digits(25000, 600 + i)} {digits(25000, 700 + i)}")
        for i in range(6)
    ],
    # 7: three 50000-digit multiplications
    [mul(50000, 800 + 2 * i) for i in range(3)],
    # 8: cube root of a 150000-digit number (Newton on top of division)
    [("nroot", f"{digits(150000, 1200)} 3")],
    # 9 and 10: the heavy points are divisions with huge operands; MAL's
    # Newton reciprocal beats the schoolbook division of other libraries here.
    [("/", f"{digits(800000, 1300)} {digits(790000, 1400)}")],
    [("/", f"{digits(1000000, 1500)} {digits(990000, 1600)}")],
]


def main():
    reference = HERE / "_reference"
    res = subprocess.run(
        [CXX, "-std=c++14", "-O2", "interactive_lib.cpp", "main.cpp",
         "-o", str(reference)],
        cwd=HERE, capture_output=True, text=True)
    if res.returncode:
        print(res.stderr.strip(), file=sys.stderr)
        return 1

    slowest = 0.0
    data_files = ["interactive_lib.cpp"]
    for index, queries in enumerate(DATA, 1):
        text = str(len(queries)) + "\n" + "".join(
            f"{op}\n{args}\n" for op, args in queries)
        (HERE / f"{index}.in").write_text(text, encoding="utf-8")

        best = None
        for _ in range(3):  # take the best of several runs, the machine may be busy
            start = time.perf_counter()
            got = subprocess.run([str(reference)], input=text,
                                 capture_output=True, text=True)
            elapsed = (time.perf_counter() - start) * 1000
            best = elapsed if best is None else min(best, elapsed)
            if got.returncode:
                print(f"case {index} failed to run", file=sys.stderr)
                return 1

        (HERE / f"{index}.ans").write_text(got.stdout, encoding="utf-8")
        data_files += [f"{index}.in", f"{index}.ans"]
        slowest = max(slowest, best)
        print(f"{index:2d}: T={len(queries):2d}  {len(text)/1e6:5.2f} MB  "
              f"{best:7.1f} ms  (first answer: {got.stdout.splitlines()[0][:32]})")

    reference.unlink()

    with zipfile.ZipFile(HERE / "data.zip", "w",
                         compression=zipfile.ZIP_DEFLATED) as archive:
        for name in data_files:
            archive.write(HERE / name, name)
    print(f"data.zip rebuilt with {len(data_files)} files; "
          f"slowest point {slowest:.0f} ms")
    print("说明：这里是本机取证时间（每组取 3 次最快的一次）。机器繁忙时数字会"
          "明显偏高，实际速度请以洛谷评测时间为准；若要调整，改 DATA 里最大几组"
          "的位数重跑即可（NTT/Newton 的耗时大致与位数成正比）。")
    return 0


if __name__ == "__main__":
    sys.exit(main())
