# HP

面向 OI 的高精度数模块。源码在 `include/hp/`：

- `bigint.hpp`：任意精度整数，符号绝对值存储，limb 为 32 位。
- `bigfloat.hpp`：二进制高精度浮点数，值为 `mantissa * 2^exponent`。

构建后可直接包含 `bundles/hp/main.cpp`。所有接口都位于
`namespace mal` 中，不向全局命名空间注入符号。

下面每个接口都按同一格式说明：

1. 函数声明；
2. 每个参数是什么；
3. 函数作用。

文档里的声明有两种写法：

- 带 `namespace mal { ... }` 包裹的块是命名空间作用域声明，可以直接复制；
- 形如 `BigInt::foo` 的条目是类成员，只能写在类定义内部，不能单独复制。

`BigInt` / `BigFloat` 的实现就在头文件里，通常做法是包含
`bundles/hp/main.cpp`。库内运算符是类内友元定义，本文给出签名完全一致的
命名空间作用域声明，两者指向同一个函数，复制后可以直接调用。

交互题的 `main.cpp` 与 `interactive_lib.cpp` 分属两个编译单元，选手拿不到类定义，
所以跨编译单元时不要复制类成员，改为复制文末
[洛谷交互题封装](#洛谷交互题封装) 中的薄接口块。

## BigInt

### `BigInt()`

声明（类成员）：

```cpp
BigInt();
```

参数：无。

作用：构造值为 `0` 的整数。

### `BigInt(long long x)`

声明（类成员）：

```cpp
BigInt(long long x);
```

参数：

- `x`：要转换的 64 位有符号整数。

作用：构造与 `x` 相等的整数。

### `BigInt(const std::string& s, int base = 10)`

声明（类成员）：

```cpp
explicit BigInt(const std::string& s, int base = 10);
```

参数：

- `s`：数字字符串，可带正负号。
- `base`：进制，取值范围 `[2, 36]`，默认十进制。

作用：按 `base` 进制解析字符串并构造整数。

### `BigInt::from_limbs`

声明（类成员）：

```cpp
static BigInt from_limbs(const std::vector<std::uint32_t>& v,
                         bool neg = false);
```

参数：

- `v`：低位在前的 32 位 limb 数组。
- `neg`：是否为负数，默认 `false`。

作用：直接使用已经分好的 limb 构造整数，常用于测试和底层接口。

### `BigInt::from_string`

声明（类成员）：

```cpp
static BigInt from_string(const std::string& text, int base = 10);
```

参数：

- `text`：数字字符串。
- `base`：进制，取值范围 `[2, 36]`。

作用：静态解析字符串并返回整数，不在原对象上操作。

### `BigInt::is_zero`

声明（类成员）：

```cpp
bool is_zero() const;
```

参数：无。

作用：判断整数是否等于 `0`。

### `BigInt::is_negative`

声明（类成员）：

```cpp
bool is_negative() const;
```

参数：无。

作用：判断整数是否为负数。

### `BigInt::sign`

声明（类成员）：

```cpp
int sign() const;
```

参数：无。

作用：返回符号：负数返回 `-1`，零返回 `0`，正数返回 `1`。

### `BigInt::limb_count`

声明（类成员）：

```cpp
std::size_t limb_count() const;
```

参数：无。

作用：返回当前 limb 数组的长度。

### `BigInt::limbs`

声明（类成员）：

```cpp
const std::vector<std::uint32_t>& limbs() const;
```

参数：无。

作用：返回低位在前的 limb 数组只读引用。

### `BigInt::bit_length`

声明（类成员）：

```cpp
std::size_t bit_length() const;
```

参数：无。

作用：返回整数的二进制有效位数；`0` 返回 `0`。

### `BigInt::bit`

声明（类成员）：

```cpp
bool bit(std::size_t i) const;
```

参数：

- `i`：从最低位开始的位下标。

作用：返回第 `i` 个二进制位。

### `BigInt::any_low_bits`

声明（类成员）：

```cpp
bool any_low_bits(std::size_t bits) const;
```

参数：

- `bits`：检查的低位位数。

作用：判断最低 `bits` 位中是否存在 `1`。

### `BigInt::low64`

声明（类成员）：

```cpp
unsigned long long low64() const;
```

参数：无。

作用：返回整数的低 64 位。

### `BigInt::abs`

声明（类成员）：

```cpp
BigInt abs() const;
```

参数：无。

作用：返回当前整数的绝对值。

### `BigInt::operator-`

声明（类成员）：

```cpp
BigInt operator-() const;
```

参数：无。

作用：返回当前整数的相反数。

### `BigInt::operator+`

声明（类成员）：

```cpp
BigInt operator+() const;
```

参数：无。

作用：返回当前整数本身。

### `BigInt::to_string`

声明（类成员）：

```cpp
std::string to_string(int base = 10) const;
```

参数：

- `base`：输出进制，取值范围 `[2, 36]`，默认十进制。

作用：返回当前整数的字符串表示。

### `BigInt::div_small`

声明（类成员）：

```cpp
BigInt div_small(std::uint32_t d, std::uint32_t* rem = nullptr) const;
```

参数：

- `d`：32 位正整数除数，不能为 `0`。
- `rem`：可选输出指针，用于返回余数。

作用：返回除以 `d` 的商；商向零截断，余数为非负的绝对值余数。

### `BigInt::divmod`

声明（类成员）：

```cpp
static void divmod(const BigInt& a, const BigInt& b,
                   BigInt& q, BigInt& r);
```

参数：

- `a`：被除数。
- `b`：除数，不能为 `0`。
- `q`：输出商。
- `r`：输出余数。

作用：计算整数除法，商向零截断，余数符号与被除数相同。

### `BigInt::sqr`

声明（类成员）：

```cpp
BigInt sqr() const;
```

参数：无。

作用：返回当前整数的平方。大数路径会复用 NTT 正变换。

### `BigInt::pow`

声明（类成员）：

```cpp
BigInt pow(unsigned long long e) const;
```

参数：

- `e`：非负指数。

作用：返回当前整数的 `e` 次幂。

### `BigInt::sqrt`

声明（类成员）：

```cpp
BigInt sqrt() const;
```

参数：无；当前整数不能为负。

作用：返回 `floor(sqrt(*this))`。

### `BigInt::nroot`

声明（类成员）：

```cpp
BigInt nroot(unsigned long long k) const;
```

参数：

- `k`：根次数，必须大于 `0`。

作用：返回 `floor((*this)^(1/k))`；当前整数不能为负。

### BigInt 算术运算符

声明（命名空间作用域，可直接复制）：

```cpp
namespace mal {

BigInt operator+(const BigInt& a, const BigInt& b);
BigInt operator-(const BigInt& a, const BigInt& b);
BigInt operator*(const BigInt& a, const BigInt& b);
BigInt operator/(const BigInt& a, const BigInt& b);
BigInt operator%(const BigInt& a, const BigInt& b);

} // namespace mal
```

参数：

- `a`：左操作数。
- `b`：右操作数；除法和取模时不能为 `0`。

作用：分别执行加、减、乘、向零截断除法、余数运算。

### BigInt 复合运算符

声明（类成员）：

```cpp
BigInt& operator+=(const BigInt& b);
BigInt& operator-=(const BigInt& b);
BigInt& operator*=(const BigInt& b);
BigInt& operator/=(const BigInt& b);
BigInt& operator%=(const BigInt& b);
```

参数：

- `b`：右操作数；除法和取模时不能为 `0`。

作用：原地执行对应运算并返回 `*this`。

### BigInt 位移运算符

声明（命名空间作用域，可直接复制）：

```cpp
namespace mal {

BigInt operator<<(const BigInt& a, std::size_t bits);
BigInt operator>>(const BigInt& a, std::size_t bits);

} // namespace mal
```

参数：

- `a`：操作数。
- `bits`：移动的二进制位数。

作用：返回左移或右移后的整数；右移是向零截断。

### BigInt 比较运算符

声明（命名空间作用域，可直接复制）：

```cpp
namespace mal {

int compare(const BigInt& a, const BigInt& b);
bool operator==(const BigInt& a, const BigInt& b);
bool operator!=(const BigInt& a, const BigInt& b);
bool operator<(const BigInt& a, const BigInt& b);
bool operator>(const BigInt& a, const BigInt& b);
bool operator<=(const BigInt& a, const BigInt& b);
bool operator>=(const BigInt& a, const BigInt& b);

} // namespace mal
```

参数：

- `a`：左操作数。
- `b`：右操作数。

作用：按整数大小进行比较；`compare` 返回 `-1`、`0`、`1`，
其余运算符返回对应的布尔值。

### BigInt 后端上限

声明（类成员）：

```cpp
static constexpr std::size_t ntt_max_slots();
static constexpr std::size_t ntt_max_decimal_digits();
static constexpr std::size_t karatsuba_threshold();
static constexpr std::size_t toom_threshold();
static constexpr std::size_t fft_threshold();
```

参数：无。

作用：返回当前编译期乘法后端的硬上限和切换阈值。

### BigInt 输出运算符

声明（命名空间作用域，可直接复制）：

```cpp
namespace mal {

std::ostream& operator<<(std::ostream& os, const BigInt& x);

} // namespace mal
```

参数：

- `os`：输出流。
- `x`：要输出的整数。

作用：以十进制形式输出整数。

## BigFloat

`BigFloat` 的精度单位是二进制有效位，值表示为
`mantissa * 2^exponent`。舍入方式为最近偶数。

### `BigFloat()`

声明（类成员）：

```cpp
BigFloat();
```

参数：无。

作用：构造值为 `0`、精度为 `DEFAULT_PRECISION` 的浮点数。

### `BigFloat(long long v, int p = DEFAULT_PRECISION)`

声明（类成员）：

```cpp
explicit BigFloat(long long v, int p = DEFAULT_PRECISION);
```

参数：

- `v`：初始整数值。
- `p`：目标二进制精度，必须大于 `0`。

作用：构造与 `v` 相等的浮点数。

### `BigFloat(const BigInt& v, int p = DEFAULT_PRECISION)`

声明（类成员）：

```cpp
explicit BigFloat(const BigInt& v, int p = DEFAULT_PRECISION);
```

参数：

- `v`：初始整数。
- `p`：目标二进制精度。

作用：将整数按指定精度转换为浮点数。

### `BigFloat(const BigInt& mantissa, long long exp2, int p)`

声明（类成员）：

```cpp
BigFloat(const BigInt& mantissa, long long exp2, int p);
```

参数：

- `mantissa`：尾数。
- `exp2`：二进制指数，值为 `mantissa * 2^exp2`。
- `p`：目标二进制精度。

作用：用尾数和二进制指数直接构造浮点数。

### `BigFloat::from_double`

声明（类成员）：

```cpp
static BigFloat from_double(double v, int p = DEFAULT_PRECISION);
```

参数：

- `v`：有限浮点数。
- `p`：目标二进制精度。

作用：将 `double` 转换为高精度浮点数；非有限值会抛出异常。

### `BigFloat(const std::string& text, int p = DEFAULT_PRECISION)`

声明（类成员）：

```cpp
explicit BigFloat(const std::string& text, int p = DEFAULT_PRECISION);
```

参数：

- `text`：十进制浮点字符串，支持小数点、正负号和 `e/E` 指数。
- `p`：目标二进制精度。

作用：解析十进制字符串并构造浮点数。

### `BigFloat::precision`

声明（类成员）：

```cpp
int precision() const;
```

参数：无。

作用：返回当前对象使用的二进制精度。

### `BigFloat::exponent`

声明（类成员）：

```cpp
long long exponent() const;
```

参数：无。

作用：返回二进制指数 `e`，满足 `value = mantissa * 2^e`。

### `BigFloat::mantissa`

声明（类成员）：

```cpp
const BigInt& mantissa() const;
```

参数：无。

作用：返回尾数的只读引用。

### `BigFloat::is_zero`

声明（类成员）：

```cpp
bool is_zero() const;
```

参数：无。

作用：判断值是否为零。

### `BigFloat::sign`

声明（类成员）：

```cpp
int sign() const;
```

参数：无。

作用：返回符号：负数返回 `-1`，零返回 `0`，正数返回 `1`。

### `BigFloat::abs`

声明（类成员）：

```cpp
BigFloat abs() const;
```

参数：无。

作用：返回绝对值。

### `BigFloat::operator-`

声明（类成员）：

```cpp
BigFloat operator-() const;
```

参数：无。

作用：返回当前浮点数的相反数。

### `BigFloat::operator+`

声明（类成员）：

```cpp
BigFloat operator+() const;
```

参数：无。

作用：返回当前浮点数本身。

### `BigFloat::order`

声明（类成员）：

```cpp
long long order() const;
```

参数：无。

作用：返回 `floor(log2(|x|))` 的近似阶码，用于比较数量级。

### `BigFloat::with_precision`

声明（类成员）：

```cpp
BigFloat with_precision(int p) const;
```

参数：

- `p`：目标二进制精度。

作用：返回按 `p` 位舍入后的副本，不修改原对象。

### `BigFloat::set_precision`

声明（类成员）：

```cpp
void set_precision(int p);
```

参数：

- `p`：目标二进制精度。

作用：原地改变精度并舍入当前值。

### `BigFloat::ldexp`

声明（类成员）：

```cpp
BigFloat ldexp(long long k) const;
```

参数：

- `k`：二进制指数增量。

作用：返回 `x * 2^k`，不会改变尾数有效位。

### `BigFloat::from_string`

声明（类成员）：

```cpp
static BigFloat from_string(const std::string& text,
                            int p = DEFAULT_PRECISION);
```

参数：

- `text`：十进制浮点字符串。
- `p`：目标二进制精度。

作用：静态解析字符串并返回浮点数。

### `BigFloat::to_double`

声明（类成员）：

```cpp
double to_double() const;
```

参数：无。

作用：转换为 `double`；超出范围时返回 `0` 或无穷大。

### `BigFloat::to_string`

声明（类成员）：

```cpp
std::string to_string() const;
std::string to_string(int digits) const;
```

参数：

- `digits`：输出的十进制有效位数。

作用：返回十进制字符串；无参数的版本根据当前二进制精度自动选择位数。

### BigFloat 算术运算符

声明（命名空间作用域，可直接复制）：

```cpp
namespace mal {

BigFloat operator+(const BigFloat& a, const BigFloat& b);
BigFloat operator-(const BigFloat& a, const BigFloat& b);
BigFloat operator*(const BigFloat& a, const BigFloat& b);
BigFloat operator/(const BigFloat& a, const BigFloat& b);

} // namespace mal
```

参数：

- `a`：左操作数。
- `b`：右操作数；除法时不能为 `0`。

作用：执行加、减、乘、除法，结果精度取两个操作数精度的最大值。

### BigFloat 复合运算符

声明（类成员）：

```cpp
BigFloat& operator+=(const BigFloat& b);
BigFloat& operator-=(const BigFloat& b);
BigFloat& operator*=(const BigFloat& b);
BigFloat& operator/=(const BigFloat& b);
```

参数：

- `b`：右操作数。

作用：原地执行对应运算并返回 `*this`。

### BigFloat 比较运算符

声明（命名空间作用域，可直接复制）：

```cpp
namespace mal {

int compare(const BigFloat& a, const BigFloat& b);
bool operator==(const BigFloat& a, const BigFloat& b);
bool operator!=(const BigFloat& a, const BigFloat& b);
bool operator<(const BigFloat& a, const BigFloat& b);
bool operator>(const BigFloat& a, const BigFloat& b);
bool operator<=(const BigFloat& a, const BigFloat& b);
bool operator>=(const BigFloat& a, const BigFloat& b);

} // namespace mal
```

参数：

- `a`：左操作数。
- `b`：右操作数。

作用：比较两个高精度浮点数；`compare` 返回 `-1`、`0`、`1`，
其余运算符返回对应的布尔值。

### `BigFloat::div_small`

声明（类成员）：

```cpp
BigFloat div_small(long long d, int p) const;
```

参数：

- `d`：小的整数除数，不能为 `0`。
- `p`：结果二进制精度。

作用：返回除以小整数的结果。

### `BigFloat::exp`

声明（类成员）：

```cpp
static BigFloat exp(const BigFloat& x);
```

参数：

- `x`：指数。

作用：返回 `exp(x)`；大精度时自动切换到 Newton + AGM 后端。

### `BigFloat::log`

声明（类成员）：

```cpp
static BigFloat log(const BigFloat& x);
```

参数：

- `x`：必须为正数。

作用：返回自然对数 `log(x)`；大精度时自动切换到 AGM/theta 后端。

### `BigFloat::sqrt`

声明（类成员）：

```cpp
static BigFloat sqrt(const BigFloat& x);
```

参数：

- `x`：必须为正数。

作用：返回平方根。

### `BigFloat::log_agm`

声明（类成员）：

```cpp
static BigFloat log_agm(const BigFloat& x);
```

参数：

- `x`：必须为正数。

作用：强制使用 AGM/theta 对数后端。

### `BigFloat::exp_newton_agm`

声明（类成员）：

```cpp
static BigFloat exp_newton_agm(const BigFloat& x);
```

参数：

- `x`：指数。

作用：强制使用 Newton + AGM 指数后端。

### `BigFloat::pi`

声明（类成员）：

```cpp
static BigFloat pi(int p);
```

参数：

- `p`：目标二进制精度。

作用：返回指定精度的圆周率，并按精度缓存。

### `BigFloat::ln2`

声明（类成员）：

```cpp
static BigFloat ln2(int p);
```

参数：

- `p`：目标二进制精度。

作用：返回指定精度的 `log(2)`，并按精度缓存。

### `BigFloat::pow`

声明（类成员）：

```cpp
static BigFloat pow(const BigFloat& x, long long e);
```

参数：

- `x`：底数。
- `e`：整数指数，可以为负数。

作用：返回 `x^e`；负指数通过倒数计算。

### BigFloat 输出运算符

声明（命名空间作用域，可直接复制）：

```cpp
namespace mal {

std::ostream& operator<<(std::ostream& os, const BigFloat& x);

} // namespace mal
```

参数：

- `os`：输出流。
- `x`：要输出的浮点数。

作用：按默认十进制位数输出浮点数。

## 自由函数

声明（命名空间作用域，可直接复制）：

```cpp
namespace mal {

BigFloat exp(const BigFloat& x);
BigFloat log(const BigFloat& x);
BigFloat sqrt(const BigFloat& x);
BigFloat pow(const BigFloat& x, long long e);

} // namespace mal
```

参数：

- `x`：输入值。
- `e`：整数指数。

作用：分别等价于调用 `BigFloat::exp`、`BigFloat::log`、
`BigFloat::sqrt`、`BigFloat::pow`。

## 使用示例

```cpp
#include "bundles/hp/main.cpp"
#include <iostream>

int main() {
    mal::BigInt a("123456789012345678901234567890");
    mal::BigInt b("98765432109876543210");
    std::cout << (a * b).to_string() << '\n';

    mal::BigFloat x("1.25", 512);
    mal::BigFloat y = mal::BigFloat::exp(x);
    mal::BigFloat z = mal::BigFloat::log(y);
    std::cout << z.to_string(80) << '\n';
}
```

## 洛谷交互题封装

洛谷的 `main.cpp` 和 `interactive_lib.cpp` 是两个独立编译单元，
`interactive_lib.cpp` 不会出现在选手源码目录。若希望选手直接使用
`BigInt` / `BigFloat` 的 `+ - * /`，可以在题目模板中预置
`include/remote/interface.hpp` 的声明块；对应的
`mal::remote::operator+ - * /` 在交互库内部调用
`mal::BigInt` / `mal::BigFloat` 实现。

下面这段是完整的接口声明块，可以直接复制到选手代码（或题目模板）里，
不需要包含交互库，也不需要复制 MAL 实现。块内容与
`include/remote/interface.hpp` 一致：

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

复制后即可直接写 `mal::BigInt` / `mal::BigFloat` 的 `+ - * /`：

```cpp
mal::BigInt x(a), y(b);
std::cout << (x * y + x).to_string() << '\n';

mal::BigFloat u("1.5", 512), v("2.5", 512);
std::cout << ((u + v) * v / u - v).to_string(40) << '\n';
```

完整提交模板与数据见 `examples/T793310/README.md`。

## 乘法分层

`BigInt` 会根据规模自动切换：

1. schoolbook：小数据；
2. Karatsuba：中等数据；
3. Toom-3：较大的平衡数据；
4. 精确 NTT：大数据。

NTT 使用两个 30 位左右模数，输入按 16 位拆分成数字。DIF 正变换和 DIT
逆变换保持位逆序，不在变换前后做位逆序重排。

## 精度与算法档位

下面用十进制位数描述精度；`1` 位十进制精度约等于 `3.321928` 个二进制位。
`10^8` 位十进制精度约等于 `3.322e8` 个二进制位。

乘法阈值集中在 `bigint.hpp` 顶部：

```text
min limbs <= 32                       -> schoolbook
max limbs >= 1024 且 min limbs >= 512 -> NTT
min limbs >= 192 且 max <= 2 * min    -> Toom-3
其余                                   -> Karatsuba
```

| 档位 | 平衡操作数大约十进制位 | 算法 | 备注 |
|---|---:|---|---|
| A | `<= 3.1e2` | schoolbook | 小数据常数最小 |
| B | `3.1e2 - 1.8e3` | Karatsuba | 中等平衡/不平衡数据 |
| C | `1.8e3 - 1.0e4` | Toom-3 | 较大的平衡数据 |
| D | `1.0e4 - 5.05e6` | 小 NTT 双模 | `998244353` + `1004535809` |
| E | `5.05e6 - 3.23e8` | 大 NTT 双模 | `2013265921` + `2281701377` |
| F | `> 3.23e8` | 抛出 `length_error` | 当前 NTT 硬上限 |

| 运算 | 当前实际范围 | 说明 |
|---|---|---|
| `+ -` | 受内存限制，`1e8` 位级可用 | 线性扫描 |
| `*` | 到 `3.23e8` 位十进制 | 大 NTT |
| `BigInt /` | 大商自动走 Newton 倒数 | 其余情况 Knuth D |
| `BigFloat /` | `>= 8192` bit 自动走 Newton 倒数 | 避免二次复杂度 |
| `exp` | `>= 1<<16` bit 用 Newton + AGM | 小精度用级数 |
| `log` | `>= 1<<14` bit 用 AGM/theta | 小精度用降幅级数 |

`pi` 与 `ln2` 按精度缓存。`sqrt` 使用逆平方根 Newton 迭代。

## 速度与内存量级

下面是在 Apple M 系列 CPU、单线程、`-O2` 下的实测或外推量级：

| 单次乘法规模 | 算法 | 粗估时间 | 粗估峰值内存 |
|---|---:|---:|---:|
| `1.6e5` 位十进制 | 小 NTT | `~2e-2 s` | `< 10 MB` |
| `5.8e6` 位十进制 | 大 NTT | `~8e-1 s` | `~80 MB` |
| `4.0e7` 位十进制 | 大 NTT | `~7 s` | `~5e2 MB` |
| `1.0e8` 位十进制 | 大 NTT | `~1.5e1 s` | `~1.2 GB` |
| `3.2e8` 位十进制 | 大 NTT | `~3e1 - 6e1 s` | `~3 - 4 GB` |

当前单线程实测量级：

| 运算 | 精度 | 时间 |
|---|---:|---:|
| `log` | `2^17` bit | `~4.2 s` |
| `log` | `2^19` bit | `~22 s` |
| `exp`（Newton+AGM） | `2^16` bit | `~3.5 s` |
| `BigFloat /` | `2^22` bit | `~1.1 s` |

## exp/log 后端

`log` 的大精度路径使用 Brent 的 AGM/theta 公式：

1. 将 `x` 乘以 `2^n`，把参数移到接近零的区段；
2. 用 `jtheta2^2`、`jtheta3^2` 和 AGM 计算 `-log(x*2^n)`；
3. 用 `n*log(2)` 修正回原参数。

`exp` 的大精度路径是 Newton 迭代
`y_{k+1} = y_k * (1 + x - log(y_k))`，其中每次 `log` 调用 AGM 后端。
平方根改用逆平方根 Newton 迭代，只依赖乘法。`pi` 用 Gauss-Legendre AGM
计算，`ln2` 由 `-log(2^-n)/n` 计算，两者都按精度缓存。
固定切换点是 `AGM_LOG_THRESHOLD = 1<<14` 和 `AGM_EXP_THRESHOLD = 1<<16`。

整数侧的除法与开根采用精度倍增的 Newton 算法：

- `inv_mag` 递归计算定点倒数，`divmod_mag_recip` 用它完成大商除法；
- `sqr_mag` 在 NTT 路径上只做一次正变换，平方时复用点值；
- NTT 按层缓存 twiddle 表，蝴蝶变换不再在内层循环更新 `w`；
- 逆变换的 `1/n` 也用预计算的 `2^{-k}` 缓存，避免重复快速幂；
- `BigInt::sqrt()` / `BigInt::nroot(k)` 提供整数平方根和整数 k 次根。

当前 MAL 使用精确 NTT；AVX2/FMA 复数 FFT 可以作为后续大数据乘法后端，
但需要为二进制 limb 设计拆分与舍入策略。

## 进制转换

`from_string` / `to_string` 使用分治进制转换：

- 以 `10^9` 为一块；
- 预计算 `10^(9*2^k)` 的幂表；
- 大数输出时预计算固定点倒数并复用；
- 字符串输入时递归合并左右半段。

## 测试

```bash
g++ -std=c++14 -O2 -I. tests/hp/test_bigint.cpp -o test_bigint
g++ -std=c++14 -O2 -I. tests/hp/test_bigfloat.cpp -o test_bigfloat
python3 tests/docs/check_declarations.py
```

最后一条把本文档里的声明块抽出来真正编译、链接并运行：交互题接口块作为
独立编译单元与 `bundles/interactive_lib.cpp` 链接，命名空间作用域声明块
与 `bundles/hp/main.cpp` 一起编译，并检查文档中的声明都存在于发布产物中。
