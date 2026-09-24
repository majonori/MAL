#pragma once
#include "../common/consts.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <ostream>
#include <stdexcept>
#include <string>
#include <utility>

#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
#define MAL_X86_SIMD 1
#include <immintrin.h>
#endif

namespace mal {

namespace bigint_detail {

using limb = std::uint32_t;
using dlimb = std::uint64_t;
using vec = std::vector<limb>;

constexpr int LIMB_BITS = 32;
constexpr dlimb LIMB_BASE = dlimb(1) << LIMB_BITS;

inline int clz32(limb x) {
    return x ? __builtin_clz(x) : 32;
}

inline void trim(vec& a) {
    while (!a.empty() && a.back() == 0) a.pop_back();
}

inline int cmp_mag(const vec& a, const vec& b) {
    if (a.size() != b.size()) return a.size() < b.size() ? -1 : 1;
    for (size_t i = a.size(); i--;) {
        if (a[i] != b[i]) return a[i] < b[i] ? -1 : 1;
    }
    return 0;
}

inline vec add_mag(const vec& a, const vec& b) {
    vec c(std::max(a.size(), b.size()) + 1);
    dlimb carry = 0;
    for (size_t i = 0; i < c.size(); ++i) {
        dlimb x = carry;
        if (i < a.size()) x += a[i];
        if (i < b.size()) x += b[i];
        c[i] = limb(x);
        carry = x >> LIMB_BITS;
    }
    trim(c);
    return c;
}

// a >= b
inline vec sub_mag(const vec& a, const vec& b) {
    vec c(a.size());
    dlimb borrow = 0;
    for (size_t i = 0; i < a.size(); ++i) {
        dlimb x = dlimb(a[i]) - borrow;
        if (i < b.size()) x -= b[i];
        if (x >> LIMB_BITS) {
            x += LIMB_BASE;
            borrow = 1;
        } else {
            borrow = 0;
        }
        c[i] = limb(x);
    }
    trim(c);
    return c;
}

inline vec mul_basic(const vec& a, const vec& b) {
    if (a.empty() || b.empty()) return vec();
    vec c(a.size() + b.size(), 0);
    for (size_t i = 0; i < a.size(); ++i) {
        dlimb carry = 0;
        const dlimb ai = a[i];
        for (size_t j = 0; j < b.size(); ++j) {
            dlimb cur = dlimb(c[i + j]) + ai * b[j] + carry;
            c[i + j] = limb(cur);
            carry = cur >> LIMB_BITS;
        }
        size_t p = i + b.size();
        while (carry) {
            dlimb cur = dlimb(c[p]) + carry;
            c[p] = limb(cur);
            carry = cur >> LIMB_BITS;
            ++p;
        }
    }
    trim(c);
    return c;
}

inline vec add_at(const vec& a, const vec& b, size_t off, size_t reserve) {
    vec c(std::max(reserve, a.size() + off + b.size() + 1), 0);
    std::copy(a.begin(), a.end(), c.begin());
    dlimb carry = 0;
    for (size_t i = 0; i < b.size(); ++i) {
        dlimb x = dlimb(c[off + i]) + b[i] + carry;
        c[off + i] = limb(x);
        carry = x >> LIMB_BITS;
    }
    size_t p = off + b.size();
    while (carry) {
        dlimb x = dlimb(c[p]) + carry;
        c[p] = limb(x);
        carry = x >> LIMB_BITS;
        ++p;
    }
    trim(c);
    return c;
}

struct Signed {
    bool neg;
    vec v;
};

inline void signed_trim(Signed& x) {
    trim(x.v);
    if (x.v.empty()) x.neg = false;
}

inline Signed signed_neg(const Signed& x) {
    Signed r = x;
    if (!r.v.empty()) r.neg = !r.neg;
    return r;
}

inline Signed signed_from_vec(const vec& v, bool neg = false) {
    Signed r{neg, v};
    signed_trim(r);
    return r;
}

inline Signed signed_add(const Signed& a, const Signed& b) {
    if (a.neg == b.neg) return signed_from_vec(add_mag(a.v, b.v), a.neg);
    int c = cmp_mag(a.v, b.v);
    if (c == 0) return Signed{false, vec()};
    if (c > 0) return signed_from_vec(sub_mag(a.v, b.v), a.neg);
    return signed_from_vec(sub_mag(b.v, a.v), b.neg);
}

inline Signed signed_sub(const Signed& a, const Signed& b) {
    return signed_add(a, signed_neg(b));
}

inline Signed signed_shift(const Signed& a, size_t limbs) {
    if (a.v.empty() || limbs == 0) return a;
    vec v(limbs + a.v.size(), 0);
    std::copy(a.v.begin(), a.v.end(), v.begin() + limbs);
    return signed_from_vec(v, a.neg);
}

inline Signed signed_small(int value) {
    if (value == 0) return Signed{false, vec()};
    bool neg = value < 0;
    dlimb x = neg ? dlimb(-(long long)value) : dlimb(value);
    vec v;
    while (x) {
        v.push_back(limb(x));
        x >>= LIMB_BITS;
    }
    return Signed{neg, v};
}

inline Signed signed_mul_small(const Signed& a, int value) {
    if (value == 0 || a.v.empty()) return Signed{false, vec()};
    bool neg = a.neg ^ (value < 0);
    dlimb m = value < 0 ? dlimb(-(long long)value) : dlimb(value);
    vec v(a.v.size() + 1, 0);
    dlimb carry = 0;
    for (size_t i = 0; i < a.v.size(); ++i) {
        dlimb cur = dlimb(a.v[i]) * m + carry;
        v[i] = limb(cur);
        carry = cur >> LIMB_BITS;
    }
    v.back() = limb(carry);
    return signed_from_vec(v, neg);
}

// Exact division by a small positive integer.
inline bool signed_div_small_exact(Signed& x, dlimb d) {
    if (x.v.empty()) return true;
    dlimb rem = 0;
    for (size_t i = x.v.size(); i--;) {
        dlimb cur = (rem << LIMB_BITS) | x.v[i];
        x.v[i] = limb(cur / d);
        rem = cur % d;
    }
    signed_trim(x);
    return rem == 0;
}

// Fixed, deterministic crossover points, measured (minimum of many runs, -O2)
// against the backends they switch between.  In decimal digits the four
// thresholds are about 3.1e2, 3.1e3, 4.3e3 and 9.9e3 for balanced operands.
constexpr size_t KARATSUBA_THRESHOLD = 32;
constexpr size_t TOOM_THRESHOLD = 320;
constexpr size_t FFT_THRESHOLD = 1024;
// The transform also wins for operands below FFT_THRESHOLD as long as they are
// wide enough to amortise its setup and the power-of-two padding of the
// convolution length stays small (see the dispatch in mul_mag).
constexpr size_t FFT_MIN_OPERAND = 448;
constexpr size_t NEWTON_DIV_MIN_DIVISOR_LIMBS = 8;
constexpr size_t NEWTON_DIV_MIN_QUOTIENT_BITS = 2048;

vec mul_mag(const vec& a, const vec& b);

inline vec mul_karatsuba(const vec& a, const vec& b) {
    // z0 + z1 * B^k + z2 * B^(2k), with
    // z1 = (a0+a1)(b0+b1) - z0 - z2.
    size_t n = std::max(a.size(), b.size());
    if (std::min(a.size(), b.size()) <= KARATSUBA_THRESHOLD) return mul_basic(a, b);
    size_t k = n >> 1;
    auto cut = [](const vec& x, size_t p, size_t q) {
        q = std::min(q, x.size());
        p = std::min(p, q);
        return vec(x.begin() + p, x.begin() + q);
    };
    vec a0 = cut(a, 0, k), a1 = cut(a, k, a.size());
    vec b0 = cut(b, 0, k), b1 = cut(b, k, b.size());
    vec z0 = mul_mag(a0, b0);
    vec z2 = mul_mag(a1, b1);
    vec sa = add_mag(a0, a1), sb = add_mag(b0, b1);
    vec z1 = mul_mag(sa, sb);
    z1 = sub_mag(z1, z0);
    z1 = sub_mag(z1, z2);
    vec res = add_at(z0, z1, k, a.size() + b.size() + 1);
    res = add_at(res, z2, k << 1, res.size() + z2.size() + 1);
    trim(res);
    return res;
}

inline vec mul_toom3(const vec& a, const vec& b) {
    // Evaluate the two 3-part polynomials at 0, 1, -1, 2 and infinity.
    size_t n = std::max(a.size(), b.size());
    size_t k = (n + 2) / 3;
    auto cut = [](const vec& x, size_t p, size_t q) {
        q = std::min(q, x.size());
        p = std::min(p, q);
        return vec(x.begin() + p, x.begin() + q);
    };
    Signed x0 = signed_from_vec(cut(a, 0, k));
    Signed x1 = signed_from_vec(cut(a, k, 2 * k));
    Signed x2 = signed_from_vec(cut(a, 2 * k, a.size()));
    Signed y0 = signed_from_vec(cut(b, 0, k));
    Signed y1 = signed_from_vec(cut(b, k, 2 * k));
    Signed y2 = signed_from_vec(cut(b, 2 * k, b.size()));

    auto ev = [](const Signed& p0, const Signed& p1, const Signed& p2, int t) {
        if (t == 0) return p0;
        if (t == 1) return signed_add(signed_add(p0, p1), p2);
        if (t == -1) return signed_add(signed_sub(p0, p1), p2);
        // t = 2
        Signed r = signed_add(p0, signed_mul_small(p1, 2));
        return signed_add(r, signed_mul_small(p2, 4));
    };
    Signed w0 = signed_from_vec(mul_mag(x0.v, y0.v), x0.neg ^ y0.neg);
    Signed w1 = signed_from_vec(mul_mag(ev(x0, x1, x2, 1).v, ev(y0, y1, y2, 1).v),
                                ev(x0, x1, x2, 1).neg ^ ev(y0, y1, y2, 1).neg);
    Signed wm1 = signed_from_vec(mul_mag(ev(x0, x1, x2, -1).v, ev(y0, y1, y2, -1).v),
                                 ev(x0, x1, x2, -1).neg ^ ev(y0, y1, y2, -1).neg);
    Signed w2 = signed_from_vec(mul_mag(ev(x0, x1, x2, 2).v, ev(y0, y1, y2, 2).v),
                                ev(x0, x1, x2, 2).neg ^ ev(y0, y1, y2, 2).neg);
    Signed w4 = signed_from_vec(mul_mag(x2.v, y2.v), x2.neg ^ y2.neg);

    Signed c0 = w0;
    Signed c4 = w4;
    // Sign-aware exact halving: compute (w1 + wm1) / 2.
    Signed sum = signed_add(w1, wm1);
    if (!signed_div_small_exact(sum, 2)) return mul_karatsuba(a, b);
    Signed c2 = signed_sub(signed_sub(sum, c0), c4);

    Signed d = signed_sub(w1, wm1);
    if (!signed_div_small_exact(d, 2)) return mul_karatsuba(a, b);
    Signed r = signed_sub(signed_sub(signed_sub(w2, c0), signed_mul_small(c2, 4)),
                          signed_mul_small(c4, 16));
    Signed two_d = signed_mul_small(d, 2);
    Signed c3 = signed_sub(r, two_d);
    if (!signed_div_small_exact(c3, 6)) return mul_karatsuba(a, b);
    Signed c1 = signed_sub(d, c3);

    Signed acc = c0;
    acc = signed_add(acc, signed_shift(c1, k));
    acc = signed_add(acc, signed_shift(c2, 2 * k));
    acc = signed_add(acc, signed_shift(c3, 3 * k));
    acc = signed_add(acc, signed_shift(c4, 4 * k));
    if (acc.neg) return mul_karatsuba(a, b);
    return acc.v;
}

namespace ntt {

constexpr std::uint32_t SMALL_MOD1 = 998244353;
constexpr std::uint32_t SMALL_MOD2 = 1004535809;
constexpr std::uint32_t SMALL_ROOT = 3;
constexpr std::uint32_t MAX_SMALL = std::uint32_t(1) << 21;

// p - 1 = 15 * 2^27 and 17 * 2^27 respectively.
constexpr std::uint32_t LARGE_MOD1 = 2013265921;
constexpr std::uint32_t LARGE_ROOT1 = 31;
constexpr std::uint32_t LARGE_MOD2 = 2281701377;
constexpr std::uint32_t LARGE_ROOT2 = 3;
constexpr std::uint32_t MAX_LARGE = std::uint32_t(1) << 27;

inline std::uint32_t mod_pow(std::uint32_t a, std::uint32_t e, std::uint32_t mod) {
    dlimb r = 1, x = a % mod;
    while (e) {
        if (e & 1) r = r * x % mod;
        x = x * x % mod;
        e >>= 1;
    }
    return std::uint32_t(r);
}

template <std::uint32_t MOD>
inline std::uint32_t add_mod(std::uint32_t a, std::uint32_t b) {
    if (MOD < (std::uint32_t(1) << 31)) {
        std::uint32_t s = a + b;
        return s >= MOD ? s - MOD : s;
    }
    dlimb s = dlimb(a) + b;
    return std::uint32_t(s >= MOD ? s - MOD : s);
}

template <std::uint32_t MOD>
inline std::uint32_t sub_mod(std::uint32_t a, std::uint32_t b) {
    return a >= b ? a - b : std::uint32_t(dlimb(a) + MOD - b);
}

template <std::uint32_t MOD, std::uint32_t ROOT>
inline std::uint32_t mod_pow_const(std::uint32_t e) {
    dlimb r = 1, x = ROOT % MOD;
    while (e) {
        if (e & 1) r = r * x % MOD;
        x = x * x % MOD;
        e >>= 1;
    }
    return std::uint32_t(r);
}

template <std::uint32_t MOD, std::uint32_t ROOT>
struct twiddle_cache {
    std::vector<std::vector<std::uint32_t>> fwd, inv;
    std::vector<std::vector<std::uint32_t>> fwd_hi;
    // Shoup quotients of the inverse twiddles.  Only the vectorised inverse
    // transform needs them, so they are built on demand.
    std::vector<std::vector<std::uint32_t>> inv_hi;
    std::vector<std::uint32_t> inv2;

