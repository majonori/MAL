<img src="https://cdn.luogu.com.cn/upload/image_hosting/fij9kawb.webp" width = "300" alt="MAL" align=right />
<div align="center">

# MAL

_Mathematical Algorithms Library for OI._

[luogu 团队](https://www.luogu.com.cn/team/125531) | [历史赛题](https://www.luogu.com.cn/training/1083540) | [English](README_EN.md)

> 有哪些优秀的百合同人作品？

</div>

---

面向信息学竞赛的数学算法库。  
包含大量小常数数学算法模板，并支持压行后直接嵌入赛题交互库。

MAL 的核心目标是：  
&emsp;&emsp;**数学向比赛题目的难度，不应过分受模板代码影响。**

同时，MAL 不应影响不使用本库的选手正常完成题目。  
接口使用方式见 [MAL接口测试](https://www.luogu.com.cn/problem/T793310)。

## 原理

根据 [luogu 的交互库说明](https://help.luogu.com.cn/manual/luogu/problem/interactive-problems)，  
选手程序 `main.cpp` 会与题目提供的 `interactive_lib.cpp` 共同编译、链接。

本地可以近似使用：
```bash
g++ -std=c++14 -O2 interactive_lib.cpp main.cpp -o main
```  
因此，MAL 可以将算法实现放入 `interactive_lib.cpp`，选手只需声明并调用对应接口。

一般交互题会由 `interactive_lib.cpp` 接管 `main()`；  
MAL 的使用方式不同：库代码只提供函数，不接管选手程序，因此不使用 MAL 的选手仍可正常作答。

## 仓库结构

```text
MAL/
├── bundles/             # 发布：压行代码，与源码文件结构对应
│   ├── common/
│   │   └── main.cpp           # common 模块合并、压行版本
│   ├── hp/
│   │   ├── main.cpp           # 高精度模块合并、压行版本
│   │   └── README.md          # 高精度模块使用手册 / 算法说明
│   ├── poly/
│   │   └── main.cpp           # poly 模块合并、压行版本
│   └── interactive_lib.cpp    # 洛谷交互库版本
├── examples/             # 洛谷题目示例
│   ├── ex1.cpp                # P2293 单文件提交版
│   ├── ex2.cpp                # P5432 单文件提交版
│   └── src/
│       ├── ex1.cpp            # P2293 可读源码
│       └── ex2.cpp            # P5432 可读源码
├── include/             # 开发：模块化源码
│   ├── common/
│   │   ├── consts.hpp
│   │   └── modint.hpp         # 静态模数类模板
│   ├── hp/                    # 高精度整数 / 二进制浮点
│   │   ├── bigfloat.hpp
│   │   └── bigint.hpp
│   └── poly/                  # 多项式全家桶
│       ├── fft.hpp
│       └── ntt.hpp
├── scripts/             # 构建：脚本合并工具
│   └── build.cpp              # 依赖解析 + 合并 + 压行
├── tests/               # 验证：正确性与性能测试
│   ├── benchmark/
│   │   └── bench_hp.cpp
│   └── hp/
│       ├── test_bigfloat.cpp
│       └── test_bigint.cpp
├── ex1.cpp                 # 根目录提交版，等价于 examples/ex1.cpp
├── ex2.cpp                 # 根目录提交版，等价于 examples/ex2.cpp
├── .gitignore
├── README.md
└── README_EN.md
```

高精度模块的精度档位、算法切换点与内存/时间量级见
[`bundles/hp/README.md`](bundles/hp/README.md)。

## 规范

- include/ 保存可读、可维护的正式源码。
- 压行等操作统一由脚本完成，不直接污染源码。
- 库代码默认兼容 C++14。
- 尽量避免宏污染、全局状态和命名冲突。
