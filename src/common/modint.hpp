#pragma once
template <int MOD>
struct mint {
    int v;
    mint(ll v_ = 0) : v(int(v_ % MOD)) { if (v < 0) v += MOD; }

    friend mint operator+(mint a, mint b) { int r = a.v + b.v; return r >= MOD ? r - MOD : r; }
    friend mint operator-(mint a, mint b) { int r = a.v - b.v; return r < 0 ? r + MOD : r; }
    friend mint operator*(mint a, mint b) { return (ll)a.v * b.v % MOD; }
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
};

/* Edited on 2026/07/27
提供一个零开销的模类
静态模数，编译期确定以利于内联优化
*/