    void ensure(int levels) {
        if (int(fwd.size()) > levels) return;
        fwd.resize(levels + 1);
        inv.resize(levels + 1);
        fwd_hi.resize(levels + 1);
        inv2.resize(levels + 1);
        inv_hi.resize(levels + 1);
        const std::uint32_t half_mod = std::uint32_t((MOD + 1) / 2);
        inv2[0] = 1;
        for (int h = 1; h <= levels; ++h)
            inv2[h] = std::uint32_t(dlimb(inv2[h - 1]) * half_mod % MOD);
        for (int h = 0; h <= levels; ++h) {
            const size_t half = size_t(1) << h;
            if (!fwd[h].empty()) continue;
            fwd[h].assign(half, 1);
            inv[h].assign(half, 1);
            fwd_hi[h].assign(half, 1);
            inv_hi[h].clear();
            if (half == 1) continue;
            const std::uint32_t wf =
                mod_pow_const<MOD, ROOT>(std::uint32_t((MOD - 1) / (half << 1)));
            const std::uint32_t wi = mod_pow_const<MOD, ROOT>(
                MOD - 1 - std::uint32_t((MOD - 1) / (half << 1)));
            for (size_t j = 1; j < half; ++j) {
                fwd[h][j] = std::uint32_t(dlimb(fwd[h][j - 1]) * wf % MOD);
                inv[h][j] = std::uint32_t(dlimb(inv[h][j - 1]) * wi % MOD);
            }
            // Shoup quotients: floor(w * 2^32 / MOD) lets the butterfly
            // replace the 64-bit remainder by a high multiply and two narrow
            // multiplies (see mul_mod_shoup below).
            for (size_t j = 0; j < half; ++j)
                fwd_hi[h][j] = std::uint32_t((dlimb(fwd[h][j]) << LIMB_BITS) / MOD);
        }
    }
};

// d * w mod MOD, with w_hi = floor(w * 2^32 / MOD) precomputed.  The high
// product gives q = floor(d * w / MOD) up to one, so d * w - q * MOD lands in
// [0, 2*MOD) and one conditional step finishes the reduction.  For moduli
// below 2^31 that value still fits in 32 bits, so the whole reduction stays
// narrow.
template <std::uint32_t MOD>
inline std::uint32_t mul_mod_shoup(std::uint32_t d, std::uint32_t w,
                                   std::uint32_t w_hi) {
    const dlimb q = dlimb(d) * w_hi;
    if (MOD < (std::uint32_t(1) << 31)) {
        const std::uint32_t r =
            std::uint32_t(dlimb(d) * w - (q >> LIMB_BITS) * MOD);
        return r >= MOD ? r - MOD : r;
    }
    const dlimb r = dlimb(d) * w - (q >> LIMB_BITS) * MOD;
    return std::uint32_t(r >= MOD ? r - MOD : r);
}

template <std::uint32_t MOD, std::uint32_t ROOT>
inline twiddle_cache<MOD, ROOT>& get_twiddle_cache() {
    static twiddle_cache<MOD, ROOT> cache;
    return cache;
}

#ifdef MAL_X86_SIMD
// AVX2 stages; other targets use the scalar code below.
inline bool avx2_ok() {
#if defined(__GNUC__) || defined(__clang__)
    static const bool ok = __builtin_cpu_supports("avx2");
#else
    static const bool ok = false;
#endif
    return ok;
}

// Eight lanes of d * w mod MOD, same precomputed-quotient scheme as
// mul_mod_shoup.
template <std::uint32_t MOD>
__attribute__((target("avx2"))) inline __m256i shoup8(__m256i d, __m256i w,
                                                      __m256i wh) {
    const __m256i mod = _mm256_set1_epi32((int)MOD);
    const __m256i modm1 = _mm256_set1_epi32((int)MOD - 1);
    const __m256i pe = _mm256_mul_epu32(d, wh);
    const __m256i po =
        _mm256_mul_epu32(_mm256_srli_epi64(d, 32), _mm256_srli_epi64(wh, 32));
    const __m256i hi =
        _mm256_blend_epi32(_mm256_srli_epi64(pe, 32),
                           _mm256_slli_epi64(_mm256_srli_epi64(po, 32), 32), 0xAA);
    const __m256i r =
        _mm256_sub_epi32(_mm256_mullo_epi32(d, w), _mm256_mullo_epi32(hi, mod));
    const __m256i ge = _mm256_cmpgt_epi32(r, modm1);
    return _mm256_sub_epi32(r, _mm256_and_si256(ge, mod));
}

// a[i] = a[i] * m mod MOD for a constant m, eight lanes at a time.
template <std::uint32_t MOD>
__attribute__((target("avx2"))) inline void scale_avx2(
    std::vector<std::uint32_t>& a, int n, std::uint32_t m) {
    const std::uint32_t mh = std::uint32_t((dlimb(m) << LIMB_BITS) / MOD);
    const __m256i w = _mm256_set1_epi32((int)m);
    const __m256i wh = _mm256_set1_epi32((int)mh);
    int i = 0;
    for (; i + 8 <= n; i += 8)
        _mm256_storeu_si256(
            (__m256i*)&a[i],
            shoup8<MOD>(_mm256_loadu_si256((const __m256i*)&a[i]), w, wh));
    for (; i < n; ++i) a[i] = std::uint32_t(dlimb(a[i]) * m % MOD);
}

// One radix-2 DIF stage over 2^h blocks, eight lanes at a time.
template <std::uint32_t MOD>
__attribute__((target("avx2"))) inline void dif_stage_avx2(
    std::vector<std::uint32_t>& a, int n, int len,
    const std::vector<std::uint32_t>& w, const std::vector<std::uint32_t>& wh) {
    const int half = len >> 1;
    const __m256i mod = _mm256_set1_epi32((int)MOD);
    const __m256i modm1 = _mm256_set1_epi32((int)MOD - 1);
    for (int i = 0; i < n; i += len) {
        for (int j = 0; j < half; j += 8) {
            const __m256i u = _mm256_loadu_si256((const __m256i*)&a[i + j]);
            const __m256i v = _mm256_loadu_si256((const __m256i*)&a[i + j + half]);
            __m256i s = _mm256_add_epi32(u, v);
            s = _mm256_sub_epi32(s,
                                 _mm256_and_si256(_mm256_cmpgt_epi32(s, modm1), mod));
            __m256i d = _mm256_sub_epi32(u, v);
            d = _mm256_add_epi32(
                d, _mm256_and_si256(_mm256_cmpgt_epi32(v, u), mod));
            _mm256_storeu_si256((__m256i*)&a[i + j], s);
            _mm256_storeu_si256(
                (__m256i*)&a[i + j + half],
                shoup8<MOD>(d, _mm256_loadu_si256((const __m256i*)&w[j]),
                            _mm256_loadu_si256((const __m256i*)&wh[j])));
        }
    }
}

// Matching radix-2 DIT stage for the inverse transform.
template <std::uint32_t MOD>
__attribute__((target("avx2"))) inline void dit_stage_avx2(
    std::vector<std::uint32_t>& a, int n, int len,
    const std::vector<std::uint32_t>& w, const std::vector<std::uint32_t>& wh) {
    const int half = len >> 1;
    const __m256i mod = _mm256_set1_epi32((int)MOD);
    const __m256i modm1 = _mm256_set1_epi32((int)MOD - 1);
    for (int i = 0; i < n; i += len) {
        for (int j = 0; j < half; j += 8) {
            const __m256i u = _mm256_loadu_si256((const __m256i*)&a[i + j]);
            const __m256i v = shoup8<MOD>(
                _mm256_loadu_si256((const __m256i*)&a[i + j + half]),
                _mm256_loadu_si256((const __m256i*)&w[j]),
                _mm256_loadu_si256((const __m256i*)&wh[j]));
            __m256i s = _mm256_add_epi32(u, v);
            s = _mm256_sub_epi32(s,
                                 _mm256_and_si256(_mm256_cmpgt_epi32(s, modm1), mod));
            __m256i d = _mm256_sub_epi32(u, v);
            d = _mm256_add_epi32(
                d, _mm256_and_si256(_mm256_cmpgt_epi32(v, u), mod));
            _mm256_storeu_si256((__m256i*)&a[i + j], s);
            _mm256_storeu_si256((__m256i*)&a[i + j + half], d);
        }
    }
}

// Quotients of the inverse twiddles, needed only by the vectorised inverse.
template <std::uint32_t MOD, std::uint32_t ROOT>
inline void ensure_inv_hi(twiddle_cache<MOD, ROOT>& t, int h) {
    const size_t half = t.inv[h].size();
    if (half < 8 || !t.inv_hi[h].empty()) return;
    t.inv_hi[h].resize(half);
    for (size_t j = 0; j < half; ++j)
        t.inv_hi[h][j] = std::uint32_t((dlimb(t.inv[h][j]) << LIMB_BITS) / MOD);
}
#endif

constexpr int TWIDDLE_CACHE_MAX = 1 << 20;

template <std::uint32_t MOD, std::uint32_t ROOT>
inline void transform(std::vector<std::uint32_t>& a, bool invert) {
    const int n = int(a.size());
    if (n <= 1) return;
    const int levels = __builtin_ctz(std::uint32_t(n));
    const bool cached = n <= TWIDDLE_CACHE_MAX;
    twiddle_cache<MOD, ROOT>* table = nullptr;
    if (cached) {
        table = &get_twiddle_cache<MOD, ROOT>();
        table->ensure(levels);
    }
#ifdef MAL_X86_SIMD
    const bool simd = cached && avx2_ok();
#else
    const bool simd = false;
#endif
    (void)simd;
    if (!invert) {
        // DIF: natural order -> bit-reversed order.
        if (cached) {
            int h = levels - 1;
            for (int len = n; len > 2; len >>= 1, --h) {
                const int half = len >> 1;
#ifdef MAL_X86_SIMD
                if (simd && half >= 8) {
                    dif_stage_avx2<MOD>(a, n, len, table->fwd[h],
                                        table->fwd_hi[h]);
                    continue;
                }
#endif
                const std::vector<std::uint32_t>& w = table->fwd[h];
                const std::vector<std::uint32_t>& wh = table->fwd_hi[h];
                for (int i = 0; i < n; i += len) {
                    for (int j = 0; j < half; ++j) {
                        const std::uint32_t u = a[i + j], v = a[i + j + half];
                        const std::uint32_t s = add_mod<MOD>(u, v);
                        const std::uint32_t d = sub_mod<MOD>(u, v);
                        a[i + j] = s;
                        // The twiddle only feeds this one store, so the
                        // high-multiply form hides its latency better than the
                        // 64-bit remainder it replaces.
                        a[i + j + half] = mul_mod_shoup<MOD>(d, w[j], wh[j]);
                    }
                }
            }
            // Last stage: every twiddle is one, so the multiply disappears.
            for (int i = 0; i < n; i += 2) {
                const std::uint32_t u = a[i], v = a[i + 1];
                a[i] = add_mod<MOD>(u, v);
                a[i + 1] = sub_mod<MOD>(u, v);
            }
        } else {
            for (int len = n; len > 1; len >>= 1) {
                const int half = len >> 1;
                const std::uint32_t wlen = mod_pow_const<MOD, ROOT>((MOD - 1) / len);
                for (int i = 0; i < n; i += len) {
                    dlimb w = 1;
                    for (int j = 0; j < half; ++j) {
                        const std::uint32_t u = a[i + j], v = a[i + j + half];
                        const std::uint32_t s = add_mod<MOD>(u, v);
                        const std::uint32_t d = sub_mod<MOD>(u, v);
                        a[i + j] = s;
                        a[i + j + half] = std::uint32_t(d * w % MOD);
                        w = w * wlen % MOD;
                    }
                }
            }
        }
    } else {
        // DIT: bit-reversed order -> natural order.
        // First stage: every twiddle is one, so the multiply disappears.
        for (int i = 0; i < n; i += 2) {
            const std::uint32_t u = a[i], v = a[i + 1];
            a[i] = add_mod<MOD>(u, v);
            a[i + 1] = sub_mod<MOD>(u, v);
        }
        for (int len = 4; len <= n; len <<= 1) {
            const int half = len >> 1;
            const int h = __builtin_ctz(std::uint32_t(half));
            if (cached) {
#ifdef MAL_X86_SIMD
                if (simd && half >= 8) {
                    ensure_inv_hi<MOD, ROOT>(*table, h);
                    dit_stage_avx2<MOD>(a, n, len, table->inv[h],
                                        table->inv_hi[h]);
                    continue;
                }
#endif
                const std::vector<std::uint32_t>& w = table->inv[h];
                for (int i = 0; i < n; i += len) {
                    for (int j = 0; j < half; ++j) {
                        const std::uint32_t u = a[i + j];
                        const std::uint32_t v =
                            std::uint32_t(dlimb(a[i + j + half]) * w[j] % MOD);
                        const std::uint32_t s = add_mod<MOD>(u, v);
                        const std::uint32_t d = sub_mod<MOD>(u, v);
                        a[i + j] = s;
                        a[i + j + half] = d;
                    }
                }
            } else {
                const std::uint32_t wlen =
                    mod_pow_const<MOD, ROOT>(MOD - 1 - (MOD - 1) / len);
                for (int i = 0; i < n; i += len) {
                    dlimb w = 1;
                    for (int j = 0; j < half; ++j) {
                        const std::uint32_t u = a[i + j];
                        const std::uint32_t v =
                            std::uint32_t(a[i + j + half] * w % MOD);
                        const std::uint32_t s = add_mod<MOD>(u, v);
                        const std::uint32_t d = sub_mod<MOD>(u, v);
                        a[i + j] = s;
                        a[i + j + half] = d;
                        w = w * wlen % MOD;
                    }
                }
            }
        }
        const std::uint32_t inv_n =
            cached ? table->inv2[levels] : mod_pow(std::uint32_t(n), MOD - 2, MOD);
#ifdef MAL_X86_SIMD
        if (simd && n >= 8) {
            scale_avx2<MOD>(a, n, inv_n);
            return;
        }
#endif
        for (int i = 0; i < n; ++i)
            a[i] = std::uint32_t(dlimb(a[i]) * inv_n % MOD);
    }
}

template <std::uint32_t MOD, std::uint32_t ROOT>
inline std::vector<std::uint32_t> convolution_mod(
    const std::vector<std::uint32_t>& x,
    const std::vector<std::uint32_t>& y,
    size_t n,
    size_t need) {
    std::vector<std::uint32_t> fa(n, 0), fb(n, 0), out(need, 0);
    for (size_t i = 0; i < x.size(); ++i) fa[i] = x[i] % MOD;
    for (size_t i = 0; i < y.size(); ++i) fb[i] = y[i] % MOD;
    transform<MOD, ROOT>(fa, false);
    transform<MOD, ROOT>(fb, false);
    for (size_t i = 0; i < n; ++i)
        fa[i] = std::uint32_t(dlimb(fa[i]) * fb[i] % MOD);
    transform<MOD, ROOT>(fa, true);
    std::copy(fa.begin(), fa.begin() + need, out.begin());
    return out;
}

template <std::uint32_t MOD, std::uint32_t ROOT>
inline std::vector<std::uint32_t> convolution_mod_sqr(
    const std::vector<std::uint32_t>& x,
    size_t n,
    size_t need) {
    std::vector<std::uint32_t> fa(n, 0), out(need, 0);
    for (size_t i = 0; i < x.size(); ++i) fa[i] = x[i] % MOD;
    transform<MOD, ROOT>(fa, false);
    for (size_t i = 0; i < n; ++i)
        fa[i] = std::uint32_t(dlimb(fa[i]) * fa[i] % MOD);
    transform<MOD, ROOT>(fa, true);
    std::copy(fa.begin(), fa.begin() + need, out.begin());
    return out;
}

} // namespace ntt

// Recombines the two convolution results by CRT and packs them back into
// 32-bit limbs.  Each coefficient fits in MOD1*MOD2, so the 16-bit digits are
// emitted as they are produced and only the (single digit wide) carry is left
// over at the end - no intermediate coefficient array.
template <std::uint32_t MOD1, std::uint32_t MOD2>
inline vec crt_pack(const std::vector<std::uint32_t>& r1,
                    const std::vector<std::uint32_t>& r2,
                    size_t need) {
    const std::uint32_t inv = ntt::mod_pow(MOD1 % MOD2, MOD2 - 2, MOD2);
    const std::uint32_t inv_hi = std::uint32_t((dlimb(inv) << LIMB_BITS) / MOD2);
    vec out((need + 2) >> 1, 0);
    dlimb carry = 0;
    for (size_t i = 0; i < need; ++i) {
        const dlimb diff = r2[i] >= r1[i] ? dlimb(r2[i] - r1[i])
                                          : dlimb(MOD2) + r2[i] - r1[i];
        const std::uint32_t t =
            ntt::mul_mod_shoup<MOD2>(std::uint32_t(diff), inv, inv_hi);
        const dlimb cur = dlimb(r1[i]) + dlimb(MOD1) * t + carry;
        const limb low = limb(cur & 0xffffu);
        if (i & 1) out[i >> 1] |= low << 16;
        else out[i >> 1] = low;
        carry = cur >> 16;
    }
    if (carry) {
        const limb top = limb(carry & 0xffffu);
        if (need & 1) out[need >> 1] |= top << 16;
        else out[need >> 1] = top;
    }
    trim(out);
    return out;
}

template <std::uint32_t MOD1, std::uint32_t ROOT1,
          std::uint32_t MOD2, std::uint32_t ROOT2>
inline vec mul_ntt_pair(
    const std::vector<std::uint32_t>& x,
    const std::vector<std::uint32_t>& y,
    size_t n,
    size_t need) {
    // For every supported transform size each exact convolution coefficient
    // is below MOD1 * MOD2, so two NTTs recover it by CRT without any
    // floating-point error analysis.
    const std::vector<std::uint32_t> r1 = ntt::convolution_mod<MOD1, ROOT1>(x, y, n, need);
    const std::vector<std::uint32_t> r2 = ntt::convolution_mod<MOD2, ROOT2>(x, y, n, need);
    return crt_pack<MOD1, MOD2>(r1, r2, need);
}

inline vec mul_ntt(const vec& a, const vec& b) {
    if (a.empty() || b.empty()) return vec();
    // 32-bit limbs -> 16-bit digits.  This is the representation whose
    // coefficients are reconstructed exactly from the two NTT moduli.
    std::vector<std::uint32_t> x;
    std::vector<std::uint32_t> y;
    x.reserve(a.size() * 2);
    y.reserve(b.size() * 2);
    for (limb v : a) {
        x.push_back(v & 0xffffu);
        x.push_back(v >> 16);
    }
    for (limb v : b) {
        y.push_back(v & 0xffffu);
        y.push_back(v >> 16);
    }
    trim(x);
    trim(y);
    if (x.empty() || y.empty()) return vec();
    const size_t need = x.size() + y.size() - 1;
    size_t n = 1;
    while (n < need) n <<= 1;
    if (n <= ntt::MAX_SMALL) {
        return mul_ntt_pair<ntt::SMALL_MOD1, ntt::SMALL_ROOT,
                            ntt::SMALL_MOD2, ntt::SMALL_ROOT>(x, y, n, need);
    }
    if (n <= ntt::MAX_LARGE) {
        return mul_ntt_pair<ntt::LARGE_MOD1, ntt::LARGE_ROOT1,
                            ntt::LARGE_MOD2, ntt::LARGE_ROOT2>(x, y, n, need);
    }
    throw std::length_error(
        "mal::BigInt NTT backend supports up to 2^27 transform slots; "
        "this is about 3.2e8 decimal digits");
}

// Decimal conversion divides by the same power of ten at every node of a
// level, so the forward transform of that divisor - and of the reciprocal used
// to estimate the quotient - can be computed once and reused.  A product
// against a stored transform then costs one transform instead of two.  Only
// the small-prime transforms are kept, which covers conversions of a few
// million digits within a bounded amount of memory.
constexpr size_t CACHED_FACTOR_MAX_SLOTS = size_t(1) << 19;

struct cached_ntt_factor {
    size_t n = 0;      // slot count of the stored transform, 0 when unused
    size_t digits = 0; // 16-bit digit count of the operand
    std::vector<std::uint32_t> t1, t2;
    // Shoup quotients of the stored transforms, for the pointwise product.
    std::vector<std::uint32_t> t1_hi, t2_hi;
};

inline size_t ntt_slots_for(size_t need) {
    size_t n = 1;
    while (n < need) n <<= 1;
    return n;
}

inline void split_digits(const vec& a, std::vector<std::uint32_t>& d) {
    d.clear();
    d.reserve(a.size() * 2);
    for (limb v : a) {
        d.push_back(v & 0xffffu);
        d.push_back(v >> 16);
    }
    trim(d);
}

// Forward transform of b at exactly n slots, for reuse by many products.
inline bool build_cached_factor(const vec& b, cached_ntt_factor& f, size_t n) {
    if (n == 0 || n > ntt::MAX_SMALL || n > CACHED_FACTOR_MAX_SLOTS) return false;
    std::vector<std::uint32_t> d;
    split_digits(b, d);
    if (d.empty() || d.size() > n) return false;
    f.n = n;
    f.digits = d.size();
    f.t1.assign(n, 0);
    f.t2.assign(n, 0);
    std::copy(d.begin(), d.end(), f.t1.begin());
    std::copy(d.begin(), d.end(), f.t2.begin());
    ntt::transform<ntt::SMALL_MOD1, ntt::SMALL_ROOT>(f.t1, false);
    ntt::transform<ntt::SMALL_MOD2, ntt::SMALL_ROOT>(f.t2, false);
#ifdef MAL_X86_SIMD
    if (ntt::avx2_ok()) {
        f.t1_hi.resize(n);
        f.t2_hi.resize(n);
        for (size_t i = 0; i < n; ++i) {
            f.t1_hi[i] = std::uint32_t((dlimb(f.t1[i]) << LIMB_BITS) / ntt::SMALL_MOD1);
            f.t2_hi[i] = std::uint32_t((dlimb(f.t2[i]) << LIMB_BITS) / ntt::SMALL_MOD2);
        }
    }
#endif
    return true;
}

#ifdef MAL_X86_SIMD
template <std::uint32_t MOD>
__attribute__((target("avx2"))) inline void pointwise_avx2(
    std::vector<std::uint32_t>& fa, const std::vector<std::uint32_t>& tb,
    const std::vector<std::uint32_t>& tb_hi, size_t n) {
    size_t i = 0;
    for (; i + 8 <= n; i += 8) {
        const __m256i x = _mm256_loadu_si256((const __m256i*)&fa[i]);
        const __m256i w = _mm256_loadu_si256((const __m256i*)&tb[i]);
        const __m256i wh = _mm256_loadu_si256((const __m256i*)&tb_hi[i]);
        _mm256_storeu_si256((__m256i*)&fa[i], ntt::shoup8<MOD>(x, w, wh));
    }
    for (; i < n; ++i)
        fa[i] = std::uint32_t(dlimb(fa[i]) * tb[i] % MOD);
}
#endif

template <std::uint32_t MOD, std::uint32_t ROOT>
inline void conv_against_transform(const std::vector<std::uint32_t>& d,
                                   const std::vector<std::uint32_t>& tb,
                                   const std::vector<std::uint32_t>& tb_hi,
                                   size_t n, size_t need,
                                   std::vector<std::uint32_t>& out) {
    std::vector<std::uint32_t> fa(n, 0);
    for (size_t i = 0; i < d.size(); ++i) fa[i] = d[i] % MOD;
    ntt::transform<MOD, ROOT>(fa, false);
#ifdef MAL_X86_SIMD
    if (!tb_hi.empty() && ntt::avx2_ok()) pointwise_avx2<MOD>(fa, tb, tb_hi, n);
    else
#endif
        for (size_t i = 0; i < n; ++i)
            fa[i] = std::uint32_t(dlimb(fa[i]) * tb[i] % MOD);
    ntt::transform<MOD, ROOT>(fa, true);
    out.assign(fa.begin(), fa.begin() + need);
}

// a * b where b has a stored transform.  Falls back to the normal path when
// the product does not land on the stored slot count, so the caller never pays
// for a padded transform.
inline vec mul_mag_cached(const vec& a, const vec& b, const cached_ntt_factor* f) {
    if (!f || f->n == 0 || a.empty() || b.empty()) return mul_mag(a, b);
    std::vector<std::uint32_t> da;
    split_digits(a, da);
    if (da.empty()) return vec();
    const size_t need = da.size() + f->digits - 1;
    if (need > f->n || ntt_slots_for(need) != f->n) return mul_mag(a, b);
    std::vector<std::uint32_t> r1, r2;
    conv_against_transform<ntt::SMALL_MOD1, ntt::SMALL_ROOT>(da, f->t1, f->t1_hi, f->n, need, r1);
    conv_against_transform<ntt::SMALL_MOD2, ntt::SMALL_ROOT>(da, f->t2, f->t2_hi, f->n, need, r2);
    return crt_pack<ntt::SMALL_MOD1, ntt::SMALL_MOD2>(r1, r2, need);
}

template <std::uint32_t MOD1, std::uint32_t ROOT1,
          std::uint32_t MOD2, std::uint32_t ROOT2>
inline vec sqr_ntt_pair(const std::vector<std::uint32_t>& x, size_t n, size_t need) {
    const std::vector<std::uint32_t> r1 =
        ntt::convolution_mod_sqr<MOD1, ROOT1>(x, n, need);
    const std::vector<std::uint32_t> r2 =
        ntt::convolution_mod_sqr<MOD2, ROOT2>(x, n, need);
    return crt_pack<MOD1, MOD2>(r1, r2, need);
}

inline vec sqr_ntt(const vec& a) {
    if (a.empty()) return vec();
    std::vector<std::uint32_t> x;
    x.reserve(a.size() * 2);
    for (limb v : a) {
        x.push_back(v & 0xffffu);
        x.push_back(v >> 16);
    }
    trim(x);
    if (x.empty()) return vec();
    const size_t need = x.size() * 2 - 1;
    size_t n = 1;
    while (n < need) n <<= 1;
    if (n <= ntt::MAX_SMALL) {
        return sqr_ntt_pair<ntt::SMALL_MOD1, ntt::SMALL_ROOT,
                            ntt::SMALL_MOD2, ntt::SMALL_ROOT>(x, n, need);
    }
    if (n <= ntt::MAX_LARGE) {
        return sqr_ntt_pair<ntt::LARGE_MOD1, ntt::LARGE_ROOT1,
                            ntt::LARGE_MOD2, ntt::LARGE_ROOT2>(x, n, need);
    }
    throw std::length_error(
        "mal::BigInt NTT backend supports up to 2^27 transform slots; "
        "this is about 3.2e8 decimal digits");
}

inline vec sqr_mag(const vec& a) {
    if (a.empty()) return vec();
    if (a.size() <= KARATSUBA_THRESHOLD) return mul_basic(a, a);
    if (a.size() >= FFT_THRESHOLD) return sqr_ntt(a);
    return mul_mag(a, a);
}

// Overlap-add for a very unbalanced product: the short operand is transformed
// once, then reused for each block of the long one.  Returns false when the
// shape does not fit the transform budget, in which case the caller falls back.
inline bool mul_blocked(const vec& large, const vec& small, vec& out) {
    if (small.empty() || large.empty()) {
        out.clear();
        return true;
    }
    size_t n = 1;
    while (n < 2 * small.size()) n <<= 1;
    if (n > CACHED_FACTOR_MAX_SLOTS || n > ntt::MAX_SMALL) return false;
    cached_ntt_factor f;
    if (!build_cached_factor(small, f, n)) return false;
    const size_t half = n / 2;                       // limb capacity of one block + operand
    if (half <= small.size()) return false;
    const size_t block = half - small.size() + 1;
    out.clear();
    for (size_t l = 0; l < large.size(); l += block) {
        const size_t len = std::min(block, large.size() - l);
        const vec part(large.begin() + l, large.begin() + l + len);
        const vec prod = mul_mag_cached(part, small, &f);
        out = add_at(out, prod, l, large.size() + small.size() + 1);
    }
    trim(out);
    return true;
}

inline vec mul_mag(const vec& a, const vec& b) {
    if (a.empty() || b.empty()) return vec();
    const size_t mn = std::min(a.size(), b.size());
    const size_t mx = std::max(a.size(), b.size());
    if (mn <= KARATSUBA_THRESHOLD) return mul_basic(a, b);
    // A short operand makes the products overlap: transform it once and reuse
    // that transform for every block of the long operand.
    if (mx >= 4 * mn && mx >= FFT_MIN_OPERAND) {
        vec out;
        if (mul_blocked(a.size() >= b.size() ? a : b, a.size() >= b.size() ? b : a, out))
            return out;
    }
    if (mx >= FFT_THRESHOLD && mn >= FFT_THRESHOLD / 2) return mul_ntt(a, b);
    // Below that the choice depends on how much the transform has to pad: the
    // convolution needs 2*(mx+mn) 16-bit slots rounded up to a power of two,
    // and paying for slots that stay zero is only worth it when the padding
    // wastes less than a third of the transform.  Shapes that land just above
    // a power of two (640 by 512 limbs, say) are better off with Karatsuba,
    // which has no such step.
    if (mx >= FFT_MIN_OPERAND) {
        const size_t need = 2 * (mx + mn);
        size_t slots = 1;
        while (slots < need) slots <<= 1;
        if (3 * need >= 2 * slots) return mul_ntt(a, b);
    }
    if (mn >= TOOM_THRESHOLD && mx <= mn * 2) return mul_toom3(a, b);
    return mul_karatsuba(a, b);
}

inline vec shift_left_mag(const vec& a, size_t bits) {
    if (a.empty() || bits == 0) return a;
    const size_t ls = bits / LIMB_BITS, bs = bits % LIMB_BITS;
    vec c(a.size() + ls + (bs ? 1 : 0), 0);
    if (!bs) {
        std::copy(a.begin(), a.end(), c.begin() + ls);
    } else {
        for (size_t i = 0; i < a.size(); ++i) {
            dlimb x = dlimb(a[i]) << bs;
            c[i + ls] |= limb(x);
            c[i + ls + 1] |= limb(x >> LIMB_BITS);
        }
    }
    trim(c);
    return c;
}

inline vec shift_right_mag(const vec& a, size_t bits) {
    const size_t ls = bits / LIMB_BITS, bs = bits % LIMB_BITS;
    if (ls >= a.size()) return vec();
    vec c(a.size() - ls);
    if (!bs) {
        std::copy(a.begin() + ls, a.end(), c.begin());
    } else {
        for (size_t i = 0; i < c.size(); ++i) {
            dlimb x = dlimb(a[i + ls]) >> bs;
            if (i + ls + 1 < a.size()) x |= dlimb(a[i + ls + 1]) << (LIMB_BITS - bs);
            c[i] = limb(x);
        }
    }
    trim(c);
    return c;
}

inline size_t bit_length_mag(const vec& a) {
    if (a.empty()) return 0;
    return (a.size() - 1) * LIMB_BITS + (LIMB_BITS - clz32(a.back()));
}

inline bool bit_mag(const vec& a, size_t i) {
    return i / LIMB_BITS < a.size() && ((a[i / LIMB_BITS] >> (i % LIMB_BITS)) & 1);
}

inline bool any_low_bits_mag(const vec& a, size_t bits) {
    if (bits == 0) return false;
    const size_t full = bits / LIMB_BITS, rem = bits % LIMB_BITS;
    for (size_t i = 0; i < full && i < a.size(); ++i)
        if (a[i]) return true;
    if (rem && full < a.size() && (a[full] & ((limb(1) << rem) - 1))) return true;
    return false;
}

inline void divmod_mag_small(const vec& a, limb d, vec& q, limb& rem) {
    if (a.empty()) {
        q.clear();
        rem = 0;
        return;
    }
    q.assign(a.size(), 0);
    dlimb r = 0;
    for (size_t i = a.size(); i--;) {
        dlimb cur = (r << LIMB_BITS) | a[i];
        q[i] = limb(cur / d);
        r = cur % d;
    }
    rem = limb(r);
    trim(q);
}

// Same as divmod_mag_small but in place, for callers that break a value up
// chunk by chunk and would otherwise copy the shrinking remainder every time.
inline void divmod_mag_small_inplace(vec& a, limb d, limb& rem) {
    dlimb r = 0;
    for (size_t i = a.size(); i--;) {
        const dlimb cur = (r << LIMB_BITS) | a[i];
        a[i] = limb(cur / d);
        r = cur % d;
    }
    rem = limb(r);
    trim(a);
}

inline void divmod_mag_knuth(const vec& u0, const vec& v, vec& q, vec& r) {
    // Knuth Algorithm D.
    if (v.empty()) throw std::domain_error("division by zero");
    if (cmp_mag(u0, v) < 0) {
        q.clear();
        r = u0;
        return;
    }
    if (v.size() == 1) {
        limb rem = 0;
        divmod_mag_small(u0, v[0], q, rem);
        r.clear();
        if (rem) r.push_back(rem);
        return;
    }

    const size_t n = v.size();
    const size_t m = u0.size() - n;
    const int s = clz32(v.back());
    const vec vn = shift_left_mag(v, s);
    vec un = shift_left_mag(u0, s);
    un.resize(u0.size() + 1, 0);

    vec qq(m + 1, 0);
    const dlimb B = LIMB_BASE;
    for (size_t jj = m + 1; jj--;) {
        const size_t j = jj;
        dlimb nh = (dlimb(un[j + n]) << LIMB_BITS) | un[j + n - 1];
        dlimb qhat = nh / vn[n - 1];
        dlimb rhat = nh % vn[n - 1];
        while (qhat >= B ||
               qhat * vn[n - 2] > ((rhat << LIMB_BITS) | un[j + n - 2])) {
            --qhat;
            rhat += vn[n - 1];
            if (rhat >= B) break;
        }

        dlimb carry = 0;
        dlimb borrow = 0;
        for (size_t i = 0; i < n; ++i) {
            dlimb p = qhat * vn[i] + carry;
            carry = p >> LIMB_BITS;
            dlimb sub = (p & (B - 1)) + borrow;
            if (sub >= B) {
                sub -= B;
                ++carry;
            }
            if (un[j + i] >= sub) {
                un[j + i] = limb(dlimb(un[j + i]) - sub);
                borrow = 0;
            } else {
                un[j + i] = limb(dlimb(un[j + i]) + B - sub);
                borrow = 1;
            }
        }
        dlimb top = dlimb(un[j + n]) - carry - borrow;
        bool negative = (top >> LIMB_BITS) != 0;
        if (negative) {
            --qhat;
            dlimb c = 0;
            for (size_t i = 0; i < n; ++i) {
                dlimb x = dlimb(un[j + i]) + vn[i] + c;
                un[j + i] = limb(x);
                c = x >> LIMB_BITS;
            }
            un[j + n] = limb(dlimb(un[j + n]) + c);
        } else {
            un[j + n] = limb(top);
        }
        qq[j] = limb(qhat);
    }
    trim(qq);
    q = qq;
    r.assign(un.begin(), un.begin() + n);
    trim(r);
    r = shift_right_mag(r, s);
}

inline bool divmod_mag_newton(const vec& u, const vec& v, vec& q, vec& r) {
    // Fixed-point Newton reciprocal.  This is the large-quotient path; it is
    // only entered when the quotient is wide enough to amortise the Newton
    // iterations.
    if (v.size() < NEWTON_DIV_MIN_DIVISOR_LIMBS) return false;
    const size_t du = bit_length_mag(u);
    const size_t dv = bit_length_mag(v);
    if (du < dv) {
        q.clear();
        r = u;
        return true;
    }
    const size_t qbits = du - dv + 1;
    if (qbits < NEWTON_DIV_MIN_QUOTIENT_BITS) return false;

    const vec one(1, 1);
    const size_t guard = 64;
    const size_t scale_bits = qbits + guard;
    const size_t M = dv + scale_bits;

    // Initial reciprocal: use the top 32 bits of v.  The relative error is
    // below 2^-31, which is more than enough for Newton convergence.
    const size_t top_bits = 32;
    vec top = shift_right_mag(v, dv - top_bits);
    if (top.empty() || top[0] == 0) return false;
    const limb vh = top[0];
    const size_t shift0 = M - dv + top_bits;
    vec R;
    limb rem0 = 0;
    divmod_mag_small(shift_left_mag(one, shift0), vh, R, rem0);
    if (R.empty()) return false;

    const vec two_pow = shift_left_mag(one, M + 1);
    size_t accuracy = 31;
    const size_t needed = scale_bits + 16;
    for (int iter = 0; accuracy < needed && iter < 128; ++iter) {
        vec vR = mul_mag(v, R);
        if (cmp_mag(vR, two_pow) > 0) return false;
        vec T = sub_mag(two_pow, vR);
        R = shift_right_mag(mul_mag(R, T), M);
        accuracy <<= 1;
        if (accuracy == 0) break;
    }

    vec qq = shift_right_mag(mul_mag(u, R), M);
    vec pv = mul_mag(qq, v);
    int adjust = 0;
    while (cmp_mag(pv, u) > 0) {
        if (qq.empty() || adjust++ > 1024) return false;
        qq = sub_mag(qq, one);
        pv = sub_mag(pv, v);
    }
    adjust = 0;
    for (;;) {
        vec nxt = add_mag(pv, v);
        if (cmp_mag(nxt, u) > 0) break;
        if (adjust++ > 1024) return false;
        pv = nxt;
        qq = add_mag(qq, one);
    }
    trim(qq);
    q = qq;
    r = sub_mag(u, pv);
    return true;
}

// Recursive fixed-point reciprocal: returns floor(B^(2*m) / a), where
// m = a.size() and B = 2^32.  This is the precision-doubling Newton scheme
// used by high-performance integer libraries.
inline vec inv_mag(const vec& a) {
    const size_t m = a.size();
    if (m <= 32) {
        vec num(2 * m + 1, 0);
        num.back() = 1;
        vec q, r;
        divmod_mag_knuth(num, a, q, r);
        return q;
    }
    const size_t k = (m + 5) >> 1;
    vec b(a.end() - k, a.end());
    b = inv_mag(b);
    const vec bsq = sqr_mag(b);
    vec t = mul_mag(a, bsq);
    t = shift_right_mag(t, 64 * k);
    vec two_b = add_mag(b, b);
    two_b = shift_left_mag(two_b, 32 * (m - k));
    vec ans = sub_mag(two_b, t);
    ans = sub_mag(ans, vec(1, 1));
    return ans;
}

inline bool divmod_mag_recip(const vec& u, const vec& v, vec& q, vec& r) {
    const size_t n0 = u.size();
    size_t m = v.size();
    // n0 <= m is written as a separate test: with u shorter than v the
    // subtraction below would wrap and the narrow-quotient branch would be
    // entered with a huge limb count.  Knuth handles u < v directly.
    if (m <= 32 || n0 <= m + 32) return false;

    vec us = u;
    vec vs = v;
    vec qq;
    const size_t narrow = n0 - m + 1;             // quotient limb bound
    if (n0 <= m + m / 2 && m > narrow + 2) {
        // Narrow quotient: the top K+2 limbs of the divisor decide the
        // result, so the Newton reciprocal is built from that slice only.
        const size_t k = narrow;
        const size_t s = k + 2;
        const vec vh = shift_right_mag(v, 32 * (m - s));
        const vec rec = inv_mag(vh);
        const vec uh = shift_right_mag(u, 32 * (n0 - s));
        qq = shift_right_mag(mul_mag(uh, rec), 64 * s - 32 * (k - 1));
    } else {
    if (n0 > 2 * m) {
        const size_t sh = n0 - 2 * m;
        us = shift_left_mag(u, 32 * sh);
        vs = shift_left_mag(v, 32 * sh);
        m = n0 - m;
    }
    const vec inv = inv_mag(vs);
    // Both operands of the estimate are truncated: the top |us|-m+1 limbs of
    // the dividend and the top qlimbs+2 limbs of the reciprocal, each with an
    // error below one unit.  A narrow quotient then costs a product of
    // quotient width.
    const size_t qlimbs = us.size() - m + 1;
    const size_t keep = qlimbs + 2;
    const size_t inv_cut = inv.size() > keep ? inv.size() - keep : 0;
    const vec top = shift_right_mag(us, 32 * (m - 1));
    const vec inv_h = inv_cut ? shift_right_mag(inv, 32 * inv_cut) : inv;
    const size_t shift = 64 * m - 32 * (m - 1) - 32 * inv_cut;
    qq = shift_right_mag(mul_mag(top, inv_h), shift);
    }
    vec pv = mul_mag(qq, v);
    int guard = 0;
    while (cmp_mag(pv, u) > 0) {
        if (qq.empty() || guard++ > 1024) return false;
        qq = sub_mag(qq, vec(1, 1));
        pv = sub_mag(pv, v);
    }
    guard = 0;
    for (;;) {
        vec nxt = add_mag(pv, v);
        if (cmp_mag(nxt, u) > 0) break;
        if (guard++ > 1024) return false;
        pv = nxt;
        qq = add_mag(qq, vec(1, 1));
    }
    trim(qq);
    q = qq;
    r = sub_mag(u, pv);
    return true;
}

inline void divmod_mag(const vec& u, const vec& v, vec& q, vec& r) {
    if (v.empty()) throw std::domain_error("division by zero");
    if (divmod_mag_recip(u, v, q, r)) return;
    divmod_mag_knuth(u, v, q, r);
}

inline void divmod_by_reciprocal(const vec& u, const vec& v, const vec& rec,
                                 size_t scale_bits, vec& q, vec& r,
                                 const cached_ntt_factor* rec_factor = nullptr,
                                 const cached_ntt_factor* div_factor = nullptr) {
    // Barrett division with a precomputed fixed-point reciprocal.  Only the
    // top m+1 limbs of u matter (the dropped low bits contribute < 1 to
    // u*rec/2^scale), so the estimate uses operands of the divisor's size.
    // The result is at most two short of floor(u/v); the loop below fixes it.
    const vec one(1, 1);
    const size_t m = v.size();
    const size_t cut = 32 * (m - 1);
    const vec hi = cut ? shift_right_mag(u, cut) : u;
    q = shift_right_mag(mul_mag_cached(hi, rec, rec_factor), scale_bits - cut);
    vec pv = mul_mag_cached(q, v, div_factor);
    while (cmp_mag(pv, u) > 0) {
        q = sub_mag(q, one);
        pv = sub_mag(pv, v);
    }
    for (;;) {
        vec nxt = add_mag(pv, v);
        if (cmp_mag(nxt, u) > 0) break;
        pv = nxt;
        q = add_mag(q, one);
    }
    r = sub_mag(u, pv);
}

} // namespace bigint_detail

class BigInt {
    using limb = bigint_detail::limb;
    using vec = bigint_detail::vec;

