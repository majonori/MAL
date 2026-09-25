#include "../../include/dgf/ln_exp.hpp"
#include "../../include/dgf/min25.hpp"
#include <cassert>
#include <random>

using namespace mal::dgf;

int main() {
    for (int k = 0; k <= 4; ++k)
        assert(power_sum(998244353LL + 7, k) == power_sum(7, k));
    std::mt19937 rng(9876);
    for (int n : {2, 9, 30, 64, 190, 401}) {
        QuotientGrid grid(n);
        mal::number_theory::Sieve sieve(n);
        int t = std::min(n, std::max(grid.root(), int(std::pow(n, 2.0 / 3.0))));
        for (int trial = 0; trial < 5; ++trial) {
            std::vector<coef> poly = {coef(rng() % 17), coef(rng() % 17), coef(rng() % 17)};
            auto prime_prefix = min25_prime_polynomial(grid, poly);
            auto ln_prime_prefix = prime_polynomial_via_ln_division(grid, poly);
            series q(n + 1), f(n + 1);
            std::vector<coef> bells(n + 1);
            for (int p : sieve.primes) {
                q[p] = bells[p] = polynomial_at(poly, p);
                for (int power = p; power <= n / p; ) {
                    power *= p;
                    bells[power] = rng() % 37;
                }
            }
            coef sum = 0;
            for (int x = 1; x <= n; ++x) {
                if (x >= 2 && sieve.prime(x)) sum += polynomial_at(poly, x);
                if (grid.contains(x)) {
                    assert(prime_prefix.at(x) == sum);
                    assert(ln_prime_prefix.at(x) == sum);
                }
            }
            f[1] = 1;
            for (int x = 2; x <= n; ++x) {
                int p = sieve.spf[x], power = 1, core = x;
                do { core /= p; power *= p; } while (core % p == 0);
                f[x] = f[core] * bells[power];
            }
            BlockPrefix expected(grid, f);
            auto actual = min25_bell(grid, poly, [&](int p, int e) {
                int power = 1;
                while (e--) power *= p;
                return bells[power];
            }, t);
            for (int x : grid.values()) assert(actual.at(x) == expected.at(x));
            auto independent = bell_via_ln_exp(grid, poly, [&](int p, int e) {
                int power = 1;
                while (e--) power *= p;
                return bells[power];
            });
            for (int x : grid.values()) assert(independent.at(x) == expected.at(x));
            for (int cut : {0, std::min(3, grid.root()), grid.root()}) {
                auto hybrid = bell_via_prime_prefix_hybrid(grid, poly,
                    [&](int p, int e) {
                        int power = 1;
                        while (e--) power *= p;
                        return bells[power];
                    }, cut);
                for (int x : grid.values()) assert(hybrid.at(x) == expected.at(x));
            }
        }
    }
    {
        QuotientGrid grid(100);
        std::vector<coef> polynomial(1, coef(0));
        auto hybrid = bell_via_prime_prefix_hybrid(grid, polynomial,
            [](int, int e) { return e == 2 ? coef(1) : coef(0); }, 10);
        series f(101); f[1] = 1;
        for (int x = 2; x <= 100; ++x) {
            int core = x, p = 0, e = 0;
            for (int candidate = 2; candidate <= core; ++candidate) {
                if (core % candidate == 0) {
                    p = candidate;
                    do { core /= p; ++e; } while (core % p == 0);
                    break;
                }
            }
            f[x] = (e == 2 ? f[core] : coef(0));
        }
        BlockPrefix expected(grid, f);
        for (auto x : grid.values()) assert(hybrid.at(x) == expected.at(x));
    }
}
