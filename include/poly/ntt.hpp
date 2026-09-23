#pragma once
#include "../common/modint.hpp"   // 提供 mint<MOD>
#include "../common/consts.hpp"   // 提供 glim
#include <vector>
#include <valarray>
#include <algorithm>
#include <cstddef>

namespace mal {

using std::vector;
using std::valarray;

constexpr int NTT_MOD = 998244353;
using ntt_mint = mint<NTT_MOD>;

// 单位根表（静态）：wt 给正变换，wti 给逆变换，都按块号索引
static vector<ntt_mint> wt, wti;

inline vector<ntt_mint>& ntt_init(int n) {
    if (wt.empty()) {
        wt = {1};
        wti = {1};
    }
    while ((int)wt.size() < n) {
        int m = (int)wt.size();
        const int e = (NTT_MOD - 1) / m >> 2;
        ntt_mint wn = ntt_mint(3).pow(e);
        ntt_mint wni = ntt_mint(3).pow(NTT_MOD - 1 - e);
        wt.resize(m << 1);
        wti.resize(m << 1);
        for (int i = m; i < m << 1; i++) {
            wt[i] = wn * wt[i ^ m];
            wti[i] = wni * wti[i ^ m];
        }
    }
    return wt;
}

// DIF：系数 -> 点值，输出位逆序；同一块的旋转因子在循环外取一次
inline valarray<ntt_mint> ntt_dif(const vector<ntt_mint>& src, int n) {
    auto &w = ntt_init(n);
    valarray<ntt_mint> a(ntt_mint(0), n);
    ntt_mint *p = &a[0];
    std::copy(src.begin(), src.end(), p);
    for (int len = n, k = n >> 1; k >= 1; len >>= 1, k >>= 1) {
        for (int i = 0, t = 0; i < n; i += len, t++) {
            const ntt_mint w_t = w[t];
            ntt_mint *lo = p + i, *hi = lo + k;
            for (int j = 0; j < k; j++) {
                const ntt_mint x = lo[j];
                const ntt_mint y = hi[j] * w_t;
                lo[j] = x + y;
                hi[j] = x - y;
            }
        }
    }
    return a;
}

// DIT：点值（位逆序）-> 系数，用逆根，不做位逆序重排也不做反转
inline vector<ntt_mint> ntt_dit(const valarray<ntt_mint>& src) {
    int n = (int)src.size();
    ntt_init(n);
    vector<ntt_mint> a(begin(src), end(src));
    ntt_mint *p = a.data();
    for (int k = 1, len = 2; len <= n; k <<= 1, len <<= 1) {
        for (int i = 0, t = 0; i < n; i += len, t++) {
            const ntt_mint w_t = wti[t];
            ntt_mint *lo = p + i, *hi = lo + k;
            for (int j = 0; j < k; j++) {
                const ntt_mint x = lo[j];
                const ntt_mint y = hi[j];
                lo[j] = x + y;
                hi[j] = (x - y) * w_t;
            }
        }
    }
    ntt_mint inv_n = NTT_MOD - (NTT_MOD - 1) / n;
    for (int i = 0; i < n; i++) a[i] *= inv_n;
    return a;
}

// 普通卷积
inline vector<ntt_mint> ntt_mul(const vector<ntt_mint>& a, const vector<ntt_mint>& b) {
    int need = (int)a.size() + (int)b.size() - 1;
    int len = glim(need);
    auto A = ntt_dif(a, len);
    auto B = ntt_dif(b, len);
    A *= B;
    auto c = ntt_dit(A);
    c.resize(need);
    return c;
}

// 差卷积
inline vector<ntt_mint> ntt_conv(vector<ntt_mint> a, const vector<ntt_mint>& b) {
    std::reverse(a.begin(), a.end());
    int len = glim(a.size() + b.size() - 1);
    auto A = ntt_dif(a, len);
    auto B = ntt_dif(b, len);
    A *= B;
    auto c = ntt_dit(A);
    c.erase(c.begin(), c.begin() + a.size() - 1);
    c.resize(b.size() - a.size() + 1);
    return c;
}

namespace poly_detail {

// Taking the address of each entry point makes the compiler emit its body here,
// so contestant code can call these functions through plain declarations.
__attribute__((used)) vector<ntt_mint> (*const ntt_mul_kept)(const vector<ntt_mint>&,
                                       const vector<ntt_mint>&) = &ntt_mul;
__attribute__((used)) vector<ntt_mint> (*const ntt_conv_kept)(vector<ntt_mint>,
                                        const vector<ntt_mint>&) = &ntt_conv;
__attribute__((used)) valarray<ntt_mint> (*const ntt_dif_kept)(const vector<ntt_mint>&,
                                         int) = &ntt_dif;
__attribute__((used)) vector<ntt_mint> (*const ntt_dit_kept)(const valarray<ntt_mint>&) = &ntt_dit;
__attribute__((used)) vector<ntt_mint>& (*const ntt_init_kept)(int) = &ntt_init;

} // namespace poly_detail

} // namespace mal