    bool neg_;
    vec d_;

    void trim_() {
        bigint_detail::trim(d_);
        if (d_.empty()) neg_ = false;
    }

    static BigInt from_mag(vec v, bool neg) {
        BigInt r;
        r.neg_ = neg;
        r.d_ = std::move(v);
        r.trim_();
        return r;
    }

    static int cmp_abs(const BigInt& a, const BigInt& b) {
        return bigint_detail::cmp_mag(a.d_, b.d_);
    }

    // Writes exactly nine zero padded decimal digits of v at out[pos].
    static void write_chunk9(std::string& out, size_t pos, limb v);
    // Powers of 10^(9*2^k) and their fixed-point reciprocals only depend on the
    // level, so they are computed once per program instead of once per call.
    struct decimal_tables {
        std::vector<BigInt> powers;
        std::vector<BigInt> recips;
        std::vector<size_t> scales;
        // Forward transforms of the divisors and their reciprocals.  Nodes of
        // one level divide by the same power of ten, so these are shared.
        std::vector<bigint_detail::cached_ntt_factor> rec_factors;
        std::vector<bigint_detail::cached_ntt_factor> div_factors;
    };

    static const decimal_tables& decimal_tables_for(int level) {
        decimal_tables& tables = decimal_tables_store();
        decimal_powers(level);
        while ((int)tables.recips.size() < level) {
            const int k = (int)tables.recips.size();
            const size_t bits = tables.powers[k].bit_length();
            const size_t m = 2 * bits + 64;
            tables.recips.push_back((BigInt(1) << m) / tables.powers[k]);
            tables.scales.push_back(m);
        }
        ensure_div_factors(tables, level);
        while ((int)tables.rec_factors.size() < level) {
            const int k = (int)tables.rec_factors.size();
            tables.rec_factors.emplace_back();
            const size_t m = tables.powers[k].d_.size();
            // Only levels whose products would pick the transform anyway:
            // below that Karatsuba/Toom is faster and caching a transform
            // would also force the wrong backend.
            if (m < bigint_detail::FFT_MIN_OPERAND) continue;
            std::vector<limb> digits;
            bigint_detail::split_digits(tables.recips[k].d_, digits);
            // A node's dividend has at most m+1 limbs above the cut and the
            // dividend's low limbs are dropped before the product.
            bigint_detail::build_cached_factor(
                tables.recips[k].d_, tables.rec_factors[k],
                bigint_detail::ntt_slots_for(2 * (m + 1) + digits.size() - 1));
        }
        return tables;
    }

