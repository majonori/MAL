# 选手教程：把 MAL 复制进你的代码

这份教程假设你完全不懂 C++ 的 class 和 namespace。
你只要会复制、粘贴、改数字，就能用上 MAL 的高精度。

MAL 的算法已经在题目的 `interactive_lib.cpp` 里了，你不需要下载它，
也不需要 `#include` 它。只要在自己代码的最上面放一段“声明”，
就能直接用 `mal::BigInt`（高精度整数）和 `mal::BigFloat`（高精度小数）。

## 三步搞定

### 第 1 步：把下面一整段复制下来

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

如果你打开提交框时，题目已经替你写好了上面这段（能搜到 `namespace mal`），
那就不用再复制了，直接跳到第 3 步。

### 第 2 步：粘到你代码的最上面

粘在 `#include <bits/stdc++.h>` 的下一行，放在 `main()` **外面**，
不要改里面任何一个字母。

```cpp
#include <bits/stdc++.h>

// ← 上面那段声明粘在这里（main 的外面）

int main() {
    // ← 你的代码写在这里
    return 0;
}
```

### 第 3 步：在 main 里直接写

```cpp
int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::string a, b;
    std::cin >> a >> b;

    mal::BigInt x(a), y(b);              // 把输入的字符串变成高精度整数
    std::cout << (x + y) << '\n';        // 像普通整数一样直接输出
    return 0;
}
```

`mal::BigInt x(a)` 的意思是“用字符串 `a` 造一个高精度整数，名字叫 `x`”。
`x + y` 就是两个高精度整数相加，结果可以直接 `std::cout <<` 打印，
也可以写成 `(x + y).to_string()` 变成字符串。
只要记住“输入用字符串读”，其他照抄就行。

## 完整成品（整段复制就能提交）

```cpp
#include <bits/stdc++.h>

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

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::string a, b;
    std::cin >> a >> b;

    mal::BigInt x(a), y(b);
    std::cout << (x + y) << '\n';
    return 0;
}
```

输入 `123 456`，输出 `579`：这是最小可用的完整代码。
`examples/T793310`（MAL 接口测试）把它扩成了 7 种运算的完整题目，
可以照着 [`examples/T793310/main.cpp`](examples/T793310/main.cpp) 和
[`examples/T793310/statement.md`](examples/T793310/statement.md) 抄。

## 加减乘除

四种运算写法完全一样，直接把 `+` 换成 `-`、`*`、`/`：

```cpp
mal::BigInt x(a), y(b);

mal::BigInt s = x + y;   // 加
mal::BigInt d = x - y;   // 减
mal::BigInt p = x * y;   // 乘：一万位乘一万位也是一瞬间
mal::BigInt q = x / y;   // 除：整数除法，和小时候学的除法一样，丢掉余数

std::cout << p << '\n';
```

例如 `7 / 2` 得到 `3`，`9 / 3` 得到 `3`。
`x`、`y` 也可以直接用整数写：`mal::BigInt z(100);`。
输出既可以 `std::cout << p << '\n';`，也可以 `std::cout << p.to_string() << '\n';`。

## 高精度小数 BigFloat

第二个参数是保留多少位二进制有效数字，越大越精确（1000 位约等于 300 位十进制）；
`to_string(50)` 表示输出 50 位十进制有效数字：

```cpp
mal::BigFloat u("1.5", 1000), v("2.5", 1000);
mal::BigFloat w = (u + v) * v / u - v;
std::cout << w << '\n';            // 自动输出约 300 位十进制有效数字
std::cout << w.to_string(50) << '\n';   // 也可以自己指定输出 50 位
```

## 除了 + - * / 还有什么

第 1 步那段声明里已经把接口都声明好了，直接调用即可，
不需要再去读文档拼声明：

