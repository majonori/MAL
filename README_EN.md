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
│   └── interactive_lib.cpp    # Luogu interactive library version
├── include/             # Development: modular source code
│   ├── common/
│   │   ├── consts.hpp
│   │   └── modint.hpp         # Static modulus class template
│   ├── hp/                    # Big integers / binary floating point
│   │   ├── bigfloat.hpp
│   │   └── bigint.hpp
│   └── poly/                  # Polynomial full toolkit
│       ├── fft.hpp
│       └── ntt.hpp
├── scripts/             # Build: script merging tools
│   └── build.cpp              # Dependency resolution + merging + Minify
├── tests/               # Verification: correctness and performance tests
│   ├── benchmark/
│   │   └── bench_hp.cpp
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

## Guidelines

- `include/` stores readable, maintainable canonical source code.
- Minification and similar operations are performed uniformly by scripts, without directly polluting the source code.
- The library code is C++14-compatible by default.
- Avoid macro pollution, global state, and naming conflicts as much as possible.