    // Transform of 10^(9*2^k).  Reading decimals multiplies by it and writing
    // them multiplies the quotient by it, so both paths share the cache.
    static void ensure_div_factors(decimal_tables& tables, int level) {
        while ((int)tables.div_factors.size() < level) {
            const size_t k = tables.div_factors.size();
            tables.div_factors.emplace_back();
            const size_t m = tables.powers[k].d_.size();
            if (m < bigint_detail::FFT_MIN_OPERAND) continue;
            std::vector<limb> digits;
            bigint_detail::split_digits(tables.powers[k].d_, digits);
            // Both operands of these products are about m limbs wide.
            bigint_detail::build_cached_factor(
                tables.powers[k].d_, tables.div_factors[k],
                bigint_detail::ntt_slots_for(2 * m + digits.size() - 1));
        }
    }

    // Powers plus the products that reading decimals performs against them;
    // the reciprocals that writing needs are only built by
    // decimal_tables_for.
    static const decimal_tables& decimal_tables_for_parse(int level) {
        decimal_tables& tables = decimal_tables_store();
        decimal_powers(level);
        ensure_div_factors(tables, level);
        return tables;
    }

    // a * 10^(9*2^k) reusing the stored transform of that power.
    static BigInt scale_by_power(const BigInt& a, const decimal_tables& tables, size_t k) {
        return from_mag(bigint_detail::mul_mag_cached(a.d_, tables.powers[k].d_,
                                                      &tables.div_factors[k]),
                        a.neg_);
    }

