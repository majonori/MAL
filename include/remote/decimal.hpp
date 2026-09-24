#pragma once
// Decimal chunk core for the remote interface.
//
// Problems here read and write decimal, so the interface layer keeps numbers
// as base-10^5 chunks: parsing and printing are plain block copies, while the
// binary core in include/hp still owns every non-decimal interface.
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <stdexcept>
#include <string>
#include <vector>

#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
#define MAL_REMOTE_X86_SIMD 1
#include <immintrin.h>
#endif

namespace mal {
namespace remote_detail {

using u32 = std::uint32_t;
using u64 = std::uint64_t;
using chunk_vec = std::vector<u32>;

constexpr u32 CHUNK_BASE = 100000;   // 10^5, one decimal block
constexpr int CHUNK_WIDTH = 5;

inline void trim(chunk_vec& a) {
    while (!a.empty() && !a.back()) a.pop_back();
}

inline int cmp(const chunk_vec& a, const chunk_vec& b) {
    if (a.size() != b.size()) return a.size() < b.size() ? -1 : 1;
    for (size_t i = a.size(); i--;)
        if (a[i] != b[i]) return a[i] < b[i] ? -1 : 1;
    return 0;
}

inline chunk_vec from_u64(u64 x) {
    chunk_vec a;
    while (x) {
        a.push_back(u32(x % CHUNK_BASE));
        x /= CHUNK_BASE;
    }
    return a;
}

// Accepts an optional sign; leading zeros are skipped.
inline chunk_vec parse(const std::string& s) {
    size_t p = 0;
    if (!s.empty() && (s[0] == '+' || s[0] == '-')) p = 1;
    while (p < s.size() && s[p] == '0') ++p;
    chunk_vec a;
    a.reserve((s.size() - p + CHUNK_WIDTH - 1) / CHUNK_WIDTH);
    for (size_t r = s.size(); r > p;) {
        const size_t l = r - p > CHUNK_WIDTH ? r - CHUNK_WIDTH : p;
        u32 v = 0;
        for (size_t j = l; j < r; ++j) v = v * 10 + u32(s[j] - '0');
        a.push_back(v);
        r = l;
    }
    return a;
}

// Four decimal digits per table slot, built once and only ever read.
inline const char* digits4() {
    static const std::string table = [] {
        std::string t(size_t(10000) * 4, '0');
        for (size_t v = 0; v < 10000; ++v) {
            size_t x = v;
            for (int j = 3; j >= 0; --j) {
                t[v * 4 + size_t(j)] = char('0' + x % 10);
                x /= 10;
            }
        }
        return t;
    }();
    return table.data();
}

inline std::string str(const chunk_vec& a) {
    if (a.empty()) return "0";
    std::string s = std::to_string(a.back());
    const size_t lead = s.size();
    s.resize(lead + CHUNK_WIDTH * (a.size() - 1));
    const char* lut = digits4();
    for (size_t i = a.size() - 1; i--;) {
        const u32 x = a[i];
        const size_t p = lead + CHUNK_WIDTH * (a.size() - 2 - i);
        s[p] = char('0' + x / 10000);
        std::copy(lut + size_t(x % 10000) * 4, lut + size_t(x % 10000) * 4 + 4,
                  s.begin() + std::ptrdiff_t(p + 1));
    }
    return s;
}

inline bool have_avx2() {
#ifdef MAL_REMOTE_X86_SIMD
    static const bool ok = __builtin_cpu_supports("avx2");
    return ok;
#else
    return false;
#endif
}

#ifdef MAL_REMOTE_X86_SIMD
// Eight decimal limbs per iteration: the carry bits are an exact
// generate/propagate prefix scan, so no per-limb branch is needed.
__attribute__((target("avx2"))) inline size_t add_blocks(
    const chunk_vec& a, const chunk_vec& b, chunk_vec& c, size_t m, u32& carry) {
    const __m256i last = _mm256_set1_epi32(int(CHUNK_BASE) - 1);
    const __m256i base = _mm256_set1_epi32(int(CHUNK_BASE));
    const __m256i one = _mm256_set1_epi32(1);
    const __m256i bit = _mm256_setr_epi32(1, 2, 4, 8, 16, 32, 64, 128);
    size_t i = 0;
    for (; i + 16 <= m; i += 16) {
        for (int rep = 0; rep < 2; ++rep) {
            const size_t j = i + 8 * size_t(rep);
            const __m256i sum = _mm256_add_epi32(
                _mm256_loadu_si256((const __m256i*)&a[j]),
                _mm256_loadu_si256((const __m256i*)&b[j]));
            u32 g = u32(_mm256_movemask_ps(_mm256_castsi256_ps(
                _mm256_cmpgt_epi32(sum, last))));
            u32 p = u32(_mm256_movemask_ps(_mm256_castsi256_ps(
                _mm256_cmpeq_epi32(sum, last))));
            // A carry crosses exactly a run of limbs that sum to base-1.
            u32 in = (p + ((g << 1) | carry)) ^ p;
            g = in >> 1;
            const __m256i ci = _mm256_cmpeq_epi32(
                _mm256_and_si256(_mm256_set1_epi32(int(in)), bit), bit);
            const __m256i co = _mm256_cmpeq_epi32(
                _mm256_and_si256(_mm256_set1_epi32(int(g)), bit), bit);
            _mm256_storeu_si256(
                (__m256i*)&c[j],
                _mm256_sub_epi32(_mm256_add_epi32(sum, _mm256_and_si256(ci, one)),
                                 _mm256_and_si256(co, base)));
            carry = g >> 7;
        }
    }
    for (; i + 8 <= m; i += 8) {
        const __m256i sum = _mm256_add_epi32(
            _mm256_loadu_si256((const __m256i*)&a[i]),
            _mm256_loadu_si256((const __m256i*)&b[i]));
        u32 g = u32(_mm256_movemask_ps(_mm256_castsi256_ps(
            _mm256_cmpgt_epi32(sum, last))));
        u32 p = u32(_mm256_movemask_ps(_mm256_castsi256_ps(
            _mm256_cmpeq_epi32(sum, last))));
        // A carry crosses exactly a run of limbs that sum to base-1.
        u32 in = (p + ((g << 1) | carry)) ^ p;
        g = in >> 1;
        const __m256i ci = _mm256_cmpeq_epi32(
            _mm256_and_si256(_mm256_set1_epi32(int(in)), bit), bit);
        const __m256i co = _mm256_cmpeq_epi32(
            _mm256_and_si256(_mm256_set1_epi32(int(g)), bit), bit);
        _mm256_storeu_si256(
            (__m256i*)&c[i],
            _mm256_sub_epi32(_mm256_add_epi32(sum, _mm256_and_si256(ci, one)),
                             _mm256_and_si256(co, base)));
        carry = g >> 7;
    }
    return i;
}

// Borrow scan for subtraction, the mirror image of the carry scan above.
__attribute__((target("avx2"))) inline size_t sub_blocks(
    const chunk_vec& a, const chunk_vec& b, chunk_vec& c, size_t m, u32& borrow) {
    const __m256i base = _mm256_set1_epi32(int(CHUNK_BASE));
    const __m256i one = _mm256_set1_epi32(1);
    const __m256i bit = _mm256_setr_epi32(1, 2, 4, 8, 16, 32, 64, 128);
    size_t i = 0;
    for (; i + 16 <= m; i += 16) {
        for (int rep = 0; rep < 2; ++rep) {
            const size_t j = i + 8 * size_t(rep);
            const __m256i av = _mm256_loadu_si256((const __m256i*)&a[j]);
            const __m256i bv = _mm256_loadu_si256((const __m256i*)&b[j]);
            u32 g = u32(_mm256_movemask_ps(_mm256_castsi256_ps(
                _mm256_cmpgt_epi32(bv, av))));
            u32 p = u32(_mm256_movemask_ps(_mm256_castsi256_ps(
                _mm256_cmpeq_epi32(av, bv))));
            u32 in = (p + ((g << 1) | borrow)) ^ p;
            g = in >> 1;
            const __m256i bi = _mm256_cmpeq_epi32(
                _mm256_and_si256(_mm256_set1_epi32(int(in)), bit), bit);
            const __m256i bo = _mm256_cmpeq_epi32(
                _mm256_and_si256(_mm256_set1_epi32(int(g)), bit), bit);
            _mm256_storeu_si256(
                (__m256i*)&c[j],
                _mm256_sub_epi32(_mm256_add_epi32(_mm256_sub_epi32(av, bv),
                                                  _mm256_and_si256(bo, base)),
                                 _mm256_and_si256(bi, one)));
            borrow = g >> 7;
        }
    }
    for (; i + 8 <= m; i += 8) {
        const __m256i av = _mm256_loadu_si256((const __m256i*)&a[i]);
        const __m256i bv = _mm256_loadu_si256((const __m256i*)&b[i]);
        u32 g = u32(_mm256_movemask_ps(_mm256_castsi256_ps(_mm256_cmpgt_epi32(bv, av))));
        u32 p = u32(_mm256_movemask_ps(_mm256_castsi256_ps(_mm256_cmpeq_epi32(av, bv))));
        u32 in = (p + ((g << 1) | borrow)) ^ p;
        g = in >> 1;
        const __m256i bi = _mm256_cmpeq_epi32(
            _mm256_and_si256(_mm256_set1_epi32(int(in)), bit), bit);
        const __m256i bo = _mm256_cmpeq_epi32(
            _mm256_and_si256(_mm256_set1_epi32(int(g)), bit), bit);
        _mm256_storeu_si256(
            (__m256i*)&c[i],
            _mm256_sub_epi32(_mm256_add_epi32(_mm256_sub_epi32(av, bv),
                                              _mm256_and_si256(bo, base)),
                             _mm256_and_si256(bi, one)));
        borrow = g >> 7;
    }
    return i;
}
#endif

inline chunk_vec add(const chunk_vec& a, const chunk_vec& b) {
    chunk_vec c(std::max(a.size(), b.size()) + 1);
    u32 carry = 0;
    size_t i = 0;
    const size_t m = std::min(a.size(), b.size());
#ifdef MAL_REMOTE_X86_SIMD
    if (m >= 32 && have_avx2()) i = add_blocks(a, b, c, m, carry);
#endif
    for (; i < m; ++i) {
        const u32 z = a[i] + b[i] + carry;
        carry = z >= CHUNK_BASE;
        c[i] = z - carry * CHUNK_BASE;
    }
    const chunk_vec& t = a.size() > b.size() ? a : b;
    for (; i < t.size(); ++i) {
        const u32 z = t[i] + carry;
        carry = z >= CHUNK_BASE;
        c[i] = z - carry * CHUNK_BASE;
    }
    c[i] = carry;
    trim(c);
    return c;
}

// a >= b
inline chunk_vec sub(const chunk_vec& a, const chunk_vec& b) {
    chunk_vec c(a.size());
    u32 borrow = 0;
    size_t i = 0;
#ifdef MAL_REMOTE_X86_SIMD
    if (b.size() >= 32 && have_avx2()) i = sub_blocks(a, b, c, b.size(), borrow);
#endif
    for (; i < b.size(); ++i) {
        const u32 t = b[i] + borrow;
        borrow = a[i] < t;
        c[i] = a[i] + borrow * CHUNK_BASE - t;
    }
    for (; i < a.size(); ++i) {
        const u32 t = borrow;
        borrow = a[i] < t;
        c[i] = a[i] + borrow * CHUNK_BASE - t;
    }
    trim(c);
    return c;
}

inline void inc(chunk_vec& a) {
    size_t i = 0;
    while (i < a.size() && a[i] == CHUNK_BASE - 1) a[i++] = 0;
    if (i == a.size()) a.push_back(1);
    else ++a[i];
}

inline void dec(chunk_vec& a) {
    size_t i = 0;
    while (i < a.size() && !a[i]) a[i++] = CHUNK_BASE - 1;
    if (i < a.size()) --a[i];
    trim(a);
}

inline chunk_vec shl(const chunk_vec& a, size_t k) {
    if (a.empty()) return {};
    chunk_vec c(k + a.size());
    std::copy(a.begin(), a.end(), c.begin() + k);
    return c;
}

inline chunk_vec shr(const chunk_vec& a, size_t k) {
    if (k >= a.size()) return {};
    return chunk_vec(a.begin() + k, a.end());
}

inline chunk_vec slice(const chunk_vec& a, size_t l, size_t r) {
    r = std::min(r, a.size());
    if (l >= r) return {};
    chunk_vec c(a.begin() + l, a.begin() + r);
    trim(c);
    return c;
}

inline chunk_vec pow_base(size_t k) {
    chunk_vec c(k + 1);
    c[k] = 1;
    return c;
}

inline chunk_vec mul_small(const chunk_vec& a, u32 x) {
    if (!x || a.empty()) return {};
    chunk_vec c(a.size() + 1);
    u64 carry = 0;
    for (size_t i = 0; i < a.size(); ++i) {
        const u64 z = u64(a[i]) * x + carry;
        c[i] = u32(z % CHUNK_BASE);
        carry = z / CHUNK_BASE;
    }
    c[a.size()] = u32(carry);
    while (c.back() >= CHUNK_BASE) {
        const u32 z = c.back();
        c.back() = z % CHUNK_BASE;
        c.push_back(z / CHUNK_BASE);
    }
    trim(c);
    return c;
}

inline chunk_vec div_small(const chunk_vec& a, u32 d, u32* remainder = nullptr) {
    chunk_vec c(a.size());
    u64 r = 0;
    for (size_t i = a.size(); i--;) {
        const u64 z = r * CHUNK_BASE + a[i];
        c[i] = u32(z / d);
        r = z % d;
    }
    if (remainder) *remainder = u32(r);
    trim(c);
    return c;
}

// ---------------------------------------------------------------------------
// Scratch slabs. The multiplication engines need a few full length vectors per
// call; handing them out from a pool keyed by the current nesting depth keeps
// the allocator and the page faults out of the inner loops, and guarantees that
// an inner call can never reuse a buffer an outer call is still holding.
// ---------------------------------------------------------------------------

// deque: growing the pool must not move the slots that outer frames hold.
inline std::deque<chunk_vec>& scratch_pool() {
    static thread_local std::deque<chunk_vec> pool;
    return pool;
}

inline size_t& scratch_top() {
    static thread_local size_t top = 0;
    return top;
}

// A stack of slots: the frame claims the next "slots" buffers and releases them
// on scope exit, so nested calls never share a live buffer.
struct scratch_frame {
    const size_t base;
    explicit scratch_frame(size_t slots) : base(scratch_top()) {
        scratch_top() = base + slots;
    }
    ~scratch_frame() { scratch_top() = base; }
    scratch_frame(const scratch_frame&) = delete;
    scratch_frame& operator=(const scratch_frame&) = delete;
    chunk_vec& at(size_t index) const;
};

inline chunk_vec& scratch_vec(size_t index) {
    std::deque<chunk_vec>& pool = scratch_pool();
    if (pool.size() <= index) pool.resize(index + 1);
    return pool[index];
}

inline chunk_vec& scratch_frame::at(size_t index) const {
    return scratch_vec(base + index);
}

// ---------------------------------------------------------------------------
// Multiplication: schoolbook, Karatsuba, NTT with two small primes, and an
// overlap-add path for very unbalanced operands.
// ---------------------------------------------------------------------------

inline chunk_vec carry_coeff(const std::vector<u64>& c) {
    chunk_vec a(c.size());
    u64 carry = 0;
    for (size_t i = 0; i < c.size(); ++i) {
        const u64 z = c[i] + carry;
        a[i] = u32(z % CHUNK_BASE);
        carry = z / CHUNK_BASE;
    }
    while (carry) {
        a.push_back(u32(carry % CHUNK_BASE));
        carry /= CHUNK_BASE;
    }
    trim(a);
    return a;
}

inline chunk_vec school_mul(const chunk_vec& a, const chunk_vec& b) {
    std::vector<u64> c(a.size() + b.size());
    for (size_t i = 0; i < a.size(); ++i)
        for (size_t j = 0; j < b.size(); ++j) c[i + j] += u64(a[i]) * b[j];
    return carry_coeff(c);
}

// Both primes stay below 2^29 so the butterflies can keep values in [0, 2p)
// and use Montgomery/Shoup arithmetic without 64-bit reductions.
template <u32 P>
struct transform {
    static constexpr u32 neg_inv() {
        u32 x = P;
        for (int i = 0; i < 5; ++i) x *= 2 - P * x;
        return u32(0) - x;
    }
    static constexpr u32 ni = neg_inv();
    static constexpr u32 r_one = u32((u64(1) << 32) % P);

