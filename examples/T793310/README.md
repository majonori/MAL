# T793310 MAL 接口测试

完全不懂 C++ class / namespace 的选手请先看
[`../../TUTORIAL.md`](../../TUTORIAL.md)：那里用三步说明白要把哪一段复制到哪。

题目有 `T` 组询问，每组要求实现 7 种运算中的一种：

| `op` | 含义 | 操作数 | 结果 |
|---|---|---|---|
| `+` | 高精度整数加法 | 两个整数 | `a + b` |
| `-` | 高精度整数减法 | 两个整数 | `a - b` |
| `*` | 高精度整数乘法 | 两个整数 | `a * b` |
| `/` | 高精度整数除法 | 两个整数 | `a / b`，向零取整 |
| `exp` | 自然指数 | 一个实数 | `e^x` |
| `log` | 自然对数 | 一个正实数 | `ln x` |
| `nroot` | 整数 `k` 次根 | 非负整数 `a`、正整数 `k` | `floor(a^(1/k))` |

数据刻意开到最慢的一组要跑 0.5 秒左右，可以直观感受到 MAL 的速度。
完整题面见 [`statement.md`](statement.md)，上传洛谷时直接照抄即可。

## 选手要做的三件事

1. 把下面的接口声明整段复制到 `#include <bits/stdc++.h>` 的下一行；
2. 按题面读入 `op` 和操作数（用 `std::string` 读，别用 `int`）；
3. 调用 `mal::BigInt` / `mal::BigFloat` 算完直接 `std::cout <<` 输出。

## 接口声明（直接复制）

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

## 完整提交模板

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

这份模板就是 `main.cpp`，可以直接提交。

## 文件说明

| 文件 | 说明 |
|---|---|
| `statement.md` | 洛谷题面（题目背景 / 描述 / 输入输出 / 样例 / 提示） |
| `interactive_lib.cpp` | MAL 发布产物，由 `generate_interface.py` 复制 |
| `main.cpp` | 参考程序，由 `generate_interface.py` 生成 |
| `1.in ... 10.ans` | 10 组数据 |
| `data.zip` | 上传洛谷的数据包 |
| `generate_interface.py` | 同步接口声明，重新生成 `interactive_lib.cpp` 与 `main.cpp` |
| `generate_data.py` | 用参考程序重新生成 10 组数据与 `data.zip` |

## 数据

参考程序（MAL）与一份 CPython 3.14 实现的对比，同一台机器、同一时间测得
（Apple M 系列，单线程，`-O2`，参考程序取 5 次最快）：

| 编号 | `T` | 内容 | 输入大小 | MAL | CPython | 比值 |
|---:|---:|---|---:|---:|---:|---:|
| 1 | 7 | 7 种运算各一条，30 位小数/整数 | `0.00 MB` | `8 ms` | `17 ms` | `2.1x` |
| 2 | 10 | 10 次 1000 位整数乘法 | `0.02 MB` | `14 ms` | `17 ms` | `1.2x` |
| 3 | 5 | 5 次 2 万位整数乘法 | `0.20 MB` | `77 ms` | `57 ms` | `0.74x` |
| 4 | 4 | 2 万位整数开 `k` 次根，`k = 2..5` | `0.08 MB` | `173 ms` | `63 ms` | `0.36x` |
| 5 | 5 | `exp` / `log`，含一条 `10^6` 位整数的 `log` | `1.00 MB` | `13 ms` | `24 ms` | `1.8x` |
| 6 | 6 | 2.5 万位整数加减 | `0.30 MB` | `85 ms` | `62 ms` | `0.73x` |
| 7 | 3 | 3 次 5 万位整数乘法 | `0.30 MB` | `191 ms` | `138 ms` | `0.72x` |
| 8 | 1 | 15 万位整数开三次根 | `0.15 MB` | `439 ms` | `301 ms` | `0.69x` |
| 9 | 1 | **80 万 ÷ 79 万位（最慢点之一）** | `1.59 MB` | `435 ms` | `590 ms` | `1.4x` |
| 10 | 1 | **100 万 ÷ 99 万位（最慢点）** | `1.99 MB` | `474 ms` | `854 ms` | `1.8x` |
| 合计 | | | `5.6 MB` | `1910 ms` | `2123 ms` | `1.11x` |

两点说明：

- 第 9、10 组是 MAL 明显占优的场合：两个超大整数相除走 Newton 倒数，
  耗时几乎只由十进制读写决定，而按位相除的实现会随位数平方增长；
- 第 5 组体现的是「长整数取对数」的加速：只取前 60 位有效数字配合位数算
  `log(d) + e * log(10)`，不去解析整条百万位输入（从 `174 ms` 降到 `11 ms`）；
- 第 3、4、6、7、8 组以十进制转换为主：`to_string` / `from_string` 走分治
  进制转换，开销与同规模乘法同阶，但常数仍高于 libmpdec、OpenSSL 这类
  二进制大数实现，这是后续优化的方向（不改变接口与结果）。

洛谷评测机速度与本机不同，上传后按评测结果确认一下：如果最慢点跑出
300-600 ms 之外（本机空闲时第 10 组约 `470 ms`），改 `generate_data.py` 里 `DATA` 的位数再跑一次即可
（这几组大数据都是 NTT/Newton，耗时大致与位数成正比）。

`data.zip` 的根目录只包含 `interactive_lib.cpp` 和 10 组 `.in` / `.ans`。

## 重新生成

```bash
python3 examples/T793310/generate_interface.py
CXX=g++ python3 examples/T793310/generate_data.py
```

`generate_data.py` 会用 `main.cpp` + `interactive_lib.cpp` 编译出参考程序，
再用它跑出每组答案，所以数据和参考程序永远一致；它还会打印每组数据的实测耗时，
方便确认最慢点仍在 300-600 ms 这个区间。

## 本地测试

```bash
cd examples/T793310

g++ -std=c++14 -O2 interactive_lib.cpp main.cpp -o main
for i in $(seq 1 10); do
    ./main < "$i.in" | diff - "$i.ans"
done
```

检查导出符号：

```bash
g++ -std=c++14 -O2 -c interactive_lib.cpp -o /tmp/t793310.o
nm -C /tmp/t793310.o | grep 'mal::remote::'
```
