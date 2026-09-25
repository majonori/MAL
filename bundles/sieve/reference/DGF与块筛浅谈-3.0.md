# 「DGF」与「块筛」浅谈

# 前言

>本文仍在撰写中，目前更新至「块筛 - 块筛卷积 - 杜教筛与块筛」部分。  
>先发布是为了避免我鸽了，或者我~~暴毙~~跑路了，欢迎关注后续更新。

自从周康阳提出了 [块筛卷积](https://www.cnblogs.com/zkyJuruo/p/17544928.html) 后，  
数论函数求和问题在理论上也迎来了一个爆发。  
本文较为详细讨论了「DGF」和「块筛」的内容。

由于作者已经高三，时间紧迫且知识储备堪忧，所以定有所疏漏。  
又由于作者的兴趣所在，所以并不会考虑太多 OI，本文主要是理论。  
本文会给出尽可能多的可供参考的连接，欢迎私信补充。  

我受《复分析：可视化方法》影响颇深，因此这篇文章可能会别具一格：  
里面可能有很多邪门乃至大逆不道的看法、做法，不过这是我故意保持的。  
而且会有很多我感兴趣的偏门的内容，您可以先大致浏览一遍。  
感兴趣某内容时可以尝试使用 `CTRL+F` 再仔细阅读。

此文的节奏非常快，而且默认了您有一定的前置知识。  
但愿我能给出一些比较宏观的看法，希望能助大家理解动机，距本质更进一步。  

# DGF

暴论：  
作用上来说：DGF 的用处就是用来**推式子**和**预处理**。  
思想上来说：DGF 就是**质数指数表示法**下的**多元多项式**。

下文中我会一直提到多元多项式的视角，不过其作为脚手架也是有局限性的。  
我建议您平时使用时还是用更标准的符号，下文的符号只是辅助你理解。  

这一部分的内容一定程度上也借助了 EI 的**生成函数计算理论框架**，  
详见 李白天 21 年 的集训队论文《信息学竞赛中的生成函数计算理论框架》。  
其实他原本估计也想讲这一部分的，他的 TeX 源码已经明示了：

```TeX
% \section{狄利克雷生成函数}

% \begin{definition}
% 令 $I = \mathbb Z \cap [0, n]$，对于 $i,j\in \IZ$，令
% $$
% i\circ j = \begin{cases}
% ij & ij \le n\\
% 0 & \mathrm{else}
% \end{cases}
% $$

% 定义在 $(I,\circ)$ 上的生成函数即为我们计算时取的狄利克雷生成函数（DGF）。
```

但很可惜，不知道什么原因将其注释掉了，那就由我斗胆补充罢（

## 乘法卷积

我们知道加法卷积，$x^n$ 在指数上具有相加的特征，  
具体的就是 $x^ax^b=x^{a+b}$。  
我们用此做形式符号，就有 [普通生成函数](https://oi-wiki.org/math/poly/ogf/) (OGF)：
$$F(x)=\sum_{i\ge 0}f(i)x^i$$
有时也会记为：
$$F=\sum_{i}f_ix^i$$
卷积时会满足：
$$[x^n]F*G=\sum_{a+b=n}f_ag_b$$
这体现了「下标之和为定值」的组合特征，故称为加法卷积。  
*※ 这也是我为什么不写成更常见的 $f_ig_{n-i}$ 的原因。*

---

如果我们定义 $\mathcal S^a\mathcal S^b=\mathcal S^{ab}$，  
用此形式符号，就有 [Dirichlet 生成函数](https://oi-wiki.org/math/poly/dgf/) (DGF)：
$$\tilde{F}(\mathcal S)=\sum_if_i\mathcal S^i$$

卷积时会满足乘积为定值的特征（乘法卷积）：
$$[\mathcal S^n]F*G=\sum_{ab=n}f_ag_b$$

*※ 如同普通多项式，我们一般也可以省略 $*$ 符号。*

但实际上在数学中我们已经有类似的形式符号了，  
即 $a^xb^x=(ab)^x$，但由于历史渊源，DGF 真正的定义是：
$$\tilde{F}(s)=\sum_{n\ge 1}\frac{f_n}{n^s}$$

*※ 后文就会当做这两种形式是等价的，当做 $\mathcal S^n:=n^{-s}$。*

## 高维视角

注意到数论中经常会用到「质数指数表示法」：  
记第 $i$ 个质数为 $p_i$，例如 $p_1=2,p_2=3$。  
那么任意一个数定可以唯一分解为：
$$a=p_1^{a_1}p_2^{a_2}p_3^{a_3}\dots$$

对应的**指数向量**就是：
$$\bm a=\langle a_1,a_2,a_3,\dots\rangle$$

注意到： $ab\Leftrightarrow\bm a+\bm b$。  
*※ 后文会经常用到这个性质，将不再说明。*

于是：
$$\tilde{F}(x)=\sum_{\bm i}f_{\bm i}x^{\bm i}$$

由于高维向量每一维的正交性，我们可以将向量拆成多元多项式：
$$\tilde{F}(x)=\sum_{\bm i}f_{\bm i}x_1^{i_1}x_2^{i_2}x_3^{i_3}\dots$$

---

另一种角度是我们从标准形式也能推出这个结果：
$$\tilde{F}(s)=\sum_{i\ge 1}\frac{f_i}{i^s}=\sum_{\bm i}\frac{f_i}{(p_1^{i_1}p_2^{i_2}p_3^{i_3}\dots)^x}=\sum_{\bm i}f_i(p_1^{-si_1}p_2^{-si_2}p_3^{-si_3}\dots)$$
$$\tilde{F}(\mathcal S)=\sum_{i\ge 1}f_i\mathcal S^i=\sum_{\bm i}f_i\mathcal S^{p_1^{i_1}p_2^{i_2}p_3^{i_3}}=\sum_{\bm i}f_i(\mathcal S^{p_1^{i_1}}\mathcal S^{p_2^{i_2}}\mathcal S^{p_3^{i_3}}\dots)$$

于是定义 $x_k=p_k^{-s}$ 或 $x_k=\mathcal S^{p_k}$，我们也得到了：
$$\tilde{F}(x)=\sum_{\bm i}f_ix_1^{i_1}x_2^{i_2}x_3^{i_3}\dots$$

因此，DGF 中的许多结论也便不用证明，直接套用多元多项式现成的理论即可。

## 封闭形式

数列 $\{1\}_{n=0}^\infty$ 的 OGF 封闭形式为：
$$\sum_{n\ge0}x^n=\frac1{1-x}$$

而数列 $\{1\}_{n=1}^\infty$ 的 DGF 封闭形式为：
$$\sum_{n\ge1}\frac1{n^s}:=\zeta(s)$$

这个函数就是著名的 **Riemann-zeta 函数**。  
根据上文的多元多项式视角，它可以这么化简：
$$
\begin{aligned}
\zeta(x)&=\sum_{\bm i}x_1^{i_1}x_2^{i_2}x_3^{i_3}\dots\\
&=\left(\sum_{i_1\ge 0}x_1^{i_1}\right)\left(\sum_{i_2\ge 0}x_2^{i_2}\right)\dots\\
&=\prod_{k\ge 1}\frac1{1-x_k}\\
\end{aligned}
$$

这就是 **Euler 乘积**公式，其本质也可以看作每一维的正交性。  
写回更标准的 DGF 就是：
$$\zeta(s)=\prod_{k\ge 1}\frac1{1-p_k^{-s}}=\prod_{p\in\mathcal{P}}\frac1{1-p^{-s}}$$

---

如果一个函数是 [积性函数](https://oi-wiki.org/math/number-theory/basic/#积性函数)，那么它在这个视角下也可以分离变量。  
例如 $\varphi$ 函数：
$$
\begin{aligned}
\tilde{\Phi}(s)&:=\sum_{n\ge1}\frac{\varphi(n)}{n^s}\\
\tilde{\Phi}(x)&=\sum_{\bm i}[\varphi(p_1^{i_1})x_1^{i_1}][\varphi(p_2^{i_2})x_2^{i_2}][\varphi(p_3^{i_3})x_3^{i_3}]\dots\\
&=\left(\sum_{i_1\ge 0}\varphi(p_1^{i_1})x_1^{i_1}\right)\left(\sum_{i_2\ge 0}\varphi(p_2^{i_2})x_2^{i_2}\right)\dots\\
&=\prod_{k\ge 1}(1+\varphi(p_k)x_k+\varphi(p_k^2)x_k^2+\dots)\\
&=\prod_{k\ge 1}\frac{1-x_k}{1-p_kx_k}\\
\tilde{\Phi}(s)&=\prod_{p\in\mathcal{P}}\frac{1-p^{-s}}{1-p^{1-s}}=\frac{\zeta(s-1)}{\zeta(s)}\\
\end{aligned}
$$
*※ 这里的方括号仅用于区分圆括号。*

其实贝尔级数就是多元视角（Euler 乘积）视角下的某一维罢了：
$$\varphi_p(x)=\frac{1-x}{1-px}$$
所以这也体现了多元视角的本质性。  
*※ 这何尝不是一种傅里叶变化呢。*

以此类推，这里直接给出其他常见函数的封闭形式：  
单位函数 $\epsilon(n)=[n=1]$：
$$\tilde{\epsilon}(s) = 1$$
幂函数 ${\rm id}_k(n)=n^k$，特别的 $id_1=1$：
$$\tilde{I}_k(s)=\zeta(s-k)$$
Möbius 函数 $\mu(n)$：
$$\tilde{M}(s)=1/\zeta(s)=\zeta(s)^{-1}$$
除数函数 $\sigma_k(n)=\sum_{d|n}d^k$，特别的 $\sigma_0=d,\sigma_1=\sigma$：
$$\tilde{S}_k(s)=\zeta(s)\zeta(s-k)$$

更详细的推导可以参考 [gxy001 的文章](https://www.luogu.com.cn/article/sdcbuewo)。

---

我们定义 [Euler 函数的重数函数](https://oeis.org/A014197) 为 $\psi(n)=\#\{x:\varphi(x)=n\}$。  
*※ OI 中更常使用 [指示函数](https://mathworld.net.cn/IversonBracket.html)，可以这么定义：$\psi(n)=\sum_x[\varphi(x)=n]$*  
其实这个函数也可以这么推导：
$$
\begin{aligned}
\tilde{\Psi}(s)&=\sum_{n\ge1}\frac{\psi(n)}{n^s}=\sum_{n\ge1}\frac{1}{\varphi(n)^s}\\
&=\sum_{i\ge 1}\varphi(p_1^{i_1})^{-s}\varphi(p_2^{i_2})^{-s}\varphi(p_3^{i_3})^{-s}\dots\\
&=\prod_{p\in\mathcal{P}}(1+\varphi(p)^{-s}+\varphi(p^2)^{-s}+\dots)\\
&=\prod_{p\in\mathcal{P}}\frac{1+(p-1)^{-s}-p^{-s}}{1-p^{-s}}\\
&=\zeta(s)\prod_{p\in\mathcal{P}}(1+(p-1)^{-s}-p^{-s})
\end{aligned}
$$

这正是我出的 [这题](https://www.luogu.com.cn/problem/P12470) 的解法。  
虽然其不具备积性，但由于它也可以按质数幂分离变量，  
所以仍可以写成类似于 Euler 乘积的形式，这是贝尔级数所做不到的。  

## 卷积

### 理论

根据前文推导的 $\varphi$ 函数的封闭形式：
$$\tilde{\Phi}(s)=\frac{\zeta(s-1)}{\zeta(s)}$$

我们可以得到一个 Dirichlet 卷积的式子：
$$\tilde{\Phi}(s)\zeta(s)=\zeta(s-1)$$

也可以写作：
$$\varphi*1=id$$

也就是说我们有：
$$\sum_{ab=n}\varphi(a)\times1=\sum_{d\mid n}\varphi(d)=n$$

这就是对比系数的结果，和 OGF 是一样的道理。

---
### 工程

我们考虑在工程上计算 Dirichlet 卷积：
$$\tilde{H}(s)=\tilde{F}(s)\tilde{G}(s)$$

一般的，要考虑 $F,G$ 前 $N$ 项的截断去推 $H$ 前 $N$ 项的截断。  
*※ 下文中也默认为 $N$ 为总项数（截断），$n$ 为具体的某一项。*

我们考虑暴力计算每一项（被动转移）：
$$h_n=\sum_{ab=n}f_ag_b=\sum_{d\mid n}f_ng_{n/d}$$

发现暴力计算第 $n$ 项时需要进行 $d(n)$ 次乘法，  
由于 $\sum_{n=1}^Nd(n)=O(N\log N)$，所以只用进行 $O(N\log N)$ 次的乘法，

工程上我们一般会采用更简洁的主动转移：  
直接枚举 $ab\le N$，再贡献 $f_ag_b\to h_{ab}$。  
时间复杂度也是 $\sum_{a=1}^N\lfloor N/a\rfloor=O(N\log N)$。

你可以用 $a,b$ 为坐标系画一张图来帮助理解。

实现：
```C++
for(int a=1;a<=n;a++)for(int b=1;a*b<=n;b++)h[a*b]+=f[a]*g[b];
```

---
### Dirichlet 前缀和

就像数组的前缀和一样，我们经常也要计算 **Dirichlet 前缀和**。  
对于 OGF 是卷上 $\frac1{1-x}$，而对于 DGF 是卷上 $\zeta(s)$。  
因此 **Dirichlet 前缀和**有时被称为 **zeta 变换**。

在一维情况，我们是怎么做 $O(N)$ 的前缀和的呢。  
只要 $g_i$ 的值是正确的，我们就可以用 $g_{i+1}\leftarrow f_{i+1}+g_i$ 递推。

既然普通的前缀和可以直接计算做到 $O(N)$，那么这个也应该可以加速：  
$$\tilde{G}(s)=\tilde{F}(s)\zeta(s)$$

我们再看回多元多项式的形式：
$$\tilde{G}(x)=\tilde{F}(x)\prod_{k\ge 1}\frac1{1-x_k}$$

我们直接考虑枚举 $k$，依次卷上 $\frac1{1-x_k}$ 即可。

对于某个 $x_k=x^{\bm e}=x^{\langle\dots,0,1,0,\dots\rangle}$（看作高维指数向量）。  
只要 $f'_{\bm i}$ 的值是正确的，我们就可以用 $f'_{\bm i+\bm e}\leftarrow f_{\bm i+\bm e}+f'_{\bm i}$ 递推。  
换回 DGF 的下标就是 $f'_{ip_k}\leftarrow f_{ip_k}+f'_i$，且 $f'_i$ 正确。  
因此实际上没必要用高维向量，按照 $i$ 递增来枚举也可以保证算到 $f'_{ip_k}$ 时 $f'_i$ 正确。  
这样就卷上了 $\frac1{1-x_k}$。

每次卷积都只用考虑对 $\lfloor N/p_k\rfloor$ 个元素做前缀和。  
时间复杂度为 $\sum_{p\in\mathcal{P}}\lfloor N/p\rfloor=O(N\log\log N)$

这正是 [这题](https://www.luogu.com.cn/problem/P5495) 的解法。

实现：
```C++
//先筛素数,cnt是素数个数,p是素数数组
for(int k=1;k<=cnt;k++)for(int i=1;i*p[k]<=n;i++)f[i*p[k]]+=f[i];
```

---
### Dirichlet 差分

差分作为前缀和的逆运算，也是需要经常用到的。  
对于 OGF 是卷上 $(1-x)$，而对于 DGF 是卷上 $\zeta(s)^{-1}$。  
因此 **Dirichlet 差分**有时被称为 **Möbius 变换**。

我们再仍旧以多元多项式的形式理解：
$$\tilde{G}(x)=\tilde{F}(x)\prod_{k\ge 1}(1-x_k)$$

如果你考虑对比提取系数，还可以发现高维差分就是容斥原理。  
换句话来说 Möbius 变换 = 高维差分 = 容斥原理。

工程上我们也直接考虑枚举 $k$，依次卷上 $(1-x_k)$ 即可。  
每次卷积都只用考虑对 $\lfloor N/p_k\rfloor$ 个元素做从后往前的差分即可。  
其逻辑和前缀和刚好相反，你可以对比前缀和。  
时间复杂度也是 $\sum_{p\in\mathcal{P}}\lfloor N/p\rfloor=O(N\log\log N)$

实现：
```C++
for(int k=1;k<=cnt;k++)for(int i=n/p[k];i;i--)a[i*p[k]]-=a[i];
```

---
### lcm 卷积

定义 $\tilde F,\tilde G\to \tilde H$ 的 ${\rm lcm}$ 卷积为每一项做如下操作：
$$h_n=\sum_{{\rm lcm}(a,b)=n}f_ag_b$$

我们再看回指数向量的形式，可以发现：
$${\rm lcm}(a,b)\Leftrightarrow\max(\bm a,\bm b)$$

其中 $\max$ 操作是逐坐标进行的，相当于对每个质因子取 $\max$，这就是 $\rm lcm$ 的本质。  
所以仍旧可以用多元多项式形式化简。

$$
\begin{aligned}
h_n&=\sum_{{\rm lcm}(a,b)=n}f_ag_b\\
h_{\bm n}&=\sum_{\max(\bm a,\bm b)=\bm n}f_{\bm a}g_{\bm b}\\
\sum_{i\le\bm n}h_{\bm i}&=\sum_{\bm a,\bm b\le\bm n}f_{\bm a}g_{\bm b}\\
\sum_{i\le\bm n}h_{\bm i}&=\left(\sum_{\bm a\le\bm n}f_{\bm a}\right)\left(\sum_{\bm b\le\bm n}g_{\bm b}\right)\\
\end{aligned}
$$

所以我们想求 lcm 卷积就是：  
分别对 $\tilde F,\tilde G$ 做 Dirichlet 前缀和，  
再点乘（逐位相乘）后做 Dirichlet 差分即可。 

---

为了更好的阐述其思想，我们也用生成函数来刻画：  
定义 $\mathcal{L}^a\mathcal{L}^b=\mathcal{L}^{{\rm lcm}(a,b)}$。  
在这种形式符号占位下，lcm 卷积可以写作：
$$\tilde{H}(\mathcal{L})=\tilde{F}(\mathcal{L})\tilde{G}(\mathcal{L})$$

根据先前的推导，有：
$$\zeta(\mathcal{S})\tilde{H}(\mathcal{S})=\zeta(\mathcal{S})\tilde{F}(\mathcal{S})\cdot_{\mathcal{S}}\zeta(\mathcal{S})\tilde{G}(\mathcal{S})$$

这里的点乘的运算优先级低于卷积，定义为逐项相乘：
$$a\mathcal{S}^n\cdot_{\mathcal{S}}b\mathcal{S}^n=ab\mathcal{S}^n$$

---
### gcd 卷积

其实 $\rm gcd$ 和 $\rm lcm$ 很像：

$$
\begin{aligned}
h_n&=\sum_{{\rm gcd}(a,b)=n}f_ag_b\\
h_{\bm n}&=\sum_{\min(\bm a,\bm b)=\bm n}f_{\bm a}g_{\bm b}\\
\sum_{i\ge\bm n}h_{\bm i}&=\left(\sum_{\bm a\ge\bm n}f_{\bm a}\right)\left(\sum_{\bm b\ge\bm n}g_{\bm b}\right)\\
\end{aligned}
$$

同上文，我们做 狄利克雷后缀和、差分 即可。

狄利克雷后缀和 等于 狄利克雷前缀和 的转移顺序相反：
```C++
for(int k=1;k<=cnt;k++)for(int i=n/p[k];i;i--)a[i]+=a[i*p[k]];
```
狄利克雷后缀差分 等于 狄利克雷前缀差分 的转移顺序相反：
```C++
for(int k=1;k<=cnt;k++)for(int i=1;i*p[k]<=n;i++)f[i]-=f[i*p[k]];
```

这部分就留给读者自证了，你可以比对一下前面的代码。  
你或许还可以看 [GhostLX 的文章](https://zhuanlan.zhihu.com/p/670327603)，虽然可能不够透彻（雾）。

---
### fg 为积性的快速 Dirichlet 卷积

我们还是考虑在工程上计算 Dirichlet 卷积：
$$\tilde{H}(s)=\tilde{F}(s)\tilde{G}(s)$$

其中 $\tilde{F},\tilde{G}$ 是积性函数。  
容易发现这两者都可以写成正交的 Euler 乘积形式，  
因此 $\tilde{H}$ 也可以写成这个形式，即 $\tilde{H}$ 也是积性函数。

众所周知，线性筛可以求积性函数，例如筛 [Euler 函数](https://oi-wiki.org/math/number-theory/sieve/#筛法求欧拉函数)。  
当然也可以筛 $\tilde{H}$：  

设此时考虑到了 `not_prime[x]`，  
若 $x=p^k$，那么 $h_{x}=\sum_{i=0}^kf_ig_{k-i}$  
若 $x=mp^k,m\perp p^k$，那么 $h_{x}=h_mh_{p^k}$  
*※ 这里的 $\perp$ 指的是多元向量意义下的垂直，就是互质。*

时间复杂度比较复杂的是 $x=p^k$ 的部分，为：
$$
\begin{aligned}
\sum_{p<N}\lfloor\log_pN\rfloor^2&<\log^2N\sum_{p<N}\frac{1}{\log^2p}\\
&=\log^2N\int_2^N\frac{1}{\log^2t}d\pi(t)\\
&\approx\log^2N\int_2^N\frac{1}{\log^3t}dt\\
&\sim\log^2N\frac{N}{\log^3N}=\frac{N}{\log N}=O(N)\\
\end{aligned}
$$

所以总时间复杂度为 $O(N)$。

另一种时间复杂度分析详见 [EI 的文章](https://zhuanlan.zhihu.com/p/32303115)。

---
### g 为积性的快速 Dirichlet 卷积

我们需要计算：
$$\tilde{H}(s)=\tilde{F}(s)\tilde{G}(s)$$  
其中 $\tilde{G}$ 是积性函数。  
可以仿照 Dirichlet 前缀和的思路，看作多元多项式：
$$\tilde{H}(x)=\tilde{F}(x)\prod_{k\ge 1}g_{p_k}(x_k)$$  
*※ 这里的 $g_{p_k}$ 是贝尔级数，详见前文。*

和 Dirichlet 前缀和一样的方法递推即可：  
即枚举 $k$，每次卷上 $g_{p_k}(x_k)$。  
即为卷上 $g_{p_k}x_k$，$g_{p_k^2}x_k^2$，$g_{p_k^3}x_k^3$，等等。  
每次卷上 $g_{p_k^i}x_k^i$ 时都要考虑 $O(\lfloor N/p_k^i\rfloor)$ 个元素。  
这一部分的时间复杂度就是：$O(\sum_{i\ge1}\lfloor N/p_k^i\rfloor)=O(N/(p_k-1))$。

总时间复杂度还是 $\sum_{p\in\mathcal{P}}N/(p-1)=O(N\log\log N)$。

实现：
```C++
for(int k=1;k<=cnt;k++)for(int t=n/p[k];t;t--)
  for(int i=p[k];i*t<=n;i*=p[k])f[i*t]+=f[t]*g[i];
```

这部分的原理你可以看 [masterhuang 的文章](https://masterhuang.blog.uoj.ac/blog/9279)，但他的时间复杂度分析有一些笔误。

---

这一部分我再做个简单的推广：  
我们考虑前文提到的 Euler 函数的重数函数 $\psi(n)$。  
令 $\tilde{G}(s)=\tilde{\Psi}(s)$，也可以暴力推：
$$\tilde{H}(x)=\tilde{F}(x)\prod_{k\ge1}(1+\varphi(p_k)^{-s}+\varphi(p_k^2)^{-s}+\dots)$$  

时间复杂度也是 $O(N\log\log N)$，和上方的分析方法差不多。  
至于正确性，你可以将 $p_k-1$ 具体质因子分解，看作多元多项式。

令 $\tilde{F}(s)=1=\tilde{\epsilon}$，这样就可以筛出 $\psi(n)$ 了。  
大部分能写成这种 DGF 乘积的函数都可以尝试这么做。

---
### 快速 Dirichlet 卷积

我们先定义 正交 Dirichlet 卷积:
$$h_n=\sum_{\substack{ab=n\\a\perp b}}f_ag_b$$

可以写作：
$$h_n=\sum_{\substack{{\rm lcm}(a,b)=n\\\omega(a)+\omega(b)=\omega(n)}}f_ag_b$$

其中 $\omega$ 是质因子数量，所以用 $\omega(\bullet)$ 占位，构造多项式：
$$\tilde{H}(\mathcal{L},y)=\sum_ih_i\mathcal{L}^iy^{\omega(i)}$$
$$h_i=[\mathcal{L}^iy^{\omega(i)}]H(\mathcal{L},y)$$

$F(\mathcal{L},y),G(\mathcal{L},y)$ 同理，于是有：
$$\tilde{H}(\mathcal{L},y)=\tilde{F}(\mathcal{L},y)\tilde{G}(\mathcal{L},y)$$
$$\zeta(\mathcal{S})\tilde{H}(\mathcal{S},y)=\zeta(\mathcal{S})\tilde{F}(\mathcal{S},y)\cdot_{\mathcal{S}}\zeta(\mathcal{S})\tilde{G}(\mathcal{S},y)$$

如同上述的 $\rm lcm$ 卷积，我们想求正交卷积也可以做 zeta 变换和点乘：

![](https://cdn.luogu.com.cn/upload/image_hosting/la3uke8q.png)

如图所示，这就是我们要开的数组大小，我们只用考虑再这上面操作即可。

更具体的，我们先做 zeta 变换：  
每次卷积 $\frac1{1-x_k}$ 时要考虑 $f'_{ip_k,t}\leftarrow f_{ip_k,t}+f'_{i,t}$，  
此时需要满足 $i<\lfloor N/p_k\rfloor$ 且 $t\le \omega(i)$，  
每次卷积都只用转移 $\sum_{i=1}^{N/p_k}(\omega(i)+1)=O(N/p_k\log\log N)$ 个元素。  
总时间复杂度为 $\sum_{p\in\mathcal{P}}N/p\log\log N=O(N(\log\log N)^2)$

再考虑点乘：  
点乘是在 $\mathcal{S}$ 轴上的点乘，  
但是对于相同的 $\mathcal{S}^i$，我们还是要考虑 $y$ 轴上的加法卷积。  
暴力卷积的时间复杂度是 $\sum_{x=1}^N\omega(x)^2=O(N(\log\log N)^2)$。

Möbius 变换也和 zeta 变换一样，是 $O(N(\log\log N)^2)$ 的。  
于是正交 Dirichlet 卷积可以在 $O(N(\log\log N)^2)$ 时间内计算.

---

Dirichlet 卷积可以在同样的时间内计算：
$$
\begin{aligned}
h_n&=\sum_{ab=n}f_ag_b\\
&=\sum_{k^2\mid n}\sum_{\substack{ab=n\\{\rm gcd}(a,b)=k}}f_ag_b\\
&=\sum_{k^2\mid n}\sum_{\substack{ab=n/k^2\\a\perp b}}f_{ak}g_{bk}\\
\end{aligned}
$$

于是可以枚举 $k\in[1,\sqrt N]$，分别做正交卷积即可。  
由于正交卷积的时间复杂度是 $T(n)=O(n(\log\log n)^2)$，  
所以 Dirichlet 卷积的时间复杂度是：
$$\sum_{k=1}^{\sqrt N}T(N/k^2)=O(N(\log\log N)^2)$$

这一部分的内容相当于是讲解了 [EI 的文章](https://www.cnblogs.com/Elegia/p/18795045/alternative-dirichlet-convolution)。

---

这里再提供一个常数优化的方法：  

如果暴力计算正交卷积，可以做到 $T_0(n)=O(n\log n)$，  
当 $n$ 较小时，由于其常数足够小，比这个方法快得多。

于是可以考虑按 $N^B$ 分类讨论：  
当 $n$ 较大时，即 $k\le N^B$:
$$\sum_{k=1}^{N^B}T(N/k^2)=O(N(\log\log N)^2)$$
当 $n$ 较小时，即 $k\ge N^B$:
$$\sum_{k=N^B}^{\sqrt N}T_0(N/k^2)=O(N^{1-B}\log N)$$

代码可以直接按 $N>B$ 来分类讨论实现：
```C++
const long long B=100000;
for(int k=1;k*k<=N;k++){if(N>k*k*B)T(N,k);else T_0(N,k);}
```

---

其实还有另一个更早的，由 [_fewq](https://www.luogu.com.cn/user/819212) 提出的算法。  
也可以做到 $O(n(\log\log n)^2)$，EI 的方法其实是这个方法的变体。

[飞雨烟雁的文章](https://www.luogu.com.cn/article/1c7ic51n) 也讲的很清楚了：  
类似于正交 Dirichlet 卷积，我们先做 Square free 处的 Dirichlet 卷积。  
再根据数的 square-free number, powerful number 唯一分解，补全 Dirichlet 卷积即可。

不过囿于其常数比 EI 的方法更大，思想也与这里的基调不同，这里就不展开了。

## DGF 全家桶

对于乘法，先前的卷积部分已经写的足够清晰了，这里不再赘述。

为了更好的普适性，我们所探讨的 DGF 不限制积性。  
如果是积性，可以仿照前文「fg 为积性的快速 Dirichlet 卷积」部分，  
按照 多元多项式/贝尔级数 的思想思考，大部分情况都可以做到 $O(n)$。  
我们还限制系数都是模素数意义下的，否则你可能要考虑精度误差。

为了避免重复定义，我们规定 $\tilde{H}$ 是要求的 DGF，其他则是已知的。  
至于代码实现，你还是可以参考这篇 [gxy001 的文章](https://www.luogu.com.cn/article/sdcbuewo)。

除了微分和复合，这一部分的每个的算法都有更优秀的时间复杂度，将会在下个板块给出。  
不过这一个板块作为前置知识还是有必要的，代码也相对简单。

---
### 除法

我们需要计算：
$$\tilde{H}(s)=\tilde{F}(s)/\tilde{G}(s)$$
其中 $g_1\neq0$。

这里定义除法为乘法的逆运算，相当于：
$$\tilde{F}(s)=\tilde{G}(s)\tilde{H}(s)$$
于是可以推导计算的递推式：
$$
\begin{aligned}
f_n&=\sum_{ab=n}g_ah_b\\
f_n&=g_1h_n+\sum_{\substack{ab=n\\a>1}}g_ah_b\\
h_n&=g_1^{-1}\left(f_n-\sum_{\substack{ab=n\\a>1}}g_ah_b\right)
\end{aligned}
$$

其中的 $a>1$ 相当于 $b<n$，也便可以递推了。

时间复杂度和卷积相同，显然是 $O(N\log N)$ 的。  
实现上考虑主动转移，考虑 $h_n\to h_{kn}$ 贡献更方便。

---
### 微分与积分

我们想求 $\tilde{F}(s)$ 的微分 $\tilde{H}(s)$，一个想法是直接对其求导：
$$\tilde{H}(s)=\tilde{F}'(s)=\sum_{i\ge 1}-\ln x\frac{f_n}{n^s}$$

但这产生了 $\ln$，在工程上极为不方便。

事实上我们要求导的主要目的是为了处理复合，  
这要求我们定义微分形算子 $\mathfrak{D}$ 满足 Leibniz 律：
$$\mathfrak{D}(\tilde{F}*\tilde{G})=\tilde{F}*\mathfrak{D}\tilde{G}+\tilde{G}*\mathfrak{D}\tilde{F}$$
只要满足这个要求，在复合多项式 $G$ 时就具备链式法则：
$$\mathfrak{D}(G(\tilde{F}))=\mathfrak{D}(G\circ\tilde{F})=(G'\circ\tilde{F})\mathfrak{D}F$$
*※ 这里的 $G$ 是普通的一元多项式，复合后 $x^n$ 意味着卷积 $n$ 次。*

那么我们怎么构造更好的微分形算子呢？  
把 $\tilde{F}(s)$ 仍看作多元多项式，立即可以得到一个很好的微分形算子：
$$\mathfrak{D}=\sum_{k\ge1}x_k\frac{\partial}{\partial x_k}$$

这还是一个点乘型算子：
$$\mathfrak{D}x^{\bm i}=\mathfrak{D}\left(\sum_{k\ge1}x^{i_k}\right)=\left(\sum_{k\ge1}i_k\right)x^{\bm i}$$

定义 $\Omega$ 为 含重数素因子个数（相当于唯一分解中的指数和）函数，  
于是我们又可以写回一般的 DGF 形式了：
$$\mathfrak{D}n^{-s}=\Omega(n)n^{-s}=\Omega_nn^{-s}$$

注意到 $\Omega_n$ 与上文的 $-\ln n$ 都是 [完全加性函数](https://oi-wiki.org/math/number-theory/basic/#加性函数)。  
更一般的，点乘任意一个 完全加性函数 都对应着一个微分形算子。

使用线性筛可以 $O(n)$ 预处理完全加性函数的值。  
点乘的时间复杂度也是 $O(n)$ 的，即微分可以做到 $O(n)$。

---

若我们以点乘完全加性函数 $g$ 作为微分，
那么点乘 $g^{-1}$ 相当于做它的积分。

但 $g_1=0$，所以 $g_1^{-1}$ 不存在，一定会丢失信息。  
因此我们令 $g_p\neq0$，于是其他项都不会丢失信息。  
实际上 $g_1$ 我们也不怎么在意，仿照一元多项式处理即可。

我们仍旧取 $g=\Omega$，这是一个很好的选择，$\Omega_p=1$，  
预处理时也只用考虑几个数的逆元就够了，总时间复杂度还是 $O(n)$的。

---
### 对数

仿照我们在普通的一元多项式上的方法，定义：
$$\tilde{H}=\ln\tilde{F}=\sum_{i\ge1}\frac{(-1)^{i-1}}{i}(\tilde{F}-1)^i$$

其中 $f_1=1$。

根据上文的链式法则，我们有：
$$\mathfrak{D}\ln(\tilde{F}*\tilde{G})=\mathfrak{D}\ln\tilde{F}+\mathfrak{D}\ln\tilde{G}$$
相当于满足对数乘法公式：
$$\ln(\tilde{F}*\tilde{G})=\ln \tilde{F}+\ln\tilde{G}$$

---

想要具体的计算对数，我们可以借助微分构造递推式：

$$
\begin{aligned}
\mathfrak{D}\tilde{H}&=\mathfrak{D}\ln\tilde{F}\\
\mathfrak{D}\tilde{H}&=\frac{\mathfrak{D}\tilde{F}}{\tilde{F}}\\
\tilde{H}&=\mathfrak{D}^{-1}\left(\frac{\mathfrak{D}\tilde{F}}{\tilde{F}}\right)
\end{aligned}
$$

它的递推式仿照前文 DGF 除法 的推导过程即可。

我们也可以直接调用 DGF 除法、微分、积分，这样也不需要单独实现了。  
时间复杂度和 DGF 除法 一样，是 $O(N\log N)$ 的。 

---
### 指数

仿照我们在普通的一元多项式上的方法，定义：
$$\tilde{H}=\exp\tilde{F}=\sum_{i\ge0}\frac{\tilde{F}^i}{i!}$$

其中 $f_1=0$。

根据上文的链式法则，我们有：
$$\mathfrak{D}\exp(\tilde{F}+\tilde{G})=\exp(\tilde{F}+\tilde{G})(\mathfrak{D}\tilde{F}+\mathfrak{D}\tilde{G})$$
$$\mathfrak{D}(\exp\tilde{F}*\exp\tilde{G})=\mathfrak{D}\tilde{F}\exp\tilde{F}\exp\tilde{G}+\mathfrak{D}\tilde{G}\exp\tilde{F}\exp\tilde{G}$$
他们都满足微分方程：
$$\mathfrak{D}\tilde{Y}=\tilde{Y}(\mathfrak{D}\tilde{F}+\mathfrak{D}\tilde{G})$$
由于解唯一，所以二者相等。  
相当于满足指数加法公式：
$$\exp(\tilde{F}+\tilde{G})=\exp\tilde{F}\exp\tilde{G}$$

---

想要具体的计算指数，我们也可以借助微分构造递推式：

$$
\begin{aligned}
\mathfrak{D}\tilde{H}&=\mathfrak{D}\exp\tilde{F}\\
\mathfrak{D}\tilde{H}&=\exp\tilde{F}*\mathfrak{D}\tilde{F}\\
\mathfrak{D}\tilde{H}&=\tilde{H}*\mathfrak{D}\tilde{F}\\
\Omega_nh_n&=\sum_{ab=n}h_af_b\Omega_b\\
\end{aligned}
$$

特别的 $h_1=1$。  
其中 $b=1$ 时 $\Omega_b=0$，相当于 $a<n$，也便可以递推了。
  
实现上考虑主动转移，仿照 DGF 除法，考虑 $h_n\to h_{kn}$ 贡献更方便。  
时间复杂度和 DGF 除法 一样，是 $O(N\log N)$ 的。

---
### 快速幂

我们需要计算：
$$\tilde{H}=\tilde{F}^m$$

其中 $f_1=1$。

事实上这里 $\exp$ 和 $\ln$ 是互逆的，和经典结论完全一样，  
于是我们可以照搬 $\ln-\exp$ 方法：

$$\tilde{H}=\exp(m\ln\tilde{F})$$

当 $m$ 是分数考虑处理逆元也可以做。  
直接调用 DGF 对数、指数 即可做到 $O(N\log N)$。

这正是 [这题](https://loj.ac/p/6713) 的解法。

---
### 等比求和（广义求逆）

我们需要计算：
$$\tilde{H}=1+\tilde{A}+\tilde{A}^2+\dots+\tilde{A}^m$$

其中 $f_1=1$。

我们可以利用等比数列求和化简为：
$$\tilde{H}=\frac{\tilde{A}^{m+1}-1}{\tilde{A}-1}$$

分母可以由上方的快速幂方法求出，但做除法是发现 $f_1=1$，不可逆。

对于普通的一元多项式，我们可以考虑分子分母同乘一个 $x^{-1}$。  
但 DGF 本质是多元多项式，乘 $x_k^{-1}$ 会在其他不含 $x_k$ 的项产生 $x_k^{-1}$。  
这是不可行的，于是我们要解决的核心便是广义求逆。

尝试仿照前文的除法推递推式，定义：
$$\tilde{F}=\tilde{A}^{m+1}-1$$
$$\tilde{G}=\tilde{A}-1$$

其中 $f_1=g_1=0,g_2\neq0$，我们需要计算：
$$\tilde{H}(s)=\tilde{F}(s)/\tilde{G}(s)$$

相当于：
$$\tilde{F}(s)=\tilde{G}(s)\tilde{H}(s)$$

由于 $g_1=0,g_2\neq0$，我们尝试考虑 $2n$ 项并提取前两项：
$$
\begin{aligned}
f_{2n}&=\sum_{ab=2n}g_ah_b\\
f_n&=g_1h_{2n}+g_2h_n+\sum_{\substack{ab=2n\\a>2}}g_ah_b\\
f_n&=g_2h_n+\sum_{\substack{ab=2n\\a>2}}g_ah_b\\
h_n&=g_2^{-1}\left(f_n-\sum_{\substack{ab=2n\\a>2}}g_ah_b\right)
\end{aligned}
$$

其中的 $a>2$ 相当于 $b<n$，也便可以递推了。

时间复杂度和 DGF 除法 相同，显然是 $O(N\log N)$ 的。  
实现上考虑主动转移，考虑 $h_n\to h_{kn}$ 贡献更方便。

这个做法需要得知 $\tilde F,\tilde G$ 的前 $2n$ 项，才能算出 $H$ 的前 $n$ 项。  
看似浪费了很多已知信息，实则我们已经利用了所有信息。  
因为想要唯一确定 $h(n)$，就必须知道 $f(2n)$ 是多少。  
即便是递推式中未出现的 $f$ 的奇数值也很重要，它可以告诉我们这样的 $\tilde H$ 是否存在。  
不过，如果我们已知 $\tilde H$ 是存在的，那奇数值确实没啥用（  
*※ 这段文字是搬运飞雨烟雁的。*

这个方法最初是我用在 [这个问题](https://www.luogu.com.cn/article/8q9wzkam) 上的，为了处理线性相关问题而提出的。  
和飞雨烟雁讨论之后他也写了 [这篇文章](https://www.luogu.com.cn/article/2fb6escy)，~~写的比我好多了~~。  

这正是 [这题](https://www.luogu.com.cn/problem/P1998) 的解法。

---
### 多项式复合

给你一个普通的一元多项式 $G$，我们需要计算：
$$\tilde{H}(s)=G(\tilde{F}(s))$$

设 $\tilde{F}$ 的首项为 $k$，定义：
$$G_0(x)=G(x+k),\tilde{F_0}(s)=\tilde{F}(s)-k$$
所以：
$$\tilde{H}(s)=G(\tilde{F}(s)-k+k)=G_0(\tilde{F_0}(s))$$

可以变换为 $G(x)\leftarrow G_0(x),\tilde{F}(s)\leftarrow \tilde{F_0}(s)$，  
该变换的时间复杂度一般可以忽略。  

通过该变换，$\tilde{F}$ 的首项即为 $0$，  
所以 $\tilde{F}^i$ 的最小非零项为 $2^i$，因此我们只用枚举 $i\le\log N$，相当于:

$$\tilde{H}=\sum_{i=1}^{\log N}g_i\tilde{F}^i$$

一种方法是直接做 $\log N$ 次卷积，总时间复杂度为 $O(N\log^2N)$。

---

观察 $\tilde{F}^{i+1}\leftarrow \tilde{F}*\tilde{F}^i$，其中 $\tilde{F}^i$ 实际上只有 $\Omega(n)\ge i$ 的项非零。  
我们可以借此稀疏性，只考虑非零项，使用转主动转移，做到：

$$\sum_{i=1}^{\log N}\sum_{ab\le N}[\Omega(a)\ge i]=N\sum_{a=1}^N\frac{\Omega(a)}{a}=O(N\log N\log\log N)$$

----

用前面的 $\zeta$ 变换（zeta 变换）方法来优化还可以做到更快，  
回顾上文的正交 Dirichlet 卷积的时间复杂度为 $T(n)=O(n(\log\log n)^2)$。  
所以 Dirichlet 卷积的时间复杂度为 $\sum_{k\le\sqrt N}T(N/k^2)=O(N(\log\log N)^2)$。  
我们就只需要考虑正交 Dirichlet 卷积的 $\zeta,\zeta^{-1}$ 变换与点乘即可。

值得注意的是我们不能考虑像 FFT 一样连续点乘，因为这么做算出来是多次正交卷积的结果。  
多次正交卷积就限制了多个自变量都是互质的，这很难贡献回普通的卷积。

我们仍旧观察 $\tilde{F}^{i+1}\leftarrow \tilde{F}*\tilde{F}^i$：  
对 $\tilde{F}$ 做 $\zeta$ 变换是 $O(N(\log\log N)^2)$ 的。  
对 $\tilde{F}^i$ 做 $\zeta$ 变换，我们也只用考虑 $\Omega(k)\ge i$ 的非零项，为：
$$\sum_{p}\sum_{k=1}^{N/p}[\Omega(k)\ge i]\omega(k)$$

对于所有的 $i$，总时间复杂度为：
$$\sum_{i\ge 0}\sum_{p}\sum_{k=1}^{N/p}[\Omega(k)\ge i]\omega(k)=\sum_{p}\sum_{k=1}^{N/p}\Omega(k)\omega(k)=O(N(\log\log N)^3)$$

对 $\zeta\tilde{F}^{i+1}$ 做 $\zeta^{-1}$ 变换也可以用类似的方法做到相同的时间复杂度。

对于 $\tilde{F}$ 与 $\tilde{F}^i$ 的点乘，非零项仍旧满足 $\Omega(k)\ge i$ ，总时间复杂度为：

$$\sum_{i\ge 0}\sum_{k=1}^N[\Omega(k)\ge i]\omega^2(k)=\sum_{k=1}^N\Omega(k)\omega^2(k)=O(N(\log\log N)^3)$$

因此我们做到了 $O(N(\log\log N)^3)$ 的复合。

这个算法也是我收飞雨烟雁启发想到的，不过也没什么用罢了。

## DGF 牛顿迭代

这个方法是我和飞雨烟雁的同一个讨论中他提出的，写了 [这篇文章](https://www.luogu.com.cn/article/fvuj6pau)。  
不过后来发现早就有许多人在用这个方法了：

在 李白天 21 年 的集训队论文《信息学竞赛中的生成函数计算理论框架》2.4 节，  
详细介绍了一般的以卷积下标系统 $I$ 为下标的生成函数的牛顿迭代法。  
推广到 DGF 也是水到渠成的。

在块筛的相关研究中甚至也直接提到了该思想，例如 [negiizhao 的文章](https://negiizhao.blog.uoj.ac/blog/9019)。  
其实 周康阳 24 年 的集训队论文《关于积性函数求和问题的一些进展》4.2 节，  
本质上也是这个想法，不过并没有提到，不排除他为了简洁性而拆除脚手架的可能。

在学这个方法之前你最好对普通生成函数的牛顿迭代有所了解。  
这篇 [command_block 的文章](https://www.luogu.com.cn/article/oy8l7j3n) 对多项式讲的极为详尽。  
这篇 [negiizhao 的文章](https://negiizhao.blog.uoj.ac/blog/4671) 做的常数讨论比较好。

---
### 理论原理

牛顿迭代的本质其实就是 Taylor 级数。  
这就要用到先前的所定义的微分了。

我们还是将 $\tilde{F}$ 看作多元多项式，$\bm n$ 也显然是个向量。  
给你一个普通的一元多项式 $G$，求解：
$$G(\tilde{F}(x))\equiv0\pmod{x^{\bm n}}$$

我们可以直接套用一元形式幂级数的牛顿迭代结论：  
若已知 $\tilde{F}_0$ 满足 $G(\tilde{F}_0(x))\equiv0\pmod{x^{\bm m}}$，那么：
$$\tilde{F}\equiv\tilde{F}_0-\frac{G(\tilde{F}_0)}{G'(\tilde{F}_0)}\pmod{x^{\bm{2m}}}$$

证明也是可以直接套用一元的结论的，  
观察 $G(\tilde{F})$ 在 $\tilde{F}_0$ 的 Taylor 级数得到：
$$G(\tilde{F})=G(\tilde{F}_0)+\frac{G'(\tilde{F}_0)}{1!}(\tilde{F}-\tilde{F}_0)^1+\frac{G''(\tilde{F}_0)}{2!}(\tilde{F}-\tilde{F}_0)^2+\cdots$$

又由于 $\tilde{F}-\tilde{F}_0\equiv0\pmod{x^{\bm m}}$，$G(\tilde{F})=0$   
因此我们对 Taylor 级数模 $x^{\bm{2m}}$ 得到：
$$0\equiv G(\tilde{F}_0)+G'(\tilde{F}_0)(\tilde{F}-\tilde{F}_0)\pmod{x^{\bm {2m}}}$$
$$\tilde{F}\equiv\tilde{F}_0-\frac{G(\tilde{F}_0)}{G'(\tilde{F}_0)}\pmod{x^{\bm{2m}}}$$

这就是牛顿迭代很经典的倍增原理了。

---

我们换成最初定义的形式记号 $\mathcal S$ 便是取：
$$G(\tilde{F}_0(x))\equiv0\pmod{\mathcal S^m}$$

可以发现迭代时每一维上都发生了倍增，相当于：
$$x^{\bm{2m}}\Leftrightarrow\mathcal S^{m^2}$$
*※ 可以翻前文的高维视角部分。*

即为：
$$\tilde{F}\equiv\tilde{F}_0-\frac{G(\tilde{F}_0)}{G'(\tilde{F}_0)}\pmod{\mathcal S^{m^2}}$$

由于精度增长的足够快，以至于我们几乎只用考虑 DGF 复合的时间复杂度了，  
若 DGF 复合的时间复杂度为 $M(N)=O(N)$，牛迭时间复杂度也可以做到线性：
$$T(N)=T\left(\sqrt N\right)+M(N)=M(N)=O(N)$$

关于实现，我们需要计算到 $N$ 的截断，一般有两种思路：  

一种是直接按照上面的式子实现，不过可能要考虑初始的边界情况。  

另一种是直接暴力计算前 $\sqrt N$ 项，最后再进行一次迭代收尾即可，  
令 $\tilde{F}_0=\tilde{F}\bmod S^{\sqrt N}$，也就说牛迭就化为了这么一条式子：
$$\tilde{F}=\tilde{F}_0-\frac{G(\tilde{F}_0)}{G'(\tilde{F}_0)}$$

---
### 优化

下文这些运算的时间复杂度等价于 DGF 卷积，  
采用前文的优化可以做到：
$$O(N(\log\log N)^2)$$

这里只详细讲解 DGF 除法 的优化，其他函数自己推导即可。

**除法：**

我们需要计算：
$$\tilde{H}(s)=\tilde{F}(s)/\tilde{G}(s)$$
其中 $g_1\neq0$。

暴力算前 $\sqrt N$ 项：$\tilde{H}_0=\tilde{H}\bmod S^{\sqrt N}$。  
可以参考前文，时间复杂度是 $O(\sqrt N\log N)$ 的。

牛迭相当于是要求解：$A(\tilde{H})=\tilde{H}\tilde{G}-\tilde{F}=0$。  
对其求导相当于：$A'(\tilde{H})=\tilde{G}$。  
于是可以列为：$\tilde{H}=2\tilde{H}_0-\tilde{F}\tilde{H}_0^2$。  
时间复杂度等价于 DGF 卷积，采用前文的优化可以做到 $O(N(\log\log N)^2)$。

如果 $\tilde{F}$ 是积性函数，还可以做到 $O(N\log\log N)$。  
这正是 [这题](https://www.luogu.com.cn/problem/P2025) 的解法。

**广义求逆：**

广义求逆的牛迭式子显然和上方的除法相同，  
唯一的区别就是初始的边界情况，这也说明了进行第二种方法的优越性。  
时间复杂度也是相同的 $O(N(\log\log N)^2)$。

**对数：**

$\ln$ 的时间复杂度是依赖于 DGF 除法 的，显然可以做到 $O(N(\log\log N)^2)$。

**指数：**

$\exp$ 的牛迭式子可以写作：
$$\tilde{H}=(1−\ln\tilde{H}_0+\tilde{F})\tilde{H}_0$$
时间复杂度是依赖于 DGF 对数 的，可以做到 $O(N(\log\log N)^2)$。

**快速幂：**

快速幂的时间复杂度是依赖于 $\ln$ 和 $\exp$ 的，可以做到 $O(N(\log\log N)^2)$。

# 块筛

> ~~我破防了，不写了。~~

暴论：  
筛法（信息学）就是利用**块筛分块**加速计算 **DGF 的 Euler 乘积式**。

我其实写到这就不想写了，前人已经写的够好了。  

更让我心态爆炸的是我做的好几个研究和前人重复了，  
我以为我的发明 [Naszt 筛](https://www.luogu.com.cn/article/2jbt2rrb) 是完全新的想法新的路径，  
但是，聪明的，你告诉我，为什么早在多年以前，就有 [一模一样](https://negiizhao.blog.uoj.ac/blog/8961) 的东西了？

这几篇文章是写的比较好的，也大量参考了其中的一些文章，这里就直接统一列出来了：  
zzt（朱震霆）的文章 [《积性函数求和问题的一种筛法》](https://blog.csdn.net/whzzt/article/details/104105025)  
negiizhao 的文章 [《OI 中常用数论函数求和法的简化陈述》](https://negiizhao.blog.uoj.ac/blog/7165)  
negiizhao 的文章 [《OI 积性函数求和传统做法的最后一块拼图》](https://negiizhao.blog.uoj.ac/blog/8961)  
negiizhao 的文章 [《积性函数求和新做法初步研究》](https://negiizhao.blog.uoj.ac/blog/9019)  
周康阳 24 年 的集训队论文《关于积性函数求和问题的一些进展》

> ~~为什么我没有早点发现啊！为什么没人告诉我啊！~~

## 块筛卷积

### 求和与几何

我们看回最普通的加法卷积：
$$H(x)=F(x)G(x)$$

其实我们可以从更几何的视角理解它：

![](https://cdn.luogu.com.cn/upload/image_hosting/o7vw2pic.png)

可以发现每个不同的 $h_i$ 都对应着一条不同的斜线。  
如果我们需要求：
$$\sum_{i<N}h_i$$
则对应的则是整一块三角形。  
虽然我们转换了问题，但这个问题实际上还是需要 $O(N\log N)$ 的。

---

对于乘法卷积则有所不同：
$$\tilde{H}(s)=\tilde{F}(s)\tilde{G}(s)$$

我们现在需要计算的是：
$$\sum_{i\le N}h_i$$

可以发现这一块双曲线下的面积可以拆成 $2\lfloor\sqrt N\rfloor$ 个矩形 减 一个正方形：

![](https://cdn.luogu.com.cn/upload/image_hosting/1ay22dyf.png)

定义：
$$S_f(n)=\sum_{i=1}^{\lfloor n\rfloor}f_n$$
我们立即可以得到：

$$
\begin{aligned}
\sum_{i\le N}h_i&=S_h(N)\\
&=\sum_{i\le\sqrt N}f_iS_g\left(\frac Ni\right)+\sum_{i\le\sqrt N}g_iS_f\left(\frac Ni\right)-S_f(\sqrt N)S_g(\sqrt N)
\end{aligned}
$$

这就是 Dirichlet 双曲求和法。  
于是我们只需要 $O(\sqrt N)$ 的时间复杂度即可。


可以发现这一块双曲线下的面积也可以直接拆成若干个矩形：

![](https://cdn.luogu.com.cn/upload/image_hosting/gnk2f26t.png)

其实这张图可以用这个式子表示：
$$S_h(N)=\sum_{i\le N}f_iS_g\left(\frac Ni\right)$$

直接计算这个式子相当于 竖着 一条一条的求和。  
但可以发现 $\left\lfloor\frac Ni\right\rfloor$ 只有 $O(\sqrt N)$ 个取值，详见后文 整除集合。  
也就是说部分 $S_g\left(\frac Ni\right)$ 是相同取值的，此时对 $f_i$ 求和再相乘就是上图红色的矩形了。  
这种做法被称为 [整除分块 / 数论分块](https://oi-wiki.org/math/number-theory/sqrt-decomposition)，也是 $O(\sqrt N)$ 的。

对比这两个做法：  
Dirichlet 双曲求和法 和 整除分块法 只差一个 Abel 变换。  
整除分块法 在推导公式时会显得简洁一些，书写方便。  
Dirichlet 双曲求和法 常数会更小一些，它的形式也更为对称。

---
### 杜教筛与块筛

观察 Dirichlet 双曲求和法 的式子，  
我们只需要关于 $\tilde{F},\tilde{G}$ 的 $O(\sqrt N)$ 个信息即可确定 $S_h(N)$。

一种直观的取法是取 $\tilde{F},\tilde{G}$ 的 整除集合 上的前缀和。  
其中整除集合定义为：

$$D_N=\left\{\left\lfloor\frac Ni\right\rfloor:i\in\mathbb Z_+\right\}=\left\{0,1,\dots,\left\lfloor\sqrt N\right\rfloor,\dots,\left\lfloor\frac N2\right\rfloor,N\right\}$$

于是可以知道 $D_N$ 上的 $S_f,S_g$ 可以 $O(\sqrt N)$ 推出 $S_h(N)$。

整除集合还有一个重要性质：

$$\left\lfloor\frac{N}{ij}\right\rfloor=\max\{m\in\mathbb{Z}:mij\le N\}=\max\{m\in\mathbb{Z}:mj\le \lfloor N/i\rfloor\}=\left\lfloor\frac{\lfloor N/i\rfloor}{j}\right\rfloor$$

因此 $D(\lfloor N/i\rfloor)\subset D(N)$。  
于是 $D_N$ 上的 $S_f,S_g$ 可以 $O(\sqrt{N/i})$ 推出 $S_h(N/i)$，  
即为 $D_N$ 上的 $S_f,S_g$ 可以推出 $D_N$ 上的 $S_{f*g}$。

---

杜教筛算法即为计算：  
由 $D_N$ 上的 $S_f,S_g$ 推出 $D_N$ 上的 $S_h$，其中 $h=f*g$。  

$S_h$ 的前 $\sqrt N$ 项 采用 **主动转移**：  
$S_f,S_g$ 的前 $\sqrt N$ 项可以推出 $f,g$ 的前 $\sqrt N$ 项。  
$f,g$ 的前 $\sqrt N$ 项可以直接 $O(\sqrt N\log N)$ 推出 $h$ 的前 $\sqrt N$ 项。

$S_h$ 的后 $\sqrt N$ 项 采用 **被动转移**：  
计算 $S_h(N/i)$ 需要 $O(\sqrt{N/i})$，后 $\sqrt N$ 项即为 $i\le\sqrt N$，  
时间复杂度为 $O(\sum_{i\le\sqrt N}\sqrt{N/i})=O(N^{3/4})$。

总时间复杂度也便是 $O(N^{3/4})$。

---

块筛的一般的定义即为 $D_N$ 上的 $S_f$（**前缀和**），块筛卷积为 $h=f*g$ 的块筛。  
积性函数求和与大部分的数论函数求和都以此为基础。

但不同于上面一般的定义，下文中我们都把块筛（块）定义为：
$$K_f(x)=S_f\left(\frac{N}{x}\right)-S_f\left(\frac{N}{x+1}\right)$$
即为块筛数组的差分，或者也可以理解为 **块内和**。  
相比于 **前缀和**，这样做差分的好处是更新某一处的值只会影响一个块。  
对于有稀疏性的数论函数也更方便处理。

这种写法推式子也很方便：倒数第 $x$ 块就是 $K_f(x)$，第 $x$ 块就是 $f_x$。

---
### 杜教筛优化

根据我们新的块筛的定义，我们也可以方便的做出类似的图：

![](https://cdn.luogu.com.cn/upload/image_hosting/orztx8c0.png)

### 特殊函数的优化

### 传统最快的做法

### 更全面的讨论

### zak 的块筛卷积

### zak 的块筛卷积的优化

## 块筛全家桶

### 块筛牛顿迭代

### 块筛除法

### 块筛 exp

### zak 的块筛 exp

### 块筛 ln

## 筛法原理

### PN 筛

### $id_k$ 块筛

### 素数幂块筛 - $\exp$

### 素数幂块筛 - $\ln$

### 积性函数块筛

## 非 FFT 筛法优化

### 小素数贡献“稀疏化”优化

### 数据结构优化小素数贡献

### 速度瓶颈与 Stern-Brocot Tree 拟合

## 特殊函数筛法

### 加性函数

### PN 筛优化

$$\bigcup_{i=1}^{\sqrt[3]{N}}D\left(\left\lfloor\sqrt\frac{N}{i}\right\rfloor\right)$$

上界：
$$k<\sqrt{N/i}$$
$$i<N/k^2$$
$$
\begin{aligned}
&\sum_{k=\sqrt[4]N}^{\sqrt N}\left(1-\prod_{i=1}^{\min\{\sqrt[3]N,N/k^2\}}\left(1-\frac{\sqrt{N/i}}{k^2}\right)\right)\\
=&\sum_{k=\sqrt[4]N}^{\sqrt N}\left(1-\exp\left(\sum_{i=1}^{\min\{\sqrt[3]N,N/k^2\}}\ln\left(1-\frac{\sqrt{N/i}}{k^2}\right)\right)\right)\\
\le&\sum_{k=\sqrt[4]N}^{\sqrt N}\left(1-\exp\left(-\sum_{i=1}^{\min\{\sqrt[3]N,N/k^2\}}\frac{\sqrt{N/i}}{k^2}\right)\right)\\
=&\sum_{k=\sqrt[4]N}^{\sqrt N}\left(1-\exp\left(-\sum_{i=1}^{\min\{\sqrt[3]N,N/k^2\}}\frac{\sqrt{N/i}}{k^2}\right)\right)\\
\sim&\sum_{k=\sqrt[3]N}^{\sqrt N}\left(1-\exp\left(-\sum_{i=1}^{N/k^2}\frac{\sqrt{N/i}}{k^2}\right)\right)\\
=&\sum_{k=\sqrt[3]N}^{\sqrt N}\left(1-\exp\left(-\frac{N}{k^3}\right)\right)\\
<&\sum_{k=\sqrt[3]N}^{\sqrt N}\left(1-1+\frac{N}{k^3}\right)=N\sum_{k=\sqrt[3]N}^{\sqrt N}k^{-3}=O(N^{1/3})\\
\end{aligned}
$$


### 二维积性函数

# 展望

OI 以及数学中的理论研究一直有个很严重的问题就是**重复研究**。  
我

# 后话

人名对照：
```
狄利克雷：Dirichlet
莱布尼茨：Leibniz
莫比乌斯：Möbius
泰勒：Taylor
黎曼：Riemann
欧拉：Euler
```


感谢大家看到这里。