    static u32 mpow(u32 x, u32 n) {
        u32 y = 1;
        while (n) {
            if (n & 1) y = u32(u64(y) * x % P);
            x = u32(u64(x) * x % P);
            n >>= 1;
        }
        return y;
    }
    static u32 lazy(u32 x) { return x >= 2 * P ? x - 2 * P : x; }
    static u32 shoup(u32 x, u32 w, u32 wh) {
        return x * w - u32(u64(x) * wh >> 32) * P;
    }
    static u32 mont(u32 x, u32 y) {
        const u64 z = u64(x) * y;
        return u32((z + u64(u32(z) * ni) * P) >> 32);
    }

    std::vector<u32> w{0, 1}, wh{0, u32((u64(1) << 32) / P)};
    std::vector<u32> iw{0, 1}, iwh{0, u32((u64(1) << 32) / P)};

    void ensure(size_t n) {
        if (w.size() >= n) return;
        const size_t old = w.size();
        w.resize(n);
        wh.resize(n);
        iw.resize(n);
        iwh.resize(n);
        for (size_t h = old; h < n; h *= 2) {
            const u32 f = mpow(3, (P - 1) / u32(2 * h));
            const u32 g = mpow(f, P - 2);
            u32 x = 1, y = 1;
            for (size_t j = 0; j < h; ++j) {
                w[h + j] = x;
                wh[h + j] = u32((u64(x) << 32) / P);
                iw[h + j] = y;
                iwh[h + j] = u32((u64(y) << 32) / P);
                x = u32(u64(x) * f % P);
                y = u32(u64(y) * g % P);
            }
        }
    }