    static decimal_tables& decimal_tables_store() {
        static decimal_tables tables;
        return tables;
    }

    // 10^(9*2^k) for k < level.  Parsing only needs these, so the reciprocals
    // that to_string adds on top are kept out of this path.
    static const std::vector<BigInt>& decimal_powers(int level) {
        decimal_tables& tables = decimal_tables_store();
        while ((int)tables.powers.size() < level) {
            if (tables.powers.empty()) tables.powers.push_back(BigInt(1000000000));
            else tables.powers.push_back(tables.powers.back().sqr());
        }
        return tables.powers;
    }

    static int ceil_log2_size(size_t x) {
        int r = 0;
        size_t v = 1;
        while (v < x) {
            v <<= 1;
            ++r;
        }
        return r;
    }
    // Writes the value of x as exactly 9 * 2^level decimal digits at out[pos].
    // Every node of the recursion fills its whole slot, so the digits can be
    // emitted straight into their final place without any string growth.
    static void to_decimal_rec(const BigInt& x, int level,
                               const decimal_tables& tables,
                               std::string& out, size_t pos);
    static BigInt from_decimal_rec(const std::vector<limb>& chunks, size_t l, int level,
                                   const decimal_tables& tables);

public:
    static constexpr size_t ntt_max_slots() { return size_t(1) << 27; }
    static constexpr size_t ntt_max_decimal_digits() { return 323228993; }
    static constexpr size_t karatsuba_threshold() { return bigint_detail::KARATSUBA_THRESHOLD; }
    static constexpr size_t toom_threshold() { return bigint_detail::TOOM_THRESHOLD; }
    static constexpr size_t fft_threshold() { return bigint_detail::FFT_THRESHOLD; }

