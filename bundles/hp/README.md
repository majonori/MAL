# HP

面向 OI 的高精度数模块。源码在 `include/hp/`：

- `bigint.hpp`：任意精度整数，符号绝对值存储，limb 为 32 位。
- `bigfloat.hpp`：二进制高精度浮点数，值为 `mantissa * 2^exponent`，支持 `+ - * / exp log`。

构建后可直接包含 `bundles/hp/main.cpp`。

## 示例

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

`BigFloat` 的精度按有效二进制位计，结果精度取两个操作数的最大值。需要改变精度时用
`set_precision` / `with_precision`。

输出 `p` 位二进制精度大约只保证 `p * log10(2)` 位十进制有效数字；`to_string(k)`
只是把当前二进制值按 `k` 位十进制打印，不会创造原本不存在的精度。

## 乘法分层

`BigInt` 会根据规模自动切换：

1. schoolbook：小数据；
2. Karatsuba：中等数据；
3. Toom-3：较大的平衡数据；
4. 精确 NTT：大数据。

NTT 使用两个 30 位左右模数，输入按 16 位拆分成数字，因此所得卷积系数远小于两个模数的乘积，不需要浮点误差估计。正变换和逆变换分别采用 DIF/DIT 蝶形，输入/输出保持位逆序，不在变换前后做位逆序重排。

## 精度与算法档位

下面用十进制位数描述精度；`1` 位十进制精度约等于 `3.321928` 个二进制位。
因此 `10^8` 位十进制精度约等于 `3.322e8` 个二进制位，单个数的尾数约
`41.5 MB`（不含向量管理开销）。若你说的“位”是二进制位，则 `10^8` bit
约等于 `3.01e7` 位十进制精度。

### 乘法

乘法后端按操作数规模自动且确定地切换，阈值集中在 `bigint.hpp` 顶部：

```text
min limbs <= 32                       -> schoolbook
max limbs >= 1024 且 min limbs >= 512 -> NTT（小模数对或大模数对）
min limbs >= 192 且 max <= 2 * min    -> Toom-3
其余                                   -> Karatsuba
```

平衡操作数时的近似精度档位如下：

| 档位 | 平衡操作数大约十进制位 | 算法 | 备注 |
|---|---:|---|---|
| A | `<= 3.1e2` | schoolbook | 小数据常数最小 |
| B | `3.1e2 – 1.8e3` | Karatsuba | 中等平衡/不平衡数据 |
| C | `1.8e3 – 1.0e4` | Toom-3 | 较大的平衡数据 |
| D | `1.0e4 – 5.05e6` | 小 NTT 双模 | `998244353` + `1004535809`，最多 `2^21` 个变换点 |
| E | `5.05e6 – 3.23e8` | 大 NTT 双模 | `2013265921` + `2281701377`，最多 `2^27` 个变换点 |
| F | `> 3.23e8` | 抛出 `length_error` | 当前 NTT 硬上限 |

两个模数都按 16 位数字拆分输入计算精确卷积，再通过 CRT 合并；因此不需要
估计浮点 FFT 的舍入误差。DIF 正变换输出位逆序，DIT 逆变换直接消费位逆序，
中间不做 bit-reversal 重排。

代码里可以用 `mal::BigInt::ntt_max_slots()` 和
`mal::BigInt::ntt_max_decimal_digits()` 查询当前后端硬上限。

### 速度与内存量级

下面是在 Apple M 系列 CPU 单线程、`-O2` 下的实测/外推量级，只用于判断算法
是否会在目标机器上“不可接受”，不是跨机器承诺：

| 单次乘法规模 | 算法 | 粗估时间 | 粗估峰值内存 |
|---|---|---:|---:|
| `1.6e5` 位十进制 | 小 NTT | `~2e-2 s` | `< 10 MB` |
| `5.8e6` 位十进制 | 大 NTT | `~8e-1 s` | `~80 MB` |
| `4.0e7` 位十进制 | 大 NTT | `~7 s` | `~5e2 MB` |
| `1.0e8` 位十进制 | 大 NTT | `~1.5e1 s` | `~1.2 GB` |
| `3.2e8` 位十进制 | 大 NTT | `~3e1 – 6e1 s` | `~3 – 4 GB` |

`10^8` 位十进制目标落在 E 档内，乘法不需要换算法，主要是内存和缓存压力。

### 加减乘除与 exp/log 的实际范围

