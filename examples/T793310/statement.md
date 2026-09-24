# T793310 MAL接口测试

## 题目背景

[MAL](https://github.com/majonori/MAL) 是面向信息学竞赛的数学算法库。
本题用来演示：选手**不需要**把高精度实现写进自己的代码，只要复制一小段声明，
就能使用题目 `interactive_lib.cpp` 里已经提供的算法。

## 题目描述

本题有 $T$ 组询问。每组询问给出一个运算符 $op$ 和操作数，求对应运算的结果。
$op$ 只会是以下 7 种之一：

| $op$ | 含义 | 操作数 | 结果 |
|---|---|---|---|
| `+` | 高精度整数加法 | 两个整数 $a,b$ | $a+b$ |
| `-` | 高精度整数减法 | 两个整数 $a,b$ | $a-b$ |
| `*` | 高精度整数乘法 | 两个整数 $a,b$ | $a\times b$ |
| `/` | 高精度整数除法 | 两个整数 $a,b$，$b\neq 0$ | $\lfloor a/b\rfloor$ |
| $\exp$ | 自然指数 | 一个实数 $x$ | $e^x$ |
| $\log$ | 自然对数 | 一个正实数 $x$ | $\ln x$ |
| $\operatorname{nroot}$ | 整数 $k$ 次根 | 非负整数 $a$，正整数 $k$ | $\lfloor a^{1/k}\rfloor$ |

整数结果可能远超过 64 位，请输出精确值；小数结果请输出 30 位有效数字（四舍五入）。

## 输入格式

第一行一个正整数 $T$，表示询问组数。

接下来 $T$ 组，每组两行：

- 第一行一个字符串 $op$，为 `+`、`-`、`*`、`/`、`exp`、`log`、`nroot` 之一；
- 第二行：
  - 若 $op$ 为 `+`、`-`、`*`、`/`，是两个整数 $a,b$，用一个空格隔开；
  - 若 $op$ 为 `exp`、`log`，是一个实数 $x$（十进制小数，可带负号）；
  - 若 $op$ 为 `nroot`，是非负整数 $a$ 和正整数 $k$，用一个空格隔开。

## 输出格式

共 $T$ 行，每行是这一组询问的结果：

- `+`、`-`、`*`、`/`、`nroot`：一个整数；
- `exp`、`log`：30 位有效数字的小数。

## 输入输出样例 #1

### 输入 #1

```
1
+
123 456
```

### 输出 #1

```
579
```

## 输入输出样例 #2

### 输入 #2

```
1
/
-7 2
```

### 输出 #2

```
-3
```

## 输入输出样例 #3

### 输入 #3

```
1
nroot
1000 3
```

### 输出 #3

```
10
```

## 输入输出样例 #4

### 输入 #4

```
2
log
2
exp
1.5
```

### 输出 #4

```
0.693147180559945309417232121458
4.48168907033806482260205546012
```

## 说明/提示

本题的 `interactive_lib.cpp` 已经提供 MAL 的高精度算法。
选手只要把下面这一整段复制到自己代码的最上面（`#include <bits/stdc++.h>` 的下一行），
不需要 `#include` 交互库，也不需要复制任何算法实现：

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

复制之后直接写程序即可，例如：

```cpp
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
            if (op == "+") std::cout << (x + y) << '\n';
            else if (op == "-") std::cout << (x - y) << '\n';
            else if (op == "*") std::cout << (x * y) << '\n';
            else std::cout << (x / y) << '\n';
        } else if (op == "nroot") {
            std::string a;
            long long k;
            std::cin >> a >> k;
            std::cout << mal::nroot(mal::BigInt(a), (unsigned long long)k) << '\n';
        } else {
            std::string x;
            std::cin >> x;
            mal::BigFloat v(x);        // 默认 256 位二进制精度
            if (op == "exp") std::cout << mal::exp(v).to_string(30) << '\n';
            else std::cout << mal::log(v).to_string(30) << '\n';
        }
    }
    return 0;
}
```

数据范围：

$1\le T\le 10$。

| 运算 | 范围 |
|---|---|
| `+`、`-` | 操作数不超过 $5\times10^4$ 位十进制 |
| `*` | 每个操作数不超过 $10^5$ 位十进制 |
| `/` | 每个操作数不超过 $2\times10^6$ 位 |
| `exp`、`log` | 参数不超过 30 位有效数字；另有一组 `log` 的参数是 $10^6$ 位整数 |
| `nroot` | $a$ 不超过 $3\times10^5$ 位十进制，$2\le k\le5$ |
