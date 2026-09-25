# 朴素 SBT 筛：单个对数的完整做法与复杂度证明

研究稿，GPT 整理，2026-09-18。

本文计算

$$
D_{1,1}(N)=\#\{(x,y)\in\mathbb Z_{>0}^2:xy\le N\}
=\sum_{x=1}^N\left\lfloor\frac Nx\right\rfloor.
$$

给出一个 Farey 递归版的朴素 SBT 筛，时间为

$$
\boxed{O(N^{1/3}\log(N+2))}.
$$

算法中的 SBT 操作只是取中项，不给每条边重新二分，也不对每个节点调用一次 $O(\log N)$ 的类欧。关键是明确加入**分母截止规则**，再把全部叶子的直接求和一起估计。

这个上界只对应本文写出的算法。不能不加分析就把它移植给任意一个连续 Farey 栈实现，更不能只用凸包的顶点数替代运行时间。

## 1. 计算模型，以及先前需要纠正的两点

主定理统计定长整数算术操作：加减乘、整除、比较、精确平方根和精确立方根各计 $O(1)$。所有中间整数只有 $O(\log N)$ 位，即只占常数个 word。这是下面所称的“单个对数”；它不是 bit complexity。

若不把精确开方视为单位操作，设一次所需开方的成本为 $R(N)$，则应把上界写成

$$
O(N^{1/3}\log(N+2)\,R(N)).
$$

不能一边用二分实现每次开方，一边仍声称标准 word-RAM 操作数只有一个 $\log$。对于具体 OI 范围，可用定长浮点近似加整数不等式认证实现开方，但必须检查精度、溢出和修正次数；本文的验证程序将近似值修正到精确整数根。

另外，先前笔记中有两处不能沿用：

1. **所有中央区边方向均满足 $uv=O(N^{1/3})$ 是错误的。** 取 $s=t(t+1),N=s^2$，从 $(s,s)$ 向右的第一条凸包边到达 $(s+t+1,s-t)$，primitive 方向是 $(t+1,-t)$，两个正坐标的乘积恰为 $s=N^{1/2}$。这两个端点都在中央区，故不是未剪掉尾部的问题。不能用 $uv\le C N^{1/3}$ 枚举全部边。
2. **部分 SBT 的某一层分母和不能按完整层的 $3^d$ 计算。** 分支可能极不平衡，$1/1,1/2,\ldots,1/k$ 的长链是基本反例。不能由完整二叉层数推导部分树的工作量或 $O(\log N)$ 栈深。

本文不需要以上任何一个断言，也不需要双曲线凸包顶点数的上界。

上述反例的凸包指 $xy\ge N$ 的整数凸包，其下支撑链贴近双曲线。反例也可直接验证：点 $(s+r,s-q)$ 满足 $xy\ge N$ 的必要条件是 $s(r-q)\ge rq$。若 $q/r>t/(t+1)$，令 $j=r-q$，则 $j\ge1$，且 $q>tj,r>(t+1)j$，从而 $rq>t(t+1)j^2\ge sj$，矛盾；而 $(r,q)=(t+1,t)$ 恰好取等号。因此它确实给出 $(s,s)$ 右侧的第一条支撑边。

## 2. 对称性与整数斜率分段

令 $m=\lfloor\sqrt N\rfloor$。先计算

$$
S(N)=\sum_{x=1}^{m}\left\lfloor\frac Nx\right\rfloor,
\qquad D_{1,1}(N)=2S(N)-m^2.
$$

记 $F(x)=N/x$。在 $x\le m$ 上，切线斜率的绝对值为

$$
\sigma(x)=-F'(x)=\frac N{x^2}\ge1.
$$

取

$$
L=\min\left(m,\;2\left\lceil(2N)^{1/3}\right\rceil\right).
$$

直接枚举 $x<L$，成本 $O(N^{1/3})$。剩下按相邻整数斜率 $k,k+1$ 分段。斜率 $k$ 对应的切点横坐标是

$$
t_k=\sqrt{\frac Nk}.
$$

这一段的实际整数横跨度记为 $A_k$，则

