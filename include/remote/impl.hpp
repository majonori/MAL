#pragma once
#include "interface.hpp"
#include "decimal.hpp"
#include "../hp/bigfloat.hpp"
#include "../hp/bigint.hpp"
#include <algorithm>
#include <cmath>

namespace {

int mal_remote_digits(int p) {
    return std::max(17, int(std::ceil(p * 0.3010299956639812)) + 2);
}

std::string mal_remote_small(const ::mal::BigFloat& v, int p) {
    return v.to_string(mal_remote_digits(p));
}

// True when the stored text is already the canonical decimal form of an
// integer ("0" or -?[1-9][0-9]*), in which case printing it needs no work.
bool mal_remote_canonical(const std::string& s) {
    size_t i = (!s.empty() && s[0] == '-') ? 1 : 0;
    if (i >= s.size()) return false;
    if (s[i] == '0') return i + 1 == s.size();
    for (; i < s.size(); ++i)
        if (s[i] < '0' || s[i] > '9') return false;
    return true;
}


// Integer operations run on the decimal core: the stored text is the number,
// so reading and printing are block copies and no base conversion happens.
std::string mal_remote_decimal(const ::mal::remote_detail::chunk_vec& v, bool neg) {
    std::string out = ::mal::remote_detail::str(v);
    if (neg && !v.empty()) out.insert(out.begin(), '-');
    return out;
}

// Sign of a decimal literal with leading zeros stripped.
int mal_remote_sign(const std::string& text) {
    size_t i = 0;
    if (i < text.size() && (text[i] == '+' || text[i] == '-')) ++i;
    bool neg = i > 0 && text[0] == '-';
    while (i < text.size() && text[i] == '0') ++i;
    if (i == text.size()) return 0;
    return neg ? -1 : 1;
}

} // namespace

