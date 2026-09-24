#include "../../include/number_theory/sieve.hpp"
#include "../../include/number_theory/primality.hpp"
#include <cassert>
#include <iostream>

int main() {
    using namespace mal::number_theory;
    auto s = make_sieve(100000);
    auto phi = totients(s), mu = mobius(s);
    assert(phi[1] == 1 && phi[36] == 12);
    assert(mu[1] == 1 && mu[30] == -1 && mu[12] == 0);
    assert(s.factor(1).empty());
    assert((s.factor(360) == std::vector<std::pair<int,int>>{{2,3},{3,2},{5,1}}));
    for (int i = 0; i <= 100000; ++i) assert(is_prime(i) == s.prime(i));
    assert(is_prime(2305843009213693951ULL));       // 2^61-1
    assert(is_prime(18446744073709551557ULL));      // 2^64-59
    assert(!is_prime(341550071728321ULL));         // strong pseudoprime to small bases
    assert(!is_prime(3825123056546413051ULL));
    assert(!is_prime(18446744073709551615ULL));
    assert(pow_mod(2, 10, 1000) == 24);
    assert(mul_mod(18446744073709551615ULL, 18446744073709551615ULL,
                   18446744073709551557ULL) == 3364);
    std::cout << "number_theory OK\n";
}
