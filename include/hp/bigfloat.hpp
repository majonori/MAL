#pragma once
#include "bigint.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <ostream>
#include <stdexcept>
#include <string>

namespace mal {

// Binary floating point with a signed BigInt mantissa:
//     value = m_ * 2 ^ e_
// The precision is the number of significant mantissa bits; it is carried by
// each object so a computation can adapt to its caller.
class BigFloat {
    BigInt m_;
    long long e_;
    int prec_;

    static int require_prec(int p) {
        if (p <= 0) throw std::invalid_argument("BigFloat precision must be positive");
        return p;
    }

    static int ceil_log2(long long x) {
        int r = 0;
        long long v = 1;
        while (v < x) {
            v <<= 1;
            ++r;
        }
        return r;
    }

    void round_to(int p, int sticky_sign = 0) {
        if (m_.is_zero()) {
            e_ = 0;
            return;
        }
        BigInt mag = m_.abs();
        const size_t bits = mag.bit_length();
        const size_t sh = bits > size_t(p) ? bits - size_t(p) : 0;
        BigInt q = mag >> sh;
        // sticky_sign is the sign of the discarded tail in the signed value.
        // Convert it to the direction in which the magnitude is moving.
        const int mag_tail = m_.is_negative() ? -sticky_sign : sticky_sign;
        bool round_up = false;
        if (sh) {
            const bool half = mag.bit(sh - 1);
            const bool lower = mag.any_low_bits(sh - 1);
            if (half) {
                if (lower) round_up = true;
                else if (mag_tail > 0) round_up = true;
                else if (mag_tail == 0) round_up = q.bit(0);
            }
        }
        if (round_up) q += BigInt(1);
        e_ += (long long)sh;
        if (q.bit_length() > size_t(p)) {
            q = q >> 1;
            ++e_;
        }
        m_ = m_.is_negative() ? -q : q;
    }

    static BigFloat from_value(const BigInt& m, long long e, int p) {
        BigFloat r;
        r.m_ = m;
        r.e_ = e;
        r.prec_ = require_prec(p);
        r.round_to(p);
        return r;
    }

    static BigFloat ln2_const(int p);
    static BigFloat sqrt_positive(const BigFloat& x, int p);
    static BigFloat inv_sqrt_positive(const BigFloat& x, int p);
    static BigFloat reciprocal_positive(const BigFloat& x, int p);
    static BigFloat pi_const(int p);
    static BigFloat log_agm_kernel(const BigFloat& x, int p);
    static BigFloat log_agm_value(const BigFloat& x, int p);
    static BigFloat exp_newton(const BigFloat& x, int p);

public:
    static constexpr int DEFAULT_PRECISION = 256;
    // Below this size exact scaled integer division is cheaper; above it
    // Newton iteration avoids the quadratic Knuth division.
    static constexpr int NEWTON_DIV_THRESHOLD = 8192;
    // Below these sizes the elementary series are faster; above them use AGM
    // so the asymptotic cost follows the fast multiplier.
    static constexpr int AGM_LOG_THRESHOLD = 1 << 14;
    static constexpr int AGM_EXP_THRESHOLD = 1 << 16;

    BigFloat() : e_(0), prec_(DEFAULT_PRECISION) {}

    explicit BigFloat(long long v, int p = DEFAULT_PRECISION)
        : m_(v), e_(0), prec_(require_prec(p)) {
        round_to(prec_);
    }

    explicit BigFloat(const BigInt& v, int p = DEFAULT_PRECISION)
        : m_(v), e_(0), prec_(require_prec(p)) {
        round_to(prec_);
    }

    BigFloat(const BigInt& mantissa, long long exp2, int p)
        : m_(mantissa), e_(exp2), prec_(require_prec(p)) {
        round_to(prec_);
    }

    static BigFloat from_double(double v, int p = DEFAULT_PRECISION) {
        BigFloat r;
        r.e_ = 0;
        r.prec_ = require_prec(p);
        if (!std::isfinite(v)) throw std::invalid_argument("non-finite BigFloat");
        if (v == 0) {
            r.round_to(r.prec_);
            return r;
        }
        bool neg = v < 0;
        if (neg) v = -v;
        int ex = 0;
        double f = std::frexp(v, &ex); // v = f * 2^ex, f in [0.5, 1)
        long long mant = (long long)std::ldexp(f, 53);
        r.m_ = BigInt(neg ? -mant : mant);
        r.e_ = (long long)ex - 53;
        r.round_to(r.prec_);
        return r;
    }

