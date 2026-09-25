#pragma once
#include "euler_product.hpp"

namespace mal { namespace dgf {

inline coef polynomial_at(const std::vector<coef>& a, int x) {
    coef result = 0;
    for (auto it = a.rbegin(); it != a.rend(); ++it) result = result * coef(x) + *it;
    return result;
}

// Lagrange interpolation of 1^k+...+x^k over the field 998244353.
// Only small polynomial degrees are intended (the ordinary Min25 setting).
inline coef power_sum(std::int64_t x, int k) {
    if (k < 0 || x < 0 || k + 2 >= 998244353)
        throw std::invalid_argument("Min25: degree or argument out of range");
    // For 0<=k<p-1 every complete block of p residues sums to zero.
    // Reduce x only when evaluating the Faulhaber polynomial in F_p.
    int residue = int(x % 998244353);
    int m = k + 2;
    std::vector<coef> y(m + 1), fact(m + 1), invfact(m + 1), left(m + 2), right(m + 2);
    for (int i = 1; i <= m; ++i) y[i] = y[i - 1] + coef(i).pow(k);
    if (residue <= m) return y[residue];
    fact[0] = 1;
    for (int i = 1; i <= m; ++i) fact[i] = fact[i - 1] * coef(i);
    invfact[m] = fact[m].inv();
    for (int i = m; i >= 1; --i) invfact[i - 1] = invfact[i] * coef(i);
    left[0] = right[m + 1] = 1;
    for (int i = 0; i <= m; ++i) left[i + 1] = left[i] * coef(residue - i);
    for (int i = m; i >= 0; --i) right[i] = right[i + 1] * coef(residue - i);
    coef sum = 0;
    for (int i = 0; i <= m; ++i) {
        coef term = y[i] * left[i] * right[i + 1] * invfact[i] * invfact[m - i];
        sum += (m - i) % 2 ? coef(0) - term : term;
    }
    return sum;
}

// The easy id_k block sieve, used as the starting point of both prime-prefix
// routes. Only quotient endpoints are materialized.
inline BlockPrefix id_k_block(const QuotientGrid& grid, int degree) {
    if (degree < 0) throw std::invalid_argument("id_k block: negative degree");
    BlockPrefix result(grid);
    for (std::size_t i = 0; i < result.size(); ++i)
        result[i] = power_sum(grid.values()[i], degree);
    return result;
}

inline BlockPrefix block_exp(const BlockPrefix& q, const series& dense_q) {
    const QuotientGrid& grid = q.grid();
    int t = block_threshold(grid, dense_q);
    if (q.at(1) != coef(0) || dense_q[1] != coef(0))
        throw std::invalid_argument("block exp: q(1) must be zero");
    BlockPrefix answer(grid), term(grid);
    for (std::size_t i = 0; i < term.size(); ++i) term[i] = answer[i] = 1;
    series dense_term(t + 1); dense_term[1] = 1;
    // Minimum nonzero index of q is at least 2: q^k vanishes for 2^k>N.
    for (int k = 1; ; ++k) {
        dense_term = mul(dense_term, dense_q);
        coef inv_k = coef(k).inv();
        term = block_convolve(term, q, dense_term);
        for (int x = 1; x <= t; ++x) dense_term[x] *= inv_k;
        for (std::size_t i = 0; i < answer.size(); ++i) {
            term[i] *= inv_k;
            answer[i] += term[i];
        }
        if (k >= 62 || (std::int64_t(1) << k) > grid.limit() / 2) break;
    }
    return answer;
}

// Pure block-sieve exponential: no dense prefix beyond D(N), O(sqrt N)
// working space. Uses O(log N) exact hyperbola block convolutions.
inline BlockPrefix block_exp(const BlockPrefix& q) {
    const QuotientGrid& grid = q.grid();
    if (q.at(1) != coef(0))
        throw std::invalid_argument("block exp: q(1) must be zero");
    BlockPrefix answer(grid), term(grid);
    for (std::size_t i = 0; i < answer.size(); ++i)
        answer[i] = term[i] = 1;
    for (int k = 1; ; ++k) {
        term = block_convolve(term, q);
        coef inverse = coef(k).inv();
        for (std::size_t i = 0; i < term.size(); ++i) {
            term[i] *= inverse;
            answer[i] += term[i];
        }
        if (k >= 62 || (std::int64_t(1) << k) > grid.limit() / 2) break;
    }
    return answer;
}

// Formal Dirichlet ln from quotient endpoints only. Let q=ln_*(f), retain
// q0 on [1,sqrt(N)], and set h=exp_*(q0). The tail q1=q-q0 squares to zero,
// so f/h=epsilon+q1. One exact block division recovers q1. This generic
// route costs O(log N) block convolutions plus one short-tail block division,
// with O(sqrt N) space. It makes no multiplicativity assumption.
inline BlockPrefix block_log(const BlockPrefix& f) {
    const QuotientGrid& grid = f.grid();
    if (f.at(1) != coef(1))
        throw std::invalid_argument("block log: f(1) must be one");
    int root = grid.root();
    series dense(root + 1);
    for (int x = 1; x <= root; ++x) dense[x] = f.point(x);
    series small_log = ln(dense);
    BlockPrefix q0(grid);
    coef running = 0;
    for (std::size_t i = 0; i < q0.size(); ++i) {
        std::int64_t x = grid.values()[i];
        if (x <= root) running += small_log[int(x)];
        q0[i] = running;
    }
    BlockPrefix h = block_exp(q0);
    BlockPrefix quotient = dujiao_zak(h, f);
    for (std::size_t i = 0; i < q0.size(); ++i)
        q0[i] += quotient[i] - coef(1);
    return q0;
}

} } // namespace mal::dgf
