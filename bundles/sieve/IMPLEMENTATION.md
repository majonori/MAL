# 块筛与 Bell 级数（C++14，64 位商块）

本模块沿用 `mal::dgf::coef = mint<998244353>`；统一头文件为
`include/dgf/sieve.hpp`，也可按算法单独引用 `block_sieve.hpp`、
`min25.hpp`、`pn2d.hpp` 等。
按原稿《DGF 与块筛浅谈》目录排列的简明导读与实现状态见
`bundles/sieve/GUIDE.md`；原稿「块筛」部分只展开到
「杜教筛与块筛」，后续标题不可当作现成的算法说明。
`scripts/build.cpp include/dgf/sieve.hpp bundles/sieve/main.cpp`
生成只含筛法及其依赖的压行版。独立提交程序见
[`examples/P17465_submission.cpp`](examples/P17465_submission.cpp)。

## 块表示与卷积

`QuotientGrid grid(N)` 保存升序、去重的
`D(N)={floor(N/k):1<=k<=N}`；`grid.index(x)` 和 `BlockPrefix::at(x)`
只接受 `D(N)` 中的 `x`。设 `F(x)=sum_{i<=x}f(i)`，
`BlockPrefix(grid, f)` 从完整点值建表，`BlockPrefix(grid)` 可直接填块前缀。
所有参与同一运算的块对象须引用**同一个** `grid`。

统一入口为 `include/dgf/sieve.hpp`。本筛法包对照新版仓库
`main@5ef948e` 整理；`bundles/sieve/main.cpp` 是此入口生成的压行版，
而库原有的正式 DGF 文件及说明保持新版结构。

`BlockSums(grid)` 是统一的块内和：第 `j` 项存区间
`(D[j-1],D[j]]` 的和。`add_static(j,delta)` 对尚未查询的静态增量
是 `O(1)`；`add_block` 和 `prefix_at(x)` 在发生动态查询后由树状数组
提供 `O(log sqrt(N))` 操作；`to_prefix()` 把块内和转回块前缀。
静态卷积只用静态增量，树状数组不会参与内层循环。

**默认 `block_convolve(F,G)`** 只接受两个 `D(N)` 上的块前缀，
不要求积性，也不要求额外的稠密点值。把小×小贡献按
`a<=b<=floor(sqrt(N))` 分行，以目标商块合并连续的 `b`；
对角贡献一次，非对角贡献分别计入 `f(a)g(b)` 和 `f(b)g(a)`。
小×大的两半区由块端点的前缀差计算。行内商块组数满足

```text
sum_{a<=N^(1/3)} O(sqrt(N/a)) +
sum_{N^(1/3)<a<=sqrt(N)} O(N/a^2) = O(N^(2/3)).
```

小×大部分为 `O(sqrt(N) log N)`，包含在 `O(N^(2/3))`
渐近界内。整条路径使用 `O(sqrt(N))` 空间且**没有乘在主项上的 log**。
这个界来自商块分组，并不假设 `h=f*g` 具有积性。

`block_convolve(F,G,dense_h)` 是另一个双曲线接口：事先有
`h[1..T]` 的点值时，以如下公式补算大块：

```text
H(x) = sum_{a<=sqrt(x)} f(a) G(floor(x/a))
     + sum_{b<=sqrt(x)} g(b) F(floor(x/b))
     - F(floor(sqrt(x))) G(floor(sqrt(x))).
```

其块计算为 `O(N/sqrt(T))`；若取 `T≈N^(2/3)`，
输入自身却占 `O(N^(2/3))` 空间，不能冒充仅维护 `sqrt(N)` 项。
同理，点卷积的 `dense_f,dense_g` 重载依赖外部的点值数组。

