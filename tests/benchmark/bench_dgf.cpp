#include "../../include/dgf/dgf.hpp"
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <random>

using namespace mal::dgf;

template <class F>
long long measure(const char* name, F fn) {
    auto t0 = std::chrono::steady_clock::now();
    series result = fn();
    auto t1 = std::chrono::steady_clock::now();
    volatile int checksum = result[result.size() / 2].v;
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    std::cout << name << " " << elapsed << " ms " << checksum << '\n';
    return elapsed;
}

int main(int argc, char** argv) {
    int n = argc > 1 ? std::atoi(argv[1]) : 200000;
    if (n < 2) return 1;
    std::mt19937 rng(67321);
    series a(n + 1), b(n + 1), z(n + 1, 1), prime_power(n + 1);
    z[0] = 0;
    a[1] = b[1] = 1;
    PrimePowers table(n);
    for (int i = 2; i <= n; ++i) {
        a[i] = rng() % 17;
        b[i] = rng() % 19;
        if (table.core[i] == 1) prime_power[i] = rng() % 11;
    }
    measure("general mul", [&] { return mul(a, b); });
    measure("ranked mul", [&] { return fast_mul(a, b); });
    measure("general * zeta", [&] { return mul(a, z); });
    measure("multiplicative factor", [&] { return DGF::general(a).multiply(DGF::zeta(n)).coefficients(); });
    measure("zeta * zeta general", [&] { return mul(z, z); });
    measure("zeta * zeta structured", [&] { return DGF::zeta(n).multiply(DGF::zeta(n)).coefficients(); });
    measure("inv(zeta) general", [&] { return inv(z); });
    measure("inv(zeta) structured", [&] { return DGF::zeta(n).inverse().coefficients(); });
    measure("general exp", [&] { return exp(prime_power); });
    measure("prime-power exp", [&] { return DGF::prime_power(prime_power).exponential().coefficients(); });
    measure("general ln(zeta)", [&] { return ln(z); });
    measure("multiplicative ln(zeta)", [&] { return DGF::zeta(n).log().coefficients(); });
}
