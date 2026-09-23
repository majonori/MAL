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
│   ├── remote/
│   │   └── main.cpp           # 跨编译单元薄接口发布版
│   └── interactive_lib.cpp    # 洛谷交互库版本
├── examples/T793310/    # 洛谷交互题接口与数据
│   ├── interactive_lib.cpp    # 跨编译单元接口交互库
│   ├── main.cpp               # 选手示例
│   ├── 1.in ... 10.ans        # 10 组数据
│   ├── generate_interface.py  # 接口生成脚本
│   └── data.zip               # 洛谷上传数据包
├── include/             # 开发：模块化源码
│   ├── common/
│   │   ├── consts.hpp
│   │   └── modint.hpp         # 静态模数类模板
│   ├── hp/                    # 高精度整数 / 二进制浮点
│   │   ├── bigfloat.hpp
│   │   └── bigint.hpp
│   ├── remote/                # 跨编译单元薄接口
│   │   ├── impl.hpp
│   │   └── interface.hpp
│   └── poly/                  # 多项式全家桶
│       ├── fft.hpp
│       └── ntt.hpp
├── scripts/             # 构建：脚本合并工具
│   └── build.cpp              # 依赖解析 + 合并 + 压行
├── tests/               # 验证：正确性与性能测试
│   ├── benchmark/
│   │   └── bench_hp.cpp
│   ├── docs/
│   │   └── check_declarations.py  # 文档声明块可直接复制的编译检查
│   └── hp/
│       ├── test_bigfloat.cpp
│       └── test_bigint.cpp
├── .gitignore
├── README.md
└── README_EN.md
```

高精度模块的精度档位、算法切换点与内存/时间量级见
[`bundles/hp/README.md`](bundles/hp/README.md)。

仓库不保存内嵌 MAL 实现的选手提交示例。题目侧的 `interactive_lib.cpp`
负责提供 `mal::` 接口，选手代码只需声明并调用这些接口。

## 命名空间

所有库代码都位于 `namespace mal` 中，内部实现放在 `mal::bigint_detail`
等子命名空间。公开接口统一通过 `mal::` 调用，例如：

```cpp
mal::BigInt x("12345678901234567890");
mal::BigFloat y("1.25", 512);
mal::fft_mul(a, b);
mal::ntt_mul(a, b);
mal::mint<998244353> z(1);
```

头文件不会向全局命名空间引入 `using`、全局函数或宏。`interactive_lib.cpp`
只提供 `mal::` 下的符号，因此即使选手的 `main.cpp` 定义了同名全局函数，
也不会与 MAL 库发生链接期重名。

## 洛谷交互题：选手怎么声明

洛谷的 `main.cpp` 和 `interactive_lib.cpp` 是两个独立编译单元，
`interactive_lib.cpp` 不会出现在选手源码目录，因此不能使用
`#include "interactive_lib.cpp"`。选手必须在自己代码中写下接口声明，
或者在题目模板中预置这些声明。

以 `examples/T793310` 的接口为例，把下面整段复制到
`#include <bits/stdc++.h>` 后面：

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

然后就能直接使用运算符：

```cpp
#include <bits/stdc++.h>

// 上面是接口声明块

int main() {
    std::string a, b;
    std::cin >> a >> b;
    mal::BigInt x(a), y(b);
    std::cout << (x + y).to_string() << '\n';
}
```

这段声明只描述接口，不包含 MAL 实现；真正的
`BigInt` / `BigFloat` 实现仍然在 `interactive_lib.cpp` 中。
完整提交模板见
[`examples/T793310/README.md`](examples/T793310/README.md)。

如果题目接口是普通函数，则只需要声明函数：

```cpp
namespace mal {
std::string bigint_add(const std::string& a, const std::string& b);
}
```

## 规范

- include/ 保存可读、可维护的正式源码。
- 压行等操作统一由脚本完成，不直接污染源码。
- 库代码默认兼容 C++14。
- 尽量避免宏污染、全局状态和命名冲突。
