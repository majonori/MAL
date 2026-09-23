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

// 高精度整数：内部存十进制串，算法在交互库里。
struct BigInt {
    std::string s;
    BigInt(const std::string& x = "0") : s(x) {}
    BigInt(long long x) : s(std::to_string(x)) {}
    std::string to_string() const;
    std::string to_string(int base) const;
    bool is_zero() const;
    int sign() const;
    unsigned long long bit_length() const;
};

BigInt operator+(const BigInt& a, const BigInt& b);
BigInt operator-(const BigInt& a, const BigInt& b);
BigInt operator*(const BigInt& a, const BigInt& b);
BigInt operator/(const BigInt& a, const BigInt& b);
BigInt operator%(const BigInt& a, const BigInt& b);
BigInt operator-(const BigInt& a);
BigInt operator<<(const BigInt& a, std::size_t bits);
BigInt operator>>(const BigInt& a, std::size_t bits);
bool operator==(const BigInt& a, const BigInt& b);
bool operator!=(const BigInt& a, const BigInt& b);
bool operator<(const BigInt& a, const BigInt& b);
bool operator>(const BigInt& a, const BigInt& b);
bool operator<=(const BigInt& a, const BigInt& b);
bool operator>=(const BigInt& a, const BigInt& b);
BigInt abs(const BigInt& a);
BigInt pow(const BigInt& a, unsigned long long e);
BigInt sqrt(const BigInt& a);
BigInt nroot(const BigInt& a, unsigned long long k);
std::ostream& operator<<(std::ostream& os, const BigInt& x);
std::istream& operator>>(std::istream& is, BigInt& x);

// 高精度小数：内部存十进制串 + 二进制有效位数。
struct BigFloat {
    std::string s;
    int p;
    BigFloat(const std::string& x = "0", int p_ = 256) : s(x), p(p_) {}
    BigFloat(long long x, int p_ = 256) : s(std::to_string(x)), p(p_) {}
    std::string to_string() const;
    std::string to_string(int digits) const;
    bool is_zero() const;
    int sign() const;
    int precision() const;
};

BigFloat operator+(const BigFloat& a, const BigFloat& b);
BigFloat operator-(const BigFloat& a, const BigFloat& b);
BigFloat operator*(const BigFloat& a, const BigFloat& b);
BigFloat operator/(const BigFloat& a, const BigFloat& b);
BigFloat operator-(const BigFloat& a);
bool operator==(const BigFloat& a, const BigFloat& b);
bool operator!=(const BigFloat& a, const BigFloat& b);
bool operator<(const BigFloat& a, const BigFloat& b);
bool operator>(const BigFloat& a, const BigFloat& b);
bool operator<=(const BigFloat& a, const BigFloat& b);
bool operator>=(const BigFloat& a, const BigFloat& b);
BigFloat abs(const BigFloat& a);
BigFloat exp(const BigFloat& x);
BigFloat log(const BigFloat& x);
BigFloat sqrt(const BigFloat& x);
BigFloat pow(const BigFloat& x, long long e);
BigFloat pi(int p);
BigFloat ln2(int p);
std::ostream& operator<<(std::ostream& os, const BigFloat& x);
std::istream& operator>>(std::istream& is, BigFloat& x);

} // namespace remote
using remote::BigInt;
using remote::BigFloat;
using remote::abs;
using remote::exp;
using remote::log;
using remote::nroot;
using remote::pi;
using remote::pow;
using remote::ln2;
using remote::sqrt;

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
    std::cout << mal::pow(x, 5) << '\n';       // 整数快速幂
    std::cout << mal::sqrt(x) << '\n';         // 整数平方根
    std::cout << x.to_string(16) << '\n'; // 转成十六进制输出

    mal::BigFloat u(a, 256), v(b, 256);
    std::cout << ((u + v) * v / u - v) << '\n';
    std::cout << mal::exp(u / v) << '\n';      // 指数
    std::cout << mal::log(u) << '\n';          // 对数
    std::cout << mal::sqrt(u) << '\n';         // 小数平方根
    std::cout << mal::pi(256) << '\n';         // 圆周率
    return 0;
}
```

输入 `123 456` 时输出：

```text
579
56088
0
28153056843
11
7b
1690.5365853658536585365853658536585365853658536585365853658536585365853658536732
1.3096197686011135844950457712947778112264862926651243997268282583883106640634635
4.8121843553724174952620086099599332930239010272220510853539572438974729096242397
11.090536506409417162051600102609932918463376742454020022877312839085001633101352
3.1415926535897932384626433832795028841971693993751058209749445923078164062861980
```

## 接口速查

| 名字 | 说明 |
|---|---|
| `mal::BigInt x(s, base)` | 用字符串构造（默认十进制），`base` 取 `2..36` |
| `mal::BigInt x(n)` | 用整数构造，例如 `mal::BigInt x(100);` |
| `+ - * / %` | 四则与取余，除法向零截断 |
| `-x` | 取负 |
| `<< >>` | 左移、右移（按二进制位） |
| `== != < > <= >=` | 比较大小 |
| `abs(x)` | 绝对值 |
| `pow(x, e)` | 整数快速幂 |
| `sqrt(x)`、`nroot(x, k)` | 整数平方根、整数 `k` 次根 |
| `std::cin >> x`、`std::cout << x` | 直接读入、直接输出十进制 |
| `x.to_string()`、`x.to_string(base)` | 转十进制串、转任意进制串 |
| `x.is_zero()`、`x.sign()`、`x.bit_length()` | 判零、符号、二进制位数 |
| `mal::BigFloat x(s, p)` | 用字符串构造，`p` 是二进制有效位（默认 256） |
| `+ - * /`、`-x`、比较 | 高精度小数四则与比较，精度取两边较大者 |
| `abs(x)` | 绝对值 |
| `exp(x)`、`log(x)`、`sqrt(x)`、`pow(x, k)` | 指数、对数、平方根、整数次幂 |
| `pi(p)`、`ln2(p)` | 圆周率、`ln2`，`p` 位二进制精度 |
| `std::cin >> x`、`std::cout << x` | 读入、按精度自动选位数输出 |
| `x.to_string(k)` | 输出 `k` 位十进制有效数字 |
| `x.is_zero()`、`x.sign()`、`x.precision()` | 判零、符号、当前精度 |

## 题目一侧怎么导出

`include/remote/impl.hpp` 里的运算符都写成**非 inline** 定义，
因此会在交互库的目标文件里留下强符号，选手只写声明即可链接。
新增接口时保持这个约定即可；`interactive_lib.cpp` 由
`scripts/build.cpp` 合并生成，不需要手工维护。

两条与速度有关的实现说明：

- `std::cout << x`：结果的十进制串在运算时已经规范化，输出时直接复用，
  不再重新解析、重新转换一次（这一条让输出较多的数据快了约 2 倍）；
- `log(x)`：当 `x` 是很长的十进制整数时，只取前 60 位有效数字并配合位数
  计算 `log(d) + e * log(10)`，不去解析整条百万位的输入
  （尾数贡献小于 `10^-60`，对 30 位有效数字的输出没有影响）。

检查符号是否导出：

```bash
g++ -std=c++14 -O2 -c bundles/interactive_lib.cpp -o /tmp/t.o
nm -C /tmp/t.o | grep 'mal::remote::'
```

选手视角的三步复制粘贴说明见 [`../../TUTORIAL.md`](../../TUTORIAL.md)。