    void scalar(chunk_vec& a, bool inv) const {
        const size_t n = a.size();
        if (!inv) {
            for (size_t h = n / 2; h; h /= 2)
                for (size_t i = 0; i < n; i += 2 * h)
                    for (size_t j = 0; j < h; ++j) {
                        const u32 u = a[i + j], v = a[i + j + h];
                        a[i + j] = lazy(u + v);
                        a[i + j + h] = shoup(u + 2 * P - v, w[h + j], wh[h + j]);
                    }
        } else {
            for (size_t h = 1; h < n; h *= 2)
                for (size_t i = 0; i < n; i += 2 * h)
                    for (size_t j = 0; j < h; ++j) {
                        const u32 u = a[i + j];
                        const u32 v = shoup(a[i + j + h], iw[h + j], iwh[h + j]);
                        a[i + j] = lazy(u + v);
                        a[i + j + h] = lazy(u + 2 * P - v);
                    }
        }
    }

#ifdef MAL_REMOTE_X86_SIMD
    __attribute__((target("avx2"))) static __m256i reduce2(__m256i x) {
        return _mm256_min_epu32(x, _mm256_sub_epi32(x, _mm256_set1_epi32(int(2 * P))));
    }
    __attribute__((target("avx2"))) static __m256i smul(__m256i x, __m256i y, __m256i yh) {
        const __m256i e = _mm256_mul_epu32(x, yh);
        const __m256i o = _mm256_mul_epu32(_mm256_srli_epi64(x, 32),
                                           _mm256_srli_epi64(yh, 32));
        const __m256i q = _mm256_blend_epi32(_mm256_srli_epi64(e, 32), o, 0xAA);
        return _mm256_sub_epi32(_mm256_mullo_epi32(x, y),
                                _mm256_mullo_epi32(q, _mm256_set1_epi32(int(P))));
    }
    __attribute__((target("avx2"))) static __m256i mmul(__m256i x, __m256i y) {
        const __m256i p = _mm256_set1_epi32(int(P));
        const __m256i ni_v = _mm256_set1_epi32(int(ni));
        __m256i e = _mm256_mul_epu32(x, y);
        __m256i o = _mm256_mul_epu32(_mm256_srli_epi64(x, 32), _mm256_srli_epi64(y, 32));
        const __m256i me = _mm256_mul_epu32(e, ni_v);
        const __m256i mo = _mm256_mul_epu32(o, ni_v);
        e = _mm256_add_epi64(e, _mm256_mul_epu32(me, p));
        o = _mm256_add_epi64(o, _mm256_mul_epu32(mo, p));
        return _mm256_blend_epi32(_mm256_srli_epi64(e, 32), o, 0xAA);
    }