    explicit BigFloat(const std::string& text, int p = DEFAULT_PRECISION)
        : e_(0), prec_(require_prec(p)) {
        *this = from_string(text, p);
    }

    int precision() const { return prec_; }
    long long exponent() const { return e_; }
    const BigInt& mantissa() const { return m_; }
    bool is_zero() const { return m_.is_zero(); }
    int sign() const { return m_.sign(); }
    BigFloat abs() const {
        BigFloat r = *this;
        if (r.m_.is_negative()) r.m_ = -r.m_;
        return r;
    }

    long long order() const {
        if (is_zero()) return std::numeric_limits<long long>::min() / 4;
        return e_ + (long long)m_.abs().bit_length() - 1;
    }

    BigFloat with_precision(int p) const {
        BigFloat r = *this;
        r.set_precision(p);
        return r;
    }

    void set_precision(int p) {
        p = require_prec(p);
        round_to(p);
        prec_ = p;
    }

    BigFloat ldexp(long long k) const {
        BigFloat r = *this;
        if (__builtin_add_overflow(r.e_, k, &r.e_))
            throw std::overflow_error("BigFloat exponent overflow");
        return r;
    }

    static BigFloat from_string(const std::string& text, int p = DEFAULT_PRECISION) {
        p = require_prec(p);
        size_t i = 0;
        bool neg = false;
        if (i < text.size() && (text[i] == '+' || text[i] == '-')) {
            neg = text[i] == '-';
            ++i;
        }
        std::string digit_text;
        digit_text.reserve(text.size());
        long long frac_digits = 0;
        bool seen_dot = false, any = false;
        for (; i < text.size(); ++i) {
            char c = text[i];
            if (c == '.') {
                if (seen_dot) throw std::invalid_argument("bad BigFloat literal");
                seen_dot = true;
                continue;
            }
            if (c < '0' || c > '9') break;
            any = true;
            digit_text.push_back(c);
            if (seen_dot) ++frac_digits;
        }
        if (!any) throw std::invalid_argument("bad BigFloat literal");
        BigInt digits = BigInt::from_string(digit_text, 10);
        long long exp10 = -frac_digits;
        if (i < text.size() && (text[i] == 'e' || text[i] == 'E')) {
            ++i;
            bool eneg = false;
            if (i < text.size() && (text[i] == '+' || text[i] == '-')) {
                eneg = text[i] == '-';
                ++i;
            }
            if (i == text.size()) throw std::invalid_argument("bad BigFloat exponent");
            long long ev = 0;
            for (; i < text.size(); ++i) {
                if (text[i] < '0' || text[i] > '9')
                    throw std::invalid_argument("bad BigFloat exponent");
                ev = ev * 10 + (text[i] - '0');
                if (ev > (long long)1e18) throw std::overflow_error("BigFloat exponent overflow");
            }
            if (eneg) {
                if (exp10 < std::numeric_limits<long long>::min() + ev)
                    throw std::overflow_error("BigFloat exponent overflow");
                exp10 -= ev;
            } else {
                if (exp10 > std::numeric_limits<long long>::max() - ev)
                    throw std::overflow_error("BigFloat exponent overflow");
                exp10 += ev;
            }
        }
        if (i != text.size()) throw std::invalid_argument("bad BigFloat literal");
        if (neg) digits = -digits;
        const int wp = p + 64;
        BigFloat r(BigInt(digits), wp);
        if (exp10 != 0) {
            BigInt ten(10);
            long long ae = exp10 < 0 ? -exp10 : exp10;
            BigInt scale = ten.pow((unsigned long long)ae);
            if (exp10 > 0) r = r * BigFloat(scale, wp);
            else r = r / BigFloat(scale, wp);
        }
        r.set_precision(p);
        return r;
    }