$$
\begin{aligned}
A_k
&\le t_k-t_{k+1}+O(1)\\
&=O\!\left(\sqrt N\,k^{-3/2}+1\right). \tag{1}
\end{aligned}
$$

段数

$$
K=O(N/L^2+1)=O(N^{1/3}+1). \tag{2}
$$

第 3--6 节给出每一段的精确计数；第 7 节证明一段只需 $O(1+A_k^{2/3})$ 次操作。先看总复杂度：

$$
\begin{aligned}
\sum_{k\le K}O(1+A_k^{2/3})
&=O\!\left(K+N^{1/3}\sum_{k\le K}\frac1k\right)\\
&=O(N^{1/3}\log(N+2)). \tag{3}
\end{aligned}
$$

**唯一的 $\log$ 就是这个调和级数。** 单段的 SBT 递归没有另外一个 $\log$ 因子。

## 3. Farey 坐标中的局部区域

设两条负斜率直线为

$$
a x+b y=c,\qquad d x+e y=f,
$$

其中 $a,b,d,e$ 为正整数，且

$$
ae-bd=1,\qquad \frac ab>\frac de. \tag{4}
$$

定义

$$
u=ax+by-c,\qquad v=dx+ey-f.
$$

由 (4)，反变换是

$$
x=e(u+c)-b(v+f),\qquad
y=a(v+f)-d(u+c). \tag{5}
$$

这是幺模变换：整数点与整数点一一对应，没有格点密度因子，也没有遗漏的剩余类。

原点对应

$$
x_0=ec-bf,\qquad y_0=af-dc.
$$

局部区域 $R(w,h;a,b,c,d,e,f)$ 的两个轴端点为

$$
P_1=(0,h),\qquad P_2=(w,0)
$$

（本节坐标均为 $u,v$）。区域内的目标点是正整数 $u,v$、$u\le w,v\le h$，且位于双曲线的相应单调分支下方的点。

需要维护的几何不变量是：

- 原坐标 $x,y$ 均为正；
- 所处理的双曲线弧在这里能写成单调递减、凸的图像 $v=V(u)$，或反过来写成 $u=U(v)$；
- 端点取在弧下方；经过下一节的整行、整列剥离后，弧在两个端点处距相应坐标轴不足 1。

这些性质来自凸曲线的仿射变换；在斜率位于 $[d/e,a/b]$ 的弧上，$u$ 随原横坐标递增，$v$ 递减。初始化与递归都会保留这组性质。

### 3.1 单列与单行的精确计数

令

$$
H(u,v)=\bigl(e(u+c)-b(v+f)\bigr)
       \bigl(a(v+f)-d(u+c)\bigr).
$$

在上述单调分支上，解 $H(u,v)=N$ 得

$$
V(u)=\frac{(ae+bd)(u+c)-\sqrt{(u+c)^2-4abN}}{2ab}-f,
$$

$$
U(v)=\frac{(ae+bd)(v+f)-\sqrt{(v+f)^2-4deN}}{2de}-c.
$$

完成第 4 节的整行、整列剥离后，曲线整数高度不超过区域边界，直接求和可以沿较短的坐标轴进行：

$$
\#R=\sum_{u=1}^{w}\lfloor V(u)\rfloor
    =\sum_{v=1}^{h}\lfloor U(v)\rfloor. \tag{6}
$$

公式中的根号不能用未经认证的浮点数直接取整。对整数 $M$ 和正整数 $q$，有

$$
\left\lfloor\frac{M-\sqrt D}{q}\right\rfloor
=\left\lfloor\frac{M-\lceil\sqrt D\rceil}{q}\right\rfloor. \tag{7}
$$

所以每次单列计数只需一次精确 ceiling square root 和一次整除。

## 4. 整行、整列剥离

在进入细分或直接求和前，先剥掉完整的矩形。

若 $H(w,1)\le N$，则令

$$
r=\min(h,\lfloor V(w)\rfloor).
$$

由于 $V$ 递减，前 $r$ 行全都合法，加入 $wr$，并更新