    // One pass per stage, eight limbs at a time; the last three stages are
    // inside a single register (shuffles instead of memory traffic).
    __attribute__((target("avx2"))) void avx(chunk_vec& a, bool inv) const {
        const size_t n = a.size();
        const __m256i p2 = _mm256_set1_epi32(int(2 * P));
        if (!inv) {
            for (size_t h = n / 2; h >= 8; h /= 2)
                for (size_t i = 0; i < n; i += 2 * h) {
                    size_t j = 0;
                    for (; j + 16 <= h; j += 16) {
                        for (int rep = 0; rep < 2; ++rep) {
                            const size_t k = j + 8 * size_t(rep);
                            const __m256i u = _mm256_loadu_si256((const __m256i*)&a[i + k]);
                            const __m256i v = _mm256_loadu_si256((const __m256i*)&a[i + k + h]);
                            const __m256i s = reduce2(_mm256_add_epi32(u, v));
                            __m256i d = _mm256_sub_epi32(_mm256_add_epi32(u, p2), v);
                            d = smul(d, _mm256_loadu_si256((const __m256i*)&w[h + k]),
                                     _mm256_loadu_si256((const __m256i*)&wh[h + k]));
                            _mm256_storeu_si256((__m256i*)&a[i + k], s);
                            _mm256_storeu_si256((__m256i*)&a[i + k + h], d);
                        }
                    }
                    for (; j < h; j += 8) {
                        const __m256i u = _mm256_loadu_si256((const __m256i*)&a[i + j]);
                        const __m256i v = _mm256_loadu_si256((const __m256i*)&a[i + j + h]);
                        const __m256i s = reduce2(_mm256_add_epi32(u, v));
                        __m256i d = _mm256_sub_epi32(_mm256_add_epi32(u, p2), v);
                        d = smul(d, _mm256_loadu_si256((const __m256i*)&w[h + j]),
                                 _mm256_loadu_si256((const __m256i*)&wh[h + j]));
                        _mm256_storeu_si256((__m256i*)&a[i + j], s);
                        _mm256_storeu_si256((__m256i*)&a[i + j + h], d);
                    }
                }
            const __m256i w4 = _mm256_setr_epi32(1, 1, 1, 1, int(w[4]), int(w[5]), int(w[6]), int(w[7]));
            const __m256i q4 = _mm256_setr_epi32(int(wh[1]), int(wh[1]), int(wh[1]), int(wh[1]),
                                                 int(wh[4]), int(wh[5]), int(wh[6]), int(wh[7]));
            const __m256i w2 = _mm256_setr_epi32(1, 1, int(w[2]), int(w[3]), 1, 1, int(w[2]), int(w[3]));
            const __m256i q2 = _mm256_setr_epi32(int(wh[1]), int(wh[1]), int(wh[2]), int(wh[3]),
                                                 int(wh[1]), int(wh[1]), int(wh[2]), int(wh[3]));
            size_t i = 0;
            for (; i + 16 <= n; i += 16) {
                for (int rep = 0; rep < 2; ++rep) {
                    const size_t at = i + 8 * size_t(rep);
                    __m256i x = _mm256_loadu_si256((const __m256i*)&a[at]), y, s, d;
                    y = _mm256_permute2x128_si256(x, x, 1);
                    s = reduce2(_mm256_add_epi32(x, y));
                    d = smul(_mm256_add_epi32(_mm256_sub_epi32(y, x), p2), w4, q4);
                    x = _mm256_blend_epi32(s, d, 0xF0);
                    y = _mm256_shuffle_epi32(x, 0x4E);
                    s = reduce2(_mm256_add_epi32(x, y));
                    d = smul(_mm256_add_epi32(_mm256_sub_epi32(y, x), p2), w2, q2);
                    x = _mm256_blend_epi32(s, d, 0xCC);
                    y = _mm256_shuffle_epi32(x, 0xB1);
                    s = reduce2(_mm256_add_epi32(x, y));
                    d = reduce2(_mm256_add_epi32(_mm256_sub_epi32(y, x), p2));
                    x = _mm256_blend_epi32(s, d, 0xAA);
                    _mm256_storeu_si256((__m256i*)&a[at], x);
                }
            }
            for (; i < n; i += 8) {
                __m256i x = _mm256_loadu_si256((const __m256i*)&a[i]), y, s, d;
                y = _mm256_permute2x128_si256(x, x, 1);
                s = reduce2(_mm256_add_epi32(x, y));
                d = smul(_mm256_add_epi32(_mm256_sub_epi32(y, x), p2), w4, q4);
                x = _mm256_blend_epi32(s, d, 0xF0);
                y = _mm256_shuffle_epi32(x, 0x4E);
                s = reduce2(_mm256_add_epi32(x, y));
                d = smul(_mm256_add_epi32(_mm256_sub_epi32(y, x), p2), w2, q2);
                x = _mm256_blend_epi32(s, d, 0xCC);
                y = _mm256_shuffle_epi32(x, 0xB1);
                s = reduce2(_mm256_add_epi32(x, y));
                d = reduce2(_mm256_add_epi32(_mm256_sub_epi32(y, x), p2));
                x = _mm256_blend_epi32(s, d, 0xAA);
                _mm256_storeu_si256((__m256i*)&a[i], x);
            }
            return;
        }
        const __m256i w4 = _mm256_setr_epi32(1, 1, 1, 1, int(iw[4]), int(iw[5]), int(iw[6]), int(iw[7]));
        const __m256i q4 = _mm256_setr_epi32(int(iwh[1]), int(iwh[1]), int(iwh[1]), int(iwh[1]),
                                             int(iwh[4]), int(iwh[5]), int(iwh[6]), int(iwh[7]));
        const __m256i w2 = _mm256_setr_epi32(1, 1, int(iw[2]), int(iw[3]), 1, 1, int(iw[2]), int(iw[3]));
        const __m256i q2 = _mm256_setr_epi32(int(iwh[1]), int(iwh[1]), int(iwh[2]), int(iwh[3]),
                                             int(iwh[1]), int(iwh[1]), int(iwh[2]), int(iwh[3]));
        size_t i = 0;
        for (; i + 16 <= n; i += 16) {
            for (int rep = 0; rep < 2; ++rep) {
                const size_t at = i + 8 * size_t(rep);
                __m256i x = _mm256_loadu_si256((const __m256i*)&a[at]), y, s, d;
                y = _mm256_shuffle_epi32(x, 0xB1);
                s = reduce2(_mm256_add_epi32(x, y));
                d = reduce2(_mm256_add_epi32(_mm256_sub_epi32(y, x), p2));
                x = _mm256_blend_epi32(s, d, 0xAA);
                x = smul(x, w2, q2);
                y = _mm256_shuffle_epi32(x, 0x4E);
                s = reduce2(_mm256_add_epi32(x, y));
                d = reduce2(_mm256_add_epi32(_mm256_sub_epi32(y, x), p2));
                x = _mm256_blend_epi32(s, d, 0xCC);
                x = smul(x, w4, q4);
                y = _mm256_permute2x128_si256(x, x, 1);
                s = reduce2(_mm256_add_epi32(x, y));
                d = reduce2(_mm256_add_epi32(_mm256_sub_epi32(y, x), p2));
                x = _mm256_blend_epi32(s, d, 0xF0);
                _mm256_storeu_si256((__m256i*)&a[at], x);
            }
        }
        for (; i < n; i += 8) {
            __m256i x = _mm256_loadu_si256((const __m256i*)&a[i]), y, s, d;
            y = _mm256_shuffle_epi32(x, 0xB1);
            s = reduce2(_mm256_add_epi32(x, y));
            d = reduce2(_mm256_add_epi32(_mm256_sub_epi32(y, x), p2));
            x = _mm256_blend_epi32(s, d, 0xAA);
            x = smul(x, w2, q2);
            y = _mm256_shuffle_epi32(x, 0x4E);
            s = reduce2(_mm256_add_epi32(x, y));
            d = reduce2(_mm256_add_epi32(_mm256_sub_epi32(y, x), p2));
            x = _mm256_blend_epi32(s, d, 0xCC);
            x = smul(x, w4, q4);
            y = _mm256_permute2x128_si256(x, x, 1);
            s = reduce2(_mm256_add_epi32(x, y));
            d = reduce2(_mm256_add_epi32(_mm256_sub_epi32(y, x), p2));
            x = _mm256_blend_epi32(s, d, 0xF0);
            _mm256_storeu_si256((__m256i*)&a[i], x);
        }
        for (size_t h = 8; h < n; h *= 2)
            for (size_t i = 0; i < n; i += 2 * h) {
                size_t j = 0;
                for (; j + 16 <= h; j += 16) {
                    for (int rep = 0; rep < 2; ++rep) {
                        const size_t k = j + 8 * size_t(rep);
                        const __m256i u = _mm256_loadu_si256((const __m256i*)&a[i + k]);
                        const __m256i v = smul(_mm256_loadu_si256((const __m256i*)&a[i + k + h]),
                                               _mm256_loadu_si256((const __m256i*)&iw[h + k]),
                                               _mm256_loadu_si256((const __m256i*)&iwh[h + k]));
                        _mm256_storeu_si256((__m256i*)&a[i + k], reduce2(_mm256_add_epi32(u, v)));
                        _mm256_storeu_si256(
                            (__m256i*)&a[i + k + h],
                            reduce2(_mm256_sub_epi32(_mm256_add_epi32(u, p2), v)));
                    }
                }
                for (; j < h; j += 8) {
                    const __m256i u = _mm256_loadu_si256((const __m256i*)&a[i + j]);
                    const __m256i v = smul(_mm256_loadu_si256((const __m256i*)&a[i + j + h]),
                                           _mm256_loadu_si256((const __m256i*)&iw[h + j]),
                                           _mm256_loadu_si256((const __m256i*)&iwh[h + j]));
                    _mm256_storeu_si256((__m256i*)&a[i + j], reduce2(_mm256_add_epi32(u, v)));
                    _mm256_storeu_si256((__m256i*)&a[i + j + h],
                                        reduce2(_mm256_sub_epi32(_mm256_add_epi32(u, p2), v)));
                }
            }
    }
    __attribute__((target("avx2"))) static void point_avx(chunk_vec& a, const chunk_vec& b) {
        size_t i = 0;
        for (; i + 16 <= a.size(); i += 16) {
            for (int rep = 0; rep < 2; ++rep) {
                const size_t at = i + 8 * size_t(rep);
                _mm256_storeu_si256(
                    (__m256i*)&a[at],
                    mmul(_mm256_loadu_si256((const __m256i*)&a[at]),
                         _mm256_loadu_si256((const __m256i*)&b[at])));
            }
        }
        for (; i < a.size(); i += 8)
            _mm256_storeu_si256((__m256i*)&a[i],
                                mmul(_mm256_loadu_si256((const __m256i*)&a[i]),
                                     _mm256_loadu_si256((const __m256i*)&b[i])));
    }
    __attribute__((target("avx2"))) static void scale_avx(chunk_vec& a, u32 v) {
        const __m256i w = _mm256_set1_epi32(int(v));
        const __m256i wh = _mm256_set1_epi32(int(u32((u64(v) << 32) / P)));
        const __m256i p = _mm256_set1_epi32(int(P));
        for (size_t i = 0; i < a.size(); i += 8) {
            __m256i x = smul(_mm256_loadu_si256((const __m256i*)&a[i]), w, wh);
            x = _mm256_min_epu32(x, _mm256_sub_epi32(x, p));
            _mm256_storeu_si256((__m256i*)&a[i], x);
        }
    }
#endif

