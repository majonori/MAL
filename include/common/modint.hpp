#pragma once
#include "consts.hpp"

namespace mal {

template <int MOD>
struct mint {
    int v;
    mint(ll v_ = 0) : v(int(v_ % MOD)) { if (v < 0) v += MOD; }

    // The results below are already reduced, so they use the unchecked
    // constructor: going through mint(ll) would add a hidden % MOD per
    // butterfly, which dominates the transform time.
    friend mint operator+(mint a, mint b) {
        const int r = a.v + b.v;
        return mint(r >= MOD ? r - MOD : r, unchecked{});
    }
    friend mint operator-(mint a, mint b) {
        const int r = a.v - b.v;
        return mint(r < 0 ? r + MOD : r, unchecked{});
    }
    friend mint operator*(mint a, mint b) {
        return mint(int((ll)a.v * b.v % MOD), unchecked{});
    }
    mint& operator+=(mint b) { return *this = *this + b; }
    mint& operator-=(mint b) { return *this = *this - b; }
    mint& operator*=(mint b) { return *this = *this * b; }

    mint pow(ll k) const {
        mint r = 1, a = *this;
        for (; k; k >>= 1, a *= a) if (k & 1) r *= a;
        return r;
    }
    mint inv() const { return pow(MOD - 2); } // MOD must be prime
    bool operator==(mint b) const { return v == b.v; }
    bool operator!=(mint b) const { return v != b.v; }

private:
    struct unchecked {};
    mint(int v_, unchecked) : v(v_) {}
};

} // namespace mal

/* Edited on 2026/07/27
提供一个零开销的模类
静态模数，编译期确定以利于内联优化
*/
