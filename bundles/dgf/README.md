# DGF

Dirichlet 生成函数（数论函数）模块，模数固定为 `998244353`，兼容 C++14。
源码位于 `include/dgf/`，编译 `scripts/build.cpp` 后生成本目录的 `main.cpp`，
并将模块合入 `bundles/interactive_lib.cpp`。

**下标约定：**长度为 `N+1` 的数组 `f` 表示 `f(1),...,f(N)`，`f[0]` 必须为 0。
二元运算要求输入长度相同。输出仍有 `N+1` 项，按 `n<=N` 截断。
可直接引用头文件，也可在选手代码中复制下面的声明并与交互库链接。

## 选手复制这一段

先复制 [`../common/README.md`](../common/README.md#选手复制这一段) 中的 `mint` 定义，
然后复制以下**声明**（不含实现）：

```cpp
namespace mal { namespace dgf {
using coef = mint<998244353>;
using series = std::vector<coef>;

void divisor_zeta(series& a);
void divisor_mobius(series& a);
void multiple_zeta(series& a);
void multiple_mobius(series& a);
series lcm_convolution(series a, series b);
series gcd_convolution(series a, series b);

series mul(const series& a, const series& b);
series fast_mul(const series& a, const series& b);
series mul_multiplicative(const series& f, const series& g);
series mul_multiplicative_both(const series& f, const series& g);
series div_multiplicative_both(const series& f, const series& g);
series inv_multiplicative(const series& f);
series ln_multiplicative(const series& f);
series exp_prime_powers(const series& f);
series derivative(const series& f);
series integral(const series& f, coef constant = 0);
series inv(const series& f);
series div(const series& f, const series& g);
series ln(const series& f);
series exp(const series& f);
series pow_unit(const series& f, coef k);
series pow_int(const series& f, long long k);
series compose(const series& f, const std::vector<coef>& p);
series geometric_sum(const series& f, unsigned long long k);
series generalized_div(const series& f, const series& g, int n, bool verify = false);
} }
```

若要让后续运算自动沿用积性信息，再复制 `DGF` 类声明：

```cpp
namespace mal { namespace dgf {
class DGF {
public:
    enum class Kind { general, prime_power, multiplicative, completely_multiplicative };
    static DGF general(series f);
    static DGF prime_power(series f);
    static DGF multiplicative(series f);
    static DGF completely_multiplicative(series f);
    static DGF epsilon(int n);
    static DGF zeta(int n);
    static DGF identity(int n);
    static DGF phi(int n);
    static DGF mu(int n);

    const series& coefficients() const;
    Kind kind() const;
    int limit() const;

    DGF multiply(const DGF& rhs) const;
    DGF divide(const DGF& rhs) const;
    DGF inverse() const;
    DGF log() const;
    DGF exponential() const;
    DGF power(coef k) const;
    DGF integer_power(long long k) const;
    DGF derivative() const;
    DGF integral(coef constant = 0) const;
    DGF add(const DGF& rhs) const;
    DGF subtract(const DGF& rhs) const;
    DGF scale(coef k) const;
    DGF divisor_prefix() const;
    DGF divisor_difference() const;

private:
    series a_;
    Kind kind_;
    DGF(series f, Kind kind);
    bool is_mult() const;
};
} }
```

例如：`auto sigma = DGF::zeta(N).multiply(DGF::identity(N));` 会使用
积性函数的线性卷积；`auto f = DGF::phi(N).log().exponential();` 中，
`log()` 返回素数幂支撑类，`exponential()` 再恢复积性类。
`multiplicative(f)`、`completely_multiplicative(f)` 和 `prime_power(f)`
会在构造时以 `O(N)` 验证承诺。系数只能通过 `coefficients()` 取得只读引用，
避免改动数据后类型标签失效。加法、非单位数乘等会保守退化为一般类。

例：`series f={0,1,2,3}; auto g=mal::dgf::ln(f);`。
`ln(f)` 要求 `f[1]=1`；`exp(f)` 要求 `f[1]=0`。
`pow_unit(f,k)` 允许模意义下的分数指数，要求 `f[1]=1`。
`pow_int` 允许任意整数幂；负幂要求 `f[1]!=0`。
`compose(f,p)` 计算 `p[0]+p[1]f+...`，要求 `f[1]=0`。
`geometric_sum(f,k)` 计算 `1+f+...+f^k`，不限 `f[1]`。

`generalized_div(f,g,n)` 在 `g[1]=0` 时求前 `n` 项商。令 `d` 为 `g[d]!=0`
的最小下标，需要 `f,g` **至少包含到 `d*n` 项且等长**。默认假定商存在；
传 `verify=true` 会检查 `f[1..d*n]=g*h`，否则不兼容的输入也可能返回数组。

## 接口与复杂度

| 运算 | 复杂度 | 备注 |
|---|---:|---|
| `divisor_zeta/mobius`、`multiple_zeta/mobius` | `O(N log log N)` | 原地变换，分别对应除数与倍数偏序 |
| `gcd_convolution`、`lcm_convolution` | `O(N log log N)` | 包含截断数组全部贡献 |
| `mul`、`inv`、`div`、`ln`、`exp` | `O(N log N)` | 输入可为任意数论函数 |
| `fast_mul` | `O(N (log log N)^2)` | EI 正交卷积拆分；较小 `N` 常数可能较大 |
| `mul_multiplicative` | `O(N log log N)` | **必须保证** `g` 为积性函数且 `g[1]=1` |
| `mul_multiplicative_both`、`div_multiplicative_both`、`inv_multiplicative` | `O(N)` | 输入须是积性函数；类自动选择 |
| `ln_multiplicative`、`exp_prime_powers` | `O(N)` | 后者要求输入仅在素数幂处可能非零；类自动选择 |
| `derivative`、`integral` | `O(N)` | 按完全加性函数 `Omega(n)` 逐项操作；常数项需单独处理 |
| `pow_unit`、`pow_int`（单位） | `O(N log N)` | `pow_int` 非单位退化为 `O(N log N log k)` |
| `compose` | `O(N log^2 N)` | 最多计算 `floor(log2 N)` 次卷积 |
| `geometric_sum` | `O(N log N log k)` | 用二进制倍增，不依赖除法 |
| `generalized_div` | `O(d N log(dN))` | `verify=true` 另做一次逐项核验 |

类中的 `multiply/divide/inverse/log/exponential/power` 按标签选择上述路径。
一般函数相乘默认用直接卷积；独立的 `fast_mul` 尽管复杂度更低，
在目前的实际竞赛规模下常数仍较大，需要具体测量后使用。

这里 `Omega(n)` 是**含重数**素因子数，用于微分；`fast_mul` 中的排名
`omega(n)` 是**不含重数**的素因子数。模数必须是素数；上述指数、积分
使用的 `Omega(n)` 对 `n<=N` 必须在模数下非零（本库使用的正常数组规模满足）。

## 覆盖范围

涵盖形式 DGF 的常规运算：加、乘、除、求逆、对数、指数、幂，以及
`zeta` / `mu` / `phi` / `id` / `epsilon` 等基本函数；块筛等算法不在其中。

## 测试

```bash
g++ -std=c++14 -O2 scripts/build.cpp -o /tmp/mal_build
/tmp/mal_build --all include bundles
g++ -std=c++14 -O2 tests/dgf/test_dgf.cpp -o /tmp/mal_dgf_test
/tmp/mal_dgf_test
g++ -std=c++14 -O3 tests/benchmark/bench_dgf.cpp -o /tmp/mal_bench_dgf
/tmp/mal_bench_dgf 1000000
```

`tests/docs/check_declarations.py` 会把本文档的声明块抽出来真正编译、链接并运行。
