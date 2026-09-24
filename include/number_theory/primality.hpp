#pragma once
#include <cstdint>
#include <initializer_list>

namespace mal { namespace number_theory {

using u64 = std::uint64_t;

inline u64 mul_mod(u64 a, u64 b, u64 mod) {
    return u64((unsigned __int128)a * b % mod);
}
inline u64 pow_mod(u64 a, u64 k, u64 mod) {
    if (!mod) return 0;
    u64 result = 1 % mod;
    for (a %= mod; k; k >>= 1, a = mul_mod(a, a, mod))
        if (k & 1) result = mul_mod(result, a, mod);
    return result;
}

// Deterministic Miller-Rabin for every unsigned 64-bit integer.
inline bool is_prime(u64 n) {
    if (n < 2) return false;
    for (u64 p : {2ULL, 3ULL, 5ULL, 7ULL, 11ULL, 13ULL, 17ULL, 19ULL, 23ULL, 29ULL, 31ULL, 37ULL}) {
        if (n % p == 0) return n == p;
    }
    u64 d = n - 1; int s = 0;
    while (!(d & 1)) d >>= 1, ++s;
    for (u64 a : {2ULL, 325ULL, 9375ULL, 28178ULL, 450775ULL, 9780504ULL, 1795265022ULL}) {
        if (a % n == 0) continue;
        u64 x = pow_mod(a, d, n);
        if (x == 1 || x == n - 1) continue;
        bool witness = true;
        for (int r = 1; r < s; ++r) {
            x = mul_mod(x, x, n);
            if (x == n - 1) { witness = false; break; }
        }
        if (witness) return false;
    }
    return true;
}

namespace detail {
__attribute__((used)) auto mul_mod_kept = &mul_mod;
__attribute__((used)) auto pow_mod_kept = &pow_mod;
__attribute__((used)) auto is_prime_kept = &is_prime;
} // namespace detail

} } // namespace mal::number_theory
