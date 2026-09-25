#include "../../include/dgf/euler_product.hpp"
#include <cassert>
#include <random>

using namespace mal::dgf;

int main() {
    std::mt19937 rng(7781);
    for (int n = 2; n <= 400; ++n) {
        QuotientGrid grid(n);
        int root = grid.root();
        for (int trial = 0; trial < 20; ++trial) {
            series tail(n + 1); tail[1] = 1;
            for (int x = root + 1; x <= n; ++x)
                if (rng() % 20 == 0) tail[x] = coef(rng() % 13);
            std::vector<EulerFactor> factors;
            for (int t = 2; t <= root; ++t) {
                if (rng() % 3 != 0) continue;
                EulerFactor f;
                f.terms.emplace_back(t, coef(1 + rng() % 13));
                for (int x = t + 1; x <= n; ++x)
                    if (rng() % (n + 5) == 0)
                        f.terms.emplace_back(x, coef(1 + rng() % 13));
                factors.push_back(std::move(f));
            }
            auto result = reverse_euler_product(BlockPrefix(grid, tail), factors,
                                                root + 1);
            for (auto it = factors.rbegin(); it != factors.rend(); ++it) {
                series next = tail;
                for (auto term : it->terms)
                    for (int x = 1; x <= n / term.first; ++x)
                        next[x * term.first] += term.second * tail[x];
                tail.swap(next);
            }
            BlockPrefix expected(grid, tail);
            for (int x : grid.values()) assert(result.at(x) == expected.at(x));
        }
    }
    // Two-stage prime-prefix + reverse Euler product: f=mu.
    for (int n : {32, 127, 4096}) {
        QuotientGrid grid(n);
        mal::number_theory::Sieve sieve(n);
        series tail(n + 1); tail[1] = 1;
        std::vector<EulerFactor> factors;
        for (int p : sieve.primes) {
            if (p <= grid.root()) factors.push_back({{{p, coef(0) - coef(1)}}});
            else tail[p] = coef(0) - coef(1);
        }
        auto actual = reverse_euler_product(BlockPrefix(grid, tail), factors,
                                            grid.root() + 1);
        series mu(n + 1); mu[1] = 1;
        for (int x = 2; x <= n; ++x) {
            int p = sieve.spf[x], y = x / p;
            mu[x] = y % p == 0 ? coef(0) : coef(0) - mu[y];
        }
        BlockPrefix expected(grid, mu);
        for (int x : grid.values()) assert(actual.at(x) == expected.at(x));
    }
    {
        QuotientGrid grid(100);
        series unit(101); unit[1] = 1;
        std::vector<EulerFactor> factors = {
            {{{2, coef(3)}, {100, coef(5)}}}, {{{3, coef(7)}}}
        };
        auto actual = reverse_euler_product(BlockPrefix(grid, unit), factors, 11);
        assert(actual.at(100) == coef(37)); // 1 + 3 + 5 + 7 + 3*7
    }
}