$$
f\leftarrow f+r,\qquad h\leftarrow h-r. \tag{8}
$$

这相当于把 $v$ 原点上移 $r$。

同理，若 $H(1,h)\le N$，令 $t=\min(w,\lfloor U(h)\rfloor)$，加入 $ht$，更新

$$
c\leftarrow c+t,\qquad w\leftarrow w-t. \tag{9}
$$

空区域直接返回。两次剥离后

$$
\lfloor V(w)\rfloor=\lfloor U(h)\rfloor=0.
$$

第二次操作不会破坏第一次：更新 $c$ 和 $w$ 时，右端点的 $u+c$ 不变。

一次剥离直接算出全部完整行或列的数量，**不逐行循环**，所以每个节点的剥离成本为 $O(1)$。

## 5. SBT 中项细分与计数

令

$$
A=a+d,\qquad B=b+e.
$$

则 $A/B$ 是两斜率的中项，且与两端分别保持 Farey 邻居关系。原坐标中绝对斜率 $A/B$，在当前坐标中就是斜率 $-1$。

记该切点为 $(u_*,v_*)$。令

$$
K_u=ae+bd+2ab,\qquad K_v=ae+bd+2de,
$$

则

$$
u_*=K_u\sqrt{\frac N{AB}}-c,
\qquad
v_*=K_v\sqrt{\frac N{AB}}-f. \tag{10}
$$

其整数部分完全用整数算术计算，例如

$$
\lfloor u_*\rfloor
=\operatorname{isqrt}\!\left(\left\lfloor\frac{K_u^2N}{AB}\right\rfloor\right)-c. \tag{11}
$$

### 5.1 常规的两个子区域

当 $u_*,v_*\ge1$ 时，取

$$
u_4=\lfloor u_*\rfloor,\qquad u_5=u_4+1,
$$

$$
v_4=\lfloor V(u_4)\rfloor,\qquad v_5=\lfloor V(u_5)\rfloor,
$$

以及

$$
z_L=u_4+v_4,\qquad z_R=u_5+v_5.
$$

从 $(u_4,v_4)$ 向左作斜率 $-1$ 的射线，从 $(u_5,v_5)$ 向右作同斜率射线。这两条射线都在曲线下方：前者利用 $V'(u_4)\le-1$，后者利用 $V'(u_5)>-1$ 与凸性。

它们之间仅有一个单位宽的竖带，没有中间的整数列。因此，移除的直线多边形恰可精确计数，不需要估计面积误差。

定义 $\Delta(j)=j(j+1)/2$，允许 $j=-1$ 且 $\Delta(-1)=0$。移除部分的正整数点数为

$$
C_R=\sum_{u=1}^{u_4}(z_L-u)
    +\sum_{u=u_5}^{z_R-1}(z_R-u)
   =\Delta(z_L-1)-\Delta(z_L-u_5)+\Delta(z_R-u_5). \tag{12}
$$

这是沿 $u+v$ 对角线求和得到的三个三角数。两个剩余区域为

$$
R_L=R(u_4,h-z_L;a,b,c,A,B,c+f+z_L),
$$

$$
R_R=R(w-z_R,v_5;A,B,c+f+z_R,d,e,f). \tag{13}
$$

于是当前区域的计数是已剥离矩形的贡献，加上 $C_R$，再加两个子区域的计数。

剥离后的 $V(w)<1,U(h)<1$ 保证常规切点位于 $0<u_*<w,0<v_*<h$；上述左射线在 $u=1$ 处不高于 $V(1)<h$，故 $z_L\le h$。对右射线对称地得到 $z_R\le w$，所以两个孩子的尺寸均非负。

新坐标分别是 $(u,u+v-z_L)$ 与 $(u+v-z_R,v)$，仍为幺模变换。新轴端点处的差值等于 $V(u_i)-\lfloor V(u_i)\rfloor<1$；左右弧的单调性由 $1+V'$ 的符号保证。因此第 3 节的不变量被保留。

### 5.2 不平衡分支：切点落在第一个整数列之前

这一步不能省略，也不能假定 SBT 总是产生两个非空子区域。