    BigInt() : neg_(false) {}

    BigInt(long long x) : neg_(x < 0) {
        unsigned long long v = x < 0 ? 0ull - (unsigned long long)x : (unsigned long long)x;
        while (v) {
            d_.push_back(limb(v));
            v >>= bigint_detail::LIMB_BITS;
        }
    }

    explicit BigInt(const std::string& s, int base = 10) : neg_(false) {
        *this = from_string(s, base);
    }

    bool is_zero() const { return d_.empty(); }
    bool is_negative() const { return neg_; }
    int sign() const { return d_.empty() ? 0 : (neg_ ? -1 : 1); }

    size_t limb_count() const { return d_.size(); }
    const std::vector<limb>& limbs() const { return d_; }
    size_t bit_length() const { return bigint_detail::bit_length_mag(d_); }
    bool bit(size_t i) const { return bigint_detail::bit_mag(d_, i); }
    bool any_low_bits(size_t bits) const { return bigint_detail::any_low_bits_mag(d_, bits); }

    unsigned long long low64() const {
        unsigned long long r = 0;
        if (!d_.empty()) r = d_[0];
        if (d_.size() > 1) r |= (unsigned long long)d_[1] << 32;
        return r;
    }

    BigInt abs() const { return from_mag(d_, false); }
    BigInt operator-() const { return from_mag(d_, !neg_); }
    BigInt operator+() const { return *this; }

