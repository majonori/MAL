#pragma once
#include "block_sieve.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace mal { namespace dgf {

// A two-dimensional Bell coefficient f(p^a,p^b), with f(1,1)=1.
// The two marginal block prefixes S_L, S_R and the diagonal S_M are supplied
// separately. M_p(z) is the diagonal of f_p(x,y)/(L_p(x)R_p(y)).
// This allows the caller to use any appropriate univariate prime sieve.
struct PN2DTerm {
    std::int64_t x, y;
    coef value;
};

struct PN2DLocalPrime {
    int prime;
    std::vector<PN2DTerm> terms;
};

inline int pn2d_sqrt(std::int64_t x) {
    int r = int(std::sqrt((long double)x));
    while (1LL * (r + 1) * (r + 1) <= x) ++r;
    while (1LL * r * r > x) --r;
    return r;
}

inline std::int64_t pn2d_next_prime_bound(std::int64_t a, std::int64_t b) {
    // Every nonunit local residual needs (p,p^2) or (p^2,p).
    return std::max(std::min<std::int64_t>(a, pn2d_sqrt(b)),
                    std::min<std::int64_t>(pn2d_sqrt(a), b));
}

template<class Bell>
inline std::vector<PN2DTerm> pn2d_local_sparse(int p, std::int64_t n,
                                               Bell bell,
                                               std::vector<coef>* diagonal = nullptr) {
    std::vector<std::int64_t> powers(1, 1);
    while (powers.back() <= n / p) powers.push_back(powers.back() * p);
    int d = int(powers.size()) - 1;
    std::vector<coef> left(d + 1), right(d + 1), il(d + 1), ir(d + 1);
    for (int a = 0; a <= d; ++a) {
        left[a] = bell(p, a, 0);
        right[a] = bell(p, 0, a);
    }
    if (left[0] != coef(1) || right[0] != coef(1))
        throw std::invalid_argument("2D PN: f(1,1) must equal one");
    il[0] = ir[0] = 1;
    for (int a = 1; a <= d; ++a) {
        for (int j = 1; j <= a; ++j) {
            il[a] -= left[j] * il[a - j];
            ir[a] -= right[j] * ir[a - j];
        }
    }
    std::vector<std::vector<coef>> h(d + 1, std::vector<coef>(d + 1));
    std::vector<std::vector<coef>> residual(d + 1, std::vector<coef>(d + 1));
    for (int a = 0; a <= d; ++a) for (int b = 0; b <= d; ++b)
        for (int u = 0; u <= a; ++u) for (int v = 0; v <= b; ++v)
            h[a][b] += bell(p, u, v) * il[a - u] * ir[b - v];
    std::vector<coef> middle(d + 1);
    middle[0] = 1;
    for (int e = 1; e <= d; ++e) middle[e] = h[e][e];
    for (int a = 0; a <= d; ++a) for (int b = 0; b <= d; ++b) {
        residual[a][b] = h[a][b];
        for (int k = 1; k <= std::min(a, b); ++k)
            residual[a][b] -= middle[k] * residual[a - k][b - k];
    }
    if (diagonal) *diagonal = middle;
    std::vector<PN2DTerm> result;
    for (int a = 1; a <= d; ++a) for (int b = 1; b <= d; ++b) {
        if (a == b || residual[a][b] == coef(0)) continue;
        result.push_back({powers[a], powers[b], residual[a][b]});
    }
    return result;
}

// Exact rectangular prefix of (L(x)R(y))*M(x,y), where M is diagonal.
// All queried arguments are floor(N/k); quotient block endpoints stay in
// D(N). This direct grouped query costs O(min(A,B,sqrt(A)+sqrt(B))).
inline coef pn2d_diagonal_rectangle(const BlockPrefix& left,
                                     const BlockPrefix& right,
                                     const BlockPrefix& middle,
                                     std::int64_t a, std::int64_t b) {
    same_grid(left, right);
    same_grid(left, middle);
    if (a < 1 || b < 1) return 0;
    coef result = 0;
    for (std::int64_t k = 1, end; k <= std::min(a, b); k = end + 1) {
        std::int64_t qa = a / k, qb = b / k;
        end = std::min(a / qa, b / qb);
        result += (middle.at(end) - middle.at(k - 1))
                * left.at(qa) * right.at(qb);
    }
    return result;
}

