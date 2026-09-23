#pragma once
#include "../common/consts.hpp"
#include <vector>
#include <valarray>
#include <algorithm>
#include <cmath>

namespace mal {

using std::vector;
using std::valarray;

// 单位根表（静态）：w 给正变换，w_inv 给逆变换，都按块号索引
static vector<cpx> w_fft, w_fft_inv;

inline vector<cpx>& fft_init(int n) {
    if (w_fft.empty()) {
        w_fft = {1};
        w_fft_inv = {1};
    }
    while ((int)w_fft.size() < n) {
        int m = (int)w_fft.size();
        double ang = 2 * PI / (m * 4);
        cpx wn(cos(ang), sin(ang));
        cpx wn_inv(cos(ang), -sin(ang));
        w_fft.resize(m * 2);
        w_fft_inv.resize(m * 2);
        for (int i = m; i < m * 2; i++) {
            w_fft[i] = wn * w_fft[i ^ m];
            w_fft_inv[i] = wn_inv * w_fft_inv[i ^ m];
        }
    }
    return w_fft;
}

// DIF
inline valarray<cpx> fft_dif(const vector<cpx>& src, int n) {
    auto &w = fft_init(n);
    valarray<cpx> a(cpx(0), n);
    cpx *p = &a[0];
    std::copy(src.begin(), src.end(), p);
    for (int len = n, k = n >> 1; k >= 1; len >>= 1, k >>= 1) {
        for (int i = 0, t = 0; i < n; i += len, t++) {
            const cpx w_t = w[t];
            cpx *lo = p + i, *hi = lo + k;
            for (int j = 0; j < k; j++) {
                const cpx x = lo[j];
                const cpx y = hi[j] * w_t;
                lo[j] = x + y;
                hi[j] = x - y;
            }
        }
    }
    return a;
}

// DIT：用共轭根，不做位逆序重排也不做反转
inline vector<cpx> fft_dit(const valarray<cpx>& src) {
    int n = (int)src.size();
    fft_init(n);
    vector<cpx> a(begin(src), end(src));
    cpx *p = a.data();
    for (int k = 1, len = 2; len <= n; k <<= 1, len <<= 1) {
        for (int i = 0, t = 0; i < n; i += len, t++) {
            const cpx w_t = w_fft_inv[t];
            cpx *lo = p + i, *hi = lo + k;
            for (int j = 0; j < k; j++) {
                const cpx x = lo[j];
                const cpx y = hi[j];
                lo[j] = x + y;
                hi[j] = (x - y) * w_t;
            }
        }
    }
    const double inv_n = 1.0 / n;
    for (int i = 0; i < n; i++) a[i] *= inv_n;
    return a;
}

// 卷积
inline vector<cpx> fft_mul(const vector<cpx>& a, const vector<cpx>& b) {
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
inline vector<cpx> fft_conv(vector<cpx> a, const vector<cpx>& b) {
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

namespace poly_detail {

// Same trick as in ntt.hpp: keeps every entry point emitted in this object file.
__attribute__((used)) vector<cpx> (*const fft_mul_kept)(const vector<cpx>&,
                                  const vector<cpx>&) = &fft_mul;
__attribute__((used)) vector<cpx> (*const fft_conv_kept)(vector<cpx>, const vector<cpx>&) = &fft_conv;
__attribute__((used)) valarray<cpx> (*const fft_dif_kept)(const vector<cpx>&, int) = &fft_dif;
__attribute__((used)) vector<cpx> (*const fft_dit_kept)(const valarray<cpx>&) = &fft_dit;
__attribute__((used)) vector<cpx>& (*const fft_init_kept)(int) = &fft_init;

} // namespace poly_detail

} // namespace mal
