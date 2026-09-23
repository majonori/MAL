#!/usr/bin/env python3
"""Check that the declarations printed in the documentation can be copy-pasted.

The Luogu interactive format compiles the contestant's ``main.cpp`` and the
problem's ``interactive_lib.cpp`` as two translation units, so every interface
a contestant needs must be writable as a plain declaration. This script takes
the code blocks out of the READMEs and builds them for real:

1. no markdown may contain an in-class ``friend`` declaration;
2. the ``mal::remote`` interface block must be identical in every document and
   equal to ``include/remote/interface.hpp``; it is compiled as its own
   translation unit, linked against ``bundles/interactive_lib.cpp`` and run;
3. every namespace-scope declaration from ``bundles/hp/README.md`` must also
   exist in the shipped bundle, and the blocks must compile and run when pasted
   next to ``bundles/hp/main.cpp``.

Usage: python3 tests/docs/check_declarations.py

The checks reproduce the contest environment, so the compiler must be
GCC-compatible and provide ``bits/stdc++.h`` (set ``CXX``, for example
``CXX=g++-16``, to pick a specific compiler).
"""

import os
import pathlib
import re
import shutil
import subprocess
import sys
import tempfile

ROOT = pathlib.Path(__file__).resolve().parents[2]
CXX = os.environ.get("CXX", "g++")
STD = os.environ.get("CXXSTD", "-std=c++14")

DOCS = [
    ROOT / "README.md",
    ROOT / "README_EN.md",
    ROOT / "TUTORIAL.md",
    ROOT / "bundles/hp/README.md",
    ROOT / "examples/T793310/README.md",
]

# Per-module copy-paste examples: the README shows the code a contestant writes.
# Each entry is (prepend, extra translation units, stdin, expected lines).
MODULE_EXAMPLES = {
    "bundles/remote/README.md": {
        "prepend_from_doc": True,
        "extra": ["bundles/interactive_lib.cpp"],
        "stdin": "123 456\n",
        "expect": [
            "579",
            "56088",
            "0",
            "1690.5365853658536585365853658536585365853658536585365853658536585365853658536732",
        ],
    },
    "bundles/hp/README.md": {
        "prepend": "",
        "stdin": "",
        "expect": [
            "12193263113702179522496570642237463801111263526900",
            "1.2500000000000000000000000000000000000000000000000000000000000000000000000000000",
        ],
    },
}

# Declaration-only interfaces: a contestant pastes the README block and links the
# problem's interactive_lib.cpp, without ever including or copying MAL.
DECLARATION_ONLY = {
    "bundles/common/README.md": {
        "program": """
int main() {
    mal::mint<998244353> a = 3, b = 5;
    std::cout << (a + b).v << ' ' << (a * b).v << ' '
              << mal::mint<998244353>(2).pow(10).v << ' ' << a.inv().v << '\\n';
    std::cout << mal::glim(10) << ' ' << (int)(mal::PI * 1000) << '\\n';
    return 0;
}
""",
        "expect": ["8 15 1024 332748118", "16 3141"],
    },
    "bundles/poly/README.md": {
        "requires": ["bundles/common/README.md"],
        "program": """
int main() {
    mal::vector<mal::ntt_mint> a = {1, 2, 3}, b = {4, 5};
    for (auto x : mal::ntt_mul(a, b)) std::cout << x.v << ' ';
    std::cout << '\\n';
    for (auto x : mal::ntt_conv({1, 2, 3}, {4, 5, 6, 7})) std::cout << x.v << ' ';
    std::cout << '\\n';
    mal::vector<mal::cpx> p = {1, 2, 3}, q = {4, 5};
    for (auto x : mal::fft_mul(p, q)) std::cout << std::lround(x.real()) << ' ';
    std::cout << '\\n';
    return 0;
}
""",
        "expect": ["4 13 22 15", "32 38", "4 13 22 15"],
    },
}


def code_blocks(text):
    return re.findall(r"```(?:cpp|c\+\+)\n(.*?)```", text, re.S)


def squeeze(text):
    return re.sub(r"\s+", "", text)


def strip_comments(text):
    return re.sub(r"//[^\n]*", "", text)


def mint_definition(text):
    """Return the token sequence of `template <int MOD> struct mint { ... };`."""
    start = text.index("template <int MOD>")
    brace = text.index("{", start)
    depth = 0
    for i in range(brace, len(text)):
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
            if depth == 0:
                end = text.index(";", i)
                return squeeze(strip_comments(text[start:end + 1]))
    raise ValueError("unterminated mint definition")


