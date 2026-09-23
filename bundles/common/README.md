# COMMON

公共基础件。源码在 `include/common/`：

- `consts.hpp`：类型别名、`PI`、`glim`；
- `modint.hpp`：静态模数类模板 `mint<MOD>`。

题目的 `interactive_lib.cpp` 里已经包含本模块（`bundles/common/main.cpp`
就是合并进交互库的那一份），选手**不需要**复制实现，
只要把下面这段声明复制到自己代码的最上面。

## 选手复制这一段

```cpp
namespace mal {
using ll = long long;
using ull = unsigned long long;
using ld = long double;
using cpx = std::complex<double>;
using std::vector;
constexpr double PI = 3.141592653589793238462643383279502884;

int glim(std::size_t x);

template <int MOD>
struct mint {
    int v;
    mint(ll v_ = 0) : v(int(v_ % MOD)) { if (v < 0) v += MOD; }

    // 运算结果已经取过模，这里走"免取模"构造：否则每次蝶形都会多一次 % MOD
    friend mint operator+(mint a, mint b) {
        const int r = a.v + b.v;
        return mint(r >= MOD ? r - MOD : r, unchecked{});
    }
    friend mint operator-(mint a, mint b) {
        const int r = a.v - b.v;
        return mint(r < 0 ? r + MOD : r, unchecked{});
    }
    friend mint operator*(mint a, mint b) {
        return mint(int((ll)a.v * b.v % MOD), unchecked{});
    }
    mint& operator+=(mint b) { return *this = *this + b; }
    mint& operator-=(mint b) { return *this = *this - b; }
    mint& operator*=(mint b) { return *this = *this * b; }

    mint pow(ll k) const {
        mint r = 1, a = *this;
        for (; k; k >>= 1, a *= a) if (k & 1) r *= a;
        return r;
    }
    mint inv() const { return pow(MOD - 2); } // 要求 MOD 是质数
    bool operator==(mint b) const { return v == b.v; }
    bool operator!=(mint b) const { return v != b.v; }

private:
    struct unchecked {};
    mint(int v_, unchecked) : v(v_) {}
};
} // namespace mal
```

注意两点：

- `mint` 是模板，编译器必须看到定义才能实例化，所以上面的块里带着定义
  （题面会连这段定义一起给出）；
- `glim` 是交互库里已经导出的函数，只写声明即可链接。

## 用法示例

粘好上面那段之后，直接写：

```cpp
int main() {
    mal::mint<998244353> a = 3, b = 5;
    std::cout << (a + b).v << ' ' << (a * b).v << '\n';      // 8 15
    std::cout << mal::mint<998244353>(2).pow(10).v << '\n';  // 1024
    std::cout << a.inv().v << '\n';                          // 332748118
    std::cout << mal::glim(10) << ' ' << (int)(mal::PI * 1000) << '\n'; // 16 3141
    return 0;
}
```

## 接口速查

| 名字 | 说明 |
|---|---|
| `mal::ll` / `mal::ull` / `mal::ld` | `long long` / `unsigned long long` / `long double` 的别名 |
| `mal::cpx` | `std::complex<double>` 的别名，poly 模块的 FFT 用它 |
| `mal::vector` | `std::vector` 的别名 |
| `mal::PI` | `constexpr double` 圆周率 |
| `mal::glim(x)` | 不小于 `x` 的最小 2 的幂，用来给卷积定长度 |
| `mal::mint<MOD>` | 静态模数类；`MOD` 编译期确定，`mint<998244353> a = 3;` |

`mint<MOD>` 的成员：

| 成员 | 说明 |
|---|---|
| `.v` | 取模后的 `int` 值 |
| `+ - *` 及复合形式 | 模意义下的运算（除法用 `inv()`） |
| `.pow(k)` | 快速幂 |
| `.inv()` | 乘法逆元，要求 `MOD` 是质数 |
| `== !=` | 相等比较 |
