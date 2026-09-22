#include "../../include/hp/bigint.hpp"
#include <boost/multiprecision/cpp_int.hpp>
#include <cstdint>
#include <iostream>
#include <random>
#include <string>
#include <vector>

using boost::multiprecision::cpp_int;
using mal::BigInt;

static BigInt random_bigint(std::mt19937_64& rng, size_t limbs, bool allow_negative) {
    std::vector<std::uint32_t> v(limbs);
    for (std::uint32_t& x : v) x = std::uint32_t(rng());
    while (!v.empty() && v.back() == 0) v.pop_back();
    if (v.empty()) v.push_back(1);
    return BigInt::from_limbs(v, allow_negative && (rng() & 1));
}

static cpp_int to_boost(const BigInt& x) {
    cpp_int r = 0;
    const auto& v = x.limbs();
    for (size_t i = v.size(); i--;) {
        r <<= 32;
        r += v[i];
    }
    return x.is_negative() ? -r : r;
}

static bool check(const BigInt& got, const cpp_int& want, const char* what) {
    if (to_boost(got) == want) return true;
    std::cerr << "FAIL " << what << "\n";
    return false;
}

int main() {
    std::mt19937_64 rng(123456789);
    const size_t sizes[] = {1, 2, 7, 20, 40, 120, 240, 600, 1400, 3000};
    bool ok = true;
    for (size_t sz : sizes) {
        BigInt a = random_bigint(rng, sz, true);
        BigInt b = random_bigint(rng, sz, true);
        cpp_int x = to_boost(a), y = to_boost(b);
        if (!check(a + b, x + y, "add")) ok = false;
        if (!check(a - b, x - y, "sub")) ok = false;
        if (!check(a * b, x * y, "mul")) ok = false;
        if (!check(a / b, x / y, "div")) ok = false;
        if (!check(a % b, x % y, "mod")) ok = false;
    }

    for (int t = 0; t < 200; ++t) {
        size_t na = 1 + rng() % 80, nb = 1 + rng() % 50;
        BigInt a = random_bigint(rng, na, true);
        BigInt b = random_bigint(rng, nb, true);
        cpp_int x = to_boost(a), y = to_boost(b);
        if (!check(a * b, x * y, "small mul")) ok = false;
        if (!check(a / b, x / y, "small div")) ok = false;
        if (!check(a % b, x % y, "small mod")) ok = false;
    }

    // Exercise all multiplication backends, including the NTT switch.
    const size_t mul_sizes[] = {20, 64, 256, 768, 2048};
    for (size_t sz : mul_sizes) {
        BigInt a = random_bigint(rng, sz, true);
        BigInt b = random_bigint(rng, sz, true);
        if (!check(a * b, to_boost(a) * to_boost(b), "backend mul")) ok = false;
    }

    BigInt z("0");
    if (z.to_string() != "0") ok = false;
    if (BigInt("-123456789012345678901234567890").to_string() !=
        "-123456789012345678901234567890") ok = false;

    // Fast Newton division path with a wide quotient.
    {
        BigInt a = random_bigint(rng, 20000, false);
        BigInt b = random_bigint(rng, 200, false);
        BigInt q, r;
        BigInt::divmod(a, b, q, r);
        if (q * b + r != a || r >= b || r.is_negative()) ok = false;
    }

    // Divide-and-conquer decimal conversion.
    {
        std::string s(20000, '0');
        s[0] = '1';
        for (size_t i = 1; i < s.size(); ++i) s[i] = char('0' + (i * 7 + 3) % 10);
        BigInt x(s);
        if (x.to_string() != s) ok = false;
    }

    // Integer square root and k-th root.
    for (int t = 0; t < 80; ++t) {
        const unsigned long long k = 1 + rng() % 50;
        BigInt n = random_bigint(rng, 1 + rng() % 400, false);
        BigInt r = n.nroot(k);
        if (r.pow(k) > n || (r + BigInt(1)).pow(k) <= n) ok = false;
        if (k == 2) {
            BigInt s = n.sqrt();
            if (s != r) ok = false;
        }
    }

    std::cout << (ok ? "bigint: ok" : "bigint: FAILED") << '\n';
    return ok ? 0 : 1;
}
