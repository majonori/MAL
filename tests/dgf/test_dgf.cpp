#include "../../include/dgf/dgf.hpp"
#include <cassert>
#include <iostream>
#include <random>

using namespace mal::dgf;

int main() {
    std::mt19937 rng(1234567);
    for (int n : {1, 2, 16, 47, 257, 512}) {
        for (int rep = 0; rep < 3; ++rep) {
            series a(n + 1), b(n + 1);
            for (int i = 1; i <= n; ++i) a[i] = rng() % 19, b[i] = rng() % 19;
            if (fast_mul(a, b) != mul(a, b)) {
                auto x = fast_mul(a, b), y = mul(a, b);
                for (int i = 1; i <= n; ++i) if (x[i] != y[i]) {
                    std::cerr << "fast_mul n=" << n << " rep=" << rep
                              << " i=" << i << " got=" << x[i].v
                              << " expected=" << y[i].v << '\n';
                    break;
                }
                return 1;
            }

            auto c = a;
            divisor_zeta(c);
            for (int i = 1; i <= n; ++i) {
                coef expected = 0;
                for (int d = 1; d <= i; ++d) if (i % d == 0) expected += a[d];
                assert(c[i] == expected);
            }
            divisor_mobius(c);
            assert(c == a);
            c = a;
            multiple_zeta(c);
            for (int i = 1; i <= n; ++i) {
                coef expected = 0;
                for (int j = i; j <= n; j += i) expected += a[j];
                assert(c[i] == expected);
            }
            multiple_mobius(c);
            assert(c == a);

            series lc(n + 1), gc(n + 1);
            for (int i = 1; i <= n; ++i)
                for (int j = 1; j <= n; ++j) {
                    int g = gcd_int(i, j);
                    gc[g] += a[i] * b[j];
                    int l = i / g * j;
                    if (l <= n) lc[l] += a[i] * b[j];
                }
            assert(gcd_convolution(a, b) == gc);
            assert(lcm_convolution(a, b) == lc);

            a[1] = 1;
            b[1] = 1;
            auto ab = mul(a, b), ain = inv(a);
            series one(n + 1); one[1] = 1;
            assert(mul(a, ain) == one);
            assert(div(ab, a) == b);
            assert(exp(ln(a)) == a);
            assert(mul(pow_unit(a, coef(3)), one) == pow_int(a, 3));
            assert(pow_int(a, -1) == ain);
            auto diff = derivative(a);
            assert(integral(diff, a[1]) == a);

            a[1] = 0;
            assert(ln(exp(a)) == a);
            assert(pow_int(a, 2) == mul(a, a));
            auto comp = compose(a, {2, 3, 4});
            auto square = mul(a, a);
            for (int i = 1; i <= n; ++i)
                assert(comp[i] == coef(i == 1 ? 2 : 0) + coef(3) * a[i] + coef(4) * square[i]);
            auto geo = geometric_sum(a, 3);
            auto cube = mul(square, a);
            for (int i = 1; i <= n; ++i)
                assert(geo[i] == one[i] + a[i] + square[i] + cube[i]);

            // The prime-power Bell series of 1 is all ones, giving divisor zeta.
            series ones(n + 1, 1); ones[0] = 0;
            assert(mul_multiplicative(b, ones) == mul(b, ones));
            auto z2 = mul(ones, ones), z3 = mul(z2, ones);
            auto sum = geometric_sum(ones, 3);
            for (int i = 1; i <= n; ++i)
                assert(sum[i] == one[i] + ones[i] + z2[i] + z3[i]);
            series identity(n + 1);
            for (int i = 1; i <= n; ++i) identity[i] = i;
            assert(mul_multiplicative(b, identity) == mul(b, identity));
        }
    }
    {
        const int n = 4096;
        series a(n + 1), b(n + 1);
        for (int i = 1; i <= n; ++i) a[i] = rng() % 7, b[i] = rng() % 7;
        assert(fast_mul(a, b) == mul(a, b));
    }
    {
        const int n = 4096;
        PrimePowers t(n);
        series f(n + 1), g(n + 1), h(n + 1), arbitrary(n + 1);
        f[1] = g[1] = 1;
        for (int i = 2; i <= n; ++i) {
            if (t.core[i] == 1) {
                f[i] = rng() % 29; g[i] = rng() % 29;
                h[i] = rng() % 29;
            } else {
                f[i] = f[t.core[i]] * f[t.prime_power[i]];
                g[i] = g[t.core[i]] * g[t.prime_power[i]];
            }
            arbitrary[i] = rng() % 13;
        }
        arbitrary[1] = 7;
        auto F = DGF::multiplicative(f), G = DGF::multiplicative(g);
        auto H = DGF::prime_power(h), A = DGF::general(arbitrary);
        assert(F.multiply(G).coefficients() == mul(f, g));
        assert(F.multiply(G).kind() == DGF::Kind::multiplicative);
        assert(mul_multiplicative_both(f, g) == mul(f, g));
        assert(div_multiplicative_both(mul(f, g), f) == g);
        assert(F.divide(G).coefficients() == div(f, g));
        assert(F.inverse().coefficients() == inv(f));
        assert(F.log().coefficients() == ln(f));
        assert(F.log().kind() == DGF::Kind::prime_power);
        assert(F.log().exponential().coefficients() == f);
        assert(F.power(coef(3)).coefficients() == pow_unit(f, 3));
        assert(H.exponential().coefficients() == exp(h));
        assert(H.exponential().kind() == DGF::Kind::multiplicative);
        assert(H.add(H).kind() == DGF::Kind::prime_power);
        assert(A.multiply(F).coefficients() == mul(arbitrary, f));
        assert(A.divide(F).coefficients() == div(arbitrary, f));
        assert(F.divisor_prefix().coefficients() == DGF::general(f).divisor_prefix().coefficients());
        assert(F.divisor_difference().divisor_prefix().coefficients() == f);
        bool rejected = false;
        f[6] += 1;
        try { DGF::multiplicative(f); }
        catch (const std::invalid_argument&) { rejected = true; }
        assert(rejected);
        assert(DGF::identity(n).kind() == DGF::Kind::completely_multiplicative);
        assert(DGF::phi(n).multiply(DGF::zeta(n)).coefficients() == DGF::identity(n).coefficients());
        assert(DGF::mu(n).multiply(DGF::zeta(n)).coefficients() == DGF::epsilon(n).coefficients());
    }
    // Prime p>sqrt(INT_MAX) used to overflow q*=p after processing q=p.
    {
        const int n = 100000;
        auto z = DGF::zeta(n);
        assert(z.log().exponential().coefficients() == z.coefficients());
    }
    // Division by a nonunit uses coefficients at d*n; verification also
    // detects incompatible coefficients at indices not divisible by d.
    const int n = 30, d = 2;
    series h(d * n + 1), g(d * n + 1);
    g[d] = 7;
    for (int i = 1; i <= d * n; ++i) h[i] = rng() % 11;
    for (int i = d + 1; i <= d * n; ++i) g[i] = rng() % 13;
    auto f = mul(h, g);
    auto q = generalized_div(f, g, n, true);
    for (int i = 1; i <= n; ++i) assert(q[i] == h[i]);
    f[3] += 1;
    bool rejected = false;
    try { generalized_div(f, g, n, true); }
    catch (const std::invalid_argument&) { rejected = true; }
    assert(rejected);

    std::cout << "DGF OK\n";
}