// Dynamic S_p(x,m)=sum_{k<=x} M(k) Side(floor(m/k)). Both orientations use
// the canonical BlockSums representation on D(N). For k<=sqrt(m) its value
// occupies a singleton; larger k are grouped by t=floor(m/k).
class PN2DRectState {
    const BlockPrefix& left_;
    const BlockPrefix& right_;
    const BlockPrefix& middle_;
    BlockSums for_left_, for_right_;
    static int square_root(std::int64_t x) {
        return pn2d_sqrt(x);
    }
    void apply(BlockSums& blocks, const BlockPrefix& side,
               std::int64_t m, int k, coef sign) {
        if (m < 1 || k < 1 || k > square_root(m)) return;
        int root = square_root(m);
        blocks.add_fast(k - 1, sign * middle_.point(k) * side.at(m / k));
        std::int64_t upper = m / k;
        std::int64_t lower = std::max<std::int64_t>(root, m / (k + 1));
        if (upper > lower)
            blocks.add_fast(middle_.grid().index(upper),
                            sign * side.at(k)
                          * (middle_.at(upper) - middle_.at(lower)));
    }
    coef short_prefix(BlockSums& blocks, const BlockPrefix& side,
                      std::int64_t x, std::int64_t m) {
        if (!x) return 0;
        if (x <= square_root(m)) return blocks.prefix_fast(x);
        std::int64_t t = m / x;
        std::int64_t upper = m / t;
        return blocks.prefix_fast(upper) - side.at(t)
             * (middle_.at(upper) - middle_.at(x));
    }
public:
    PN2DRectState(const BlockPrefix& left, const BlockPrefix& right,
                  const BlockPrefix& middle)
        : left_(left), right_(right), middle_(middle),
          for_left_(left.grid()), for_right_(left.grid()) {}
    static int root(std::int64_t m) { return square_root(m); }
    void change(std::int64_t old_m, std::int64_t new_m, int k) {
        apply(for_left_, left_, old_m, k, coef(0) - coef(1));
        apply(for_right_, right_, old_m, k, coef(0) - coef(1));
        apply(for_left_, left_, new_m, k, coef(1));
        apply(for_right_, right_, new_m, k, coef(1));
    }
    coef rectangle(std::int64_t a, std::int64_t m, bool left_is_short) {
        const BlockPrefix& short_side = left_is_short ? left_ : right_;
        const BlockPrefix& long_side = left_is_short ? right_ : left_;
        BlockSums& blocks = left_is_short ? for_right_ : for_left_;
        coef result = 0;
        for (std::int64_t k = 1, end; k <= a; k = end + 1) {
            std::int64_t q = a / k;
            end = a / q;
            result += (short_side.at(end) - short_side.at(k - 1))
                    * short_prefix(blocks, long_side, q, m);
        }
        return result;
    }
};

struct PN2DQuery {
    std::uint32_t index_with_orientation;
    coef weight;
};