    static BigInt from_limbs(const std::vector<limb>& v, bool neg = false) {
        return from_mag(v, neg);
    }

    static BigInt from_string(const std::string& text, int base = 10) {
        if (base < 2 || base > 36) throw std::invalid_argument("invalid base");
        size_t i = 0;
        bool neg = false;
        if (i < text.size() && (text[i] == '+' || text[i] == '-')) {
            neg = text[i] == '-';
            ++i;
        }
        if (i == text.size()) throw std::invalid_argument("empty number");
        // Beyond a few dozen chunks the divide-and-conquer reader beats the
        // digit-at-a-time loop by an order of magnitude; below it the loop is
        // cheaper than setting the recursion up.
        if (base == 10 && text.size() - i >= 128) {
            const size_t n = text.size() - i;
            const size_t chunks = (n + 8) / 9;
            const int level = ceil_log2_size(chunks);
            const size_t total = size_t(1) << level;
            std::vector<limb> cv(total, 0);
            size_t pos = i;
            size_t first = n - (chunks - 1) * 9;
            for (size_t k = 0; k < chunks; ++k) {
                const size_t len = k == 0 ? first : 9;
                bigint_detail::dlimb v = 0;
                for (size_t j = 0; j < len; ++j) {
                    char c = text[pos++];
                    if (c < '0' || c > '9') throw std::invalid_argument("invalid digit");
                    v = v * 10 + (c - '0');
                }
                cv[total - chunks + k] = limb(v);
            }
            BigInt r = from_decimal_rec(cv, 0, level, decimal_tables_for_parse(level));
            r.neg_ = neg && !r.d_.empty();
            return r;
        }
        BigInt r;
        for (; i < text.size(); ++i) {
            int c = text[i];
            int d = c >= '0' && c <= '9' ? c - '0'
                    : c >= 'a' && c <= 'z' ? c - 'a' + 10
                    : c >= 'A' && c <= 'Z' ? c - 'A' + 10
                    : -1;
            if (d < 0 || d >= base) throw std::invalid_argument("invalid digit");
            r *= BigInt(base);
            if (d) r += BigInt(d);
        }
        r.neg_ = neg && !r.d_.empty();
        return r;
    }

    std::string to_string(int base = 10) const {
        if (base < 2 || base > 36) throw std::invalid_argument("invalid base");
        if (is_zero()) return "0";
        // From about a dozen limbs on the divide-and-conquer writer is ahead
        // of the chunk-at-a-time loop, so it takes over early.
        if (base == 10 && d_.size() >= 16) {
            size_t chunks = (bigint_detail::bit_length_mag(d_) + 28) / 29;
            const int level = ceil_log2_size(chunks);
            const decimal_tables& tables = decimal_tables_for(level);
            std::string out(size_t(9) << level, '0');
            to_decimal_rec(abs(), level, tables, out, 0);
            size_t p = out.find_first_not_of('0');
            if (p == std::string::npos) out = "0";
            else if (p) out.erase(0, p);
            if (neg_) out.insert(out.begin(), '-');
            return out;
        }
        static const char* digits = "0123456789abcdefghijklmnopqrstuvwxyz";
        std::string out;
        BigInt t = abs();
        if (base == 10) {
            const limb chunk = 1000000000u;
            std::vector<limb> parts;
            while (!t.is_zero()) {
                limb rem = 0;
                t = t.div_small(chunk, &rem);
                parts.push_back(rem);
            }
            for (size_t i = parts.size(); i--;) {
                if (i + 1 == parts.size()) {
                    out += std::to_string(parts[i]);
                } else {
                    std::string x = std::to_string(parts[i]);
                    out.append(9 - x.size(), '0');
                    out += x;
                }
            }
        } else {
            while (!t.is_zero()) {
                limb rem = 0;
                t = t.div_small(limb(base), &rem);
                out.push_back(digits[rem]);
            }
            std::reverse(out.begin(), out.end());
        }
        if (neg_) out.insert(out.begin(), '-');
        return out;
    }

    BigInt div_small(limb d, limb* rem = nullptr) const {
        if (d == 0) throw std::domain_error("division by zero");
        vec q;
        limb r = 0;
        bigint_detail::divmod_mag_small(d_, d, q, r);
        if (rem) *rem = r;
        return from_mag(q, neg_ && !q.empty());
    }

    static void divmod(const BigInt& a, const BigInt& b, BigInt& q, BigInt& r) {
        if (b.is_zero()) throw std::domain_error("division by zero");
        vec qv, rv;
        bigint_detail::divmod_mag(a.d_, b.d_, qv, rv);
        const bool qneg = (a.neg_ != b.neg_) && !qv.empty();
        const bool rneg = a.neg_ && !rv.empty();
        q = from_mag(qv, qneg);
        r = from_mag(rv, rneg);
    }

