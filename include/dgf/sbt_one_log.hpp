#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <stdexcept>

namespace mal { namespace dgf {

// Farey / Stern-Brocot lattice count for D_{1,1}(N).
// GNU C++14 __int128 is used for exact intermediates. The square root is
// seeded with long double and corrected by integer inequalities.
// Word-RAM time O(N^(1/3) log(N+2)), stack O(N^(1/6)).
class SBTDivisorSummatory {
    using i128 = __int128;
    using u128 = unsigned __int128;
    std::uint64_t n_;
    static u128 isqrt(u128 n) {
        u128 r = static_cast<u128>(std::sqrt(static_cast<long double>(n)));
        while ((r + 1) <= n / (r + 1)) ++r;
        while (r && r > n / r) --r;
        return r;
    }
    static u128 icbrt(u128 n) {
        u128 r = static_cast<u128>(std::cbrt(static_cast<long double>(n)));
        while ((r + 1) * (r + 1) * (r + 1) <= n) ++r;
        while (r * r * r > n) --r;
        return r;
    }
    static i128 triangular(i128 x) {
        return x * (x + 1) / 2;
    }
    static i128 ceil_sqrt(u128 n) {
        u128 r = isqrt(n);
        return i128(r + (r * r < n));
    }
    static i128 floor_div(i128 a, i128 b) {
        i128 q = a / b, r = a % b;
        return q - (r < 0);
    }
    struct Region {
        i128 w, h, a, b, c, d, e, f;
    };
    i128 product(const Region& r, i128 u, i128 v) const {
        i128 x = r.e * (u + r.c) - r.b * (v + r.f);
        i128 y = r.a * (v + r.f) - r.d * (u + r.c);
        return x * y;
    }
    i128 fv(const Region& r, i128 u) const {
        i128 t = u + r.c, ab = r.a * r.b;
        u128 disc = u128(t * t - 4 * ab * i128(n_));
        return floor_div((r.a * r.e + r.b * r.d) * t - ceil_sqrt(disc),
                         2 * ab) - r.f;
    }
    i128 fu(const Region& r, i128 v) const {
        i128 t = v + r.f, de = r.d * r.e;
        u128 disc = u128(t * t - 4 * de * i128(n_));
        return floor_div((r.a * r.e + r.b * r.d) * t - ceil_sqrt(disc),
                         2 * de) - r.c;
    }
    i128 count(Region r, i128 q) const {
        if (r.w <= 0 || r.h <= 0) return 0;
        i128 result = 0;
        if (product(r, r.w, 1) <= i128(n_)) {
            i128 rows = std::min(r.h, fv(r, r.w));
            result += r.w * rows;
            r.f += rows;
            r.h -= rows;
        }
        if (r.w <= 0 || r.h <= 0) return result;
        if (product(r, 1, r.h) <= i128(n_)) {
            i128 cols = std::min(r.w, fu(r, r.h));
            result += r.h * cols;
            r.c += cols;
            r.w -= cols;
        }
        if (r.w <= 0 || r.h <= 0) return result;
        if (std::min(r.w, r.h) <= 1 || r.b + r.e >= q) {
            if (r.w <= r.h) {
                for (i128 u = 1; u <= r.w; ++u) result += fv(r, u);
            } else {
                for (i128 v = 1; v <= r.h; ++v) result += fu(r, v);
            }
            return result;
        }
        i128 a = r.a + r.d, b = r.b + r.e;
        i128 ku = r.a * r.e + r.b * r.d + 2 * r.a * r.b;
        i128 kv = r.a * r.e + r.b * r.d + 2 * r.d * r.e;
        i128 tu = i128(isqrt(u128(ku * ku * i128(n_) / (a * b)))) - r.c;
        i128 tv = i128(isqrt(u128(kv * kv * i128(n_) / (a * b)))) - r.f;
        if (tu < 1) {
            i128 v1 = fv(r, 1), z = 1 + v1;
            return result + triangular(v1)
                 + count({r.w-z, v1, a, b, r.c+r.f+z, r.d, r.e, r.f}, q);
        }
        if (tv < 1) {
            i128 u1 = fu(r, 1), z = 1 + u1;
            return result + triangular(u1)
                 + count({u1, r.h-z, r.a, r.b, r.c, a, b, r.c+r.f+z}, q);
        }
        i128 u4 = tu, u5 = u4 + 1;
        i128 v4 = fv(r, u4), v5 = fv(r, u5);
        i128 zl = u4 + v4, zr = u5 + v5;
        result += triangular(zl - 1) - triangular(zl - u5)
                + triangular(zr - u5);
        result += count({u4, r.h-zl, r.a, r.b, r.c, a, b, r.c+r.f+zl}, q);
        result += count({r.w-zr, v5, a, b, r.c+r.f+zr, r.d, r.e, r.f}, q);
        return result;
    }
public:
    explicit SBTDivisorSummatory(std::uint64_t n) : n_(n) {
        if (n > 1000000000000000000ULL)
            throw std::invalid_argument("SBT divisor sum: N exceeds 10^18");
    }
    unsigned __int128 run() const {
        if (!n_) return 0;
        i128 m = i128(isqrt(n_));
        u128 cube = icbrt(u128(2) * n_);
        i128 l = std::min(m, 2 * i128(cube +
                            (cube * cube * cube < u128(2) * n_)));
        i128 result = 0;
        for (i128 x = 1; x < l; ++x) result += n_ / std::uint64_t(x);
        result += (m - l + 1) * i128(n_ / std::uint64_t(m)) + triangular(m-l);
        i128 k = 1, x2 = m, y2 = n_ / std::uint64_t(m), c2 = x2 + y2;
        for (;;) {
            i128 p = k + 1;
            i128 x4 = i128(isqrt(n_ / std::uint64_t(p))), x5 = x4 + 1;
            if (x4 <= l) break;
            i128 y4 = n_ / std::uint64_t(x4), y5 = n_ / std::uint64_t(x5);
            i128 c4 = p * x4 + y4, c5 = p * x5 + y5;
            result += triangular(c4-c2-l) - triangular(c4-c2-x5)
                    + triangular(c5-c2-x5);
            i128 w = p*x2 + y2 - c5, h = k*x5 + y5 - c2;
            i128 width = x2 - x5;
            i128 r = i128(icbrt(u128(std::max<i128>(1, width))));
            i128 q = r + (r*r*r < std::max<i128>(1,width));
            result += count({w,h,p,1,c5,k,1,c2},q);
            k = p;
            x2 = x4;
            y2 = y4;
            c2 = c4;
        }
        for (i128 x = l; x < x2; ++x)
            result += i128(n_ / std::uint64_t(x)) - (k*(x2-x) + y2);
        return static_cast<u128>(2*result - m*m);
    }
};

// The exponent tuple stays in the public API. Other D_x cases can receive
// their own specializations without misleadingly extending this algorithm.
template<int... Exponents> struct D_x_algorithm;

template<> struct D_x_algorithm<1, 1> {
    static unsigned __int128 run(std::uint64_t n) {
        return SBTDivisorSummatory(n).run();
    }
};

template<int... Exponents>
inline unsigned __int128 D_x(std::uint64_t n) {
    return D_x_algorithm<Exponents...>::run(n);
}

} } // namespace mal::dgf