| 想做的事 | 怎么写 |
|---|---|
| 整数加减乘除取余 | `x + y`、`x - y`、`x * y`、`x / y`、`x % y` |
| 整数比较 | `x < y`、`x == y`、`x >= y` …… |
| 整数快速幂 | `mal::pow(x, 5)` |
| 整数平方根 / k 次根 | `mal::sqrt(x)`、`mal::nroot(x, 3)` |
| 取绝对值、取负 | `mal::abs(x)`、`-x` |
| 二进制左移右移 | `x << 10`、`x >> 10` |
| 转成其它进制 | `x.to_string(16)` |
| 读入、输出 | `std::cin >> x;`、`std::cout << x;`（小数直接 `<<` 会按精度选位数） |
| 指数、对数、小数平方根 | `mal::exp(u)`、`mal::log(u)`、`mal::sqrt(u)` |
| 小数整数次幂 | `mal::pow(u, 3)` |
| 圆周率、ln2 | `mal::pi(512)`、`mal::ln2(512)`（参数是二进制精度） |

小数的运算结果精度取两边精度的较大值；想固定精度就先按需要的位数构造
`mal::BigFloat x("1.25", 512);`。

## 报错对照表

| 你看到的报错 | 原因 | 怎么改 |
|---|---|---|
| `'mal' 未声明` | 没复制第 1 步的声明块 | 把声明块粘到文件最上面 |
| `'mal' 未声明`（明明复制了） | 声明块被粘进了 `main()` 里面 | 挪到 `main()` 外面 |
| `undefined reference to mal::remote::...` | 声明块没复制全，或改动了里面的名字 | 重新整段复制，别改字母 |
| `#include "interactive_lib.cpp"` 致命错误：没有那个文件 | 交互库不在你的目录里，不能 include | 不要 include，只复制声明块 |
| 编译报错说不认识 `mal::BigInt` 和 `mal::BigFloat` 之间的运算 | 两种类型不能混着算（`BigInt + BigFloat`） | 先把它们都变成同一种类型再算 |
| `duplicate symbol mal::remote::...` | 你把 `interactive_lib.cpp` 的内容也复制进自己代码了 | 删掉那些库代码，只留声明块 |
| 结果不对或变成 `0` | 用 `int` / `long long` 读入，数字超过 19 位就读错了 | 一律用 `std::string` 读入 |

## 本地测试

把题目的 `interactive_lib.cpp` 和你的 `main.cpp` 放在同一个文件夹，然后：

```bash
g++ -std=c++14 -O2 interactive_lib.cpp main.cpp -o main
./main
```

自己敲一组数据（例如 `123 456`），看输出对不对。

## 普通题（非交互题）想用完整版

如果题目不是交互题，而是让你在自己的 `main.cpp` 里解决整道题，
可以把仓库里对应模块的压行产物整段复制到代码最上面
（几行很长，属于正常现象）：

```text
高精度       -> bundles/hp/main.cpp
模数类、glim -> bundles/common/main.cpp
卷积         -> bundles/poly/main.cpp
```

例如复制 `bundles/hp/main.cpp` 之后，不仅能加减乘除，
还能用 `pow`、`sqrt`、`nroot`、`exp`、`log` 等接口，写法见
`bundles/hp/README.md`。

交互题**不要**这样做：库已经在 `interactive_lib.cpp` 里了，复制整份实现会和题目那份重复。

## 题面说还提供了别的接口怎么办

有些题目除了 `mal::BigInt`，还会在题面里列出更多接口
（例如 `mal::mint<998244353>`、`mal::ntt_mul`、`mal::fft_mul`）。
这时按对应模块的 README 复制那一段声明块，粘在同一个位置即可：

```text
高精度       -> bundles/hp/README.md      的「洛谷交互题封装」
模数类、glim -> bundles/common/README.md  的「选手复制这一段」
卷积         -> bundles/poly/README.md    的「选手复制这一段」
```

同样是“只复制声明块、不改一个字母”，实现都留在题目的 `interactive_lib.cpp` 里。

## 三个不要

1. 不要把声明块放进 `main()` 里面；
2. 不要 `#include "interactive_lib.cpp"`（你的文件夹里没有它）；
3. 不要把 `interactive_lib.cpp` 的内容复制进自己的代码（会重复定义，链接失败）。
