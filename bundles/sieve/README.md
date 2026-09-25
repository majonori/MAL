# MAL 筛法模块

块筛（商块前缀上的狄利克雷卷积、杜教筛、PN 筛、Min25、二维 PN、SBT）
集中放在 `include/dgf/sieve.hpp` 与它引用的同目录头文件里。
所有运算采用 `mint<998244353>`；例外是 `D_x<1,1>(N)`，
它返回精确的 `unsigned __int128`。
**按《DGF 与块筛浅谈》目录整理的阅读顺序和实现状态见
[`GUIDE.md`](GUIDE.md)**；接口前提、复杂度来源与本机基准见
[`IMPLEMENTATION.md`](IMPLEMENTATION.md)，SBT 推导见 [`sbt-one-log.md`](sbt-one-log.md)，
原稿见 [`reference/`](reference)。

## 怎么用

这个模块带不少模板接口（`pn_block`、`prime_power_block`、`bell_via_*`、
`D_x<...>`、`pn2d_*`），所以**不走**“复制声明块 + 链接交互库”那条路，
三种正常用法是：

1. 直接 `#include "include/dgf/sieve.hpp"`；
2. 或者只用需要的头，例如 `#include "include/dgf/block_sieve.hpp"`；
3. 或者把压行后的 [`main.cpp`](main.cpp)（约 60 KB，自包含）与自己的代码
   一起提交——[`examples/P17465`](../../examples/P17465) 就是第三种用法的
   完整例子。

块前缀写法：

```cpp
#include "bundles/sieve/main.cpp"   // 或 #include "include/dgf/sieve.hpp"

using namespace mal;
int main() {
    const int N = 1000;
    dgf::QuotientGrid grid(N);                 // D(N) = {floor(N/i)}
    dgf::series one(N + 1);                    // f(1..N) = 1
    for (int i = 1; i <= N; ++i) one[i] = 1;
    dgf::BlockPrefix F(grid, one);             // F(x) = sum_{i<=x} f(i)
    dgf::BlockPrefix H = dgf::block_convolve(F, F);   // h = f * f
    std::cout << H.at(1000).v << '\n';         // sum_{n<=1000} d(n) = 7069
    std::cout << H.at(500).v << '\n';          // sum_{n<=500} d(n) = 3190
    std::cout << dgf::id_k_block(grid, 0).at(1000).v << '\n';  // sum 1 = 1000
    std::cout << (unsigned long long)dgf::D_x<1, 1>(1000) << '\n';  // 7069
    return 0;
}
```

两条容易踩的接口约定：

* `BlockPrefix::at(x)`、`BlockSums::block(x)` 只接受
  `D(N)={floor(N/i)}` 里的 `x`；换成别的下标会抛
  `std::invalid_argument`（先用 `grid.contains(x)` 判断，
  或只用 `grid.values()` 中的值）。
  `BlockPrefix::point(x)=at(x)-at(x-1)` 还要求 `x-1` 也在 `D(N)` 里，
  所以它只适合 `x<=sqrt(N)+1` 的小端点。
* 参与同一次运算的块对象必须引用**同一个** `QuotientGrid`，
  否则 `same_grid` 会直接抛异常。

