#pragma once
#include "block_sieve.hpp"
#include "../poly/ntt.hpp"
#include <array>
#include <cmath>
#include <functional>

namespace mal { namespace dgf {

// Zhou Kangyang's logarithmic-bucket polynomial block convolution.
// The approximation is one-sided: ceil(S ln a)+ceil(S ln b) <= S ln x
// implies ab<=x. Missed triples ab*t near N are restored by segmented
// factorization and exact integer block placement. Segmentation, rather
// than an N/S-sized factor table, keeps peak working storage O(sqrt N).
// scale=0 picks a bounded-degree default; a positive scale tunes the tradeoff
// while staying within the modulus NTT-length and sqrt(N)-storage guards.
inline BlockPrefix block_convolve_zak(const BlockPrefix& f, const BlockPrefix& g,
                                     int scale = 0) {
    same_grid(f, g);
    const QuotientGrid& grid = f.grid();
    const std::int64_t n = grid.limit();
    const int root = grid.root();
    const auto& xs = grid.values();
    int cut = 1;
    while (1LL * (cut + 1) * (cut + 1) * (cut + 1) <= n) ++cut;
    cut = std::min(cut, root);
    if (scale < 0) throw std::invalid_argument("Zak: negative logarithm scale");
    bool automatic = scale == 0;
    if (automatic)
        scale = std::max(1, int(12.0L * root / std::log((long double)n + 2)));
    // This modulus has primitive roots only up to length 2^23. Leaving this
    // unchecked produces plausible-looking WRONG values on large grids.
    if (automatic && cut < root) {
        long double span = std::log((long double)root)
                         - std::log((long double)cut + 1);
        int safe_scale = int(((1 << 22) - 16) / std::max(0.01L, span));
        scale = std::max(1, std::min(scale, safe_scale));
    }
    // A fixed user-selected scale may require more than O(sqrt N) storage.
    // Enforce a fixed-multiple size bound independent of N.
    long double highest_bucket = scale * std::log((long double)root) + 1e-9L;
    if (highest_bucket > 2000000000.0L)
        throw std::invalid_argument("Zak: polynomial logarithm scale too large");
    int max_bucket = int(std::ceil(highest_bucket));
    std::vector<coef> fp(root + 1), gp(root + 1);
    std::vector<int> bucket(root + 1);
    // Guard floating logarithms even after multiplication by scale on 64-bit
    // grids. The conservative guard only enlarges the exact correction band.
    constexpr long double margin = 1e-5L;
    for (int a = 1; a <= root; ++a) {
        fp[a] = f.point(a); gp[a] = g.point(a);
        bucket[a] = int(std::ceil(scale * std::log((long double)a) + margin));
    }
    // The central square only uses a,b>cut: shift away the empty low buckets.
    int shift = cut < root ? bucket[cut + 1] : 0;
    int width = cut < root ? max_bucket - shift + 1 : 1;
    if (width > 32 * root)
        throw std::invalid_argument("Zak: polynomial scale exceeds sqrt(N) storage cap");
    if (2LL * width - 1 > (1 << 23))
        throw std::invalid_argument("Zak: polynomial exceeds modulus NTT length 2^23");
    std::vector<coef> sf(width), sg(width);
    for (int a = cut + 1; a <= root; ++a) {
        sf[bucket[a] - shift] += fp[a];
        sg[bucket[a] - shift] += gp[a];
    }
    auto poly = mal::ntt_mul(sf, sg);
    for (std::size_t k = 1; k < poly.size(); ++k) poly[k] += poly[k - 1];
    auto approx = [&](std::int64_t x) {
        int k = int(std::floor(scale * std::log((long double)x) - margin)) - 2 * shift;
        if (k < 0) return coef(0);
        return poly[std::min(k, int(poly.size()) - 1)];
    };
    BlockSums blocks(grid);
    // Exact edge min(a,b)<=N^(1/3); only the central square is approximated.
    auto target_block = [&](std::int64_t product) -> int {
        return product <= root ? int(product - 1) : grid.index(n / (n / product));
    };
    for (int a = 1; a <= cut; ++a) {
        blocks.add_static(target_block(1LL * a * a), fp[a] * gp[a]);
        for (int b = a + 1; b <= root; ) {
            int j = target_block(1LL * a * b);
            int end = int(std::min<std::int64_t>(root, xs[j] / a));
            blocks.add_static(j, fp[a] * (g.at(end) - g.at(b - 1))
                               + gp[a] * (f.at(end) - f.at(b - 1)));
            b = end + 1;
        }
    }
    coef previous_approx = 0;
    for (std::size_t j = root; j < xs.size(); ++j) {
        coef current = approx(xs[j]);
        blocks.add_static(j, current - previous_approx);
        previous_approx = current;
    }
    // Exactly one of a,b exceeds sqrt(N). Their prefix differences stay on D(N).
    for (int a = 1; a <= root; ++a) {
        long long first = 1LL * a * (root + 1);
        if (first > n) continue;
        auto begin = std::lower_bound(xs.begin(), xs.end(), first);
        for (auto it = begin; it != xs.end(); ++it) {
            int j = int(it - xs.begin());
            std::int64_t upper = *it / a;
            std::int64_t lower = std::max<std::int64_t>(root, j ? xs[j - 1] / a : 0);
            blocks.add_static(j, fp[a] * (g.at(upper) - g.at(lower))
                               + gp[a] * (f.at(upper) - f.at(lower)));
        }
    }
    // Conservative lower bound accounts for floor(N/t) and log rounding.
    long long lo = std::max(1LL, (long long)std::floor(
        (n - root) * std::exp(-(2.0L + 8 * margin) / scale)) - 2);
    int max_t = int(std::min<std::int64_t>(n / (root + 1),
                                  n / (1LL * (cut + 1) * (cut + 1))));
    std::vector<int> threshold(max_t + 1);
    for (int t = 1; t <= max_t; ++t)
        threshold[t] = int(std::floor(scale * std::log((long double)(n / t)) - margin));
    std::vector<coef> correction(xs.size());
    number_theory::Sieve primes(root);
    int chunk = std::max(1, root / (2 + int(std::log2((long double)n + 1))));
    for (long long begin = lo; begin <= n; begin += chunk) {
        int count = int(std::min<long long>(chunk, n - begin + 1));
        std::vector<long long> residual(count);
        // N<=10^14 has at most 12 distinct prime factors (the product of
        // the first 13 primes already exceeds 10^14). Fixed slots avoid
        // millions of small heap allocations in the correction interval.
        std::vector<std::array<std::pair<std::int64_t, int>, 13>> factors(count);
        std::vector<unsigned char> used(count);
        for (int i = 0; i < count; ++i) residual[i] = begin + i;
        for (int p : primes.primes) {
            long long first = (begin + p - 1) / p * p;
            for (long long value = first; value < begin + count; value += p) {
                int index = int(value - begin), exponent = 0;
                while (residual[index] % p == 0) {
                    residual[index] /= p;
                    ++exponent;
                }
                factors[index][used[index]++] = {p, exponent};
            }
        }
        for (int i = 0; i < count; ++i) {
            long long value = begin + i;
            if (residual[i] > 1) factors[i][used[i]++] = {residual[i], 1};
            auto visit = [&](auto&& self, int index, int a, int b) -> void {
                if (index == used[i]) {
                    if (a <= cut || b <= cut ||
                        fp[a] == coef(0) || gp[b] == coef(0)) return;
                    int t = int(value / (1LL * a * b));
                    if (t > max_t || bucket[a] + bucket[b] <= threshold[t]) return;
                    std::int64_t x = n / t;
                    correction[grid.index(x)] += fp[a] * gp[b];
                    return;
                }
                std::int64_t p = factors[i][index].first;
                int e = factors[i][index].second;
                long long aa = a;
                for (int ea = 0; ea <= e && aa <= root; ++ea) {
                    long long bb = b;
                    for (int eb = 0; ea + eb <= e && bb <= root; ++eb) {
                        self(self, index + 1, int(aa), int(bb));
                        if (bb > root / p) break;
                        bb *= p;
                    }
                    if (aa > root / p) break;
                    aa *= p;
                }
            };
            visit(visit, 0, 1, 1);
        }
    }
    coef previous = 0;
    for (std::size_t j = root; j < xs.size(); ++j) {
        blocks.add_static(j, correction[j] - previous);
        previous = correction[j];
    }
    return blocks.to_prefix();
}

} } // namespace mal::dgf