    double to_double() const {
        if (is_zero()) return 0.0;
        BigInt mag = m_.abs();
        size_t bits = mag.bit_length();
        size_t take = bits > 53 ? bits - 53 : 0;
        BigInt top = mag >> take;
        double d = (double)top.low64();
        long long ex = e_ + (long long)take;
        if (ex > 1024) return m_.is_negative() ? -HUGE_VAL : HUGE_VAL;
        if (ex < -1100) return 0.0;
        d = std::ldexp(d, (int)ex);
        return m_.is_negative() ? -d : d;
    }

    std::string to_string() const {
        int digits = std::max(17, (int)std::ceil(prec_ * 0.3010299956639812) + 2);
        return to_string(digits);
    }

    std::string to_string(int digits) const {
        if (digits < 1) throw std::invalid_argument("invalid digit count");
        if (is_zero()) return "0";
        BigInt mag = m_.abs();
        long double approx = (long double)order() * 0.301029995663981195L;
        long long dec_exp = (long long)std::floor(approx);
        auto scale_pow10 = [](long long n) {
            if (n < 0) throw std::invalid_argument("negative power");
            BigInt ten(10);
            return ten.pow((unsigned long long)n);
        };
        BigInt q, r;
        for (int attempt = 0; attempt < 2; ++attempt) {
            long long p = (long long)digits - 1 - dec_exp;
            BigInt num = mag;
            BigInt den(1);
            if (e_ >= 0) num = num << (size_t)e_;
            else den = den << (size_t)(-e_);
            if (p >= 0) num = num * scale_pow10(p);
            else den = den * scale_pow10(-p);
            BigInt::divmod(num, den, q, r);
            BigInt twice = r << 1;
            int c = compare(twice, den);
            if (c > 0 || (c == 0 && q.bit(0))) q += BigInt(1);
            std::string ds = q.to_string();
            if ((int)ds.size() <= digits) {
                if (dec_exp >= digits - 1) {
                    ds.append((size_t)(dec_exp - (digits - 1)), '0');
                } else if (dec_exp >= 0) {
                    ds.insert((size_t)dec_exp + 1, 1, '.');
                } else {
                    ds = "0." + std::string((size_t)(-dec_exp - 1), '0') + ds;
                }
                if (m_.is_negative()) ds.insert(ds.begin(), '-');
                return ds;
            }
            ++dec_exp; // the estimate was one power of ten too small
        }
        throw std::runtime_error("BigFloat formatting failed");
    }

    BigFloat& operator+=(const BigFloat& b) {
        *this = *this + b;
        return *this;
    }
    BigFloat& operator-=(const BigFloat& b) {
        *this = *this - b;
        return *this;
    }
    BigFloat& operator*=(const BigFloat& b) {
        *this = *this * b;
        return *this;
    }
    BigFloat& operator/=(const BigFloat& b) {
        *this = *this / b;
        return *this;
    }

    BigFloat operator-() const {
        BigFloat r = *this;
        r.m_ = -r.m_;
        return r;
    }

    BigFloat operator+() const { return *this; }

    friend BigFloat operator+(const BigFloat& a, const BigFloat& b) {
        const int p = std::max(a.prec_, b.prec_);
        if (a.is_zero()) return b.with_precision(p);
        if (b.is_zero()) return a.with_precision(p);
        BigInt ma = a.m_, mb = b.m_;
        long long ea = a.e_, eb = b.e_;
        if (ea < eb) {
            std::swap(ma, mb);
            std::swap(ea, eb);
        }
        const long long d = ea - eb;
        const long long cap = (long long)p + 66;
        BigInt s;
        long long e = eb;
        int sticky = 0;
        if (d <= cap) {
            s = (ma << (size_t)d) + mb;
        } else {
            const long long sh = d - cap;
            BigInt hi = ma << (size_t)cap;
            const size_t bl = mb.abs().bit_length();
            if ((unsigned long long)sh < bl) {
                BigInt tail = mb >> (size_t)sh;
                hi += tail;
                if (mb.abs().any_low_bits((size_t)sh)) sticky = mb.sign();
            } else if (!mb.is_zero()) {
                sticky = mb.sign();
            }
            s = hi;
            if (eb > std::numeric_limits<long long>::max() - sh)
                throw std::overflow_error("BigFloat exponent overflow");
            e = eb + sh;
        }
        BigFloat r;
        r.m_ = s;
        r.e_ = e;
        r.prec_ = p;
        r.round_to(p, sticky);
        return r;
    }

