#include "../../include/dgf/pn_block.hpp"
#include <cassert>
#include <random>

using namespace mal::dgf;

int main() {
    std::mt19937 rng(70633);
    for (int n : {1, 2, 7, 39, 100, 313, 1001}) {
        QuotientGrid grid(n);
        for (int degree = 0; degree <= 3; ++degree) {
            BlockPrefix identity = id_k_block(grid, degree);
            coef prefix = 0;
            for (int x = 1; x <= n; ++x) {
                prefix += coef(x).pow(degree);
                if (grid.contains(x)) assert(identity.at(x) == prefix);
            }
        }
        series points(n + 1); points[1] = 1;
        for (int x = 2; x <= n; ++x) points[x] = rng() % 31;
        BlockPrefix general(grid, points);
        BlockPrefix logarithm = block_log(general);
        series dense_log = ln(points);
        BlockPrefix expected_log(grid, dense_log);
        for (int x : grid.values())
            assert(logarithm.at(x) == expected_log.at(x));
        BlockPrefix recovered = block_exp(logarithm);
        for (int x : grid.values())
            assert(recovered.at(x) == general.at(x));

        series pp(n + 1), prime_points(n + 1);
        mal::number_theory::Sieve sieve(n);
        for (int p : sieve.primes) {
            prime_points[p] = coef(p + 5);
            for (int e = 1, power = p; ; ++e) {
                pp[power] = coef(p + 5 * e);
                if (power > n / p) break;
                power *= p;
            }
        }
        BlockPrefix prime_prefix(grid, prime_points);
        BlockPrefix got = prime_power_block(prime_prefix, [](int p, int e) {
            return coef(p + 5 * e);
        });
        BlockPrefix expected_pp(grid, pp);
        for (int x : grid.values())
            assert(got.at(x) == expected_pp.at(x));

        series pn(n + 1); pn[1] = 1;
        for (int x = 2; x <= n; ++x) {
            int p = sieve.spf[x], core = x, e = 0;
            do { core /= p; ++e; } while (core % p == 0);
            pn[x] = e == 1 ? coef(0) : pn[core] * coef(p + 5 * e);
        }
        BlockPrefix got_pn = pn_block(grid, [](int p, int e) {
            return coef(p + 5 * e);
        });
        BlockPrefix expected_pn(grid, pn);
        for (int x : grid.values())
            assert(got_pn.at(x) == expected_pn.at(x));
        series sparse(n + 1); sparse[1] = 1;
        for (int x = 2; x <= n; ++x) {
            int p = sieve.spf[x], core = x, e = 0;
            do { core /= p; ++e; } while (core % p == 0);
            sparse[x] = e <= 2 ? coef(0) : sparse[core] * coef(p + e);
        }
        BlockPrefix sparse_pn = pn_block(grid, [](int p, int e) {
            return e == 2 ? coef(0) : coef(p + e);
        });
        BlockPrefix expected_sparse(grid, sparse);
        for (int x : grid.values())
            assert(sparse_pn.at(x) == expected_sparse.at(x));
    }
}
