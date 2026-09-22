#include "../../bundles/hp/main.cpp"
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

namespace p5432 {

constexpr int MOD = 998244353;
constexpr int ROOT = 3;

inline int mod_pow(long long a, long long e) {
    long long r = 1;
    a %= MOD;
    while (e) {
        if (e & 1) r = r * a % MOD;
        a = a * a % MOD;
        e >>= 1;
    }
    return int(r);
}

struct twiddle_tables {
    std::vector<std::vector<int>> fwd, inv;
    std::vector<int> inv2;

    void ensure(int levels) {
        if (int(fwd.size()) > levels) return;
        fwd.resize(levels + 1);
        inv.resize(levels + 1);
        inv2.resize(levels + 1);
        inv2[0] = 1;
        const int half_mod = (MOD + 1) / 2;
        for (int h = 1; h <= levels; ++h)
            inv2[h] = int(1LL * inv2[h - 1] * half_mod % MOD);
        for (int h = 0; h <= levels; ++h) {
            const int half = 1 << h;
            if (!fwd[h].empty()) continue;
            fwd[h].assign(half, 1);
            inv[h].assign(half, 1);
            if (half == 1) continue;
            const int wf = mod_pow(ROOT, (MOD - 1) / (half << 1));
            const int wi = mod_pow(ROOT, MOD - 1 - (MOD - 1) / (half << 1));
            for (int j = 1; j < half; ++j) {
                fwd[h][j] = int(1LL * fwd[h][j - 1] * wf % MOD);
                inv[h][j] = int(1LL * inv[h][j - 1] * wi % MOD);
            }
        }
    }
};

inline twiddle_tables& tables() {
    static twiddle_tables t;
    return t;
}

void ntt(std::vector<int>& a, bool invert) {
    const int n = int(a.size());
    if (n <= 1) return;
    const int levels = __builtin_ctz((unsigned)n);
    tables().ensure(levels);
    if (!invert) {
        for (int len = n, h = levels - 1; len > 1; len >>= 1, --h) {
            const int half = len >> 1;
            const std::vector<int>& w = tables().fwd[h];
            for (int i = 0; i < n; i += len) {
                for (int j = 0; j < half; ++j) {
                    const int u = a[i + j], v = a[i + j + half];
                    int s = u + v;
                    if (s >= MOD) s -= MOD;
                    int d = u >= v ? u - v : u + MOD - v;
                    a[i + j] = s;
                    a[i + j + half] = int(1LL * d * w[j] % MOD);
                }
            }
        }
    } else {
        for (int len = 2; len <= n; len <<= 1) {
            const int half = len >> 1;
            const std::vector<int>& w = tables().inv[__builtin_ctz((unsigned)half)];
            for (int i = 0; i < n; i += len) {
                for (int j = 0; j < half; ++j) {
                    const int u = a[i + j];
                    const int v = int(1LL * a[i + j + half] * w[j] % MOD);
                    int s = u + v;
                    if (s >= MOD) s -= MOD;
                    int d = u >= v ? u - v : u + MOD - v;
                    a[i + j] = s;
                    a[i + j + half] = d;
                }
            }
        }
        const int inv_n = tables().inv2[levels];
        for (int& x : a) x = int(1LL * x * inv_n % MOD);
    }
}

std::vector<int> convolution(std::vector<int> a, std::vector<int> b) {
    if (a.empty() || b.empty()) return {};
    const int need = int(a.size() + b.size() - 1);
    int n = 1;
    while (n < need) n <<= 1;
    a.resize(n);
    b.resize(n);
    ntt(a, false);
    ntt(b, false);
    for (int i = 0; i < n; ++i) a[i] = int(1LL * a[i] * b[i] % MOD);
    ntt(a, true);
    a.resize(need);
    return a;
}

std::vector<int> polynomial_inverse(const std::vector<int>& f, int need) {
    std::vector<int> g(1, mod_pow(f[0], MOD - 2));
    int cur = 1;
    while (cur < need) {
        const int nxt = std::min(need, cur << 1);
        std::vector<int> h(f.begin(), f.begin() + std::min<int>(f.size(), nxt));
        std::vector<int> t = convolution(g, h);
        t.resize(nxt);
        for (int& x : t) x = x ? MOD - x : 0;
        t[0] += 2;
        if (t[0] >= MOD) t[0] -= MOD;
        g = convolution(g, t);
        g.resize(nxt);
        cur = nxt;
    }
    g.resize(need);
    return g;
}

} // namespace p5432

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::string sa, sb;
    if (!(std::cin >> sa >> sb)) return 0;
    mal::BigInt a(sa), b(sb);
    const std::string r = (a / b).to_string();
    const int n = int(r.size()) - 1;
    std::vector<int> f(n + 1);
    for (int i = 0; i <= n; ++i) f[i] = r[n - i] - '0';
    const std::vector<int> g = p5432::polynomial_inverse(f, n + 1);
    std::string out;
    out.reserve((size_t)(n + 1) * 11);
    for (int i = 0; i <= n; ++i) {
        if (i) out.push_back(' ');
        out += std::to_string(g[i]);
    }
    out.push_back('\n');
    std::cout << out;
    return 0;
}