    friend BigFloat operator-(const BigFloat& a, const BigFloat& b) {
        return a + (-b);
    }

    friend BigFloat operator*(const BigFloat& a, const BigFloat& b) {
        const int p = std::max(a.prec_, b.prec_);
        if (a.is_zero() || b.is_zero()) {
            BigFloat r;
            r.prec_ = p;
            return r;
        }
        long long e;
        if (__builtin_add_overflow(a.e_, b.e_, &e))
            throw std::overflow_error("BigFloat exponent overflow");
        return from_value(a.m_ * b.m_, e, p);
    }

    friend BigFloat operator/(const BigFloat& a, const BigFloat& b) {
        const int p = std::max(a.prec_, b.prec_);
        if (b.is_zero()) throw std::domain_error("BigFloat division by zero");
        if (a.is_zero()) {
            BigFloat r;
            r.prec_ = p;
            return r;
        }
        if (p >= NEWTON_DIV_THRESHOLD) {
            BigFloat rb = reciprocal_positive(b.abs(), p + 64);
            BigFloat out = a * rb;
            if (b.sign() < 0) out = -out;
            out.set_precision(p);
            return out;
        }
        const int wp = p + 64;
        BigInt num = a.m_.abs(), den = b.m_.abs();
        long long shift = (long long)wp + 2 + (long long)den.bit_length() -
                          (long long)num.bit_length();
        if (shift < 0) shift = 0;
        BigInt scaled = num << (size_t)shift;
        BigInt q, r;
        BigInt::divmod(scaled, den, q, r);
        if (a.sign() != b.sign() && !q.is_zero()) q = -q;
        long long e;
        if (__builtin_sub_overflow(a.e_, b.e_, &e) ||
            __builtin_sub_overflow(e, shift, &e))
            throw std::overflow_error("BigFloat exponent overflow");
        BigFloat out;
        out.m_ = q;
        out.e_ = e;
        out.prec_ = p;
        out.round_to(p, r.is_zero() ? 0 : a.sign() * b.sign());
        return out;
    }

    BigFloat div_small(long long d, int p) const {
        if (d == 0) throw std::domain_error("BigFloat division by zero");
        p = require_prec(p);
        if (is_zero()) {
            BigFloat r;
            r.prec_ = p;
            return r;
        }
        bool dneg = d < 0;
        unsigned long long da = dneg ? 0ull - (unsigned long long)d : (unsigned long long)d;
        if (da > 0xffffffffull) throw std::invalid_argument("divisor too large");
        const int wp = p + 32;
        BigInt num = m_.abs();
        long long shift = (long long)wp + 2 + (long long)BigInt((long long)da).bit_length() -
                          (long long)num.bit_length();
        if (shift < 0) shift = 0;
        BigInt scaled = num << (size_t)shift;
        bigint_detail::limb rem = 0;
        BigInt q = scaled.div_small((bigint_detail::limb)da, &rem);
        const bool qneg = (sign() < 0) ^ dneg;
        if (qneg && !q.is_zero()) q = -q;
        BigFloat out;
        out.m_ = q;
        if (__builtin_sub_overflow(e_, shift, &out.e_))
            throw std::overflow_error("BigFloat exponent underflow");
        out.prec_ = p;
        out.round_to(p, rem ? (qneg ? -1 : 1) : 0);
        return out;
    }

    friend int compare(const BigFloat& a, const BigFloat& b) {
        if (a.is_zero() || b.is_zero()) {
            if (a.is_zero() && b.is_zero()) return 0;
            return a.is_zero() ? -b.sign() : a.sign();
        }
        const long long oa = a.order(), ob = b.order();
        if (oa != ob) return oa < ob ? -1 : 1;
        long long d = a.e_ - b.e_;
        BigInt x = a.m_, y = b.m_;
        if (d > 0) x = x << (size_t)d;
        else if (d < 0) y = y << (size_t)(-d);
        return compare(x, y);
    }
    friend bool operator==(const BigFloat& a, const BigFloat& b) { return compare(a, b) == 0; }
    friend bool operator!=(const BigFloat& a, const BigFloat& b) { return compare(a, b) != 0; }
    friend bool operator<(const BigFloat& a, const BigFloat& b) { return compare(a, b) < 0; }
    friend bool operator>(const BigFloat& a, const BigFloat& b) { return compare(a, b) > 0; }
    friend bool operator<=(const BigFloat& a, const BigFloat& b) { return compare(a, b) <= 0; }
    friend bool operator>=(const BigFloat& a, const BigFloat& b) { return compare(a, b) >= 0; }
    friend std::ostream& operator<<(std::ostream& os, const BigFloat& x) {
        return os << x.to_string();
    }

