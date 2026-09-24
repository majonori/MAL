# NUMBER THEORY

基本数论模块，源码在 `include/number_theory/`。

- `sieve.hpp`：线性筛、最小质因子、表内分解、欧拉函数与 Möbius 函数表；
- `primality.hpp`：64 位整数模乘、模幂与确定性 Miller–Rabin 素性测试。

可引用头文件；若题目提供 `interactive_lib.cpp`，在选手代码中复制声明：

## 选手复制这一段

```cpp
namespace mal { namespace number_theory {
using u64 = std::uint64_t;
struct Sieve {
    std::vector<int> spf, primes;
    explicit Sieve(int n);
    bool prime(int x) const;
    std::vector<std::pair<int, int>> factor(int x) const;
};
Sieve make_sieve(int n);
std::vector<int> totients(const Sieve& s);
std::vector<int> mobius(const Sieve& s);
u64 mul_mod(u64 a, u64 b, u64 mod);
u64 pow_mod(u64 a, u64 k, u64 mod);
bool is_prime(u64 n);
} }
```

`make_sieve(N)` 需要 `O(N)` 时间和空间，`s.prime(x)`、`s.factor(x)` 仅支持
`1 <= x <= N`（`prime(0)` 也可返回假）；越界会抛异常。
`is_prime` 支持整个 `uint64_t` 范围，不依赖筛表，每次至多 7 组 Miller–Rabin
底数。模乘采用 GCC/Clang 的 `unsigned __int128`；此模块目标是 C++14 GCC 环境，
与 MAL 原本的洛谷编译方式一致。`pow_mod(...,mod=0)` 返回 0，
`mul_mod` 要求 `mod>0`。

接口示例：

```cpp
int main() {
    auto s = mal::number_theory::make_sieve(100);
    std::cout << s.prime(97) << ' '
              << mal::number_theory::is_prime(2305843009213693951ULL) << '\n';
}
```
