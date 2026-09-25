#include "../../include/dgf/pn2d.hpp"
#include <cassert>

using namespace mal::dgf;

int main() {
    for (int n : {2, 8, 19, 43, 81, 120}) {
        QuotientGrid grid(n);
        mal::number_theory::Sieve sieve(n);
        auto bell = [](int p, int a, int b) -> coef {
            if (a == 0 && b == 0) return 1;
            return (p % 19 + 3) * (a + 1) * (b + 2) + 7 * a * b;
        };
        std::vector<std::vector<coef>> middle(n + 1);
        for (int p : sieve.primes)
            (void)pn2d_local_sparse(p, n, bell, &middle[p]);
        series l(n + 1), r(n + 1), m(n + 1);
        l[1] = r[1] = m[1] = 1;
        for (int x = 2; x <= n; ++x) {
            int p = sieve.spf[x], core = x, exponent = 0;
            do { core /= p; ++exponent; } while (core % p == 0);
            l[x] = l[core] * bell(p, exponent, 0);
            r[x] = r[core] * bell(p, 0, exponent);
            m[x] = m[core] * middle[p][exponent];
        }
        BlockPrefix left(grid, l), right(grid, r), diagonal(grid, m);
        coef expected = 0;
        for (int x = 1; x <= n; ++x) for (int y = 1; y <= n; ++y) {
            int u = x, v = y;
            coef value = 1;
            for (int p : sieve.primes) {
                if (p > u && p > v) break;
                int a = 0, b = 0;
                while (u % p == 0) { u /= p; ++a; }
                while (v % p == 0) { v /= p; ++b; }
                if (a || b) value *= bell(p, a, b);
            }
            expected += value;
        }
        coef actual = pn2d_sparse_direct(left, right, diagonal, bell);
        assert(actual == expected);
        coef offline = pn2d_sparse_offline(left, right, diagonal, bell);
        if (offline != expected)
            std::fprintf(stderr, "pn2d mismatch n=%d offline=%d expected=%d\n",
                         n, offline.v, expected.v);
        assert(offline == expected);
    }
    for (int n : {317, 1001, 10001, 50021}) {
        QuotientGrid grid(n);
        mal::number_theory::Sieve sieve(n);
        auto bell = [](int p, int a, int b) -> coef {
            if (!a && !b) return 1;
            return 11 + (p % 23) * (a + 2) + 13 * b + a * b * 17;
        };
        std::vector<std::vector<coef>> coeff(n + 1);
        for (int p : sieve.primes)
            (void)pn2d_local_sparse(p, n, bell, &coeff[p]);
        series l(n + 1), r(n + 1), m(n + 1);
        l[1] = r[1] = m[1] = 1;
        for (int x = 2; x <= n; ++x) {
            int p = sieve.spf[x], core = x, e = 0;
            do { core /= p; ++e; } while (core % p == 0);
            l[x] = l[core] * bell(p, e, 0);
            r[x] = r[core] * bell(p, 0, e);
            m[x] = m[core] * coeff[p][e];
        }
        BlockPrefix L(grid, l), R(grid, r), M(grid, m);
        assert(pn2d_sparse_offline(L, R, M, bell)
            == pn2d_sparse_direct(L, R, M, bell));
    }
}