    static BigFloat exp(const BigFloat& x) {
        // Small precision uses the series below; large precision uses
        // Newton iterations with AGM logarithms.
        const int p = x.precision();
        if (x.is_zero()) return BigFloat(1, p);
        if (p >= AGM_EXP_THRESHOLD) return exp_newton(x, p);
        const bool neg = x.sign() < 0;
        BigFloat ax = neg ? -x : x;
        const long long ord = ax.order();
        // exp(2^64) already needs an exponent outside long long's range.
        if (ord >= 64) throw std::overflow_error("BigFloat exp argument too large");
        long long k = std::max(0ll, ord + ceil_log2((long long)p + 4) + 2);
        if (k > 1000000) throw std::overflow_error("BigFloat exp argument too large");
        if (k > std::numeric_limits<int>::max() - (long long)p - 40)
            throw std::overflow_error("BigFloat exp precision too large");
        const int wp = p + (int)k + 40;
        BigFloat y = ax;
        y.set_precision(wp);
        y.e_ -= k;

        BigFloat sum(1, wp), term(1, wp);
        const long long limit = std::max<long long>(1000, (long long)wp * 4 + 100);
        for (long long i = 1; i <= limit; ++i) {
            term = term * y;
            term = term.div_small(i, wp);
            if (term.is_zero() || term.order() < -(long long)wp - 8) break;
            sum = sum + term;
        }
        for (long long i = 0; i < k; ++i) sum = sum * sum;
        if (neg) sum = BigFloat(1, wp) / sum;
        sum.set_precision(p);
        return sum;
    }

    static BigFloat log(const BigFloat& x) {
        // Small precision uses sqrt reduction + atanh series; large precision
        // uses the AGM/theta backend.
        if (x.sign() <= 0) throw std::domain_error("BigFloat log domain error");
        const int p = x.precision();
        if (p >= AGM_LOG_THRESHOLD) return log_agm_value(x, p);
        const int s = std::max(2, ceil_log2((long long)p + 4) + 2);
        if ((long long)p + s + 72 > std::numeric_limits<int>::max())
            throw std::overflow_error("BigFloat log precision too large");
        const int wp = p + s + 72;
        BigInt num = x.m_.abs();
        const size_t bl = num.bit_length();
        long long E;
        if (__builtin_add_overflow(x.e_, (long long)bl - 1, &E))
            throw std::overflow_error("BigFloat log exponent overflow");
        BigFloat m(num, -((long long)bl - 1), wp);
        for (int i = 0; i < s; ++i) m = sqrt_positive(m, wp);
        BigFloat one(1, wp);
        BigFloat t = (m - one) / (m + one);
        BigFloat t2 = t * t;
        BigFloat sum = t, term = t;
        const long long limit = std::max<long long>(1000, (long long)wp * 2 + 100);
        for (long long i = 1; i <= limit; ++i) {
            term = term * t2;
            BigFloat add = term.div_small(2 * i + 1, wp);
            if (add.is_zero() || add.order() < -(long long)wp - 8) break;
            sum = sum + add;
        }
        // log(m_original) = 2 * 2^s * atanh(t).
        sum.e_ += (long long)s + 1;
        BigFloat res = sum + BigFloat(E, wp) * ln2_const(wp);
        res.set_precision(p);
        return res;
    }

    static BigFloat sqrt(const BigFloat& x) {
        return sqrt_positive(x, x.precision());
    }

    static BigFloat log_agm(const BigFloat& x) {
        return log_agm_value(x, x.precision());
    }

    static BigFloat exp_newton_agm(const BigFloat& x) {
        return exp_newton(x, x.precision());
    }

    static BigFloat pi(int p) { return pi_const(p); }
    static BigFloat ln2(int p) { return ln2_const(p); }