若 $u_*<1$，则在所有目标整数列 $u\ge1$ 上有 $V'(u)>-1$。令

$$
v_1=\lfloor V(1)\rfloor,\qquad z=1+v_1.
$$

从 $(1,v_1)$ 向右的斜率 $-1$ 射线在曲线下方。移除 $\Delta(v_1)$ 个点，只保留

$$
R(w-z,v_1;A,B,c+f+z,d,e,f). \tag{14}
$$

若 $v_*<1$，完全对称地令 $u_1=\lfloor U(1)\rfloor,z=1+u_1$，移除 $\Delta(u_1)$ 个点，只保留

$$
R(u_1,h-z;a,b,c,A,B,c+f+z). \tag{15}
$$

第一种情况下，由 $V'>-1$ 和 $V(w)<1$ 得 $v_1\le w-1$，所以 $z\le w$；由 $U(h)<1$ 得 $v_1\le h-1$。第二种情况对称。因此单支规则也不会产生负尺寸。

这是朴素中项递归的单支情况，每次仍只做 $O(1)$ 次算术；不需要为长链另做一次二分。

## 6. 初始化：完整外层算法

下面把局部递归接到 $S(N)$ 上，避免把“初始化若干区域”留成黑盒。$N=0$ 时直接返回 0；其余按第 2 节定义 $m,L$。

首先加入

$$
\sum_{x=1}^{L-1}\left\lfloor\frac Nx\right\rfloor
+(m-L+1)\left\lfloor\frac Nm\right\rfloor
+\Delta(m-L). \tag{16}
$$

第二、三项是经过 $(m,\lfloor N/m\rfloor)$ 的斜率 $-1$ 直线下方的点。这条向左的射线在双曲线下方，所以不会多计。

维护

$$
k=1,\quad x_2=m,\quad y_2=\lfloor N/m\rfloor,\quad c_2=kx_2+y_2.
$$

每轮令 $p=k+1$，取

$$
x_4=\left\lfloor\sqrt{\frac Np}\right\rfloor,\quad x_5=x_4+1,
\quad y_i=\left\lfloor\frac N{x_i}\right\rfloor\ (i=4,5),
$$

$$
c_4=px_4+y_4,\qquad c_5=px_5+y_5.
$$

若 $x_4\le L$，停止外层循环。否则：

1. 加入新旧折线之间直线多边形的点数

   $$
   M_k=\Delta(c_4-c_2-L)-\Delta(c_4-c_2-x_5)
       +\Delta(c_5-c_2-x_5). \tag{17}
   $$

   具体地，在 $L\le x\le x_4$ 上，新旧直线的整数高度差为 $c_4-c_2-x$；在 $x_5\le x<c_5-c_2$ 上，差为 $c_5-c_2-x$。对这两个区间求和就是 (17)。这里的折线由过 $(x_4,y_4)$ 的左射线与过 $(x_5,y_5)$ 的右射线组成，均在曲线下方；相邻两列之间没有遗漏的整数列。

2. 定义

   $$
   w=px_2+y_2-c_5,\qquad h=kx_5+y_5-c_2.
   $$

   调用

   $$
   R(w,h;p,1,c_5,k,1,c_2).
   $$

   两端点是原坐标的 $(x_5,y_5)$ 和 $(x_2,y_2)$，并且

   $$
   A_k=w+h=x_2-x_5. \tag{18}
   $$

   尺寸非负也有直接的整数验证：在 $[x_5,x_2]$ 上，$N/x$ 的斜率绝对值位于 $[k,p]$，相邻整数列的整数高度下降量因而位于 $[k,p]$。累加这些下降量，分别得到 $w\ge0,h\ge0$。

3. 更新 $k\leftarrow p,x_2\leftarrow x_4,y_2\leftarrow y_4,c_2\leftarrow c_4$。

外层停止后，加入剩余差值

$$
\sum_{x=L}^{x_2-1}
\left(\left\lfloor\frac Nx\right\rfloor-[k(x_2-x)+y_2]\right). \tag{19}
$$

