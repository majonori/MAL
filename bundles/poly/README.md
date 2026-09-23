# POLY

多项式卷积模块。源码在 `include/poly/`：

- `ntt.hpp`：模 `998244353` 的 NTT，DIF 正变换 + DIT 逆变换，全程保持位逆序；
- `fft.hpp`：`std::complex<double>` 的 FFT，接口与 NTT 完全对应。

题目的 `interactive_lib.cpp` 里已经包含本模块（`bundles/poly/main.cpp`
就是合并进交互库的那一份），选手**不需要**复制实现，
只要把下面这段声明复制到自己代码的最上面。

## 选手复制这一段

先复制 [`../common/README.md`](../common/README.md) 的声明块
（`mint` 是模板，必须带定义；`cpx`、`vector` 也在那段里），
再接上下面这段：

```cpp
namespace mal {
using std::valarray;

constexpr int NTT_MOD = 998244353;
using ntt_mint = mint<NTT_MOD>;

vector<ntt_mint>& ntt_init(int n);
valarray<ntt_mint> ntt_dif(const vector<ntt_mint>& src, int n);
vector<ntt_mint> ntt_dit(const valarray<ntt_mint>& src);
vector<ntt_mint> ntt_mul(const vector<ntt_mint>& a, const vector<ntt_mint>& b);
vector<ntt_mint> ntt_conv(vector<ntt_mint> a, const vector<ntt_mint>& b);

vector<cpx>& fft_init(int n);
valarray<cpx> fft_dif(const vector<cpx>& src, int n);
vector<cpx> fft_dit(const valarray<cpx>& src);
vector<cpx> fft_mul(const vector<cpx>& a, const vector<cpx>& b);
vector<cpx> fft_conv(vector<cpx> a, const vector<cpx>& b);
} // namespace mal
```

这些函数都是交互库里已经导出的符号，声明后直接调用即可。
（`ntt_init` / `fft_init` 会在其他接口里自动调用，一般不用手动调。）

## 用法示例

```cpp
int main() {
    mal::vector<mal::ntt_mint> a = {1, 2, 3}, b = {4, 5};
    for (auto x : mal::ntt_mul(a, b)) std::cout << x.v << ' ';
    std::cout << '\n';

    for (auto x : mal::ntt_conv({1, 2, 3}, {4, 5, 6, 7})) std::cout << x.v << ' ';
    std::cout << '\n';

    mal::vector<mal::cpx> p = {1, 2, 3}, q = {4, 5};
    for (auto x : mal::fft_mul(p, q)) std::cout << std::lround(x.real()) << ' ';
    std::cout << '\n';
    return 0;
}
```

输出：

```text
4 13 22 15
32 38
4 13 22 15
```

## 接口速查

记 `mal::ntt_mint = mal::mint<998244353>`，`mal::cpx = std::complex<double>`。

| 名字 | 说明 |
|---|---|
| `mal::ntt_dif(a, n)` | 正变换：系数 `vector<ntt_mint>` → `valarray<ntt_mint>`，输出位逆序 |
| `mal::ntt_dit(A)` | 逆变换：`valarray<ntt_mint>` → 系数 `vector<ntt_mint>`，输入位逆序 |
| `mal::ntt_mul(a, b)` | 普通卷积，长度 `a.size()+b.size()-1` |
| `mal::ntt_conv(a, b)` | 差卷积 `c[k] = sum_i a[i]*b[i+k]`，长度 `b.size()-a.size()+1`，要求 `a.size() <= b.size()` |
| `mal::fft_dif / fft_dit / fft_mul / fft_conv` | 同上，元素换成 `mal::cpx` |

`ntt_dif` 与 `ntt_dit` 都不做位逆序重排：正变换的输出直接喂给逆变换，
只有手写点值运算时才需要自己按位逆序取用。

## 复杂度与常数

- 卷积长度取不小于 `a.size()+b.size()-1` 的 2 的幂；
- 一次 `ntt_mul` 做两次正变换和一次逆变换，蝶形迭代按 `len` 从大到小（DIF）；
- 单位根表按需倍增缓存，`ntt_dif` / `ntt_dit` 会自动调用 `ntt_init`；
- FFT 版本用 `std::complex<double>`，精度不够时改用 NTT 版本。

## 测试

```bash
g++ -std=c++14 -O2 -I. tests/poly/test_poly.cpp -o test_poly && ./test_poly
```

随机小数据下与朴素卷积、差卷积逐项比对，并检查 DIF → DIT 能还原系数。
