#pragma once
#include "../common/consts.hpp"
#include <vector>
#include <valarray>
#include <algorithm>
#include <cmath>

using std::vector;
using std::valarray;

static vector<cpx> w_fft;

vector<cpx>& fft_init(int n) {
    if (w_fft.empty()) w_fft = {1};
    while ((int)w_fft.size() < n) {
        int m = (int)w_fft.size();
        double ang = 2 * PI / (m * 2);
        cpx wn(cos(ang), sin(ang));
        w_fft.resize(m * 2);
        for (int i = m; i < m * 2; i++) w_fft[i] = wn * w_fft[i ^ m];
    }
    return w_fft;
}

valarray<cpx> fft_dif(const vector<cpx>& src, int n) {
    auto &w = fft_init(n);
    valarray<cpx> a(cpx(0), n);
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

// DIT
vector<cpx> fft_dit(const valarray<cpx>& src) {
    int n = (int)src.size();
    auto &w = fft_init(n);
    vector<cpx> a(begin(src), end(src));
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
    for (int i = 0; i < n; i++) a[i] /= n;
    std::reverse(a.begin() + 1, a.end());
    return a;
}

// 卷积
vector<cpx> fft_mul(const vector<cpx>& a, const vector<cpx>& b) {
    int need = (int)a.size() + (int)b.size() - 1;
    int len = glim(need);
    auto A = fft_dif(a, len);
    auto B = fft_dif(b, len);
    A *= B;
    auto c = fft_dit(A);
    c.resize(need);
    return c;
}

// 差卷积
vector<cpx> fft_conv(vector<cpx> a, const vector<cpx>& b) {
    std::reverse(a.begin(), a.end());
    int len = glim(a.size() + b.size() - 1);
    auto A = fft_dif(a, len);
    auto B = fft_dif(b, len);
    A *= B;
    auto c = fft_dit(A);
    c.erase(c.begin(), c.begin() + a.size() - 1);
    c.resize(b.size() - a.size() + 1);
    return c;
}
