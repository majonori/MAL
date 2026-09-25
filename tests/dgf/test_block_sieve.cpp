#include "../../include/dgf/dynamic_bell.hpp"
#include "../../include/dgf/zak_polynomial.hpp"
#include <cassert>
#include <random>

using namespace mal::dgf;

int main() {
    std::mt19937 rng(12345);
    for (int n : {1, 7, 101, 10000}) {
        QuotientGrid grid(n);
        BlockSums blocks(grid);
        std::vector<coef> values(grid.values().size());
        for (std::size_t i = 0; i < values.size(); ++i) {
            values[i] = rng() % 97;
            blocks.add_static(i, values[i]);
        }
        for (int t = 0; t < 100; ++t) {
            std::size_t k = rng() % values.size();
            coef delta = rng() % 31;
            values[k] += delta;
            if (t % 2) blocks.add_block(k, delta);
            else blocks.add_fast(k, delta);
            coef expected = 0;
            for (std::size_t i = 0; i <= k; ++i) expected += values[i];
            assert(blocks.prefix_fast(grid.values()[k]) == expected);
            assert(blocks.prefix_at(grid.values()[k]) == expected);
        }
    }
    {
        QuotientGrid large(10000000000LL);
        BlockPrefix identity(large);
        for (std::size_t i = 0; i < identity.size(); ++i) {
            std::int64_t x = large.values()[i];
            assert(large.index(x) == int(i));
            identity[i] = coef(x % 998244353);
        }
        for (std::int64_t x : {1LL, 100000LL, 5000000000LL, 10000000000LL})
            if (large.contains(x)) assert(identity.at(x) == coef(x % 998244353));
    }
    for (int n : {1, 2, 9, 37, 100, 173, 400}) {
        QuotientGrid grid(n);
        assert(grid.values().front() == 1 && grid.values().back() == n);
        for (int x : grid.values()) {
            assert(grid.values()[grid.index(x)] == x);
            for (int k = 1; k <= x; ++k) assert(grid.contains(x / k));
        }
        int t = std::min(n, std::max(grid.root(), int(std::pow(n, 2.0 / 3.0))));
        for (int trial = 0; trial < 6; ++trial) {
            series a(n + 1), b(n + 1);
            for (int i = 1; i <= n; ++i) {
                a[i] = rng() % 97;
                b[i] = rng() % 97;
            }
            b[1] = 1 + rng() % 9;
            BlockPrefix aa(grid, a), bb(grid, b);
            series h = mul(a, b);
            series dense_a(a.begin(), a.begin() + t + 1);
            series dense_b(b.begin(), b.begin() + t + 1);
            series dense_h(h.begin(), h.begin() + t + 1);
            BlockPrefix expected(grid, h);
            auto actual = block_convolve(aa, bb, dense_h);
            auto hyperbola = block_convolve(aa, bb);
            auto generic = block_convolve(aa, bb, dense_a, dense_b);
            auto ranked = block_convolve_ranked(aa, bb, dense_a, dense_b);
            auto polynomial = block_convolve_zak(aa, bb, trial % 2 ? 3 : 0);
            for (int x : grid.values()) {
                assert(actual.at(x) == expected.at(x));
                assert(hyperbola.at(x) == expected.at(x));
                assert(generic.at(x) == expected.at(x));
                assert(ranked.at(x) == expected.at(x));
                if (polynomial.at(x) != expected.at(x)) {
                    std::fprintf(stderr, "Zak mismatch n=%d trial=%d x=%d actual=%d expected=%d\n",
                                 n, trial, x, polynomial.at(x).v, expected.at(x).v);
                    std::abort();
                }
            }
            series dense_quotient = div(dense_a, dense_b);
            BlockPrefix direct = dujiao_direct(bb, aa, dense_quotient);
            BlockPrefix zak = dujiao_zak(bb, aa, dense_b, dense_a);
            BlockPrefix direct_small = dujiao_direct(bb, aa);
            BlockPrefix zak_small = dujiao_zak(bb, aa);
            series quotient = div(a, b);
            BlockPrefix reference(grid, quotient);
            for (int x : grid.values()) {
                assert(direct.at(x) == reference.at(x));
                assert(zak.at(x) == reference.at(x));
                assert(direct_small.at(x) == reference.at(x));
                assert(zak_small.at(x) == reference.at(x));
            }
        }
        series z(n + 1);
        for (int x = 1; x <= n; ++x) z[x] = 1;
        series a(n + 1);
        for (int x = 1; x <= n; ++x) a[x] = rng() % 97;
        BlockPrefix aa(grid, a), zz(grid, z);
        series dense_a(a.begin(), a.begin() + t + 1);
        series dense_z(z.begin(), z.begin() + t + 1);
        auto mult_side = block_convolve_one_multiplicative(aa, zz, dense_a, dense_z);
        BlockPrefix expected(grid, mul(a, z));
        for (int x : grid.values()) assert(mult_side.at(x) == expected.at(x));
    }
    for (int n : {2, 9, 37, 100, 301}) {
        mal::number_theory::Sieve sieve(n);
        series bell(n + 1);
        for (int p : sieve.primes)
            for (int q = p; q <= n; ) {
                bell[q] = rng() % 101;
                if (q > n / p) break;
                q *= p;
            }
        DynamicBellBlock dynamic(sieve, bell);
        auto verify = [&]() {
            series f(n + 1); f[1] = 1;
            for (int x = 2; x <= n; ++x) {
                int p = sieve.spf[x], power = 1, core = x;
                do { core /= p; power *= p; } while (core % p == 0);
                f[x] = bell[power] * f[core];
            }
            BlockPrefix expected(dynamic.grid(), f);
            std::uint64_t ans = 0;
            for (int x : dynamic.grid().values()) {
                assert(dynamic.block().at(x) == expected.at(x));
                ans ^= std::uint64_t(x) * expected.at(x).v;
            }
            assert(ans == dynamic.answer());
        };
        verify();
        for (int op = 0; op < 50; ++op) {
            int p = sieve.primes[rng() % sieve.primes.size()];
            std::vector<coef> values;
            for (int q = p; q <= n; ) {
                values.push_back(rng() % 101);
                bell[q] = values.back();
                if (q > n / p) break;
                q *= p;
            }
            dynamic.update(p, values);
            verify();
        }
    }
    for (int n : {1024, 7001, 30000}) {
        QuotientGrid grid(n);
        series a(n + 1), b(n + 1);
        for (int i = 1; i <= n; ++i) {
            a[i] = rng() % 103;
            b[i] = rng() % 107;
        }
        BlockPrefix f(grid, a), g(grid, b);
        auto expected = block_convolve(f, g);
        for (int scale : {0, 2, 8}) {
            auto actual = block_convolve_zak(f, g, scale);
            for (int x : grid.values()) assert(actual.at(x) == expected.at(x));
        }
    }
}