    static BigFloat pow(const BigFloat& x, long long e) {
        const int p = x.precision();
        if (e == 0) return BigFloat(1, p);
        const bool neg = e < 0;
        unsigned long long u = neg ? 0ull - (unsigned long long)e : (unsigned long long)e;
        BigFloat r(1, p), a = x;
        while (u) {
            if (u & 1) r = r * a;
            u >>= 1;
            if (u) a = a * a;
        }
        if (neg) r = BigFloat(1, p) / r;
        r.set_precision(p);
        return r;
    }
};

// Namespace-scope declarations of the operators defined as in-class friends
// above. Each declaration names the same function as its friend definition, so
// other translation units can declare and call the operators without relying on
// argument-dependent lookup. Kept in sync with bundles/hp/README.md.
BigFloat operator+(const BigFloat& a, const BigFloat& b);
BigFloat operator-(const BigFloat& a, const BigFloat& b);
BigFloat operator*(const BigFloat& a, const BigFloat& b);
BigFloat operator/(const BigFloat& a, const BigFloat& b);
int compare(const BigFloat& a, const BigFloat& b);
bool operator==(const BigFloat& a, const BigFloat& b);
bool operator!=(const BigFloat& a, const BigFloat& b);
bool operator<(const BigFloat& a, const BigFloat& b);
bool operator>(const BigFloat& a, const BigFloat& b);
bool operator<=(const BigFloat& a, const BigFloat& b);
bool operator>=(const BigFloat& a, const BigFloat& b);
std::ostream& operator<<(std::ostream& os, const BigFloat& x);

inline BigFloat BigFloat::reciprocal_positive(const BigFloat& x, int p) {
    if (x.sign() <= 0) throw std::domain_error("BigFloat reciprocal domain error");
    p = require_prec(p);
    const long long ord = x.order();
    BigFloat a = x;
    a.e_ -= ord; // a is in [1, 2)
    const int work = p + 40;
    BigFloat r = from_double(1.0 / a.to_double(), 96);
    int cur = 53; // the double seed has about 53 correct bits
    const BigFloat two(2, work);
    while (cur < work) {
        const int next = std::min(work, cur << 1);
        const int guard = next + 32;
        BigFloat rr = r;
        rr.set_precision(guard);
        BigFloat aa = a;
        aa.set_precision(guard);
        rr = rr * (two - aa * rr);
        rr.set_precision(next);
        r = rr;
        cur = next;
    }
    // One polishing step absorbs the rounding of the last doubling step.
    {
        const int guard = work + 32;
        BigFloat rr = r;
        rr.set_precision(guard);
        BigFloat aa = a;
        aa.set_precision(guard);
        BigFloat two_guard = two;
        two_guard.set_precision(guard);
        rr = rr * (two_guard - aa * rr);
        rr.set_precision(work);
        r = rr;
    }
    r.e_ -= ord;
    r.set_precision(p);
    return r;
}

inline BigFloat BigFloat::pi_const(int p) {
    p = require_prec(p);
    static std::map<int, BigFloat> cache;
    auto it = cache.find(p);
    if (it != cache.end()) return it->second;
    const int wp = p + 40;
    const BigFloat one(1, wp), half = from_double(0.5, wp);
    BigFloat a = one;
    BigFloat b = one / sqrt_positive(BigFloat(2, wp), wp);
    BigFloat t = from_double(0.25, wp);
    BigFloat pw = one;
    const int iter = std::max(8, ceil_log2((long long)wp + 4) + 5);
    for (int i = 0; i < iter; ++i) {
        BigFloat an = (a + b) * half;
        BigFloat bn = sqrt_positive(a * b, wp);
        BigFloat d = a - an;
        t = t - (pw * d) * d;
        pw = pw.ldexp(1);
        a = an;
        b = bn;
        BigFloat diff = a - b;
        if (!diff.is_zero() && diff.order() < -(long long)wp - 4) break;
    }
    BigFloat pi = (a + b) * (a + b) / (t.ldexp(2));
    pi.set_precision(p);
    cache[p] = pi;
    return pi;
}

inline BigFloat BigFloat::log_agm_kernel(const BigFloat& x, int p) {
    if (x.sign() <= 0) throw std::domain_error("BigFloat log kernel domain error");
    p = require_prec(p);
    const BigFloat one(1, p);
    const BigFloat x2 = x * x;

    // jtheta2(x)^2
    BigFloat a = x2, b = x2, s = x2;
    const long long limit = (long long)p * 4 + 100;
    for (long long i = 0; i < limit; ++i) {
        b = b * x2;
        a = a * b;
        s = s + a;
        if (a.is_zero() || a.order() < -(long long)p - 8) break;
    }
    s = s + one;
    s = s * s;
    s = s.ldexp(2); // fixed-point shift by 2 means multiply by 4
    s = s * sqrt_positive(x, p);

    // jtheta3(x)^2
    a = x;
    b = x;
    BigFloat t = x;
    for (long long i = 0; i < limit; ++i) {
        b = b * x2;
        a = a * b;
        t = t + a;
        if (a.is_zero() || a.order() < -(long long)p - 8) break;
    }
    t = one + t.ldexp(1);
    t = t * t;

    BigFloat aa = s, bb = t;
    const BigFloat half = from_double(0.5, p);
    const int iter = std::max(8, ceil_log2((long long)p + 8) + 8);
    for (int i = 0; i < iter; ++i) {
        BigFloat an = (aa + bb) * half;
        BigFloat bn = sqrt_positive(aa * bb, p);
        BigFloat diff = an - bn;
        aa = an;
        bb = bn;
        if (!diff.is_zero() && diff.order() < -(long long)p - 4) break;
    }
    BigFloat agm = (aa + bb) * half;
    BigFloat res = pi_const(p) / agm;
    res.set_precision(p);
    return res;
}

inline BigFloat BigFloat::log_agm_value(const BigFloat& x, int p) {
    if (x.sign() <= 0) throw std::domain_error("BigFloat log domain error");
    p = require_prec(p);
    const long long mag = x.order();
    BigFloat ln2;

    // Exact powers of two avoid the subtraction of two nearly equal values.
    if (x.mantissa().abs() == BigInt(1)) {
        ln2 = ln2_const(p);
        BigFloat res = BigFloat(mag, p) * ln2;
        res.set_precision(p);
        return res;
    }

    const int wp = p + 64;
    long long optimal = -((long long)wp / 20);
    if (optimal >= 0) optimal = -1;
    long long n = optimal - mag;
    BigFloat xs = x.ldexp(n);
    const long long extra = -optimal;
    if (extra > (long long)std::numeric_limits<int>::max() - wp - 64)
        throw std::overflow_error("BigFloat log precision too large");
    const int kw = wp + (int)extra + 64;
    BigFloat k = log_agm_kernel(xs, kw); // -log(x * 2^n)
    ln2 = ln2_const(kw);
    BigFloat res = -k - BigFloat(n, kw) * ln2;
    res.set_precision(p);
    return res;
}

inline BigFloat BigFloat::exp_newton(const BigFloat& x, int p) {
    p = require_prec(p);
    if (x.is_zero()) return BigFloat(1, p);
    const bool neg = x.sign() < 0;
    BigFloat ax = neg ? -x : x;
    const long long ord = ax.order();
    if (ord >= 64) throw std::overflow_error("BigFloat exp argument too large");
    const long long k = std::max(0ll, ord + 1);
    if (k > std::numeric_limits<int>::max() - p - 128)
        throw std::overflow_error("BigFloat exp precision too large");
    const int wp = p + (int)k + 128;

    BigFloat y = ax;
    y.set_precision(wp);
    y.e_ -= k; // now 0 <= y <= 1
    BigFloat yc = from_double(std::exp(y.to_double()), 96);
    int cur = 53; // double seed accuracy
    while (cur < wp) {
        const int next = std::min(wp, cur << 1);
        if (next <= cur) break;
        const int gp = next + 40;
        BigFloat z = yc;
        z.set_precision(gp);
        BigFloat lg = log(z);
        lg.set_precision(gp);
        BigFloat r = y - lg;
        BigFloat cand = yc * (BigFloat(1, gp) + r);
        cand.set_precision(next);
        yc = cand;
        cur = next;
    }
    // Absorb the rounding of the last doubling step.
    {
        const int gp = wp + 40;
        BigFloat z = yc;
        z.set_precision(gp);
        BigFloat lg = log(z);
        lg.set_precision(gp);
        BigFloat r = y - lg;
        BigFloat cand = yc * (BigFloat(1, gp) + r);
        cand.set_precision(wp);
        yc = cand;
    }
    for (long long i = 0; i < k; ++i) yc = yc * yc;
    yc.set_precision(p);
    if (neg) yc = BigFloat(1, p) / yc;
    return yc;
}

inline BigFloat BigFloat::ln2_const(int p) {
    p = require_prec(p);
    static std::map<int, BigFloat> cache;
    auto it = cache.find(p);
    if (it != cache.end()) return it->second;
    if (p >= AGM_LOG_THRESHOLD) {
        const int wp = p + 64;
        const int n = std::max(1, p / 20);
        BigFloat x = BigFloat(1, wp).ldexp(-(long long)n);
        BigFloat v = log_agm_kernel(x, wp); // -log(2^-n) = n*log(2)
        BigFloat r;
        if ((unsigned long long)n <= 0xffffffffull) r = v.div_small(n, p);
        else r = v / BigFloat(n, p);
        r.set_precision(p);
        cache[p] = r;
        return r;
    }
    BigFloat t = BigFloat(1, p).div_small(3, p);
    BigFloat t2 = t * t;
    BigFloat sum = t, term = t;
    const long long limit = std::max<long long>(1000, (long long)p * 2 + 100);
    for (long long i = 1; i <= limit; ++i) {
        term = term * t2;
        BigFloat add = term.div_small(2 * i + 1, p);
        if (add.is_zero() || add.order() < -(long long)p - 8) break;
        sum = sum + add;
    }
    sum.e_ += 1; // 2 * atanh(1/3)
    cache[p] = sum;
    return sum;
}

inline BigFloat BigFloat::inv_sqrt_positive(const BigFloat& x, int p) {
    if (x.sign() <= 0) throw std::domain_error("BigFloat inverse sqrt domain error");
    p = require_prec(p);
    const int work = p + 40;
    BigFloat u = from_double(1.0 / std::sqrt(x.to_double()), 96);
    const BigFloat three(3, work);
    int cur = 53;
    while (cur < work) {
        const int next = std::min(work, cur << 1);
        const int guard = next + 32;
        BigFloat uu = u;
        uu.set_precision(guard);
        BigFloat xx = x;
        xx.set_precision(guard);
        BigFloat t = xx * (uu * uu);
        uu = uu * (three - t);
        uu.e_ -= 1; // divide by 2
        uu.set_precision(next);
        u = uu;
        cur = next;
    }
    {
        const int guard = work + 32;
        BigFloat uu = u;
        uu.set_precision(guard);
        BigFloat xx = x;
        xx.set_precision(guard);
        BigFloat t = xx * (uu * uu);
        uu = uu * (three - t);
        uu.e_ -= 1;
        uu.set_precision(work);
        u = uu;
    }
    u.set_precision(p);
    return u;
}

inline BigFloat BigFloat::sqrt_positive(const BigFloat& x, int p) {
    if (x.sign() <= 0) throw std::domain_error("BigFloat sqrt domain error");
    p = require_prec(p);
    BigInt num = x.m_.abs();
    const size_t bl = num.bit_length();
    long long E;
    if (__builtin_add_overflow(x.e_, (long long)bl - 1, &E))
        throw std::overflow_error("BigFloat sqrt exponent overflow");
    BigFloat f(num, -((long long)bl - 1), p + 32);
    if (E & 1ll) {
        f.e_ += 1;
        --E;
    }
    BigFloat u = inv_sqrt_positive(f, p + 32);
    BigFloat y = f * u;
    y.set_precision(p + 32);
    y.e_ += E / 2;
    y.set_precision(p);
    return y;
}

inline BigFloat exp(const BigFloat& x) { return BigFloat::exp(x); }
inline BigFloat log(const BigFloat& x) { return BigFloat::log(x); }
inline BigFloat sqrt(const BigFloat& x) { return BigFloat::sqrt(x); }
inline BigFloat pow(const BigFloat& x, long long e) { return BigFloat::pow(x, e); }

} // namespace mal
