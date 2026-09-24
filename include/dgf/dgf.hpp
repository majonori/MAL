#pragma once
#include "multiplicative.hpp"
#include <utility>

namespace mal { namespace dgf {

// The only public construction routes for a structured DGF validate its
// promise. No mutable reference to coefficients escapes the object.
class DGF {
public:
    enum class Kind { general, prime_power, multiplicative, completely_multiplicative };
    static DGF general(series f);
    static DGF prime_power(series f);
    static DGF multiplicative(series f);
    static DGF completely_multiplicative(series f);
    static DGF epsilon(int n);
    static DGF zeta(int n);
    static DGF identity(int n);
    static DGF phi(int n);
    static DGF mu(int n);

    const series& coefficients() const;
    Kind kind() const;
    int limit() const;

    DGF multiply(const DGF& rhs) const;
    DGF divide(const DGF& rhs) const;
    DGF inverse() const;
    DGF log() const;
    DGF exponential() const;
    DGF power(coef k) const;
    DGF integer_power(long long k) const;
    DGF derivative() const;
    DGF integral(coef constant = 0) const;
    DGF add(const DGF& rhs) const;
    DGF subtract(const DGF& rhs) const;
    DGF scale(coef k) const;
    DGF divisor_prefix() const;
    DGF divisor_difference() const;

private:
    series a_;
    Kind kind_;
    DGF(series f, Kind kind);
    bool is_mult() const;
};

inline DGF::DGF(series f, Kind k) : a_(std::move(f)), kind_(k) {}
inline bool DGF::is_mult() const {
    return kind_ == Kind::multiplicative || kind_ == Kind::completely_multiplicative;
}
inline DGF DGF::general(series f) {
    check(f); return DGF(std::move(f), Kind::general);
}
inline DGF DGF::prime_power(series f) {
    if (!is_prime_power_supported(f)) throw std::invalid_argument("dgf: not prime-power supported");
    return DGF(std::move(f), Kind::prime_power);
}
inline DGF DGF::multiplicative(series f) {
    if (!is_multiplicative(f)) throw std::invalid_argument("dgf: not multiplicative");
    return DGF(std::move(f), Kind::multiplicative);
}
inline DGF DGF::completely_multiplicative(series f) {
    if (!is_completely_multiplicative(f))
        throw std::invalid_argument("dgf: not completely multiplicative");
    return DGF(std::move(f), Kind::completely_multiplicative);
}
inline DGF DGF::epsilon(int n) {
    if (n < 1) throw std::invalid_argument("dgf: N must be positive");
    series f(n + 1); f[1] = 1;
    return DGF(std::move(f), Kind::completely_multiplicative);
}
inline DGF DGF::zeta(int n) {
    if (n < 1) throw std::invalid_argument("dgf: N must be positive");
    series f(n + 1, 1); f[0] = 0;
    return DGF(std::move(f), Kind::completely_multiplicative);
}
inline DGF DGF::identity(int n) {
    if (n < 1) throw std::invalid_argument("dgf: N must be positive");
    series f(n + 1);
    for (int i = 1; i <= n; ++i) f[i] = i;
    return DGF(std::move(f), Kind::completely_multiplicative);
}
inline DGF DGF::phi(int n) {
    if (n < 1) throw std::invalid_argument("dgf: N must be positive");
    auto p = number_theory::totients(number_theory::Sieve(n));
    series f(n + 1);
    for (int i = 1; i <= n; ++i) f[i] = p[i];
    return DGF(std::move(f), Kind::multiplicative);
}
inline DGF DGF::mu(int n) {
    if (n < 1) throw std::invalid_argument("dgf: N must be positive");
    auto p = number_theory::mobius(number_theory::Sieve(n));
    series f(n + 1);
    for (int i = 1; i <= n; ++i) f[i] = p[i];
    return DGF(std::move(f), Kind::multiplicative);
}
inline const series& DGF::coefficients() const { return a_; }
inline DGF::Kind DGF::kind() const { return kind_; }
inline int DGF::limit() const { return int(a_.size()) - 1; }

inline DGF DGF::multiply(const DGF& rhs) const {
    pair_check(a_, rhs.a_);
    if (is_mult() && rhs.is_mult())
        return DGF(mul_multiplicative_both(a_, rhs.a_), Kind::multiplicative);
    if (is_mult()) return DGF(mul_multiplicative(rhs.a_, a_), Kind::general);
    if (rhs.is_mult()) return DGF(mul_multiplicative(a_, rhs.a_), Kind::general);
    // Ranked convolution is available separately for large-scale research,
    // but measurements show the direct kernel wins at contest sizes.
    return DGF(mul(a_, rhs.a_), Kind::general);
}
inline DGF DGF::divide(const DGF& rhs) const {
    pair_check(a_, rhs.a_);
    if (is_mult() && rhs.is_mult())
        return DGF(div_multiplicative_both(a_, rhs.a_), Kind::multiplicative);
    if (rhs.is_mult())
        return DGF(mul_multiplicative(a_, inv_multiplicative(rhs.a_)), Kind::general);
    return DGF(div(a_, rhs.a_), Kind::general);
}
inline DGF DGF::inverse() const {
    if (is_mult()) return DGF(inv_multiplicative(a_), Kind::multiplicative);
    return DGF(inv(a_), Kind::general);
}
inline DGF DGF::log() const {
    if (is_mult()) return DGF(ln_multiplicative(a_), Kind::prime_power);
    return DGF(ln(a_), Kind::general);
}
inline DGF DGF::exponential() const {
    if (kind_ == Kind::prime_power)
        return DGF(exp_prime_powers(a_), Kind::multiplicative);
    return DGF(exp(a_), Kind::general);
}
inline DGF DGF::power(coef k) const {
    if (is_mult()) {
        series h = ln_multiplicative(a_);
        for (std::size_t i = 2; i < h.size(); ++i) h[i] *= k;
        return DGF(exp_prime_powers(h), Kind::multiplicative);
    }
    return DGF(pow_unit(a_, k), Kind::general);
}
inline DGF DGF::integer_power(long long k) const {
    if (is_mult()) return power(coef(k));
    return DGF(pow_int(a_, k), Kind::general);
}
inline DGF DGF::derivative() const {
    return DGF(dgf::derivative(a_), kind_ == Kind::prime_power ? Kind::prime_power : Kind::general);
}
inline DGF DGF::integral(coef c) const {
    return DGF(dgf::integral(a_, c),
               kind_ == Kind::prime_power && c == coef(0) ? Kind::prime_power : Kind::general);
}
inline DGF DGF::add(const DGF& rhs) const {
    pair_check(a_, rhs.a_);
    series h = a_;
    for (std::size_t i = 1; i < h.size(); ++i) h[i] += rhs.a_[i];
    Kind k = kind_ == Kind::prime_power && rhs.kind_ == Kind::prime_power
           ? Kind::prime_power : Kind::general;
    return DGF(std::move(h), k);
}
inline DGF DGF::subtract(const DGF& rhs) const {
    pair_check(a_, rhs.a_);
    series h = a_;
    for (std::size_t i = 1; i < h.size(); ++i) h[i] -= rhs.a_[i];
    Kind k = kind_ == Kind::prime_power && rhs.kind_ == Kind::prime_power
           ? Kind::prime_power : Kind::general;
    return DGF(std::move(h), k);
}
inline DGF DGF::scale(coef k) const {
    series h = a_;
    for (std::size_t i = 1; i < h.size(); ++i) h[i] *= k;
    Kind shape = k == coef(0) ? Kind::prime_power :
                 k == coef(1) ? kind_ :
                 kind_ == Kind::prime_power ? Kind::prime_power : Kind::general;
    return DGF(std::move(h), shape);
}
inline DGF DGF::divisor_prefix() const {
    if (is_mult()) return multiply(DGF::zeta(limit()));
    series h = a_; divisor_zeta(h);
    return DGF(std::move(h), Kind::general);
}
inline DGF DGF::divisor_difference() const {
    series h = a_; divisor_mobius(h);
    return DGF(std::move(h), is_mult() ? Kind::multiplicative : Kind::general);
}

namespace detail {
#define MAL_DGF_KEEP(name) __attribute__((used)) auto dgf_##name##_kept = &DGF::name
MAL_DGF_KEEP(general);
MAL_DGF_KEEP(prime_power);
MAL_DGF_KEEP(multiplicative);
MAL_DGF_KEEP(completely_multiplicative);
MAL_DGF_KEEP(epsilon);
MAL_DGF_KEEP(zeta);
MAL_DGF_KEEP(identity);
MAL_DGF_KEEP(phi);
MAL_DGF_KEEP(mu);
MAL_DGF_KEEP(coefficients);
MAL_DGF_KEEP(kind);
MAL_DGF_KEEP(limit);
MAL_DGF_KEEP(multiply);
MAL_DGF_KEEP(divide);
MAL_DGF_KEEP(inverse);
MAL_DGF_KEEP(log);
MAL_DGF_KEEP(exponential);
MAL_DGF_KEEP(power);
MAL_DGF_KEEP(integer_power);
MAL_DGF_KEEP(derivative);
MAL_DGF_KEEP(integral);
MAL_DGF_KEEP(add);
MAL_DGF_KEEP(subtract);
MAL_DGF_KEEP(scale);
MAL_DGF_KEEP(divisor_prefix);
MAL_DGF_KEEP(divisor_difference);
#undef MAL_DGF_KEEP
} // namespace detail

} } // namespace mal::dgf
