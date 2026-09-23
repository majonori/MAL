<img src="https://cdn.luogu.com.cn/upload/image_hosting/fij9kawb.webp" width = "300" alt="MAL" align=right />
<div align="center">

# MAL

_Mathematical Algorithms Library for OI._

[Luogu Team](https://www.luogu.com.cn/team/125531) | [Past Contest Problems](https://www.luogu.com.cn/training/1083540) | [中文](README.md)

> What are some excellent yuri doujin works?

</div>

---

A mathematical algorithms library for informatics competitions.  
It contains a large number of small-constant mathematical algorithm templates, and supports direct embedding into contest problem interactive libraries after minification.

MAL's core goal is:  
&emsp;&emsp;**The difficulty of math-oriented contest problems should not be overly affected by template code.**

At the same time, MAL should not prevent contestants who do not use this library from completing problems normally.  
See [MAL Interface Test](https://www.luogu.com.cn/problem/T793310) for how to use the interface.

## Principle

According to [Luogu's interactive library documentation](https://help.luogu.com.cn/manual/luogu/problem/interactive-problems),  
the contestant's program `main.cpp` is compiled and linked together with the problem-provided `interactive_lib.cpp`.

Locally, you can approximately use:
```bash
g++ -std=c++14 -O2 interactive_lib.cpp main.cpp -o main
```  
Therefore, MAL can place algorithm implementations into `interactive_lib.cpp`, and contestants only need to declare and call the corresponding interfaces.

Generally, in interactive problems, `interactive_lib.cpp` takes over `main()`;  
MAL is used differently: the library code only provides functions and does not take over the contestant's program, so contestants who do not use MAL can still solve problems normally.

## Repository Structure

```text
MAL/
├── bundles/             # Release: minified code, corresponding to the source file structure
│   ├── common/
│   │   └── main.cpp           # Merged, minified common module
│   ├── hp/
│   │   ├── main.cpp           # Merged, minified high-precision module
│   │   └── README.md          # User manual / Algorithm notes
│   ├── poly/
│   │   └── main.cpp           # Merged, minified polynomial module
│   ├── remote/
│   │   └── main.cpp           # Cross-TU thin interface release
│   └── interactive_lib.cpp    # Luogu interactive library version
├── examples/T793310/    # Luogu interactive problem interface and data
│   ├── interactive_lib.cpp    # Cross-TU interface interactive library
│   ├── main.cpp               # Contestant example
│   ├── 1.in ... 10.ans        # 10 test cases
│   ├── generate_interface.py  # Interface generator
│   └── data.zip               # Luogu upload package
├── include/             # Development: modular source code
│   ├── common/
│   │   ├── consts.hpp
│   │   └── modint.hpp         # Static modulus class template
│   ├── hp/                    # Big integers / binary floating point
│   │   ├── bigfloat.hpp
│   │   └── bigint.hpp
│   ├── remote/                # Cross-TU thin interface
│   │   ├── impl.hpp
│   │   └── interface.hpp
│   └── poly/                  # Polynomial full toolkit
│       ├── fft.hpp
│       └── ntt.hpp
├── scripts/             # Build: script merging tools
│   └── build.cpp              # Dependency resolution + merging + Minify
├── tests/               # Verification: correctness and performance tests
│   ├── benchmark/
│   │   └── bench_hp.cpp
│   ├── docs/
│   │   └── check_declarations.py  # Compiles the declaration blocks from the docs
│   └── hp/
│       ├── test_bigfloat.cpp
│       └── test_bigint.cpp
├── .gitignore
├── README.md
└── README_EN.md
```

Precision ranges, algorithm crossovers and memory/time estimates for the
high-precision module are documented in
[`bundles/hp/README.md`](bundles/hp/README.md).

The repository does not keep contestant-submission examples that embed MAL.
The problem-side `interactive_lib.cpp` provides the `mal::` interfaces, and
contestant code should only declare and call them.

## Namespaces

All library code lives under `namespace mal`, with implementation details in
sub-namespaces such as `mal::bigint_detail`. Public interfaces are called
through `mal::`, for example:

```cpp
mal::BigInt x("12345678901234567890");
mal::BigFloat y("1.25", 512);
mal::fft_mul(a, b);
mal::ntt_mul(a, b);
mal::mint<998244353> z(1);
```

Headers do not inject `using` declarations, global functions or macros into
the global namespace. `interactive_lib.cpp` exposes only `mal::` symbols, so
a contestant's global function with the same name cannot collide with MAL at
link time.

## Luogu Interactive Problems: What Contestants Must Declare

On Luogu, `main.cpp` and `interactive_lib.cpp` are separate translation
units, and `interactive_lib.cpp` is not available in the contestant source
directory. Therefore `#include "interactive_lib.cpp"` does not work.
Contestants must either write the interface declarations in their own source
or have the problem template provide them.

For the interface used by `examples/T793310`, copy the following block
after `#include <bits/stdc++.h>`:

```cpp
namespace mal {
namespace remote {

struct BigInt {
    std::string s;
    BigInt(const std::string& x = "0") : s(x) {}
    BigInt(long long x) : s(std::to_string(x)) {}
    std::string to_string() const;
};

BigInt operator+(const BigInt& a, const BigInt& b);
BigInt operator-(const BigInt& a, const BigInt& b);
BigInt operator*(const BigInt& a, const BigInt& b);
BigInt operator/(const BigInt& a, const BigInt& b);

struct BigFloat {
    std::string s;
    int p;
    BigFloat(const std::string& x = "0", int p_ = 256) : s(x), p(p_) {}
    BigFloat(long long x, int p_ = 256) : s(std::to_string(x)), p(p_) {}
    std::string to_string(int digits = -1) const;
};

BigFloat operator+(const BigFloat& a, const BigFloat& b);
BigFloat operator-(const BigFloat& a, const BigFloat& b);
BigFloat operator*(const BigFloat& a, const BigFloat& b);
BigFloat operator/(const BigFloat& a, const BigFloat& b);

} // namespace remote

using remote::BigInt;
using remote::BigFloat;

} // namespace mal
```

The operators can then be used directly:

```cpp
#include <bits/stdc++.h>

// The interface declarations above go here.

int main() {
    std::string a, b;
    std::cin >> a >> b;
    mal::BigInt x(a), y(b);
    std::cout << (x + y).to_string() << '\n';
}
```

This block contains declarations only; the MAL implementation remains in
`interactive_lib.cpp`. The complete submission template is shown in
[`examples/T793310/README.md`](examples/T793310/README.md).

For a plain function interface, only the function declaration is needed:

```cpp
namespace mal {
std::string bigint_add(const std::string& a, const std::string& b);
}
```

## Guidelines

- `include/` stores readable, maintainable canonical source code.
- Minification and similar operations are performed uniformly by scripts, without directly polluting the source code.
- The library code is C++14-compatible by default.
- Avoid macro pollution, global state, and naming conflicts as much as possible.
