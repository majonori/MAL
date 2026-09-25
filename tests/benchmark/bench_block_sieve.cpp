#include "../../include/dgf/dynamic_bell.hpp"
#include <chrono>
#include <cstdio>
#include <cstdlib>

int main(int argc, char** argv) {
    int n = argc > 1 ? std::atoi(argv[1]) : 10000000;
    int updates = argc > 2 ? std::atoi(argv[2]) : 1000;
    auto start = std::chrono::steady_clock::now();
    mal::number_theory::Sieve sieve(n);
    mal::dgf::series bell(n + 1);
    for (int p : sieve.primes)
        for (int q = p; q <= n; ) {
            bell[q] = 1;
            if (q > n / p) break;
            q *= p;
        }
    mal::dgf::DynamicBellBlock d(std::move(sieve), std::move(bell));
    auto built = std::chrono::steady_clock::now();
    std::vector<mal::dgf::coef> values;
    for (int q = 2; q <= n; ) {
        values.push_back(1);
        if (q > n / 2) break;
        q *= 2;
    }
    for (int i = 0; i < updates; ++i) {
        values[0] = 1 + (i & 1);
        d.update(2, values);
    }
    auto done = std::chrono::steady_clock::now();
    std::fprintf(stderr, "N=%d updates(p=2)=%d build=%.3fs update=%.3fs answer=%llu\n",
                 n, updates,
                 std::chrono::duration<double>(built - start).count(),
                 std::chrono::duration<double>(done - built).count(),
                 static_cast<unsigned long long>(d.answer()));
}
