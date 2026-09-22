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

// 单位根表（静态）
static vector<ntt_mint> wt;

inline vector<ntt_mint>& ntt_init(int n) {
    if (wt.empty()) wt = {1};
    while ((int)wt.size() < n) {
        int m = (int)wt.size();
        ntt_mint wn = ntt_mint(3).pow((NTT_MOD - 1) / m >> 2);
        wt.resize(m << 1);
        for (int i = m; i < m << 1; i++) wt[i] = wn * wt[i ^ m];
    }
    return wt;
}

// DIF：系数 -> 蝴蝶变换后的点值（输出位逆序）
inline valarray<ntt_mint> ntt_dif(const vector<ntt_mint>& src, int n) {
    auto &w = ntt_init(n);
    valarray<ntt_mint> a(ntt_mint(0), n);
    std::copy(src.begin(), src.end(), &a[0]);
    for (int len = n, k = n >> 1; k >= 1; len >>= 1, k >>= 1) {
        for (int i = 0, t = 0; i < n; i += len, t++) {
            for (int j = 0; j < k; j++) {
                auto x = a[i + j];
                auto y = a[i + j + k] * w[t];
                a[i + j] = x + y;
                a[i + j + k] = x - y;
            }
        }
    }
    return a;
}

// DIT：蝴蝶变换后的点值 -> 系数（输入位逆序，输出自然序）
inline vector<ntt_mint> ntt_dit(const valarray<ntt_mint>& src) {
    int n = (int)src.size();
    auto &w = ntt_init(n);
    vector<ntt_mint> a(begin(src), end(src));
    for (int k = 1, len = 2; len <= n; k <<= 1, len <<= 1) {
        for (int i = 0, t = 0; i < n; i += len, t++) {
            for (int j = 0; j < k; j++) {
                auto x = a[i + j];
                auto y = a[i + j + k];
                a[i + j] = x + y;
                a[i + j + k] = (x - y) * w[t];
            }
        }
    }
    ntt_mint inv_n = NTT_MOD - (NTT_MOD - 1) / n;
    for (int i = 0; i < n; i++) a[i] *= inv_n;
    std::reverse(a.begin() + 1, a.end());
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

} // namespace mal
