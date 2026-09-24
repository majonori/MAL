#!/usr/bin/env python3
"""Regenerate interactive_lib.cpp and main.cpp for T793310.

The contestant block is taken from bundles/hp/README.md (itself kept in
sync with include/remote/interface.hpp), so the reference program always
contains exactly the declarations a contestant copies.

Usage: python3 examples/T793310/generate_interface.py
"""

import pathlib
import re
import shutil

HERE = pathlib.Path(__file__).resolve().parent
ROOT = HERE.parent.parent

# The interactive library is the normal MAL bundle: include/remote/ has the
# thin value interface, and every operator/function it declares is exported as
# a non-inline symbol.
shutil.copyfile(ROOT / "bundles" / "interactive_lib.cpp", HERE / "interactive_lib.cpp")

readme = (ROOT / "bundles" / "remote" / "README.md").read_text()
blocks = re.findall(r"```cpp\n(.*?)```", readme, re.S)
interface = [b for b in blocks if "struct BigInt" in b and "int main" not in b][0].strip()

MAIN = """

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int T;
    std::cin >> T;
    while (T--) {
        std::string op;
        std::cin >> op;

        if (op == "+" || op == "-" || op == "*" || op == "/") {
            std::string a, b;
            std::cin >> a >> b;
            mal::BigInt x(a), y(b);
            if (op == "+") std::cout << (x + y) << '\\n';
            else if (op == "-") std::cout << (x - y) << '\\n';
            else if (op == "*") std::cout << (x * y) << '\\n';
            else std::cout << (x / y) << '\\n';
        } else if (op == "nroot") {
            std::string a;
            long long k;
            std::cin >> a >> k;
            std::cout << mal::nroot(mal::BigInt(a), (unsigned long long)k) << '\\n';
        } else {
            std::string x;
            std::cin >> x;
            mal::BigFloat v(x);        // 默认 256 位二进制精度
            if (op == "exp") std::cout << mal::exp(v).to_string(30) << '\\n';
            else std::cout << mal::log(v).to_string(30) << '\\n';
        }
    }
    return 0;
}
"""

main_source = "#include <bits/stdc++.h>\n\n" + interface + "\n" + MAIN
(HERE / "main.cpp").write_text(main_source, encoding="utf-8")

# Keep the "完整提交模板" block of the README identical to main.cpp.
readme_path = HERE / "README.md"
readme_text = readme_path.read_text()
start_marker = "```cpp\n#include <bits/stdc++.h>"
start = readme_text.index(start_marker)
end = readme_text.index("```", start + len(start_marker))
readme_path.write_text(
    readme_text[:start] + "```cpp\n" + main_source.strip() + "\n" + readme_text[end:])