`block_convolve_zak(F,G,scale)` 将小×小的边缘 `min(a,b)<=N^(1/3)`
精确分组，中央用 `ceil(scale*ln a)` 的桶进行多项式卷积，
再对对数取整漏掉的边界三元组进行分段分解补偿。
浮点桶只给出**单侧**近似，补偿按整数乘积和真实商块写入，
不是把浮点值当精确答案。默认 `scale≈12 sqrt(N)/ln N`，
多项式及全部分段工作区均为 `O(sqrt(N))` 大小。
本模数的 NTT 上限为 `2^23`，默认 scale 在必要时会缩小，
手动传入超过此限制的 scale 会报错，不会静默产生错误卷积。
`scale` 可以调速；过大的值也可能触发空间上限错误。
这个实现是另一条严格结果路径，实际快慢取决于数论函数、
变换常数及规模，以随包基准为准。

## 两条杜教路径

求 `A*B=C`，要求 `B(1) != 0`；两种算法都容许三个函数**非积性**；其中无稠密输入的重载只有 `O(sqrt(N))` 工作空间：

| 路径 | 接口 | 已知点值 | 大块递推 |
| --- | --- | --- | --- |
| 传统直接求逆 | `dujiao_direct(B,C,dense_a)` | `A[1..T]` | 按 `floor(x/k)` 分组，直接减去 `B(k) A(x/k)` |
| Zak 短尾求逆 | `dujiao_zak(B,C,dense_b,dense_c)` | `B[1..T]`、`C[1..T]` | 先令 `A0=A[1..T]`，算 `B*A0` 的块卷积；余量 `A1` 只枚举 `k<=x/(T+1)` |

第一种带稠密输入的大块循环 `O(N/sqrt(T))`；无稠密输入的 `dujiao_direct(B,C)` 在 `T=sqrt(N)` 时为 `O(N^(3/4))`；无稠密输入的 `dujiao_zak(B,C)` 借精确卷积达到 `O(N^(2/3))`。第二种需要一次上述块卷积，
另有 `O(T log T)` 的普通点值除法与短尾递推。
带 `dense_*` 的两个旧重载要求 `T>=sqrt(N)`，点值与传入的块前缀一致；
纯块前缀重载没有这份稠密输入。

## Min25 的对偶两段与独立的 ln–exp 路径