    friend int compare(const BigInt& a, const BigInt& b) {
        if (a.sign() != b.sign()) return a.sign() < b.sign() ? -1 : 1;
        if (a.is_zero()) return 0;
        int c = cmp_abs(a, b);
        return a.neg_ ? -c : c;
    }
    friend bool operator==(const BigInt& a, const BigInt& b) { return compare(a, b) == 0; }
    friend bool operator!=(const BigInt& a, const BigInt& b) { return compare(a, b) != 0; }
    friend bool operator<(const BigInt& a, const BigInt& b) { return compare(a, b) < 0; }
    friend bool operator>(const BigInt& a, const BigInt& b) { return compare(a, b) > 0; }
    friend bool operator<=(const BigInt& a, const BigInt& b) { return compare(a, b) <= 0; }
    friend bool operator>=(const BigInt& a, const BigInt& b) { return compare(a, b) >= 0; }
    friend std::ostream& operator<<(std::ostream& os, const BigInt& x) {
        return os << x.to_string();
    }

    BigInt& operator+=(const BigInt& b) {
        *this = *this + b;
        return *this;
    }
    BigInt& operator-=(const BigInt& b) {
        *this = *this - b;
        return *this;
    }
    BigInt& operator*=(const BigInt& b) {
        *this = *this * b;
        return *this;
    }
    BigInt& operator/=(const BigInt& b) {
        *this = *this / b;
        return *this;
    }
    BigInt& operator%=(const BigInt& b) {
        *this = *this % b;
        return *this;
    }

    friend BigInt operator+(const BigInt& a, const BigInt& b) {
        if (a.neg_ == b.neg_) return from_mag(bigint_detail::add_mag(a.d_, b.d_), a.neg_);
        int c = cmp_abs(a, b);
        if (c == 0) return BigInt();
        if (c > 0) return from_mag(bigint_detail::sub_mag(a.d_, b.d_), a.neg_);
        return from_mag(bigint_detail::sub_mag(b.d_, a.d_), b.neg_);
    }

    friend BigInt operator-(const BigInt& a, const BigInt& b) { return a + (-b); }

    friend BigInt operator*(const BigInt& a, const BigInt& b) {
        if (a.is_zero() || b.is_zero()) return BigInt();
        return from_mag(bigint_detail::mul_mag(a.d_, b.d_), a.neg_ != b.neg_);
    }

    BigInt sqr() const {
        if (is_zero()) return BigInt();
        return from_mag(bigint_detail::sqr_mag(d_), false);
    }

    friend BigInt operator/(const BigInt& a, const BigInt& b) {
        BigInt q, r;
        divmod(a, b, q, r);
        return q;
    }

    friend BigInt operator%(const BigInt& a, const BigInt& b) {
        BigInt q, r;
        divmod(a, b, q, r);
        return r;
    }

    friend BigInt operator<<(const BigInt& a, size_t bits) {
        if (a.is_zero() || bits == 0) return a;
        return from_mag(bigint_detail::shift_left_mag(a.d_, bits), a.neg_);
    }

    friend BigInt operator>>(const BigInt& a, size_t bits) {
        if (a.is_zero() || bits == 0) return a;
        return from_mag(bigint_detail::shift_right_mag(a.d_, bits), a.neg_);
    }

    BigInt pow(unsigned long long e) const {
        BigInt r(1), a = *this;
        while (e) {
            if (e & 1) r *= a;
            e >>= 1;
            if (e) a = a.sqr();
        }
        return r;
    }

    BigInt sqrt() const {
        if (is_negative()) throw std::domain_error("sqrt of a negative BigInt");
        if (*this <= BigInt(1)) return *this;
        const size_t bits = bit_length();
        const size_t target = (bits + 1) / 2;   // bits of the result
        // Newton with doubling precision: x always approximates the top
        // `prec` bits of the root, and each round divides a number of about
        // twice that width, so the loop costs a couple of the last round's
        // divisions instead of one full-width division per round.
        size_t prec = std::min<size_t>(target, 32);
        BigInt m = *this >> (2 * (target - prec));
        BigInt x = BigInt(1) << ((m.bit_length() + 1) / 2);
        // Each round converges, then steps just above the root: Newton only
        // decreases, so the estimate stays an upper bound.
        for (;;) {
            const BigInt y = (x + m / x) >> 1;
            if (y >= x) break;
            x = y;
        }
        x += BigInt(1);
        while (prec < target) {
            const size_t next = std::min(target, 2 * prec);
            x = x << (next - prec);
            m = *this >> (2 * (target - next));
            for (;;) {
                const BigInt y = (x + m / x) >> 1;
                if (y >= x) break;
                x = y;
            }
            x += BigInt(1);
            prec = next;
        }
        while (x.sqr() > *this) x -= BigInt(1);
        while ((x + BigInt(1)).sqr() <= *this) x += BigInt(1);
        return x;
    }

    BigInt nroot(unsigned long long k) const {
        if (k == 0) throw std::invalid_argument("zeroth root");
        if (k == 1 || *this <= BigInt(1)) return *this;
        if (k == 2) return sqrt();
        if (is_negative()) throw std::domain_error("nroot of a negative BigInt");
        const size_t bits = bit_length();
        const size_t target = (bits + k - 1) / k;   // bits of the result
        const BigInt kk((long long)k);
        // Same doubling-precision scheme as sqrt.
        size_t prec = std::min<size_t>(target, 32);
        BigInt m = *this >> (k * (target - prec));
        BigInt x = BigInt(1) << ((m.bit_length() + k - 1) / k);
        // As in sqrt: converge per round, then step just above the root.
        for (;;) {
            const BigInt y = ((kk - BigInt(1)) * x + m / x.pow(k - 1)) / kk;
            if (y >= x) break;
            x = y;
        }
        x += BigInt(1);
        while (prec < target) {
            const size_t next = std::min(target, 2 * prec);
            x = x << (next - prec);
            m = *this >> (k * (target - next));
            for (;;) {
                const BigInt y = ((kk - BigInt(1)) * x + m / x.pow(k - 1)) / kk;
                if (y >= x) break;
                x = y;
            }
            x += BigInt(1);
            prec = next;
        }
        while (x.pow(k) > *this) x -= BigInt(1);
        while ((x + BigInt(1)).pow(k) <= *this) x += BigInt(1);
        return x;
    }
};

// Namespace-scope declarations of the operators defined as in-class friends
// above. Each declaration names the same function as its friend definition, so
// other translation units can declare and call the operators without relying on
// argument-dependent lookup. Kept in sync with bundles/hp/README.md.
BigInt operator+(const BigInt& a, const BigInt& b);
BigInt operator-(const BigInt& a, const BigInt& b);
BigInt operator*(const BigInt& a, const BigInt& b);
BigInt operator/(const BigInt& a, const BigInt& b);
BigInt operator%(const BigInt& a, const BigInt& b);
BigInt operator<<(const BigInt& a, std::size_t bits);
BigInt operator>>(const BigInt& a, std::size_t bits);
int compare(const BigInt& a, const BigInt& b);
bool operator==(const BigInt& a, const BigInt& b);
bool operator!=(const BigInt& a, const BigInt& b);
bool operator<(const BigInt& a, const BigInt& b);
bool operator>(const BigInt& a, const BigInt& b);
bool operator<=(const BigInt& a, const BigInt& b);
bool operator>=(const BigInt& a, const BigInt& b);
std::ostream& operator<<(std::ostream& os, const BigInt& x);

inline void BigInt::write_chunk9(std::string& out, size_t pos, limb v) {
    for (int i = 8; i >= 0; --i) {
        out[pos + size_t(i)] = char('0' + v % 10);
        v /= 10;
    }
}

inline void BigInt::to_decimal_rec(const BigInt& x, int level,
                                   const decimal_tables& tables,
                                   std::string& out, size_t pos) {
    if (level == 0) {
        write_chunk9(out, pos, x.is_zero() ? 0 : x.d_[0]);
        return;
    }
    if (level <= 5) {
        // At most 10^288: split by repeated short division instead of a
        // Barrett step per tiny node.  Low chunks first, leading ones stay 0.
        limb chunks[32];
        size_t used = 0;
        vec t = x.d_;
        while (!t.empty()) {
            limb rem = 0;
            bigint_detail::divmod_mag_small_inplace(t, 1000000000u, rem);
            chunks[used++] = rem;
        }
        if (used == 0) chunks[used++] = 0;
        const size_t width = size_t(1) << level;
        size_t w = pos + 9 * (width - used);
        std::fill(out.begin() + pos, out.begin() + w, '0');
        for (size_t i = used; i-- > 0;) {
            write_chunk9(out, w, chunks[i]);
            w += 9;
        }
        return;
    }
    BigInt q, r;
    bigint_detail::divmod_by_reciprocal(
        x.d_, tables.powers[level - 1].d_, tables.recips[level - 1].d_,
        tables.scales[level - 1], q.d_, r.d_, &tables.rec_factors[level - 1],
        &tables.div_factors[level - 1]);
    q.neg_ = false;
    r.neg_ = false;
    const size_t half = size_t(9) << (level - 1);
    to_decimal_rec(q, level - 1, tables, out, pos);
    to_decimal_rec(r, level - 1, tables, out, pos + half);
}

inline BigInt BigInt::from_decimal_rec(const std::vector<limb>& chunks, size_t l,
                                       int level, const decimal_tables& tables) {
    if (level == 0) return BigInt((long long)chunks[l]);
    const size_t half = size_t(1) << (level - 1);
    const BigInt a = from_decimal_rec(chunks, l, level - 1, tables);
    const BigInt b = from_decimal_rec(chunks, l + half, level - 1, tables);
    return scale_by_power(a, tables, size_t(level - 1)) + b;
}

} // namespace mal