    static bool simd() {
#ifdef MAL_REMOTE_X86_SIMD
        static const bool ok = __builtin_cpu_supports("avx2");
        return ok;
#else
        return false;
#endif
    }

    void run(chunk_vec& a, bool inv) {
#ifdef MAL_REMOTE_X86_SIMD
        if (simd() && a.size() >= 8) {
            ensure(a.size());
            avx(a, inv);
            return;
        }
#endif
        ensure(a.size());
        scalar(a, inv);
    }

    void point(chunk_vec& a, const chunk_vec& b) {
#ifdef MAL_REMOTE_X86_SIMD
        if (simd() && a.size() >= 8) {
            point_avx(a, b);
            return;
        }
#endif
        for (size_t i = 0; i < a.size(); ++i) a[i] = mont(a[i], b[i]);
    }

    void finish(chunk_vec& a) {
        const u32 scale = mpow(u32(u64(a.size()) * r_one % P), P - 2);
#ifdef MAL_REMOTE_X86_SIMD
        if (simd() && a.size() >= 8) {
            scale_avx(a, scale);
            return;
        }
#endif
        const u32 sh = u32((u64(scale) << 32) / P);
        for (u32& x : a) {
            x = shoup(x, scale, sh);
            if (x >= P) x -= P;
        }
    }

    static chunk_vec encode(const chunk_vec& a, size_t n) {
        chunk_vec c(n);
        for (size_t i = 0; i < a.size(); ++i) c[i] = u32(u64(a[i]) * r_one % P);
        return c;
    }

    // Same as encode, but into a buffer that can be reused between calls.
    static void encode_into(const chunk_vec& a, size_t n, chunk_vec& c) {
        c.assign(n, 0);
        for (size_t i = 0; i < a.size(); ++i) c[i] = u32(u64(a[i]) * r_one % P);
    }

    void convolution(const chunk_vec& a, const chunk_vec& b, size_t n, bool square, chunk_vec& out) {
        scratch_frame frame(1);
        encode_into(a, n, out);
        run(out, false);
        if (square) {
            point(out, out);
        } else {
            chunk_vec& y = frame.at(0);
            encode_into(b, n, y);
            run(y, false);
            point(out, y);
        }
        run(out, true);
        finish(out);
    }

    // Same as convolution(), but the second operand is already transformed (the
    // block division multiplies by the same divisor over and over).
    void convolution_cached(const chunk_vec& a, const chunk_vec& tb, size_t n, chunk_vec& out) {
        encode_into(a, n, out);
        run(out, false);
        point(out, tb);
        run(out, true);
        finish(out);
    }
};

constexpr u32 NTT_P1 = 167772161, NTT_P2 = 469762049;

// C++14: function-local statics instead of inline variables.
inline transform<NTT_P1>& t1() { static transform<NTT_P1> t; return t; }
inline transform<NTT_P2>& t2() { static transform<NTT_P2> t; return t; }

// Transform of one operand, kept between multiplications. The chunked division
// multiplies thousands of times by the same divisor and by the same reciprocal,
// so building their transforms once removes a third of the transforms per step.
struct ntt_cached {
    size_t n = 0;
    chunk_vec key;                 // the cached operand itself, for the check
    chunk_vec f1, f2;

