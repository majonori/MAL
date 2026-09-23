#pragma once
#include <cstddef>
#include <istream>
#include <ostream>
#include <string>

namespace mal {
namespace remote {

// 高精度整数：内部存十进制串，算法在交互库里。
struct BigInt {
    std::string s;
    BigInt(const std::string& x = "0") : s(x) {}
    BigInt(long long x) : s(std::to_string(x)) {}
    std::string to_string() const;
    std::string to_string(int base) const;
    bool is_zero() const;
    int sign() const;
    unsigned long long bit_length() const;
};

BigInt operator+(const BigInt& a, const BigInt& b);
BigInt operator-(const BigInt& a, const BigInt& b);
BigInt operator*(const BigInt& a, const BigInt& b);
BigInt operator/(const BigInt& a, const BigInt& b);
BigInt operator%(const BigInt& a, const BigInt& b);
BigInt operator-(const BigInt& a);
BigInt operator<<(const BigInt& a, std::size_t bits);
BigInt operator>>(const BigInt& a, std::size_t bits);
bool operator==(const BigInt& a, const BigInt& b);
bool operator!=(const BigInt& a, const BigInt& b);
bool operator<(const BigInt& a, const BigInt& b);
bool operator>(const BigInt& a, const BigInt& b);
bool operator<=(const BigInt& a, const BigInt& b);
bool operator>=(const BigInt& a, const BigInt& b);
BigInt abs(const BigInt& a);
BigInt pow(const BigInt& a, unsigned long long e);
BigInt sqrt(const BigInt& a);
BigInt nroot(const BigInt& a, unsigned long long k);
std::ostream& operator<<(std::ostream& os, const BigInt& x);
std::istream& operator>>(std::istream& is, BigInt& x);

// 高精度小数：内部存十进制串 + 二进制有效位数。
struct BigFloat {
    std::string s;
    int p;
    BigFloat(const std::string& x = "0", int p_ = 256) : s(x), p(p_) {}
    BigFloat(long long x, int p_ = 256) : s(std::to_string(x)), p(p_) {}
    std::string to_string() const;
    std::string to_string(int digits) const;
    bool is_zero() const;
    int sign() const;
    int precision() const;
};

BigFloat operator+(const BigFloat& a, const BigFloat& b);
BigFloat operator-(const BigFloat& a, const BigFloat& b);
BigFloat operator*(const BigFloat& a, const BigFloat& b);
BigFloat operator/(const BigFloat& a, const BigFloat& b);
BigFloat operator-(const BigFloat& a);
bool operator==(const BigFloat& a, const BigFloat& b);
bool operator!=(const BigFloat& a, const BigFloat& b);
bool operator<(const BigFloat& a, const BigFloat& b);
bool operator>(const BigFloat& a, const BigFloat& b);
bool operator<=(const BigFloat& a, const BigFloat& b);
bool operator>=(const BigFloat& a, const BigFloat& b);
BigFloat abs(const BigFloat& a);
BigFloat exp(const BigFloat& x);
BigFloat log(const BigFloat& x);
BigFloat sqrt(const BigFloat& x);
BigFloat pow(const BigFloat& x, long long e);
BigFloat pi(int p);
BigFloat ln2(int p);
std::ostream& operator<<(std::ostream& os, const BigFloat& x);
std::istream& operator>>(std::istream& is, BigFloat& x);

} // namespace remote
} // namespace mal
