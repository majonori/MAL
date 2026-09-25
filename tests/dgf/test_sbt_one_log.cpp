#include "../../include/dgf/sbt_one_log.hpp"
#include <cassert>
#include <random>

using u64 = std::uint64_t;
using u128 = unsigned __int128;

static u128 reference(u64 n) {
    u128 ans = 0;
    for (u64 l = 1, r; l <= n; l = r + 1) {
        u64 q = n / l;
        r = n / q;
        ans += u128(r - l + 1) * q;
    }
    return ans;
}

int main() {
    for (u64 n = 0; n <= 100000; ++n)
        assert((mal::dgf::D_x<1,1>(n) == reference(n)));
    std::mt19937_64 rng(20260924);
    for (int t = 0; t < 300; ++t) {
        u64 n = rng() % 100000000ULL + 1;
        assert((mal::dgf::D_x<1,1>(n) == reference(n)));
    }
    for (u64 t : {1000ULL, 10000ULL, 100000ULL, 1000000ULL})
        for (int delta = -2; delta <= 2; ++delta) {
            u64 a = t*t+delta, b = t*(t+1)+delta;
            assert((mal::dgf::D_x<1,1>(a) == reference(a)));
            assert((mal::dgf::D_x<1,1>(b) == reference(b)));
        }
    assert((mal::dgf::D_x<1,1>(1000000000000ULL) == reference(1000000000000ULL)));
    assert((mal::dgf::D_x<1,1>(100000000000000ULL) == reference(100000000000000ULL)));
}