// Strict O(N^(2/3)) arithmetic bound using offline sparse h' terms and
// quotient-block updates of S_p. The materialized term buckets currently
// take O(N^(2/3)) space; this is a time-focused implementation.
template<class Bell>
inline coef pn2d_sparse_offline(const BlockPrefix& left,
                               const BlockPrefix& right,
                               const BlockPrefix& middle,
                               Bell bell) {
    same_grid(left, right);
    same_grid(left, middle);
    const auto& grid = left.grid();
    const auto& xs = grid.values();
    std::int64_t n = grid.limit();
    number_theory::Sieve sieve(grid.root());
    std::vector<PN2DLocalPrime> local;
    for (int p : sieve.primes) {
        auto terms = pn2d_local_sparse(p, n, bell);
        if (!terms.empty()) local.push_back({p, std::move(terms)});
    }
    std::vector<std::vector<PN2DQuery>> queries(xs.size());
    auto collect = [&](auto&& self, std::size_t start, std::int64_t x,
                       std::int64_t y, coef weight) -> void {
        bool left_short = x >= y;
        std::int64_t a = n / std::max(x, y), m = n / std::min(x, y);
        std::uint32_t encoded = std::uint32_t(grid.index(a))
                              | (left_short ? 0x80000000U : 0U);
        queries[grid.index(m)].push_back({encoded, weight});
        std::int64_t bound = pn2d_next_prime_bound(n / x, n / y);
        for (std::size_t i = start; i < local.size(); ++i) {
            std::int64_t p = local[i].prime;
            if (p > bound) break;
            for (const auto& term : local[i].terms)
                if (term.x <= n / x && term.y <= n / y)
                    self(self, i + 1, x * term.x, y * term.y,
                         weight * term.value);
        }
    };
    collect(collect, 0, 1, 1, coef(1));

    int cube = 1;
    while (1LL * (cube + 1) * (cube + 1) * (cube + 1) <= n) ++cube;
    const std::int64_t transition = 1LL * cube * cube;
    PN2DRectState state(left, right, middle);
    coef answer = 0;
    auto answer_at = [&](std::size_t j) {
        for (const auto& item : queries[j])
            answer += item.weight * state.rectangle(
                xs[item.index_with_orientation & 0x7fffffffU], xs[j],
                bool(item.index_with_orientation & 0x80000000U));
    };
    // Calendar chunks require O(sqrt(N)) working space. Scanning the
    // possible divisors once per chunk costs O(sqrt(N) log N); events
    // satisfying k>=the current quotient gap total O(N^(2/3)).
    int chunk = std::max(1, grid.root() / (2 + int(std::log2((long double)n))));
    std::vector<int> seen(grid.root() + 3, -1);
    std::size_t first = 0;
    while (first < xs.size() && xs[first] <= transition) {
        std::int64_t before = first ? xs[first - 1] : 0;
        std::size_t last = first;
        while (last + 1 < xs.size() && xs[last + 1] <= transition &&
               xs[last + 1] - before <= chunk) ++last;
        std::int64_t finish = xs[last];
        std::vector<int> destination(std::size_t(finish - before + 1));
        std::size_t j = first;
        for (std::int64_t value = before + 1; value <= finish; ++value) {
            while (xs[j] < value) ++j;
            destination[std::size_t(value - before)] = int(j - first);
        }
        std::vector<std::vector<int>> events(last - first + 1);
        std::int64_t minimum_gap = xs[first] - before;
        for (std::size_t t = first + 1; t <= last; ++t)
            minimum_gap = std::min(minimum_gap, xs[t] - xs[t - 1]);
        int upper_k = PN2DRectState::root(finish) + 1;
        for (int k = int(minimum_gap); k <= upper_k; ++k) {
            std::int64_t initial = (before / k + 1) * k;
            for (std::int64_t value = initial; value <= finish; value += k) {
                int slot = destination[std::size_t(value - before)];
                std::size_t idx = first + slot;
                std::int64_t gap = xs[idx] - (idx ? xs[idx - 1] : 0);
                if (k >= gap && k <= PN2DRectState::root(xs[idx]) + 1) {
                    events[slot].push_back(k);
                    if (k > 1) events[slot].push_back(k - 1);
                }
            }
        }
        for (std::size_t idx = first; idx <= last; ++idx) {
            std::int64_t old_m = idx ? xs[idx - 1] : 0;
            std::int64_t m = xs[idx];
            int limit = PN2DRectState::root(m) + 1;
            int direct = int(std::min<std::int64_t>(limit, m - old_m));
            for (int k = 1; k < direct; ++k) {
                seen[k] = int(idx);
                state.change(old_m, m, k);
            }
            int old_r = PN2DRectState::root(old_m);
            for (int k : {old_r, old_r + 1, limit - 1, limit})
                if (k >= 1 && k <= limit && seen[k] != int(idx)) {
                    seen[k] = int(idx);
                    state.change(old_m, m, k);
                }
            for (int k : events[idx - first])
                if (k >= 1 && k <= limit && seen[k] != int(idx)) {
                    seen[k] = int(idx);
                    state.change(old_m, m, k);
                }
            answer_at(idx);
        }
        first = last + 1;
    }
    for (std::size_t idx = first; idx < xs.size(); ++idx) {
        std::int64_t old_m = idx ? xs[idx - 1] : 0;
        int limit = PN2DRectState::root(xs[idx]) + 1;
        for (int k = 1; k <= limit; ++k) state.change(old_m, xs[idx], k);
        answer_at(idx);
    }
    return answer;
}

// The sparse part h'=f/(L*R*M) has local support only at exponents
// (a,b) with a,b>=1 and a!=b. The enumeration uses O(N^(2/3)) nodes.
// This direct-query kernel additionally pays O(N^(2/3) log N) for the
// grouped rectangular queries in the worst case. A separate fast-query
// data structure is required to remove that logarithm; do not advertise
// this kernel as the strict O(N^(2/3)) algorithm.
template<class Bell>
inline coef pn2d_sparse_direct(const BlockPrefix& left,
                              const BlockPrefix& right,
                              const BlockPrefix& middle,
                              Bell bell) {
    same_grid(left, right);
    same_grid(left, middle);
    const std::int64_t n = left.grid().limit();
    number_theory::Sieve sieve(left.grid().root());
    std::vector<PN2DLocalPrime> local;
    for (int p : sieve.primes) {
        auto terms = pn2d_local_sparse(p, n, bell);
        if (!terms.empty()) local.push_back({p, std::move(terms)});
    }
    coef result = 0;
    auto visit = [&](auto&& self, std::size_t start, std::int64_t x,
                     std::int64_t y, coef weight) -> void {
        result += weight * pn2d_diagonal_rectangle(
            left, right, middle, n / x, n / y);
        std::int64_t bound = pn2d_next_prime_bound(n / x, n / y);
        for (std::size_t i = start; i < local.size(); ++i) {
            std::int64_t p = local[i].prime;
            if (p > bound) break;
            for (const auto& term : local[i].terms) {
                if (term.x <= n / x && term.y <= n / y)
                    self(self, i + 1, x * term.x, y * term.y,
                         weight * term.value);
            }
        }
    };
    visit(visit, 0, 1, 1, coef(1));
    return result;
}

} } // namespace mal::dgf
