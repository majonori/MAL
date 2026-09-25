#include "../../include/dgf/zak_polynomial.hpp"
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <sys/resource.h>

using namespace mal::dgf;

int main(int argc, char** argv) {
    std::int64_t n = argc > 1 ? std::atoll(argv[1]) : 1000000;
    std::string mode = argc > 2 ? argv[2] : "hyperbola";
    int scale = argc > 3 ? std::atoi(argv[3]) : 0;
    QuotientGrid grid(n);
    BlockPrefix f(grid), g(grid);
    series a(grid.root() + 1), b(grid.root() + 1);
    // Analytic prefixes allow benchmarks beyond 10^13 without an O(N)
    // setup pass. Test operands are f(k)=1 and g(k)=k (mod 998244353).
    const coef inv2 = coef(2).inv();
    for (int x = 1; x <= grid.root(); ++x) {
        a[x] = 1;
        b[x] = x;
    }
    for (std::size_t j = 0; j < grid.values().size(); ++j) {
        std::int64_t x = grid.values()[j];
        coef mx = coef(x % 998244353);
        f[j] = mx;
        g[j] = mx * (mx + coef(1)) * inv2;
    }
    auto start = std::chrono::steady_clock::now();
    BlockPrefix answer(grid);
    if (mode == "hyperbola") answer = block_convolve(f, g);
    else if (mode == "zak") answer = block_convolve_zak(f, g, scale);
    else if (mode == "baseline") answer = block_convolve(f, g, mul(a, b));
    else return 2;
    auto stop = std::chrono::steady_clock::now();
    coef expected = 0;
    for (std::int64_t l = 1, r; l <= n; l = r + 1) {
        r = n / (n / l);
        expected += (coef(l % 998244353) + coef(r % 998244353))
                  * coef((r - l + 1) % 998244353) * inv2
                  * coef((n / l) % 998244353);
    }
    if (answer.at(n) != expected) {
        std::fprintf(stderr, "incorrect answer: got=%d expected=%d\n",
                     answer.at(n).v, expected.v);
        return 1;
    }
    rusage usage{};
    getrusage(RUSAGE_SELF, &usage);
    std::fprintf(stderr, "N=%lld mode=%s scale=%d time=%.3f peak_kib=%ld checksum=%d\n", (long long)n,
                 mode.c_str(), scale,
                 std::chrono::duration<double>(stop - start).count(), usage.ru_maxrss,
                 answer.at(n).v);
}