若在下一轮斜率 $p=k+1$ 处停止，则 $t_p<L+1$；由 $p/k\le2$ 得 $x_2\le t_k<\sqrt2(L+1)$。因此未处理横跨度为 $O(L)$，这次直接求和仍只需 $O(N^{1/3})$。

不重不漏可以对外层循环归纳：已经加入的是当前下折线以下的点和其右侧全部曲线差值；每轮 (17) 更新下折线，局部 $R$ 补齐新旧斜率间的曲线差值，最后 (19) 补齐最左侧余段。

## 7. 分母截止规则：单段复杂度的关键

在一段的根区域确定横跨度 $A=A_k$ 后，固定

$$
Q=\left\lceil\max(1,A)^{1/3}\right\rceil.
$$

所有后代使用同一个 $Q$。每个区域先剥离矩形，再检查：

$$
\boxed{\min(w,h)\le1\quad\text{或}\quad b+e\ge Q.} \tag{20}
$$

若成立，使用 (6) 沿较短轴直接求和；否则使用第 5 节的一次中项细分。

### 7.1 递归节点只有 $O(Q^2)$ 个

一个内部节点产生中项 $A'/B'$，其分母

$$
B'=b+e<Q.
$$

SBT 中每个既约分数只出现一次，且本段所有中项都在 $(k,k+1)$ 内。对于固定分母 $B'$，分子满足

$$
kB'<A'<(k+1)B',
$$

至多有 $B'-1$ 个候选。因此内部节点数至多

$$
\sum_{B'<Q}(B'-1)=O(Q^2). \tag{21}
$$

每个内部节点至多有两个孩子，包括单支和空孩子，故全部节点、全部叶子的数量也为 $O(Q^2+1)$。每个节点的几何处理都是 $O(1)$；没有额外的 $\log$。

### 7.2 全部叶子直接求和的总长度至多 $O(Q^2+A/Q)$

一个区域的两个原坐标横端点是

$$
x_L=x_0-bh,\qquad x_R=x_0+ew,
$$

横跨度为

$$
A_R=ew+bh. \tag{22}
$$

因为 $w,h\ge0$，有

$$
(b+e)\min(w,h)\le ew+bh=A_R. \tag{23}
$$

更重要的是，**不同最终叶子的横区间内部不相交**：

- 剥行使右端点左移，剥列使左端点右移，区间只会缩小；
- 常规细分的左孩子到原坐标的 $P_4$ 为止，右孩子从 $P_5$ 开始，而
  $$x(P_5)-x(P_4)=e+b(v_4-v_5)>0;$$
- 单支细分也只保留父区间的一个子区间。

所以

$$
\sum_{R\text{ 为最终叶子}}A_R\le A. \tag{24}
$$

由于小尺寸而停止的叶子，直接求和至多一次。由于 $b+e\ge Q$ 而停止的叶子，由 (23) 至多做 $A_R/Q$ 次。结合 (21)、(24)，全部直接求和成本为

$$
O\!\left(Q^2+\frac AQ\right). \tag{25}
$$

这里不能分别给每个叶子套一个最坏横跨度，再乘叶子数；必须利用 (24) 的总跨度预算。

取 $Q=\lceil\max(1,A)^{1/3}\rceil$ 后

$$
\boxed{T(A)=O(1+A^{2/3}).} \tag{26}
$$

代入第 2 节 (3)，总复杂度即为单个对数。

### 7.3 空间

沿一条递归路径，分母和每次至少增加 1，直到达到 $Q$；因此未经跳商压缩的朴素递归栈深至多 $O(Q)$。

最大根区间跨度为 $O(\sqrt N)$，故

$$
\boxed{\text{空间}=O(N^{1/6}+1)\text{ 个 word}.}
$$

本文不声称这棵不平衡树的深度是 $O(\log N)$。若另加商跳步，可讨论更小的栈，但不是本时间证明的前提。

## 8. 递归伪代码

省略参数列表中的 $N$，$Q$ 在同一根区域的全部调用中固定。`FV`、`FU` 使用 (7) 精确计算；`Delta` 允许参数为 $-1$。

