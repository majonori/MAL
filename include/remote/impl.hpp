#pragma once
#include "interface.hpp"
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

} // namespace

namespace mal {
namespace remote {

std::string BigInt::to_string() const {
    return ::mal::BigInt(s).to_string();
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
    return BigInt((::mal::BigInt(a.s) + ::mal::BigInt(b.s)).to_string());
}

BigInt operator-(const BigInt& a, const BigInt& b) {
    return BigInt((::mal::BigInt(a.s) - ::mal::BigInt(b.s)).to_string());
}

BigInt operator*(const BigInt& a, const BigInt& b) {
    return BigInt((::mal::BigInt(a.s) * ::mal::BigInt(b.s)).to_string());
}

BigInt operator/(const BigInt& a, const BigInt& b) {
    return BigInt((::mal::BigInt(a.s) / ::mal::BigInt(b.s)).to_string());
}

BigInt operator%(const BigInt& a, const BigInt& b) {
    return BigInt((::mal::BigInt(a.s) % ::mal::BigInt(b.s)).to_string());
}

BigInt operator-(const BigInt& a) {
    return BigInt((-::mal::BigInt(a.s)).to_string());
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
    return BigInt(::mal::BigInt(a.s).nroot(k).to_string());
}

std::ostream& operator<<(std::ostream& os, const BigInt& x) {
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