| 接口 | 目标 | 组合阶段时间 | 组合阶段空间 |
| --- | --- | --- | --- |
| `block_convolve(F,G)` | 任意两侧商块前缀的狄利克雷卷积 | `O(N^(2/3))` | `O(sqrt(N))` |
| `block_convolve_zak(F,G)` | 对数桶多项式及整数边界补偿 | 依 NTT 桶宽和补偿区间 | `O(sqrt(N))` |
| `dujiao_zak(B,C)` | 只给两侧商块前缀的除法 | `O(N^(2/3))` | `O(sqrt(N))` |
| `id_k_block(grid,k)` | 整点幂和的块前缀 | 固定小 `k` 时 `O(sqrt(N))` | `O(sqrt(N))` |
| `prime_power_block(primes,bell)` | 已给全体质数前缀，补齐素数高次幂 | 与枚举素数幂的数目成正比 | `O(sqrt(N))` |
| `pn_block(grid,bell)` | 枚举零质数项的积性 PN 因子 | 依 Bell 因子的局部稀疏度 | `O(sqrt(N))` |
| `block_log(F)` | 一般块筛 ln：短前缀、exp、一次除法 | `O(log N)` 次块卷积 + 一次块除法 | `O(sqrt(N))` |
| `min25_prime_polynomial(grid,P)` | 质数上的多项式前缀 | 此版前向段约 `N^(3/4)/log N` | `O(sqrt(N))` |
| `prime_polynomial_via_ln_division(grid,P)` | 不经 Min25 的质数前缀：截断 ln、一次除法 | 每单项式通用 exp + 一次块筛除法 | `O(sqrt(N))` |
| `bell_via_ln_exp(grid,P,bell)` | 独立 ln–exp 两段构造积性和 | 另加通用 exp 的 `O(log N)` 次块卷积 | `O(sqrt(N))` |
| `bell_via_prime_prefix_hybrid(grid,P,bell,cut)` | 前向质数和 + 逆序小素数 / ln–exp | 依 `cut` 及前向段 | `O(sqrt(N))` |
| `D_x<1,1>(N)` | Farey／SBT 精确计算二维除数和 | `O(N^(1/3) log(N+2))` 算术操作 | `O(N^(1/6))` 栈 |
| `pn2d_sparse_direct(L,R,M,bell)` | 二维 PN，直接查询 | `O(N^(2/3) log N)` | `O(sqrt(N))` |
| `pn2d_sparse_offline(L,R,M,bell)` | 二维 PN，离线维护块内和 | `O(N^(2/3))` | `O(N^(2/3))` |

二维接口的 `bell(p,a,b)` 返回 `f(p^a,p^b)`；`L,R,M` 是
左右边际及局部除法后对角因子的**一维块前缀**，须使用同一
`QuotientGrid`。调用方负责生成这三份前缀；复杂度不包含其成本。
离线版的待查询项目前需要 `O(N^(2/3))` 内存，在 (10^{13})
规模下并非可直接投入有限内存评测的版本。空间受限时请使用
直接查询版，并保留其额外的对数因子。

独立 ln–exp 入口先从 `D_k(n)=n^k` 得到短对数前缀，
再用一次块筛除法取得剩余对数并扣除素数幂；不调用 Min25 的
正向筛。当前 exp 仍是通用实现，尚未加入 Zak 论文的小素数加速。
`D_x` 以指数元组命名：当前仅有 `<1,1>` 的专用实现，
返回 `unsigned __int128`，要求 GNU C++14、`N<=10^18`。
它统计 `D_{1,1}(N)=#{(a,b):ab<=N}`；
并未实现三维 `D_x<1,1,2>` 或 `D_x<1,1,1>`。
推导见同目录 `sbt-one-log.md`，其长文属于原稿，验证本包代码请运行
`tests/dgf/test_sbt_one_log.cpp`。
接口前提、复杂度来源与本机基准见同目录 `IMPLEMENTATION.md`。
测试 `tests/dgf/test_sieve_primitives.cpp` 使用一般非积性
`ln/exp`、直接幂和与 PN 点值对拍这些新增接口。
可编译验证：

```bash
g++ -std=c++14 -O2 tests/dgf/test_block_sieve.cpp -o /tmp/test_block && /tmp/test_block
g++ -std=c++14 -O2 tests/dgf/test_min25.cpp -o /tmp/test_min25 && /tmp/test_min25
g++ -std=c++14 -O2 tests/dgf/test_sieve_primitives.cpp -o /tmp/test_primitives && /tmp/test_primitives
g++ -std=c++14 -O2 tests/dgf/test_euler_product.cpp -o /tmp/test_euler && /tmp/test_euler
g++ -std=c++14 -O2 tests/dgf/test_sbt_one_log.cpp -o /tmp/test_sbt && /tmp/test_sbt
g++ -std=c++14 -O2 tests/dgf/test_pn2d.cpp -o /tmp/test_pn2d && /tmp/test_pn2d
g++ -std=c++14 -O3 tests/benchmark/bench_pn2d.cpp -o /tmp/bench_pn2d
/tmp/bench_pn2d 1000000000
g++ -std=c++14 -O3 tests/benchmark/bench_block_sieve.cpp -o /tmp/bench_block
/tmp/bench_block 10000000 1000
g++ -std=c++14 -O3 tests/benchmark/bench_block_algorithms.cpp -o /tmp/bench_blocks
/tmp/bench_blocks 10000000000000 hyperbola
```
