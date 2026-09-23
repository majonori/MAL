# T793310 MAL 接口测试

完全不懂 C++ class / namespace 的选手请先看
[`../../TUTORIAL.md`](../../TUTORIAL.md)：那里用三步说明白要把哪一段复制到哪。

本目录提供洛谷交互题接口与数据：

- MAL 核心实现留在 `interactive_lib.cpp` 内部；
- 向选手暴露一个很薄的值包装接口；
- 选手不需要 include 交互库，也不需要复制 MAL 实现；
- 选手可以直接使用 `mal::BigInt`、`mal::BigFloat` 的 `+ - * /`，
  以及 `std::cout << x` 输出。

## 选手代码接口

洛谷题目模板中预置以下声明，选手只写 `main()`：

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

注意：必须在选手源码中看到这段声明。`interactive_lib.cpp` 只参与链接，
不会出现在选手源码目录。如果提交时只写 `main()`，就会报：

```text
错误：'mal' 未声明
```

如果题目没有配置代码模板，请把下面的接口块一起复制到提交代码中。

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

    std::string a, b;
    std::cin >> a >> b;

    mal::BigInt x(a), y(b);
    std::cout << (x + y) << '\n';
    return 0;
}
```

## 选手示例

```cpp
#include <bits/stdc++.h>

// 这里放入上面的接口声明块

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

也可以直接使用 BigFloat：

```cpp
mal::BigFloat x("1.5", 256), y("2.5", 256);
mal::BigFloat z = (x + y) * y / x - y;
std::cout << z << '\n';                 // 按精度自动选择位数
std::cout << z.to_string(20) << '\n';   // 也可以自己指定 20 位
```

## 交互库

`interactive_lib.cpp` 由 `generate_interface.py` 复制 MAL 的标准发布产物：

- MAL 核心实现仍在 `namespace mal` 中；
- `include/remote/` 提供 `mal::remote` 薄包装类型；
- `mal::remote::operator+ - * /` 是非 inline 导出符号；
- 选手通过 `using remote::BigInt` / `using remote::BigFloat` 使用。

重新生成：

```bash
python3 examples/T793310/generate_interface.py
```

检查符号：

```bash
g++ -std=c++14 -O2 -c examples/T793310/interactive_lib.cpp \
    -o /tmp/t793310.o
nm -C /tmp/t793310.o | grep 'mal::remote::operator'
```

## 数据

本目录包含 10 组数据：

```text
1.in  1.ans
2.in  2.ans
...
10.in 10.ans
```

打包后的数据文件为 `data.zip`，上传洛谷即可。压缩包根目录只包含：

```text
interactive_lib.cpp
1.in 1.ans
...
10.in 10.ans
```

## 本地测试

```bash
cd examples/T793310

g++ -std=c++14 -O2 interactive_lib.cpp main.cpp -o main
for i in $(seq 1 10); do
    ./main < "$i.in" | diff - "$i.ans"
done
```
