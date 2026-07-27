#pragma once
#include "../common/modint.hpp"   // 提供 mint<MOD>
#include <vector>
#include <valarray>
#include <algorithm>
#include <cstddef>

using std::vector;
using std::valarray;

constexpr int NTT_MOD = 998244353;
using mint = mint<NTT_MOD>;

inline int glim(size_t x) {
    return x == 1 ? 1 : 2 << (31 ^ __builtin_clz((int)x - 1)); // 等价 2<<__lg(x-1)
}

// 单位根表（静态）
static vector<mint> wt;

vector<mint>& ntt_init(int n) {
    if (wt.empty()) wt = {1};
    while ((int)wt.size() < n) {
        int m = (int)wt.size();
        mint wn = mint(3).pow((NTT_MOD - 1) / m >> 2);
        wt.resize(m << 1);
        for (int i = m; i < m << 1; i++) wt[i] = wn * wt[i ^ m];
    }
    return wt;
}

// DIF：系数 -> 蝴蝶变换后的点值（输出位逆序）
valarray<mint> dif(const vector<mint>& src, int n) {
    auto &w = ntt_init(n);
    valarray<mint> a(mint(0), n);
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
vector<mint> dit(const valarray<mint>& src) {
    int n = (int)src.size();
    auto &w = ntt_init(n);
    vector<mint> a(begin(src), end(src));
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
    mint inv_n = mint::mod - (mint::mod - 1) / n;
    for (int i = 0; i < n; i++) a[i] *= inv_n;
    std::reverse(a.begin() + 1, a.end());
    return a;
}

// 普通卷积
vector<mint> multiply(const vector<mint>& a, const vector<mint>& b) {
    int need = (int)a.size() + (int)b.size() - 1;
    int len = glim(need);
    auto A = dif(a, len);
    auto B = dif(b, len);
    A *= B;
    auto c = dit(A);
    c.resize(need);
    return c;
}

// 差卷积
vector<mint> diff_conv(vector<mint> a, const vector<mint>& b) {
    std::reverse(a.begin(), a.end());
    int len = glim(a.size() + b.size() - 1);
    auto A = dif(a, len);
    auto B = dif(b, len);
    A *= B;
    auto c = dit(A);
    c.erase(c.begin(), c.begin() + a.size() - 1);
    c.resize(b.size() - a.size() + 1);
    return c;
}
