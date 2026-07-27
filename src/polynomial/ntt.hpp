#pragma once
#include "../common/modint.hpp"
#include <algorithm>
#include <vector>
using std::vector;

constexpr int NTT_MOD = 998244353;
constexpr int NTT_G   = 3;
using mint = mint<NTT_MOD>;

void ntt(vector<mint> &a, bool invert) {
    int n = (int)a.size();
    for (int i = 1, j = 0; i < n; i++) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) std::swap(a[i], a[j]);
    }
    for (int len = 2; len <= n; len <<= 1) {
        mint wlen = mint(NTT_G).pow((NTT_MOD - 1) / len);
        if (invert) wlen = wlen.inv();
        for (int i = 0; i < n; i += len) {
            mint w = 1;
            for (int j = 0; j < len / 2; j++) {
                mint u = a[i + j], v = a[i + j + len / 2] * w;
                a[i + j] = u + v;
                a[i + j + len / 2] = u - v;
                w *= wlen;
            }
        }
    }
    if (invert) {
        mint inv_n = mint(n).inv();
        for (mint &x : a) x *= inv_n;
    }
}

/* Edited on 2026/07/27
NTT 板子
*/
