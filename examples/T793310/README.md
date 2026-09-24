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

数据刻意开到较大规模（最大一组是 200 万位除以 198 万位），用来考察 MAL 在
超大位数下是否仍然可用。
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

最大几组的位数：开三次根到 30 万位，两次除法到 160 万÷158 万位与 200 万÷198 万位。
数据规模如下（各组耗时见 `bundles/hp/README.md`）：

| 编号 | 内容 |
|---:|---|
| 1 | 7 种运算各一条，30 位小数/整数 |
| 2 | 10 次 1000 位整数乘法 |
| 3 | 5 次 4 万位整数乘法 |
| 4 | 4 万位整数开 `k` 次根，`k = 2..5` |
| 5 | `exp` / `log`，含一条 `10^6` 位整数的 `log` |
| 6 | 5 万位整数加减 |
| 7 | 3 次 10 万位整数乘法 |
| 8 | 30 万位整数开三次根 |
| 9 | 160 万 ÷ 158 万位 |
| 10 | 200 万 ÷ 198 万位（最慢点） |

几点说明：

- 9、10 两组考的是超大整数除法：MAL 走 Newton 倒数，按位相除的实现会随位数
  平方增长；
- 5 组考的是「长整数取对数」（只取前若干位有效数字配合位数计算）；
- 3、4、6、7、8 组以十进制读写为主，是后续继续优化的方向。

`data.zip` 的根目录只包含 `interactive_lib.cpp` 和 10 组 `.in` / `.ans`。

## 重新生成

```bash
python3 examples/T793310/generate_interface.py
CXX=g++ python3 examples/T793310/generate_data.py
```

`generate_data.py` 会用 `main.cpp` + `interactive_lib.cpp` 编译出参考程序，
再用它跑出每组答案，所以数据和参考程序永远一致；它还会打印每组数据的实测耗时，
方便确认每组的规模是否合适。

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