```text
CountR(w,h,a,b,c,d,e,f,Q):
    if w=0 or h=0: return 0
    ans = 0

    if H(w,1) <= N:
        r = min(h,FV(w))
        ans += w*r; f += r; h -= r
    if w=0 or h=0: return ans
    if H(1,h) <= N:
        t = min(w,FU(h))
        ans += h*t; c += t; w -= t
    if w=0 or h=0: return ans

    if min(w,h)<=1 or b+e>=Q:
        if w<=h: return ans + sum_{u=1..w} FV(u)
        else:    return ans + sum_{v=1..h} FU(v)

    A=a+d; B=b+e
    tu = floor((a*e+b*d+2*a*b)*sqrt(N/(A*B))) - c
    tv = floor((a*e+b*d+2*d*e)*sqrt(N/(A*B))) - f

    if tu<1:
        v1=FV(1); z=1+v1
        return ans + Delta(v1)
             + CountR(w-z,v1,A,B,c+f+z,d,e,f,Q)
    if tv<1:
        u1=FU(1); z=1+u1
        return ans + Delta(u1)
             + CountR(u1,h-z,a,b,c,A,B,c+f+z,Q)

    u4=tu; u5=u4+1
    v4=FV(u4); v5=FV(u5)
    zL=u4+v4; zR=u5+v5
    ans += Delta(zL-1)-Delta(zL-u5)+Delta(zR-u5)
    ans += CountR(u4,h-zL,a,b,c,A,B,c+f+zL,Q)
    ans += CountR(w-zR,v5,A,B,c+f+zR,d,e,f,Q)
    return ans
```

`tu,tv` 按 (11) 用精确整数根算，不用浮点值直接判分支。

## 9. 验证记录与结论范围

原稿引用的 `sbt-one-log-check.cpp` 未随文稿附上。本包另给出独立实现 `include/dgf/sbt_one_log.hpp` 与对拍 `tests/dgf/test_sbt_one_log.cpp`；使用 GNU C++14 和 `__int128`。下列历史验证记录属于原稿，不代表本包代码已逐项复现。

2026-09-18 实际完成：

- $N=0,1,\ldots,10^5$ 全枚举，与独立的 $O(\sqrt N)$ 狄利克雷双曲求和逐项一致；
- 3000 个 $N\le10^8$ 的随机对拍，种子 `20260918`，全部一致；
- $t\in\{10^2,10^3,10^4,10^5,10^6\}$，测试 $t^2$ 与 $t(t+1)$ 附近各 7 个整数，全部一致；
- $10^8,10^{10},10^{12}$ 再与独立算法比较，一致；
- 对 $10^{14},10^{16},10^{18}$ 跑通算法及区域不变量断言，但这些三项未用独立算法复核答案，不能称为大范围正确性对拍。

例如 $N=10^{18}$ 时记录到 4,246,744 个递归节点、3,379,059 次叶子单行/单列求值、最大栈深 664。这只是有限实验，不承担渐近复杂度证明。

数学上已经闭合的是：局部幺模分割精确计数、分母截止下的节点数、叶子总跨度预算，以及外层调和求和。因此单个对数的算术操作上界不依赖平均情况、固定递归深度或凸包顶点数猜想。

## 10. 来源

- Richard Sladkey, [A Successive Approximation Algorithm for Computing the Divisor Summatory Function](https://arxiv.org/abs/1206.3369)，第 3--4 节：Farey 坐标与切线递归的公开算法骨架。本文重新推导整数取整、加入整块剥离和退化单支规则，并使用分母截止证明单个对数；不引用该稿第 7 节声称的 $O(N^{1/3})$ 时间或 $O(\log N)$ 空间证明。
- Antal Balog, Imre Bárány, [The integer hull of the set $\{(x,y)\in\mathbb R^2:xy\ge N\}$](https://arxiv.org/abs/2602.06897)：显式整数凸包的顶点数为 $\Theta(N^{1/3}\log N)$。这是另一种输出规模结论，不是本文计数算法时间下界，也没有被用于本文上界。
