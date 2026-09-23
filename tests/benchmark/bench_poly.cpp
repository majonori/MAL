#include "../../include/poly/fft.hpp"
#include "../../include/poly/ntt.hpp"
#include <chrono>
#include <cstdint>
#include <iostream>
#include <random>
#include <vector>

using Clock = std::chrono::steady_clock;

static double ms_since(Clock::time_point t0) {
    return std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
}

namespace classic {

// Textbook NTT: bit-reversal permutation + Cooley-Tukey butterflies.
// Kept here only as a reference point for the DIF/DIT implementation.
const long long MOD = 998244353;

static long long pw(long long a, long long k) {
    long long r = 1;
    for (a %= MOD; k; k >>= 1, a = a * a % MOD) {
        if (k & 1) r = r * a % MOD;
    }
    return r;
}

static std::vector<int> roots(int n, bool invert) {
    std::vector<int> w(n);
    const long long g = pw(3, (MOD - 1) / n);
    const long long step = invert ? pw(g, MOD - 2) : g;
    w[0] = 1;
    for (int i = 1; i < n; ++i) w[i] = int(w[i - 1] * step % MOD);
    return w;
}

static void ntt(std::vector<int>& a, bool invert) {
    const int n = int(a.size());
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) std::swap(a[i], a[j]);
    }
    const std::vector<int> w = roots(n, invert);
    for (int len = 2; len <= n; len <<= 1) {
        const int half = len >> 1, stride = n / len;
        for (int i = 0; i < n; i += len) {
            for (int j = 0; j < half; ++j) {
                const int u = a[i + j];
                const int v = int((long long)a[i + j + half] * w[j * stride] % MOD);
                a[i + j] = u + v < 998244353 ? u + v : u + v - 998244353;
                a[i + j + half] = u - v >= 0 ? u - v : u - v + 998244353;
            }
        }
    }
    if (invert) {
        const int inv_n = int(pw(n, MOD - 2));
        for (int i = 0; i < n; ++i) a[i] = int((long long)a[i] * inv_n % MOD);
    }
}

static std::vector<int> mul(const std::vector<int>& a, const std::vector<int>& b) {
    const int need = int(a.size() + b.size() - 1);
    int len = 1;
    while (len < need) len <<= 1;
    std::vector<int> fa(a.begin(), a.end()), fb(b.begin(), b.end());
    fa.resize(len);
    fb.resize(len);
    ntt(fa, false);
    ntt(fb, false);
    for (int i = 0; i < len; ++i) fa[i] = int((long long)fa[i] * fb[i] % MOD);
    ntt(fa, true);
    fa.resize(need);
    return fa;
}

} // namespace classic

int main() {
    std::mt19937_64 rng(20260923);
    for (int shift : {16, 18, 20}) {
        const int len = 1 << shift;
        std::vector<mal::ntt_mint> a(len), b(len);
        std::vector<int> ia(len), ib(len);
        std::vector<mal::cpx> ca(len), cb(len);
        for (int i = 0; i < len; ++i) {
            // Small values keep the complex FFT result exact as well.
            const int x = int(rng() % 1000), y = int(rng() % 1000);
            a[i] = x;
            b[i] = y;
            ia[i] = x;
            ib[i] = y;
            ca[i] = x;
            cb[i] = y;
        }

        auto t0 = Clock::now();
        std::vector<mal::ntt_mint> got = mal::ntt_mul(a, b);
        const double dif_ms = ms_since(t0);

        t0 = Clock::now();
        std::vector<int> want = classic::mul(ia, ib);
        const double rev_ms = ms_since(t0);

        t0 = Clock::now();
        std::vector<mal::cpx> cf = mal::fft_mul(ca, cb);
        const double fft_ms = ms_since(t0);

        bool ok = true;
        // Coefficients can exceed the modulus, so sample a few indices and
        // build the exact value with a naive dot product.
        for (int idx : {0, 1, len / 3, len / 2, 2 * len / 3, len - 1}) {
            long long exact = 0;
            for (int j = 0; j < len; ++j) {
                const int k = idx - j;
                if (0 <= k && k < len) exact += (long long)ia[j] * ib[k];
            }
            if (got[idx].v != int(exact % classic::MOD)) ok = false;
            if (want[idx] != int(exact % classic::MOD)) ok = false;
            if (std::lround(cf[idx].real()) != exact) ok = false;
            if (std::fabs(cf[idx].imag()) > 1e-3 * std::fabs(cf[idx].real()) + 1e-6)
                ok = false;
        }

        std::cout << "len 2^" << shift << ": NTT(DIF/DIT) " << dif_ms
                  << " ms, NTT(bit-reversal) " << rev_ms << " ms, ratio "
                  << rev_ms / dif_ms << "x, FFT " << fft_ms
                  << " ms, match " << (ok ? "yes" : "NO") << '\n';
    }
    return 0;
}
