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

} // namespace

namespace mal {
namespace remote {

std::string BigInt::to_string() const {
    return ::mal::BigInt(s).to_string();
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

std::string BigFloat::to_string(int digits) const {
    if (digits < 0) digits = mal_remote_digits(p);
    return ::mal::BigFloat(s, p).to_string(digits);
}

BigFloat operator+(const BigFloat& a, const BigFloat& b) {
    const int p = std::max(a.p, b.p);
    const ::mal::BigFloat x(a.s, p), y(b.s, p);
    return BigFloat((x + y).to_string(mal_remote_digits(p)), p);
}

BigFloat operator-(const BigFloat& a, const BigFloat& b) {
    const int p = std::max(a.p, b.p);
    const ::mal::BigFloat x(a.s, p), y(b.s, p);
    return BigFloat((x - y).to_string(mal_remote_digits(p)), p);
}

BigFloat operator*(const BigFloat& a, const BigFloat& b) {
    const int p = std::max(a.p, b.p);
    const ::mal::BigFloat x(a.s, p), y(b.s, p);
    return BigFloat((x * y).to_string(mal_remote_digits(p)), p);
}

BigFloat operator/(const BigFloat& a, const BigFloat& b) {
    const int p = std::max(a.p, b.p);
    const ::mal::BigFloat x(a.s, p), y(b.s, p);
    return BigFloat((x / y).to_string(mal_remote_digits(p)), p);
}

std::ostream& operator<<(std::ostream& os, const BigInt& x) {
    return os << x.to_string();
}

std::ostream& operator<<(std::ostream& os, const BigFloat& x) {
    return os << x.to_string();
}

} // namespace remote
} // namespace mal
