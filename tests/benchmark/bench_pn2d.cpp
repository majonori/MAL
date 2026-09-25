#include "../../include/dgf/pn2d.hpp"
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <sys/resource.h>

using namespace mal::dgf;

int main(int argc, char** argv) {
    std::int64_t n = argc > 1 ? std::atoll(argv[1]) : 100000;
    QuotientGrid grid(n);
    BlockPrefix left(grid), right(grid), middle(grid);
    for (std::size_t j = 0; j < left.size(); ++j) {
        coef prefix(grid.values()[j] % 998244353);
        left[j] = right[j] = middle[j] = prefix;
    }
    // Local series: (1-x)^-1(1-y)^-1(1-xy)^-1
    //               *(1+2*x^2*y+3*x*y^2).
    // Its three univariate marginal/diagonal factors are all zeta.
    auto bell = [](int, int a, int b) -> coef {
        int result = std::min(a, b) + 1;
        if (a >= 2 && b >= 1) result += 2 * (std::min(a - 2, b - 1) + 1);
        if (a >= 1 && b >= 2) result += 3 * (std::min(a - 1, b - 2) + 1);
        return result;
    };
    auto start = std::chrono::steady_clock::now();
    coef result = pn2d_sparse_offline(left, right, middle, bell);
    auto stop = std::chrono::steady_clock::now();
    if (n <= 100000000)
        if (result != pn2d_sparse_direct(left, right, middle, bell)) return 1;
    rusage used{};
    getrusage(RUSAGE_SELF, &used);
    std::fprintf(stderr, "N=%lld result=%d seconds=%.3f peak_kib=%ld\n",
        (long long)n, result.v,
        std::chrono::duration<double>(stop - start).count(), used.ru_maxrss);
}
