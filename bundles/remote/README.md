# REMOTE

跨编译单元薄接口。源码在 `include/remote/`：

- `interface.hpp`：选手复制的那一份声明；
- `impl.hpp`：交互库一侧的实现，调用 `mal::BigInt` / `mal::BigFloat`。

发布产物是 `bundles/remote/main.cpp`（接口 + 实现的压行版本），
`bundles/interactive_lib.cpp` 里也包含同一份内容。

## 为什么要这个东西

洛谷的 `main.cpp`（选手）和 `interactive_lib.cpp`（题目）是两个独立编译单元，
交互库不在选手的源码目录里，选手既不能 `#include` 它，也看不到
`mal::BigInt` 的类定义。而类类型必须看到定义才能使用，
所以这里提供一层“值包装”：类型很小，声明可以直接写在选手代码里，
真正的算法仍然在交互库内部。

## 可直接复制的接口声明

选手把下面整段复制到 `#include <bits/stdc++.h>` 的下一行
（或由题目模板预先放好），**不需要**复制任何实现：

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
std::ostream& operator<<(std::ostream& os, const BigInt& x);

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
std::ostream& operator<<(std::ostream& os, const BigFloat& x);

} // namespace remote

using remote::BigInt;
using remote::BigFloat;

} // namespace mal
```

## 可直接复制的使用示例

```cpp
int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::string a, b;
    std::cin >> a >> b;

    mal::BigInt x(a), y(b);
    std::cout << (x + y) << '\n';
    std::cout << (x * y) << '\n';
    std::cout << (x / y) << '\n';

    mal::BigFloat u(a, 256), v(b, 256);
    std::cout << ((u + v) * v / u - v) << '\n';
    return 0;
}
```

输入 `123 456` 时输出：

```text
579
56088
0
1690.5365853658536585365853658536585365853658536585365853658536585365853658536732
```

## 接口速查

| 名字 | 说明 |
|---|---|
| `mal::BigInt x(s)` | 用十进制字符串构造高精度整数 |
| `mal::BigInt x(n)` | 用整数构造，例如 `mal::BigInt x(100);` |
| `+ - * /` | 四则运算，除法向零截断 |
| `std::cout << x` | 直接输出十进制 |
| `x.to_string()` | 得到十进制字符串 |
| `mal::BigFloat x(s, p)` | 用十进制字符串构造，`p` 是二进制有效位（默认 256） |
| `+ - * /` | 高精度小数四则运算 |
| `std::cout << x` | 按精度自动选位数输出 |
| `x.to_string(k)` | 输出 `k` 位十进制有效数字 |

## 题目一侧怎么导出

`include/remote/impl.hpp` 里的运算符都写成**非 inline** 定义，
因此会在交互库的目标文件里留下强符号，选手只写声明即可链接。
新增接口时保持这个约定即可；`interactive_lib.cpp` 由
`scripts/build.cpp` 合并生成，不需要手工维护。

检查符号是否导出：

```bash
g++ -std=c++14 -O2 -c bundles/interactive_lib.cpp -o /tmp/t.o
nm -C /tmp/t.o | grep 'mal::remote::'
```

选手视角的三步复制粘贴说明见 [`../../TUTORIAL.md`](../../TUTORIAL.md)。
