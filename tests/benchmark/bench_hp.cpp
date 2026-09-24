#include "../../include/hp/bigfloat.hpp"
#include <chrono>
#include <cstdint>
#include <iostream>
#include <random>
#include <vector>

using mal::BigFloat;
using mal::BigInt;
using Clock = std::chrono::steady_clock;

static std::vector<std::uint32_t> random_limbs(size_t n, std::mt19937_64& rng) {
    std::vector<std::uint32_t> v(n);
    for (auto& x : v) x = std::uint32_t(rng());
    if (!v.empty()) v.back() |= 1;
    return v;
}

int main() {
    std::mt19937_64 rng(20260922);
    for (size_t n : {64u, 256u, 1024u, 4096u, 16384u, 600000u}) {
        BigInt a = BigInt::from_limbs(random_limbs(n, rng));
        BigInt b = BigInt::from_limbs(random_limbs(n, rng));
        auto t0 = Clock::now();
        BigInt c = a * b;
        auto t1 = Clock::now();
        double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        std::cout << "mul " << n << " limbs: " << ms << " ms (" << c.limb_count()
                  << " limbs)\n";
    }
    for (int p : {256, 1024, 4096}) {
        BigFloat x("1.23456789", p);
        auto t0 = Clock::now();
        BigFloat y = BigFloat::exp(x);
        auto t1 = Clock::now();
        BigFloat z = BigFloat::log(x);
        auto t2 = Clock::now();
        double e = std::chrono::duration<double, std::milli>(t1 - t0).count();
        double l = std::chrono::duration<double, std::milli>(t2 - t1).count();
        std::cout << "precision " << p << " bits: exp " << e << " ms, log " << l
                  << " ms (" << z.to_string(20) << ")\n";
    }
    for (int p : {1 << 20, 1 << 22}) {
        BigFloat x("1.23456789", p), y("9.87654321", p);
        auto t0 = Clock::now();
        BigFloat q = x / y;
        auto t1 = Clock::now();
        std::cout << "newton div " << p << " bits: "
                  << std::chrono::duration<double, std::milli>(t1 - t0).count()
                  << " ms (" << q.to_string(20) << ")\n";
    }
    for (int p : {1 << 17, 1 << 19}) {
        BigFloat x("2.75", p);
        auto t0 = Clock::now();
        BigFloat l = BigFloat::log(x);
        auto t1 = Clock::now();
        std::cout << "agm log " << p << " bits: "
                  << std::chrono::duration<double, std::milli>(t1 - t0).count()
                  << " ms (" << l.to_string(20) << ")\n";
    }
    {
        BigFloat x("2.75", 1 << 14);
        auto t0 = Clock::now();
        BigFloat e = BigFloat::exp_newton_agm(x);
        auto t1 = Clock::now();
        std::cout << "newton exp 16384 bits: "
                  << std::chrono::duration<double, std::milli>(t1 - t0).count()
                  << " ms (" << e.to_string(20) << ")\n";
    }
    {
        std::string s = "1.";
        for (int i = 0; i < 200000; ++i) s.push_back(char('0' + (i * 7 + 3) % 10));
        auto t0 = Clock::now();
        BigFloat x(s, 666000);
        auto t1 = Clock::now();
        std::string out = x.to_string(200000);
        auto t2 = Clock::now();
        std::cout << "decimal io 200k digits: parse "
                  << std::chrono::duration<double, std::milli>(t1 - t0).count()
                  << " ms, print "
                  << std::chrono::duration<double, std::milli>(t2 - t1).count()
                  << " ms (" << out.size() << " chars)\n";
    }
    {
        // Plain integer decimal conversion: the divide-and-conquer split is
        // dominated by its multiplications, so this is the number to watch.
        for (size_t n : {4096u, 16384u, 65536u}) {
            BigInt a = BigInt::from_limbs(random_limbs(n, rng));
            std::string s = a.to_string();   // also warms the decimal tables
            auto t0 = Clock::now();
            std::string again = a.to_string();
            auto t1 = Clock::now();
            BigInt b = BigInt::from_string(s);
            auto t2 = Clock::now();
            std::cout << "bigint decimal " << s.size() << " digits: print "
                      << std::chrono::duration<double, std::milli>(t1 - t0).count()
                      << " ms, parse "
                      << std::chrono::duration<double, std::milli>(t2 - t1).count()
                      << " ms (" << (b == a && again == s) << ")\n";
        }
    }
}