def declarations_only(text):
    """Drop preprocessor lines, using-directives and blank lines."""
    kept = []
    for line in text.splitlines():
        stripped = line.strip()
        if not stripped or stripped.startswith("#"):
            continue
        if stripped.startswith("using ") or stripped.startswith("using\t"):
            continue
        kept.append(stripped)
    return "\n".join(kept)


def run(cmd, **kwargs):
    return subprocess.run([str(c) for c in cmd], **kwargs)


def compile_and_link(sources, exe, tmp):
    objects = []
    for i, source in enumerate(sources):
        obj = tmp / f"unit{i}.o"
        res = run([CXX, STD, "-O2", "-I", ROOT, "-c", source, "-o", obj],
                  cwd=ROOT, capture_output=True, text=True)
        if res.returncode:
            return res
        objects.append(obj)
    return run([CXX, STD, "-O2", *objects, "-o", exe],
               cwd=ROOT, capture_output=True, text=True)


def main():
    failures = []
    tmp = pathlib.Path(tempfile.mkdtemp(prefix="mal_doc_check_"))
    try:
        probe = tmp / "probe.cpp"
        probe.write_text("#include <bits/stdc++.h>\nint main() { return 0; }\n")
        res = run([CXX, STD, "-c", probe, "-o", tmp / "probe.o"],
                  capture_output=True, text=True)
        if res.returncode:
            print(f"SKIP: {CXX} cannot compile #include <bits/stdc++.h>; use a "
                  f"GCC-compatible compiler via CXX=...")
            return 2

        # 1. no in-class friend declarations in the documentation
        for doc in DOCS:
            for number, line in enumerate(doc.read_text().splitlines(), 1):
                if re.search(r"\bfriend\b", line):
                    failures.append(
                        f"{doc.relative_to(ROOT)}:{number}: documentation must not "
                        f"contain 'friend' declarations")

        # 2. the interactive interface block
        interface = ROOT / "include/remote/interface.hpp"
        expected_interface = squeeze(declarations_only(interface.read_text()))
        interface_blocks = []
        full_programs = []
        for doc in DOCS:
            found = [b for b in code_blocks(doc.read_text()) if "struct BigInt" in b]
            if not found:
                failures.append(f"{doc.relative_to(ROOT)}: no interface block found")
                continue
            for block in found:
                body = squeeze(declarations_only(block))
                if expected_interface not in body:
                    failures.append(
                        f"{doc.relative_to(ROOT)}: interface block differs from "
                        f"include/remote/interface.hpp")
                if "int main" in block:
                    full_programs.append((doc, block))
                else:
                    interface_blocks.append((doc, block))

        if not failures:
            doc, block = interface_blocks[0]
            main_cpp = tmp / "contestant_main.cpp"
            main_cpp.write_text(
                "#include <bits/stdc++.h>\n\n" + block + r"""

int main() {
    std::string a, b;
    if (!(std::cin >> a >> b)) return 1;
    mal::BigInt x(a), y(b);
    mal::BigFloat u(a, 256), v(b, 256);
    std::cout << (x + y) << '\n';
    std::cout << (x + y).to_string() << '\n';
    std::cout << (x * y + x - y).to_string() << '\n';
    std::cout << ((u + v) * v / u - v) << '\n';
    return 0;
}
""")
            exe = tmp / "contestant"
            res = compile_and_link(
                [main_cpp, ROOT / "bundles/interactive_lib.cpp"], exe, tmp)
            if res.returncode:
                failures.append(
                    f"interface block from {doc.relative_to(ROOT)} does not build "
                    f"as a separate translation unit:\n{res.stderr.strip()}")
            else:
                out = run([exe], input="123456789012345678901234567890 "
                                       "98765432109876543210\n",
                          capture_output=True, text=True).stdout.strip().splitlines()
                want = "12193263113702179522620027431151044047902621551580"
                if len(out) < 4:
                    failures.append(
                        f"interface block printed {out} instead of four lines")
                elif out[0] != out[1]:
                    failures.append(
                        "mal::remote::operator<< and to_string() disagree: "
                        f"{out[0]!r} vs {out[1]!r}")
                elif out[2] != want:
                    failures.append(
                        f"interface block produced {out[2]!r} instead of {want}")
                elif not out[3].startswith("79012346407."):
                    failures.append(
                        f"BigFloat operator<< printed {out[3]!r}")

        # 2b. every complete submission template in the docs must build and pass
        if not failures:
            for index, (doc, block) in enumerate(full_programs):
                program = tmp / f"template{index}.cpp"
                program.write_text(block)
                exe = tmp / f"template{index}"
                res = compile_and_link(
                    [program, ROOT / "bundles/interactive_lib.cpp"], exe, tmp)
                if res.returncode:
                    failures.append(
                        f"submission template in {doc.relative_to(ROOT)} does not "
                        f"build:\n{res.stderr.strip()}")
                    continue
                got = run([exe], input="123 456\n",
                          capture_output=True, text=True).stdout.split()
                if got[:1] != ["579"]:
                    failures.append(
                        f"submission template in {doc.relative_to(ROOT)} printed "
                        f"{got[:1] or 'nothing'} for '123 456' instead of 579")

        # 3. namespace-scope declarations documented for the hp module
        hp_readme = ROOT / "bundles/hp/README.md"
        hp_blocks = [b for b in code_blocks(hp_readme.read_text())
                     if "namespace mal" in b and "namespace remote" not in b]
        bundle = ROOT / "bundles/hp/main.cpp"
        bundle_text = squeeze(bundle.read_text())
        for block in hp_blocks:
            for line in block.splitlines():
                decl = line.strip()
                if not decl or decl.startswith("namespace") or decl.startswith("}"):
                    continue
                probe = squeeze(decl[:-1] if decl.endswith(";") else decl)
                if probe not in bundle_text:
                    failures.append(
                        f"bundles/hp/README.md documents '{decl}' but "
                        f"bundles/hp/main.cpp does not declare it")

        if not failures:
            doc_cpp = tmp / "hp_declarations.cpp"
            doc_cpp.write_text(
                '#include <bits/stdc++.h>\n#include "bundles/hp/main.cpp"\n\n'
                + "\n".join(hp_blocks) + r"""

int main() {
    using mal::BigInt;
    using mal::BigFloat;
    BigInt a("123456789012345678901234567890"), b("98765432109876543210");
    BigInt s = mal::operator+(a, b);
    BigInt d = mal::operator-(a, b);
    BigInt p = mal::operator*(a, b);
    BigInt q = mal::operator/(a, b);
    BigInt r = mal::operator%(a, b);
    BigInt sh = mal::operator<<(BigInt(1), 200);
    if (mal::operator>>(sh, 200) != BigInt(1)) return 1;
    if (mal::compare(s, a) <= 0) return 2;
    if (!mal::operator==(sh >> 200, BigInt(1))) return 3;
    if (!mal::operator!=(d, a)) return 4;
    if (!mal::operator<(d, a)) return 5;
    if (!mal::operator>(s, a)) return 6;
    if (!mal::operator<=(d, a)) return 7;
    if (!mal::operator>=(s, a)) return 8;
    if (s != a + b || d != a - b || p != a * b || q != a / b || r != a % b) return 9;
    if (mal::operator<<(std::cout, d).fail()) return 10;

    BigFloat x("1.5", 512), y("2.5", 512);
    BigFloat u = mal::operator+(mal::operator*(x, y), x);
    BigFloat v = mal::operator-(mal::operator/(u, x), y);
    if (!mal::operator==(v, BigFloat(1, 512))) return 11;
    if (!mal::operator!=(v, x)) return 12;
    if (!mal::operator<(x, y)) return 13;
    if (!mal::operator>(y, x)) return 14;
    if (!mal::operator<=(x, y)) return 15;
    if (!mal::operator>=(y, x)) return 16;
    if (mal::compare(x, x) != 0) return 17;
    if (mal::operator<<(std::cout, v).fail()) return 18;
    if (mal::exp(mal::log(BigFloat(2, 512))).to_string(20)[0] != '2') return 19;
    if (mal::sqrt(BigFloat(2, 512)).to_string(20)[0] != '1') return 20;
    if (mal::pow(x, 3) != BigFloat("3.375", 512)) return 21;
    return 0;
}
""")
            exe = tmp / "hp_declarations"
            res = compile_and_link([doc_cpp], exe, tmp)
            if res.returncode:
                failures.append(
                    f"namespace-scope declarations in bundles/hp/README.md do not "
                    f"build:\n{res.stderr.strip()}")
            else:
                run_res = run([exe], capture_output=True, text=True)
                if run_res.returncode:
                    failures.append(
                        "pasted hp declarations returned exit code "
                        f"{run_res.returncode} (see the checks inside the test)")

        # 4. every module README ships an example that really runs
        if not failures:
            for rel, spec in MODULE_EXAMPLES.items():
                doc = ROOT / rel
                blocks = [b for b in code_blocks(doc.read_text()) if "int main" in b]
                if not blocks:
                    failures.append(f"{rel}: no copy-paste example found")
                    continue
                program = tmp / (doc.parent.name + "_example.cpp")
                if spec.get("prepend_from_doc"):
                    decl = [b for b in code_blocks(doc.read_text())
                            if "struct BigInt" in b and "int main" not in b][0]
                    prefix = "#include <bits/stdc++.h>\n\n" + decl + "\n"
                else:
                    prefix = "#include <bits/stdc++.h>\n" + spec["prepend"]
                program.write_text(prefix + blocks[0])
                sources = [program] + [ROOT / s for s in spec.get("extra", [])]
                exe = tmp / (doc.parent.name + "_example")
                res = compile_and_link(sources, exe, tmp)
                if res.returncode:
                    failures.append(
                        f"copy-paste example in {rel} does not build:\n"
                        f"{res.stderr.strip()}")
                    continue
                got = run([exe], input=spec["stdin"], capture_output=True,
                          text=True).stdout.strip().splitlines()
                for i, want in enumerate(spec["expect"]):
                    have = got[i] if i < len(got) else "<missing>"
                    ok = (have.startswith(want[:-3]) if want.endswith("...")
                          else have.rstrip() == want.rstrip())
                    if not ok:
                        failures.append(
                            f"copy-paste example in {rel} printed {have!r} on line "
                            f"{i + 1} instead of {want!r}")
                        break

        # 5. declaration-only interfaces: paste the block, link the library
        if not failures:
            for rel, spec in DECLARATION_ONLY.items():
                doc = ROOT / rel
                blocks = [b for b in code_blocks(doc.read_text())
                          if "namespace mal" in b and "int main" not in b]
                if not blocks:
                    failures.append(f"{rel}: no declaration block found")
                    continue
                # Interfaces that build on another module have to be pasted after it.
                for dep in spec.get("requires", []):
                    dep_doc = ROOT / dep
                    blocks = [b for b in code_blocks(dep_doc.read_text())
                              if "namespace mal" in b and "int main" not in b] + blocks
                program = tmp / (doc.parent.name + "_decl.cpp")
                program.write_text(
                    "#include <bits/stdc++.h>\n\n" + "\n".join(blocks) + "\n"
                    + spec["program"])
                exe = tmp / (doc.parent.name + "_decl")
                res = compile_and_link(
                    [program, ROOT / "bundles/interactive_lib.cpp"], exe, tmp)
                if res.returncode:
                    failures.append(
                        f"declaration block in {rel} does not build against "
                        f"bundles/interactive_lib.cpp:\n{res.stderr.strip()}")
                    continue
                got = run([exe], capture_output=True,
                          text=True).stdout.strip().splitlines()
                for i, want in enumerate(spec["expect"]):
                    have = got[i] if i < len(got) else "<missing>"
                    if have.rstrip() != want.rstrip():
                        failures.append(
                            f"declaration block in {rel} printed {have!r} on line "
                            f"{i + 1} instead of {want!r}")
                        break

        # 6. the mint definition contestants copy must match the shipped header
        if not failures:
            header_mint = mint_definition(
                (ROOT / "include/common/modint.hpp").read_text())
            doc = ROOT / "bundles/common/README.md"
            doc_mint = None
            for block in code_blocks(doc.read_text()):
                if "struct mint" in block:
                    doc_mint = mint_definition(block)
                    break
            if doc_mint is None:
                failures.append(
                    "bundles/common/README.md: no mint definition to copy")
            elif doc_mint != header_mint:
                failures.append(
                    "bundles/common/README.md copies a different mint definition "
                    "than include/common/modint.hpp")
    finally:
        shutil.rmtree(tmp, ignore_errors=True)

    if failures:
        for failure in failures:
            print("FAIL:", failure)
        return 1
    print("documentation declarations: ok")
    return 0


if __name__ == "__main__":
    sys.exit(main())