| 运算 | 当前实际范围 | 说明 |
|---|---|---|
| `+ -` | 受内存限制，`1e8` 位级可用 | 线性扫描，单个数约 `41.5 MB` |
| `*` | 到 `3.23e8` 位十进制 | E 档大 NTT，`1e8` 位约十余秒 |
| `BigInt /` | 商宽超过 `2048` bit 且除数不小于 `8` limb 时自动走 Newton 倒数 | 其余情况保留 Knuth Algorithm D；`1e8` 位大商可运行 |
| `BigFloat /` | 精度 `>= 8192` bit 自动走 Newton 倒数 | 避免 Knuth 的二次复杂度；`1e8` 位量级约百秒级 |
| `exp` | 小精度级数；`>= 1<<16` bit 用 Newton + AGM | `1e8` 位可使用，时间受标量 NTT 和 AGM 常数影响 |
| `log` | 小精度级数；`>= 1<<14` bit 用 AGM/theta | `1e8` 位可使用；`pi` 与 `ln2` 按精度缓存 |

`log` 的大精度路径使用 Brent 的 AGM/theta 公式：

1. 将 `x` 乘以 `2^n`，把参数移到接近零的区段；
2. 用 `jtheta2^2`、`jtheta3^2` 和 AGM 计算 `-log(x*2^n)`；
3. 用 `n*log(2)` 修正回原参数。

`exp` 的大精度路径是 Newton 迭代
`y_{k+1} = y_k * (1 + x - log(y_k))`，其中每次 `log` 调用 AGM 后端。
平方根改用逆平方根 Newton 迭代，只依赖乘法。`pi` 用 Gauss-Legendre AGM
计算，`ln2` 由 `-log(2^-n)/n` 计算，两者都按精度缓存。
固定切换点是 `AGM_LOG_THRESHOLD = 1<<14` 和 `AGM_EXP_THRESHOLD = 1<<16`。
如需绕过自动阈值，可直接调用 `BigFloat::log_agm` /
`BigFloat::exp_newton_agm`；`BigFloat::pi(p)` 和 `BigFloat::ln2(p)`
可取得缓存的常数。

当前单线程实测量级：

| 运算 | 精度 | 时间 |
|---|---:|---:|
| `log` | `2^17` bit | `~4.2 s` |
| `log` | `2^19` bit | `~22 s` |
| `exp`（Newton+AGM） | `2^16` bit | `~3.5 s` |
| `BigFloat /` | `2^22` bit | `~1.1 s` |

整数侧的除法与开根也采用了精度倍增的 Newton 方案：

- `inv_mag` 递归计算定点倒数，`divmod_mag_recip` 用它完成大商除法；
- `sqr_mag` 在 NTT 路径上只做一次正变换，平方时复用点值；
- `BigInt::sqrt()` / `BigInt::nroot(k)` 提供整数平方根和整数 k 次根；
- NTT 按层缓存 twiddle 表，蝴蝶变换不再在内层循环更新 `w`；
- 逆变换的 `1/n` 也用预计算的 `2^{-k}` 缓存，避免重复快速幂；
- 递归过程中规模逐层减半，避免在全程最大规模上反复迭代。

当前 MAL 仍使用精确 NTT；给定 AVX2/FMA 复数 FFT 的经验可以继续接入为
大数据乘法后端，但需要为二进制 limb 设计拆分与舍入策略，避免直接复用
十进制 limb 的系数范围假设。

### 进制转换

`from_string` / `to_string` 已改为分治进制转换：

- 以 `10^9` 为一块；
- 预计算 `10^(9*2^k)` 的幂表；
- 大数转字符串时预计算固定点的 `2^M / 10^(9*2^k)` 倒数，并对同一层的所有节点复用；
- 字符串转大数时用左右半段乘幂后相加。

实测 `1e6` 位十进制尾数：`BigInt` 解析约 `0.3 s`、输出约 `5 s`；
`BigFloat` 解析约 `1.1 s`、输出约 `10.5 s`。`1e8` 位的十进制 I/O
可以运行，但仍有较大的常数；如果 I/O 成为瓶颈，应继续做 Newton 基数转换
或并行化分治。

总结：所有核心运算都已换成可扩展到 NTT 上界的算法路径。`1e8` 位十进制下，
乘法是十余秒级，`BigFloat /` 是百秒级，`exp/log` 与十进制 I/O 预计为
分钟到小时级；具体取决于内存带宽、是否使用 AVX/并行 NTT 以及常数缓存命中情况。

## 测试

```bash
g++ -std=c++14 -O2 -I. tests/hp/test_bigint.cpp -o test_bigint
g++ -std=c++14 -O2 -I. tests/hp/test_bigfloat.cpp -o test_bigfloat
```