从整点幂和出发，Min25 第一段按小素数**从小到大消去**
含该素数的合数贡献，得到 `sum_{p<=x}P(p)`；第二段以
大素数的因子为初值，把小素数 Bell 因子**从大到小乘回**。
两段在方向和贡献上对偶：前者剔除以定位质数，后者附加
以还原积性函数；素数幂项属于后者，不能把质数幂和误称为
完整的积性函数筛。分界到 `N^(1/6)` 后处理中素数的
ln–exp、再逆序加入小素数的思路见
[negiizhao 的讨论](https://negiizhao.blog.uoj.ac/blog/7165)；
下面的混合 API 仅实现这一拆分的通用正确性结构，尚未实现
文中对中素数稀疏贡献的加速。

`min25_prime_polynomial` 提供普通第一段；`reverse_euler_product`
提供按因子最小支撑逆序的第二段，允许一般的 `1+A_i`，但调用者
必须给出已经合并的大因子块筛。`block_exp` 提供另一条路径：
`prod_i(1+A_i)=exp_*(sum_i ln_*(1+A_i))`。积性 Bell 的
`bell_via_prime_prefix_exp` 以 **Min25 求得的质数前缀**及素数幂对数构造输入；
它是组合接口，不能称为独立的 ln–exp 筛。

真正独立的实现见 `ln_exp.hpp`：对每个单项式 `P(p)=p^k`，
从整点函数 `D_k(n)=n^k` 出发，`Q_k=ln_*(D_k)` 满足
`Q_k(p^e)=p^(ke)/e`。已知截断到 `sqrt(N)` 的 `Q_0`，
先求 `H=exp_*(Q_0)`，**一次块筛除法**给出
`D_k/H=epsilon+(Q_k-Q_0)`：尾部两项相卷积已超过 `N`。
去掉 `e>=2` 的素数幂项即可得到质数处的单项式前缀，
线性组合后再附加目标 Bell 因子的素数幂对数，最后做 `exp_*`。
`prime_polynomial_via_ln_division` 与 `bell_via_ln_exp` 都不调用
Min25 正向消去；复用的只是块卷积、除法、幂和等底层接口。
这是 ln 的「短前缀 + 一次除法」和 exp 的重新组合两部分；
Zak 论文 4.1 节还讨论在 exp 阶段暂去小素数再逐个加入以加速，
目前 `bell_via_ln_exp` 仍用通用的 `O(log N)` 次块卷积求 exp。
目前通用 `A_i` 的大因子块筛需要调用方构造，并没有宣称
自动实现任意广义 Min25。`min25_prime_polynomial` 支持 64 位商块，
幂和插值会处理超过模数的整段周期；但当前前向消去的朴素实现
约为 `O(N^(3/4)/log N)`，在 (10^{13}) 直接运行会很慢。
若要求整条筛法严格达到 `N^(2/3)`，前向段还需相应的数据结构优化。

小素数用逆序 Bell 因子、其余用 ln–exp 的结合形式见
`bell_via_prime_prefix_hybrid(grid,P,bell,cut)`；`cut=0` 时
仍由 Min25 第一段提供素数前缀，再接 ln–exp，绝非独立的
ln–exp 路径；`cut=floor(sqrt(N))` 时则直接构造
大素数初值并逆序附加小素数 Bell 因子。其他阈值的块指数
用 `O(log N)` 次精确块卷积，
虽然每次只存 `O(sqrt(N))` 个值，这个受限 Min25 + ln–exp 实现
**并不宣称整体 `O(N^(2/3))` 时间**。一般因子的逆序路径也需要
把具体稀疏度代入下面的成本式才能得到相应时间界。

### 反向 Euler 乘积的接口和复杂度前提

`euler_product.hpp` 的 `reverse_euler_product(large_tail,small,tail_support)`
输入已算好的大因子块筛与按最小支撑严格升序排列的小因子；
每个 `EulerFactor` 是 `1+sum_{m>=2}a_m[m]`，其中项按 `m` 升序。
`large_tail(1)=1`，且在 `[2,tail_support)` 没有值；`tail_support`
严格大于最后一个小因子的最小支撑，至多 `floor(sqrt(N))+1`。
标准积性函数可取小因子为素数的 Bell 级数，大因子初值为
`1+sum_{p>sqrt(N)}f(p)[p]`；一般 `1+A_i` 也可使用相同接口，
但必须单独算出大因子块筛。若某个因子含下标 1 的额外系数，
应先处理其常数项；接口仅接受单位常数项。

状态是 `D(N)` 相邻端点之间的**块内和**，树状数组提供当前旧状态的
`F(floor(x/m))` 查询。处理同一因子的各项时先收集全部增量，
再统一写入，避免把一个因子错误地平方。每个项在阈值 `B` 下枚举
`next_support..B` 的非零可能源项，随后只枚举上端点不小于
`m(B+1)` 的目标块。时间上界是
`O(|D(N)|+sum_{i,m}(max(0,B-next_support+1)+N/(m(B+1))+1)log|D(N)|)`；
**不能**对任意数量、任意稠密的 `A_i` 宣称 `O(N^(2/3))`。
文稿中特定的素数分布与稀疏因子需要代入此和式后才得到相应界。
测试 `tests/dgf/test_euler_product.cpp` 对拍了非积性因子与 Möbius 函数。

## P17465 动态 Bell 级数

`DynamicBellBlock(std::move(sieve),std::move(prime_powers))` 接收
`Sieve(N)` 与长度 `N+1` 的素数幂点值表，构造后保存块前缀和题目要求的 XOR。
`update(p,{f(p),f(p^2),...})` 修改一整条 Bell 级数；
`answer()` 常数时间读询问结果，`block()` 取得所有块前缀。
调用方只须在素数幂位置填 `prime_powers`，输入保证 `p` 是素数。

设旧/新 Bell 级数为 `U_p(z)`、`V_p(z)`，先求
`R_p(z)=V_p(z)/U_p(z)=1+sum_j r_j z^j`。于是对每个块：

```text
F_new(x) = F_old(x) + sum_{j>=1,p^j<=x} r_j F_old(floor(x/p^j))
```

按 `x` **降序**原地更新，就能直接使用尚未修改的小块。
单次修改最多 `O(sqrt(N) log_p N)`，单次询问 `O(1)`；
预处理时间 `O(N log log N)`，峰值内存约三份 `N+1` 的 32 位数组。
`N=10^7`、连续更新 `p=2` 1000 次的本机 `-O3` 基准：
初始化 `0.198s`，更新 `0.155s`。这是本机微基准，评测机器会有差异。

运行对拍及基准：

```bash
g++ -std=c++14 -O2 tests/dgf/test_block_sieve.cpp -o /tmp/test_block && /tmp/test_block
g++ -std=c++14 -O2 tests/dgf/test_min25.cpp -o /tmp/test_min25 && /tmp/test_min25
g++ -std=c++14 -O2 tests/dgf/test_sieve_primitives.cpp -o /tmp/test_primitives && /tmp/test_primitives
g++ -std=c++14 -O3 tests/benchmark/bench_block_sieve.cpp -o /tmp/bench_block
/tmp/bench_block 10000000 1000
```

## 方法全貌与实际规模

| 路径 | 已知信息 | 主时间量级 | 工作空间 | 主要常数来源 |
| --- | --- | --- | --- | --- |
| 商块双曲线卷积 | 任意两侧块前缀 | `N^(2/3)` | `sqrt(N)` | 商块查询和模乘 |
| Zak 多项式卷积 | 任意两侧块前缀 | 多项式变换 + 边界补偿 | `sqrt(N)` | NTT、分段分解 |
| Min25 两段 | 素数处的低次式、局部因子 | 依具体因子 | `sqrt(N)` 加输出 | 质数消去、反向因子 |
| 独立 ln–exp | 整点幂和、质数处多项式与 Bell 因子 | 每单项式 `log N` 次块卷积、一次块除法，再整体 exp | `sqrt(N)` | 除去短前缀、重构对数、指数 |
| 杜教直接递推 | `B,C` 块和 | `N^(3/4)` | `sqrt(N)` | 商值分组 |
| 杜教短尾 | `B,C` 块和 | `N^(2/3)` | `sqrt(N)` | 一次卷积与残差递推 |

## 朴素二维 PN 筛

`pn2d.hpp` 接受二维 Bell 回调 `bell(p,a,b)=f(p^a,p^b)`，
并要求调用方提供三条**一致**的一维块前缀 `L,R,M`：

```text
L_p(x)=sum_{a>=0}f(p^a,1)x^a,
R_p(y)=sum_{b>=0}f(1,p^b)y^b,
H_p(x,y)=F_p(x,y)/(L_p(x)R_p(y)),
M_p(z)=sum_{k>=0}H_p[k,k]z^k.
```

`pn2d_local_sparse` 负责 Bell 局部除法，得到
`H'_p=H_p/M_p(xy)` 的非零项，仅可能有 `a,b>=1,a!=b`。
`pn2d_sparse_direct(L,R,M,bell)` 直接枚举这些稀疏项，
并对每项以双变量整除分块求矩形前缀：时间
`O(N^(2/3) log N)`、额外空间 `O(sqrt(N))`。

`pn2d_sparse_offline(L,R,M,bell)` 是无主项 log 的离线版：
按 `m` 维护
`S_p(x,m)=sum_{k<=x} M(k)R(floor(m/k))`（另一方向交换 L/R），
将小 `k` 的值和大 `k` 的商块值存入同一个 `BlockSums`。
块增量在相邻 `m∈D(N)` 间用定长日历段枚举；
小下标直接更新，较大下标只在跨过倍数时更新。
日历段与状态是 `O(sqrt(N))` 大小。
`BlockSums::prefix_fast(x)` 使用按下标倍增的常数层分块，
单次更新 `O(1)`，查询 `O(x^(1/8)+log x)`；
结合稀疏项计数，总算术时间 `O(N^(2/3))`。

**离线版目前把待查询的 `h'` 项按 `m` 分桶，因此峰值空间
是 `O(N^(2/3))`，尚未达到此前讨论的全程 `O(sqrt(N))` 空间目标。**
若空间约束优先，可调用直接版；不要将其时间写成严格无 log。
二维筛需要的三条一维块前缀由调用方提供，其生成成本需另算；
本文仅对二维组合阶段给出复杂度。用户所附手稿后半的
`N^(11/18)` 设想并未由这份实现证明或实现。

`tests/dgf/test_pn2d.cpp` 对拍一般非对称二维 Bell 值与二维暴力；
`tests/benchmark/bench_pn2d.cpp` 使用解析一维前缀单独测组合阶段。

## SBT：D_x 的首个专用实现

`sbt_one_log.hpp` 暴露 `D_x<1,1>(N)`，按指数元组的记法为
`D_{1,1}(N)=#{(a,b)∈Z_{>0}²:ab<=N}`。
源码不是通用的 `D_x` 算法：其他指数元组目前没有特化，
尤其不能把二维操作数界套到 `D_{1,1,2}` 或 `D_{1,1,1}` 上。
实现使用 Farey 中项递归、整行整列剥离、每段固定分母截止，
完整推导与所需精确平方根模型见 `bundles/sieve/sbt-one-log.md`。
在定长整数算术（含精确开方）计为 `O(1)` 的模型中，
时间 `O(N^(1/3) log(N+2))`，递归栈 `O(N^(1/6))` 个 word。
当前 GNU C++14 实现用 `__int128` 和浮点初值加整数修正取得精确根，
输入约束 `N<=10^18`，答案类型为 `unsigned __int128`。
`tests/dgf/test_sbt_one_log.cpp` 使用独立整除分块解法对拍。

这些是可独立选择的内核；不同目标函数、可得的点值和
机器上的 NTT 常数会决定最快路径。`QuotientGrid` 使用 64 位
上界，允许 `N<=10^14`；动态 Bell 的全长点值状态属于另一类
在线问题，不适用静态 `sqrt(N)` 空间界。

本机 `-O3`、模 `998244353`，以解析前缀 `f(k)=1, g(k)=k`
为输入的实测（峰值是整个进程的 RSS，含两侧输入和商块表）：

| `N` | 精确商块卷积 | Zak 多项式卷积 | 峰值 RSS（商块 / Zak） |
| ---: | ---: | ---: | ---: |
| `10^12` | `11.55 s` | `18.72 s` | `70 / 154 MiB` |
| `10^13` | `58.48 s` | `109.57 s` | `219 / 411 MiB` |

两列在 `10^13` 均得到独立整除分组验证的 `458525610`。
这份 Zak 实现**目前没有实现三倍提速**：瓶颈主要是
单模数 `2^23` NTT 长度限制引起的 scale 缩小，及近边界分段分解。
比较算法不能只凭复杂度或其他实现的性能代替本机实测。

大规模基准会对 `f(n)=1, g(n)=n` 的解析块前缀运行三种卷积，
并用独立的整除分组公式验证最终答案：

```bash
g++ -std=c++14 -O3 tests/benchmark/bench_block_algorithms.cpp -o /tmp/bench_blocks
/tmp/bench_blocks 10000000000000 hyperbola
/tmp/bench_blocks 10000000000000 zak
```