    bool matches(const chunk_vec& b, size_t size) const {
        return n == size && key == b;
    }
    void build(const chunk_vec& b, size_t size) {
        n = size;
        key = b;
        transform<NTT_P1>::encode_into(b, size, f1);
        t1().run(f1, false);
        transform<NTT_P2>::encode_into(b, size, f2);
        t2().run(f2, false);
    }
};

inline u64 crt(u32 x, u32 y) {
    static const u32 inv = transform<NTT_P2>::mpow(NTT_P1, NTT_P2 - 2);
    static const u32 hi = u32((u64(inv) << 32) / NTT_P2);
    const u32 d = y >= x ? y - x : y + NTT_P2 - x;
    u32 v = transform<NTT_P2>::shoup(d, inv, hi);
    if (v >= NTT_P2) v -= NTT_P2;
    return x + u64(NTT_P1) * v;
}

inline chunk_vec ntt_mul(const chunk_vec& a, const chunk_vec& b, ntt_cached* cache = nullptr) {
    size_t need = a.size() + b.size() - 1, n = 1;
    while (n < need) n *= 2;
    if (n > (size_t(1) << 25)) throw std::length_error("decimal NTT is limited to 2^25 slots");
    if (u64(std::min(a.size(), b.size())) * (CHUNK_BASE - 1) * (CHUNK_BASE - 1) >=
        u64(NTT_P1) * NTT_P2)
        throw std::length_error("decimal NTT CRT range exceeded");
    scratch_frame frame(2);
    chunk_vec& c1 = frame.at(0);
    chunk_vec& c2 = frame.at(1);
    chunk_vec r(need);
    const bool square = &a == &b;
    if (cache && !square && cache->matches(b, n)) {
        t1().convolution_cached(a, cache->f1, n, c1);
        t2().convolution_cached(a, cache->f2, n, c2);
    } else {
        t1().convolution(a, b, n, square, c1);
        t2().convolution(a, b, n, square, c2);
        if (cache && !square) cache->build(b, n);
    }
    u64 carry = 0;
    for (size_t i = 0; i < need; ++i) {
        const u64 v = crt(c1[i], c2[i]) + carry;
        r[i] = u32(v % CHUNK_BASE);
        carry = v / CHUNK_BASE;
    }
    while (carry) {
        r.push_back(u32(carry % CHUNK_BASE));
        carry /= CHUNK_BASE;
    }
    trim(r);
    return r;
}

// Overlap-add for highly unbalanced products: the short operand is transformed
// once and reused for every block of the long one.
inline chunk_vec block_mul(const chunk_vec& large, const chunk_vec& small) {
    size_t n = 1;
    while (n < 2 * small.size()) n *= 2;
    const size_t block = n - small.size() + 1;
    chunk_vec f1 = transform<NTT_P1>::encode(small, n);
    chunk_vec f2 = transform<NTT_P2>::encode(small, n);
    t1().run(f1, false);
    t2().run(f2, false);
    std::vector<u64> c(large.size() + small.size());
    for (size_t l = 0; l < large.size(); l += block) {
        chunk_vec part = slice(large, l, std::min(large.size(), l + block));
        if (part.empty()) continue;
        chunk_vec g1 = transform<NTT_P1>::encode(part, n);
        chunk_vec g2 = transform<NTT_P2>::encode(part, n);
        t1().run(g1, false);
        t2().run(g2, false);
        t1().point(g1, f1);
        t2().point(g2, f2);
        t1().run(g1, true);
        t2().run(g2, true);
        t1().finish(g1);
        t2().finish(g2);
        for (size_t i = 0; i < part.size() + small.size() - 1; ++i)
            c[l + i] += crt(g1[i], g2[i]);
    }
    return carry_coeff(c);
}

#ifdef MAL_REMOTE_X86_SIMD
// Karatsuba leaves keep values inside signed 32 bits, so four 64-bit
// accumulators can be filled with one multiply each.
__attribute__((target("avx2"))) inline void kara_leaf_avx(const std::int64_t* a,
                                                          const std::int64_t* b,
                                                          std::int64_t* c, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        const __m256i ai = _mm256_set1_epi64x(a[i]);
        size_t j = 0;
        for (; j + 8 <= n; j += 8) {
            for (int rep = 0; rep < 2; ++rep) {
                const size_t at = j + 4 * size_t(rep);
                __m256i v = _mm256_mul_epi32(ai, _mm256_loadu_si256((const __m256i*)(b + at)));
                v = _mm256_add_epi64(v, _mm256_loadu_si256((const __m256i*)(c + i + at)));
                _mm256_storeu_si256((__m256i*)(c + i + at), v);
            }
        }
        for (; j < n; j += 4) {
            __m256i v = _mm256_mul_epi32(ai, _mm256_loadu_si256((const __m256i*)(b + j)));
            v = _mm256_add_epi64(v, _mm256_loadu_si256((const __m256i*)(c + i + j)));
            _mm256_storeu_si256((__m256i*)(c + i + j), v);
        }
    }
}
#endif

inline void kara(const std::int64_t* a, const std::int64_t* b, std::int64_t* c,
                 size_t n, std::int64_t* mem) {
    std::fill(c, c + 2 * n, std::int64_t(0));
    if (n <= 32) {
#ifdef MAL_REMOTE_X86_SIMD
        if (n >= 4 && have_avx2()) {
            kara_leaf_avx(a, b, c, n);
            return;
        }
#endif
        for (size_t i = 0; i < n; ++i)
            for (size_t j = 0; j < n; ++j) c[i + j] += a[i] * b[j];
        return;
    }
    const size_t h = n / 2;
    std::int64_t* sa = mem;
    std::int64_t* sb = mem + h;
    std::int64_t* mid = mem + n;
    std::int64_t* tmp = mem + 2 * n;
    kara(a, b, c, h, tmp);
    kara(a + h, b + h, c + n, h, tmp);
    for (size_t i = 0; i < h; ++i) {
        sa[i] = a[i] + a[h + i];
        sb[i] = b[i] + b[h + i];
    }
    kara(sa, sb, mid, h, tmp);
    for (size_t i = 0; i < n; ++i) mid[i] -= c[i] + c[n + i];
    for (size_t i = 0; i < n; ++i) c[h + i] += mid[i];
}

inline chunk_vec kara_mul(const chunk_vec& a, const chunk_vec& b) {
    size_t n = 1;
    while (n < std::max(a.size(), b.size())) n *= 2;
    std::vector<std::int64_t> mem(8 * n);
    std::int64_t* x = mem.data();
    std::int64_t* y = x + n;
    std::int64_t* z = y + n;
    std::int64_t* tmp = z + 2 * n;
    std::copy(a.begin(), a.end(), x);
    std::copy(b.begin(), b.end(), y);
    kara(x, y, z, n, tmp);
    chunk_vec r(a.size() + b.size());
    u64 carry = 0;
    for (size_t i = 0; i < r.size(); ++i) {
        const u64 v = u64(z[i]) + carry;
        r[i] = u32(v % CHUNK_BASE);
        carry = v / CHUNK_BASE;
    }
    trim(r);
    return r;
}

// Multiplying many times by the same operand (the chunked division below) can
// hand in a cache so its transform is not rebuilt for every product.
inline chunk_vec mul(const chunk_vec& a, const chunk_vec& b, ntt_cached* cache) {
    if (a.empty() || b.empty()) return {};
    const size_t mn = std::min(a.size(), b.size()), mx = std::max(a.size(), b.size());
    if (mn <= 12 || mn * mx <= 4096) return school_mul(a, b);
    if (mx <= 384) return kara_mul(a, b);
    if (mx >= 4 * mn) return a.size() > b.size() ? block_mul(a, b) : block_mul(b, a);
    return ntt_mul(a, b, cache);
}

inline chunk_vec mul(const chunk_vec& a, const chunk_vec& b) {
    return mul(a, b, nullptr);
}

// ---------------------------------------------------------------------------
// Division: Knuth at small sizes, Newton reciprocal with doubling precision
// for large ones.
// ---------------------------------------------------------------------------

using div_pair = std::pair<chunk_vec, chunk_vec>;

inline div_pair knuth_div(const chunk_vec& a, const chunk_vec& b) {
    if (b.empty()) throw std::domain_error("division by zero");
    if (cmp(a, b) < 0) return {chunk_vec{}, a};
    if (b.size() == 1) {
        u32 r = 0;
        chunk_vec q = div_small(a, b[0], &r);
        return {std::move(q), from_u64(r)};
    }
    const u32 norm = CHUNK_BASE / (b.back() + 1);
    chunk_vec v = mul_small(b, norm), u = mul_small(a, norm);
    const size_t n = b.size(), m = a.size() - n;
    u.resize(a.size() + 1);
    chunk_vec q(m + 1);
    for (size_t j = m + 1; j--;) {
        const u64 num = u64(u[j + n]) * CHUNK_BASE + u[j + n - 1];
        u64 qh = num / v[n - 1], rh = num % v[n - 1];
        while (qh >= CHUNK_BASE ||
               (rh < CHUNK_BASE && qh * v[n - 2] > rh * CHUNK_BASE + u[j + n - 2])) {
            --qh;
            rh += v[n - 1];
            if (rh >= CHUNK_BASE) break;
        }
        u64 carry = 0;
        u32 borrow = 0;
        for (size_t i = 0; i < n; ++i) {
            const u64 p = qh * v[i] + carry;
            carry = p / CHUNK_BASE;
            const u32 z = u32(p % CHUNK_BASE) + borrow;
            borrow = u[j + i] < z;
            u[j + i] = u[j + i] + borrow * CHUNK_BASE - z;
        }
        std::int64_t top = std::int64_t(u[j + n]) - std::int64_t(carry) - borrow;
        if (top < 0) {
            --qh;
            u32 c = 0;
            for (size_t i = 0; i < n; ++i) {
                const u32 z = u[j + i] + v[i] + c;
                c = z >= CHUNK_BASE;
                u[j + i] = z - c * CHUNK_BASE;
            }
            top += c;
        }
        u[j + n] = u32(top);
        q[j] = u32(qh);
    }
    trim(q);
    u.resize(n);
    trim(u);
    return {std::move(q), div_small(u, norm)};
}

// Fixed-point reciprocal B^(2m)/a, at most two integer units low.
// Precondition: a is normalised, a.back() >= CHUNK_BASE/2.
inline chunk_vec reciprocal(const chunk_vec& a) {
    const size_t m = a.size();
    if (m <= 24) return knuth_div(pow_base(2 * m), a).first;
    const size_t h = (m + 1) / 2 + 2;
    chunk_vec r = reciprocal(shr(a, m - h));
    const chunk_vec rr = mul(r, r);
    const chunk_vec t = shr(mul(a, rr), 2 * h);
    chunk_vec ans = sub(shl(mul_small(r, 2), m - h), t);
    dec(ans);
    return ans;
}

inline div_pair div_with_inverse(const chunk_vec& a, const chunk_vec& b, const chunk_vec& inv,
                                 ntt_cached* inv_cache = nullptr, ntt_cached* b_cache = nullptr) {
    if (cmp(a, b) < 0) return {chunk_vec{}, a};
    const size_t m = b.size();
    chunk_vec q = shr(mul(shr(a, m - 1), inv, inv_cache), m + 1);
    chunk_vec prod = mul(q, b, b_cache);
    // Exact corrections, also for values sitting right next to a multiple.
    while (cmp(prod, a) > 0) {
        dec(q);
        prod = sub(prod, b);
    }
    chunk_vec r = sub(a, prod);
    while (cmp(r, b) >= 0) {
        inc(q);
        r = sub(r, b);
    }
    return {std::move(q), std::move(r)};
}

inline div_pair full_div(const chunk_vec& a, const chunk_vec& b) {
    if (b.empty()) throw std::domain_error("division by zero");
    if (cmp(a, b) < 0) return {chunk_vec{}, a};
    if (b.size() <= 24 || (a.size() - b.size() + 1) * b.size() <= 4096) return knuth_div(a, b);
    const u32 norm = CHUNK_BASE / (b.back() + 1);
    const chunk_vec u = mul_small(a, norm);
    chunk_vec v = mul_small(b, norm);
    const chunk_vec inv = reciprocal(v);
    const size_t m = v.size();
    if (u.size() <= 2 * m) {
        div_pair qr = div_with_inverse(u, v, inv);
        qr.second = div_small(qr.second, norm);
        return qr;
    }
    const size_t count = (u.size() + m - 1) / m;
    chunk_vec q(count * m), rem;
    // Every block multiplies by the same divisor and the same reciprocal, so
    // their transforms are kept instead of being rebuilt per block.
    ntt_cached inv_cache, b_cache;
    for (size_t j = count; j--;) {
        chunk_vec x = shl(rem, m);
        x = add(x, slice(u, j * m, (j + 1) * m));
        div_pair qr = div_with_inverse(x, v, inv, &inv_cache, &b_cache);
        std::copy(qr.first.begin(), qr.first.end(), q.begin() + j * m);
        rem = std::move(qr.second);
    }
    trim(q);
    return {std::move(q), div_small(rem, norm)};
}

// Narrow quotients only need the top limbs of the divisor; when the truncated
// remainder is at least the quotient the same quotient is already certified.
inline chunk_vec divide(const chunk_vec& a, const chunk_vec& b) {
    if (b.empty()) throw std::domain_error("division by zero");
    const int c = cmp(a, b);
    if (c < 0) return {};
    if (!c) return {1};
    const size_t qlim = a.size() - b.size() + 1, keep = qlim + 3;
    if (b.size() > keep) {
        const size_t cut = b.size() - keep;
        const chunk_vec ah = shr(a, cut), bh = shr(b, cut);
        div_pair qr = full_div(ah, bh);
        if (cmp(qr.second, qr.first) >= 0) return std::move(qr.first);
        chunk_vec p = mul(qr.first, b);
        if (cmp(p, a) > 0) dec(qr.first);
        return std::move(qr.first);
    }
    return full_div(a, b).first;
}

// Same truncation as divide(), but the verifying multiplication of the quotient
// is skipped: the value stays within a couple of units of the true quotient.
// The k-th root iteration only needs that much, and its final correction walk
// pins the answer down exactly, so the division can stop one step earlier.
inline chunk_vec quotient_estimate(const chunk_vec& a, const chunk_vec& b) {
    if (b.empty()) throw std::domain_error("division by zero");
    const int c = cmp(a, b);
    if (c < 0) return {};
    if (!c) return {1};
    const size_t qlim = a.size() - b.size() + 1, keep = qlim + 3;
    const size_t cut = b.size() > keep ? b.size() - keep : 0;
    const chunk_vec ah = cut ? shr(a, cut) : a;
    const chunk_vec bh = cut ? shr(b, cut) : b;
    if (cmp(ah, bh) < 0) return {};
    if (bh.size() <= 24 || (ah.size() - bh.size() + 1) * bh.size() <= 4096)
        return knuth_div(ah, bh).first;
    const u32 norm = CHUNK_BASE / (bh.back() + 1);
    const chunk_vec v = mul_small(bh, norm);
    const chunk_vec u = mul_small(ah, norm);
    const size_t m = v.size();
    if (u.size() > 2 * m) return full_div(ah, bh).first;
    const chunk_vec inv = reciprocal(v);
    return shr(mul(shr(u, m - 1), inv), m + 1);
}

// x^k by square and multiply (k is small in practice).
inline chunk_vec power(const chunk_vec& x, int k) {
    if (k < 0) throw std::invalid_argument("negative power");
    chunk_vec r{1}, b = x;
    for (int e = k; e; e >>= 1) {
        if (e & 1) r = mul(r, b);
        if (e > 1) b = mul(b, b);
    }
    return r;
}

// Integer k-th root by Newton iteration with doubling precision: the estimate
// is carried at increasing accuracy, and only the last rounds are full width.
// This is the reference loop, kept for large k and for the correction fallback.
inline chunk_vec root_classic(const chunk_vec& a, int k) {
    if (k < 1) throw std::invalid_argument("invalid root order");
    if (k == 1 || a.empty() || (a.size() == 1 && a[0] == 1)) return a;
    const size_t len = (a.size() + k - 1) / k;
    chunk_vec x;
    if (len <= 4) {
        x = pow_base(len);
    } else {
        const size_t s = (len - 2) / 2;
        x = root_classic(shr(a, size_t(k) * s), k);
        inc(x);
        x = shl(x, s);
    }
    for (;;) {
        chunk_vec y = div_small(
            add(mul_small(x, u32(k - 1)), divide(a, power(x, k - 1))), u32(k));
        if (cmp(y, x) >= 0) return x;
        x = std::move(y);
    }
}

// ---------------------------------------------------------------------------
// Fast integer k-th root.
//
// A Newton step from an estimate that is only correct in its leading half
// already lands within a couple of units of the true root, so the doubling
// recursion needs a single step per level instead of iterating to a fixed
// point. The candidate is then pinned down exactly by walking over its powers:
// every step of that walk is a small linear combination of the powers that are
// already known, so it never pays for another full width division.
// ---------------------------------------------------------------------------

inline u32 binomial_coeff(int n, int r) {
    if (r < 0 || r > n) return 0;
    if (r > n - r) r = n - r;
    u64 v = 1;
    for (int i = 1; i <= r; ++i) v = v * u64(n - r + i) / u64(i);
    return u32(v);
}

// Powers y^0..y^k of the candidate root.
inline void root_chain(const chunk_vec& y, int k, std::vector<chunk_vec>& c) {
    c.assign(size_t(k) + 1, chunk_vec());
    c[0] = chunk_vec{1};
    c[1] = y;
    for (int j = 2; j <= k; ++j) c[size_t(j)] = mul(c[size_t(j - 1)], y);
}

// (y + dir)^k from the power chain of y, with dir = +1 or -1.
inline chunk_vec root_neighbour(const std::vector<chunk_vec>& c, int k, int dir) {
    chunk_vec pos, neg;
    for (int t = 0; t <= k; ++t) {
        const u32 bin = binomial_coeff(k, t);
        if (!bin) continue;
        const chunk_vec term = mul_small(c[size_t(t)], bin);
        if (dir < 0 && ((k - t) & 1)) neg = neg.empty() ? term : add(neg, term);
        else pos = pos.empty() ? term : add(pos, term);
    }
    if (neg.empty()) return pos;
    return sub(pos, neg);
}

// Move the chain to the powers of y + dir (only small multiples are needed).
inline void root_chain_step(std::vector<chunk_vec>& c, int k, int dir) {
    std::vector<chunk_vec> next(size_t(k) + 1);
    next[0] = chunk_vec{1};
    for (int j = 1; j <= k; ++j) {
        chunk_vec pos, neg;
        for (int t = 0; t <= j; ++t) {
            const u32 bin = binomial_coeff(j, t);
            if (!bin) continue;
            const chunk_vec term = mul_small(c[size_t(t)], bin);
            if (dir < 0 && ((j - t) & 1)) neg = neg.empty() ? term : add(neg, term);
            else pos = pos.empty() ? term : add(pos, term);
        }
        next[size_t(j)] = neg.empty() ? pos : sub(pos, neg);
    }
    c.swap(next);
}

// Estimate of a^(1/k) with the leading half of the digits correct, built by the
// doubling recursion with one Newton step per level.
inline chunk_vec root_estimate(const chunk_vec& a, int k) {
    const size_t len = (a.size() + size_t(k) - 1) / size_t(k);
    if (len <= 8) return root_classic(a, k);
    const size_t s = (len - 2) / 2;
    chunk_vec h = root_estimate(shr(a, size_t(k) * s), k);
    inc(h);
    inc(h);
    // The starting value is x = h * B^s, so x^(k-1) = h^(k-1) * B^((k-1)s):
    // the power and the division are both done on the short h instead of the
    // shifted x, which keeps them a factor (k-1)-ish smaller.
    const chunk_vec t = power(h, k - 1);
    const chunk_vec q = quotient_estimate(shr(a, size_t(k - 1) * s), t);
    const chunk_vec y = div_small(
        add(shl(mul_small(h, u32(k - 1)), s), q), u32(k));
    return y;
}

// Exact k-th root: from the estimate, walk to floor(a^(1/k)) with exact
// comparisons; the walk is O(k^2) small operations per unit of correction.
inline chunk_vec root_exact(const chunk_vec& a, const chunk_vec& estimate, int k) {
    std::vector<chunk_vec> c;
    root_chain(estimate, k, c);
    for (int guard = 0; guard < 8; ++guard) {
        if (cmp(c[size_t(k)], a) <= 0) {
            // c[k] <= a: the answer is the current candidate at least.
            if (cmp(root_neighbour(c, k, +1), a) > 0) return c[1];
            root_chain_step(c, k, +1);
        } else {
            // never walk below one: the fallback handles that corner
            if (c[1].size() <= 1 && (c[1].empty() || c[1][0] <= 1)) break;
            root_chain_step(c, k, -1);
        }
    }
    return root_classic(a, k);
}

inline chunk_vec root(const chunk_vec& a, int k) {
    if (k < 1) throw std::invalid_argument("invalid root order");
    if (k == 1 || a.empty() || (a.size() == 1 && a[0] == 1)) return a;
    if (k > 8) return root_classic(a, k);
    return root_exact(a, root_estimate(a, k), k);
}

inline div_pair divmod(const chunk_vec& a, const chunk_vec& b) {
    if (b.empty()) throw std::domain_error("division by zero");
    if (cmp(a, b) < 0) return {chunk_vec{}, a};
    return full_div(a, b);
}

} // namespace remote_detail
} // namespace mal
