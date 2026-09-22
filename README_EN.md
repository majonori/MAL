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
│   ├── dgf/
│   │   ├── main.hpp           # Auto-merged, minified version
│   │   └── README.md          # User manual / Algorithm notes
│   └── interactive_lib.cpp    # Luogu interactive library version
├── include/             # Development: modular source code
│   ├── common/
│   │   ├── modint.hpp         # Static modulus class template
│   │   └── ...                # ellipses omitted below
│   ├── poly/                  # Polynomial full toolkit
│   │   └── ntt.hpp
│   ├── hp/                    # Big integers / binary floating point
│   │   ├── bigint.hpp
│   │   └── bigfloat.hpp
│   └── dgf/                   # DGF full toolkit
│       ├── convolution.hpp
│       ├── transform.hpp      # zeta / mobius / gcd / lcm
│       └── fps.hpp            # inv / ln / exp / pow ...
├── scripts/             # Build: script merging tools
│   └── build.cpp              # Dependency resolution + merging + Minify
├── tests/               # Verification: correctness and performance tests
│   └── benchmark/
├── README.md
└── README_EN.md
```

Precision ranges, algorithm crossovers and memory/time estimates for the
high-precision module are documented in
[`bundles/hp/README.md`](bundles/hp/README.md).

## Guidelines

- `include/` stores readable, maintainable canonical source code.
- Minification and similar operations are performed uniformly by scripts, without directly polluting the source code.
- The library code is C++14-compatible by default.
- Avoid macro pollution, global state, and naming conflicts as much as possible.
