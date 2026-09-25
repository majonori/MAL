# P17465 动态 Bell 级数：示例提交

这个目录放 `bundles/sieve/README.md` 里那套块筛接口的完整用法示例。
题目每轮会修改若干素数位置的 Bell 级数值，并要求随时输出所有块前缀的
异或和；`mal::dgf::DynamicBellBlock` 把“改一条级数”做成
按商块降序的原地更新，单次修改不超过 `O(sqrt(N) log_p N)`。

| 文件 | 说明 |
|---|---|
| `P17465.cpp` | 参考实现，直接引用 `include/dgf/dynamic_bell.hpp` |
| `P17465_submission.cpp` | 压行后的自包含提交版（把 `include/` 里的实现合并进一个文件） |

```bash
cd examples/P17465
g++ -std=c++14 -O2 P17465.cpp -o /tmp/p17465
g++ -std=c++14 -O2 P17465_submission.cpp -o /tmp/p17465_sub
```

两个程序读入格式相同（第一个数是 `N`，第二个是操作数），输出逐字节一致：

```text
N Q
（按素数、素数幂顺序给出初始 Bell 值）
1 p a_1 … a_e      # 把 p 的 Bell 级数改成 a_1 z + … + a_e z^e
2                  # 询问当前的块前缀异或和
```

返回类型提示：`answer()` 是 `std::uint64_t`（按 32 位模值异或），
与块筛的 `coef = mal::mint<998244353>` 不是同一个东西。
