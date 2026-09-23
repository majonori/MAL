#include "../../include/poly/fft.hpp"
#include "../../include/poly/ntt.hpp"
#include <cmath>
#include <iostream>
#include <random>
#include <vector>

using mal::cpx;
using mal::ntt_mint;

static bool same_mod(const std::vector<ntt_mint>& got,
                     const std::vector<long long>& want) {
    if (got.size() != want.size()) return false;
    for (size_t i = 0; i < want.size(); ++i) {
        long long v = want[i] % mal::NTT_MOD;
        if (v < 0) v += mal::NTT_MOD;
        if (got[i].v != (int)v) return false;
    }
    return true;
}

static bool same_real(const std::vector<cpx>& got, const std::vector<long long>& want) {
    if (got.size() != want.size()) return false;
    for (size_t i = 0; i < want.size(); ++i) {
        if (std::lround(got[i].real()) != want[i]) return false;
        if (std::fabs(got[i].imag()) > 1e-6) return false;
    }
    return true;
}

int main() {
    std::mt19937_64 rng(20260923);
    bool ok = true;

    for (int test = 0; test < 60; ++test) {
        // A few small random cases plus one larger balanced case per run.
        const int n = test == 59 ? 600 : 1 + int(rng() % 40);
        const int m = test == 59 ? 600 : 1 + int(rng() % 40);
        std::vector<cpx> a(n), b(m);
        std::vector<ntt_mint> na(n), nb(m);
        for (int i = 0; i < n; ++i) {
            const int v = int(rng() % 1000);
            a[i] = v;
            na[i] = v;
        }
        for (int i = 0; i < m; ++i) {
            const int v = int(rng() % 1000);
            b[i] = v;
            nb[i] = v;
        }

        std::vector<long long> mul(n + m - 1, 0);
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < m; ++j)
                mul[i + j] += (long long)a[i].real() * (long long)b[j].real();

        if (!same_mod(mal::ntt_mul(na, nb), mul)) ok = false;
        if (!same_real(mal::fft_mul(a, b), mul)) ok = false;

        // Difference convolution c[k] = sum_i a[i] * b[i + k] needs a.size() <= b.size().
        if (n <= m) {
            std::vector<long long> conv(m - n + 1, 0);
            for (int k = 0; k + n <= m; ++k)
                for (int i = 0; i < n && i + k < m; ++i)
                    conv[k] += (long long)a[i].real() * (long long)b[i + k].real();
            if (!same_mod(mal::ntt_conv(na, nb), conv)) ok = false;
            if (!same_real(mal::fft_conv(a, b), conv)) ok = false;
        }

        // DIF then DIT must return the original coefficients.
        const int len = mal::glim(n);
        std::vector<ntt_mint> padded(na);
        padded.resize(len);
        const std::vector<ntt_mint> back = mal::ntt_dit(mal::ntt_dif(padded, len));
        for (int i = 0; i < len; ++i) if (back[i].v != padded[i].v) ok = false;

        std::vector<cpx> padded_f(a);
        padded_f.resize(len);
        const std::vector<cpx> back_f = mal::fft_dit(mal::fft_dif(padded_f, len));
        for (int i = 0; i < len; ++i) {
            if (std::lround(back_f[i].real()) != std::lround(padded_f[i].real())) ok = false;
            if (std::fabs(back_f[i].imag()) > 1e-6) ok = false;
        }
    }

    std::cout << (ok ? "poly: ok" : "poly: FAILED") << '\n';
    return ok ? 0 : 1;
}