namespace mal {
namespace remote {

std::string BigInt::to_string() const {
    if (mal_remote_canonical(s)) return s;
    const int sg = mal_remote_sign(s);
    if (!sg) return "0";
    return mal_remote_decimal(::mal::remote_detail::parse(s), sg < 0);
}

std::string BigInt::to_string(int base) const {
    return ::mal::BigInt(s).to_string(base);
}

bool BigInt::is_zero() const {
    return ::mal::BigInt(s).is_zero();
}

int BigInt::sign() const {
    return ::mal::BigInt(s).sign();
}

unsigned long long BigInt::bit_length() const {
    return (unsigned long long)::mal::BigInt(s).bit_length();
}

BigInt operator+(const BigInt& a, const BigInt& b) {
    using namespace ::mal::remote_detail;
    const int sa = mal_remote_sign(a.s), sb = mal_remote_sign(b.s);
    if (!sa) return b;
    if (!sb) return a;
    chunk_vec x = parse(a.s), y = parse(b.s);
    if (sa == sb) return BigInt(mal_remote_decimal(add(x, y), sa < 0));
    if (cmp(x, y) >= 0) return BigInt(mal_remote_decimal(sub(x, y), sa < 0));
    return BigInt(mal_remote_decimal(sub(y, x), sb < 0));
}

BigInt operator-(const BigInt& a, const BigInt& b) {
    return a + (-b);
}

BigInt operator-(const BigInt& a) {
    const int sg = mal_remote_sign(a.s);
    if (!sg) return BigInt("0");
    return BigInt(mal_remote_decimal(::mal::remote_detail::parse(a.s), sg > 0));
}

BigInt operator*(const BigInt& a, const BigInt& b) {
    using namespace ::mal::remote_detail;
    const int sa = mal_remote_sign(a.s), sb = mal_remote_sign(b.s);
    if (!sa || !sb) return BigInt("0");
    chunk_vec p = mul(parse(a.s), parse(b.s));
    return BigInt(mal_remote_decimal(p, (sa < 0) != (sb < 0)));
}

BigInt operator/(const BigInt& a, const BigInt& b) {
    using namespace ::mal::remote_detail;
    const int sa = mal_remote_sign(a.s), sb = mal_remote_sign(b.s);
    if (!sb) throw std::domain_error("division by zero");
    if (!sa) return BigInt("0");
    chunk_vec q = divide(parse(a.s), parse(b.s));
    return BigInt(mal_remote_decimal(q, (sa < 0) != (sb < 0)));
}

BigInt operator%(const BigInt& a, const BigInt& b) {
    using namespace ::mal::remote_detail;
    const int sa = mal_remote_sign(a.s), sb = mal_remote_sign(b.s);
    if (!sb) throw std::domain_error("division by zero");
    if (!sa) return BigInt("0");
    div_pair qr = divmod(parse(a.s), parse(b.s));
    return BigInt(mal_remote_decimal(qr.second, sa < 0));
}

BigInt operator<<(const BigInt& a, std::size_t bits) {
    return BigInt((::mal::BigInt(a.s) << bits).to_string());
}

BigInt operator>>(const BigInt& a, std::size_t bits) {
    return BigInt((::mal::BigInt(a.s) >> bits).to_string());
}

bool operator==(const BigInt& a, const BigInt& b) {
    return ::mal::BigInt(a.s) == ::mal::BigInt(b.s);
}

bool operator!=(const BigInt& a, const BigInt& b) {
    return ::mal::BigInt(a.s) != ::mal::BigInt(b.s);
}

bool operator<(const BigInt& a, const BigInt& b) {
    return ::mal::BigInt(a.s) < ::mal::BigInt(b.s);
}

bool operator>(const BigInt& a, const BigInt& b) {
    return ::mal::BigInt(a.s) > ::mal::BigInt(b.s);
}

bool operator<=(const BigInt& a, const BigInt& b) {
    return ::mal::BigInt(a.s) <= ::mal::BigInt(b.s);
}

bool operator>=(const BigInt& a, const BigInt& b) {
    return ::mal::BigInt(a.s) >= ::mal::BigInt(b.s);
}

BigInt abs(const BigInt& a) {
    return BigInt(::mal::BigInt(a.s).abs().to_string());
}

BigInt pow(const BigInt& a, unsigned long long e) {
    return BigInt(::mal::BigInt(a.s).pow(e).to_string());
}

BigInt sqrt(const BigInt& a) {
    return BigInt(::mal::BigInt(a.s).sqrt().to_string());
}

BigInt nroot(const BigInt& a, unsigned long long k) {
    using namespace ::mal::remote_detail;
    if (k == 0) throw std::invalid_argument("zeroth root");
    const int sg = mal_remote_sign(a.s);
    if (sg < 0) throw std::domain_error("nroot of a negative BigInt");
    if (!sg) return BigInt("0");
    if (k > 4096) return BigInt(::mal::BigInt(a.s).nroot(k).to_string());
    chunk_vec r = root(parse(a.s), int(k));
    return BigInt(mal_remote_decimal(r, false));
}

std::ostream& operator<<(std::ostream& os, const BigInt& x) {
    if (mal_remote_canonical(x.s)) return os << x.s;
    return os << x.to_string();
}

std::istream& operator>>(std::istream& is, BigInt& x) {
    std::string text;
    if (is >> text) x.s = text;
    return is;
}

std::string BigFloat::to_string() const {
    if (p <= 0) return ::mal::BigFloat(s).to_string();
    return ::mal::BigFloat(s, p).to_string(mal_remote_digits(p));
}

std::string BigFloat::to_string(int digits) const {
    if (digits < 0) return to_string();
    if (digits < 1) digits = 1;
    return ::mal::BigFloat(s, p > 0 ? p : 256).to_string(digits);
}

bool BigFloat::is_zero() const {
    return ::mal::BigFloat(s, p > 0 ? p : 256).is_zero();
}

int BigFloat::sign() const {
    return ::mal::BigFloat(s, p > 0 ? p : 256).sign();
}

int BigFloat::precision() const {
    return p;
}

BigFloat operator+(const BigFloat& a, const BigFloat& b) {
    const int p = std::max(a.p, b.p);
    const ::mal::BigFloat x(a.s, p), y(b.s, p);
    return BigFloat(mal_remote_small(x + y, p), p);
}

BigFloat operator-(const BigFloat& a, const BigFloat& b) {
    const int p = std::max(a.p, b.p);
    const ::mal::BigFloat x(a.s, p), y(b.s, p);
    return BigFloat(mal_remote_small(x - y, p), p);
}

BigFloat operator*(const BigFloat& a, const BigFloat& b) {
    const int p = std::max(a.p, b.p);
    const ::mal::BigFloat x(a.s, p), y(b.s, p);
    return BigFloat(mal_remote_small(x * y, p), p);
}

BigFloat operator/(const BigFloat& a, const BigFloat& b) {
    const int p = std::max(a.p, b.p);
    const ::mal::BigFloat x(a.s, p), y(b.s, p);
    return BigFloat(mal_remote_small(x / y, p), p);
}

BigFloat operator-(const BigFloat& a) {
    return BigFloat(mal_remote_small(-::mal::BigFloat(a.s, a.p > 0 ? a.p : 256), a.p), a.p);
}

bool operator==(const BigFloat& a, const BigFloat& b) {
    const int p = std::max(a.p, b.p);
    return ::mal::BigFloat(a.s, p) == ::mal::BigFloat(b.s, p);
}

bool operator!=(const BigFloat& a, const BigFloat& b) {
    return !(a == b);
}

bool operator<(const BigFloat& a, const BigFloat& b) {
    const int p = std::max(a.p, b.p);
    return ::mal::BigFloat(a.s, p) < ::mal::BigFloat(b.s, p);
}

bool operator>(const BigFloat& a, const BigFloat& b) {
    return b < a;
}

bool operator<=(const BigFloat& a, const BigFloat& b) {
    return !(b < a);
}

bool operator>=(const BigFloat& a, const BigFloat& b) {
    return !(a < b);
}

BigFloat abs(const BigFloat& a) {
    const int p = a.p > 0 ? a.p : 256;
    return BigFloat(mal_remote_small(::mal::BigFloat(a.s, p).abs(), p), a.p);
}

BigFloat exp(const BigFloat& x) {
    const int p = x.p > 0 ? x.p : 256;
    return BigFloat(mal_remote_small(::mal::BigFloat::exp(::mal::BigFloat(x.s, p)), p), x.p);
}

BigFloat log(const BigFloat& x) {
    const int p = x.p > 0 ? x.p : 256;
    // A long integer only needs its leading digits and its scale:
    // log(d * 10^e) = log(d) + e * log(10). Parsing a million-digit integer and
    // running AGM on it would cost orders of magnitude more than this.
    const std::string& text = x.s;
    size_t start = (!text.empty() && text[0] == '-') ? 1 : 0;
    bool plain_integer = start < text.size();
    for (size_t i = start; plain_integer && i < text.size(); ++i)
        if (text[i] < '0' || text[i] > '9') plain_integer = false;
    if (plain_integer && text.size() - start > 64) {
        size_t first = start;
        while (first < text.size() && text[first] == '0') ++first;
        const size_t digits = text.size() - first;
        if (digits > 64) {
            const size_t keep = 60;
            const long long exponent = (long long)(digits - keep);
            const ::mal::BigFloat lead(text.substr(first, keep), p + 64);
            const ::mal::BigFloat ln10 = ::mal::BigFloat::log(::mal::BigFloat(10, p + 64));
            ::mal::BigFloat value = ::mal::BigFloat::log(lead) + ln10 * ::mal::BigFloat(exponent, p + 64);
            return BigFloat(mal_remote_small(value, p), x.p);
        }
    }
    return BigFloat(mal_remote_small(::mal::BigFloat::log(::mal::BigFloat(x.s, p)), p), x.p);
}

BigFloat sqrt(const BigFloat& x) {
    const int p = x.p > 0 ? x.p : 256;
    return BigFloat(mal_remote_small(::mal::BigFloat::sqrt(::mal::BigFloat(x.s, p)), p), x.p);
}

BigFloat pow(const BigFloat& x, long long e) {
    const int p = x.p > 0 ? x.p : 256;
    return BigFloat(mal_remote_small(::mal::BigFloat::pow(::mal::BigFloat(x.s, p), e), p), x.p);
}

BigFloat pi(int p) {
    p = p > 0 ? p : 256;
    return BigFloat(mal_remote_small(::mal::BigFloat::pi(p), p), p);
}

BigFloat ln2(int p) {
    p = p > 0 ? p : 256;
    return BigFloat(mal_remote_small(::mal::BigFloat::ln2(p), p), p);
}

std::ostream& operator<<(std::ostream& os, const BigFloat& x) {
    return os << x.to_string();
}

std::istream& operator>>(std::istream& is, BigFloat& x) {
    std::string text;
    if (!(is >> text)) return is;
    x = BigFloat(text, x.p > 0 ? x.p : 256);
    return is;
}

} // namespace remote
} // namespace mal
