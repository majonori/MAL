#pragma once
#include "multiplicative.hpp"
#include <cmath>
#include <cstdint>

namespace mal { namespace dgf {

// All distinct floor(N/i), in increasing order. Quotients of members of
// D(N) are again members of D(N); the array index is found without a map.
class QuotientGrid {
    std::int64_t n_;
    int root_;
    std::vector<std::int64_t> xs_;
public:
    explicit QuotientGrid(std::int64_t n) : n_(n), root_(0) {
        if (n < 1) throw std::invalid_argument("block sieve: N must be positive");
        if (n > 100000000000000LL)
            throw std::invalid_argument("block sieve: N exceeds supported block-index range");
        while (1LL * (root_ + 1) * (root_ + 1) <= n) ++root_;
        for (int i = 1; i <= root_; ++i) xs_.push_back(i);
        for (int i = int(n / (root_ + 1)); i >= 1; --i) xs_.push_back(n / i);
    }
    std::int64_t limit() const { return n_; }
    int root() const { return root_; }
    const std::vector<std::int64_t>& values() const { return xs_; }
    int index(std::int64_t x) const {
        if (x < 1 || x > n_) throw std::out_of_range("block sieve: invalid value");
        if (x <= root_) return x - 1;
        int i = int(n_ / x);
        int id = int(xs_.size()) - i;
        if (xs_[id] != x) throw std::invalid_argument("block sieve: x not in D(N)");
        return id;
    }
    bool contains(std::int64_t x) const {
        return x >= 1 && x <= n_ && (x <= root_ || n_ / (n_ / x) == x);
    }
};

class BlockPrefix {
    const QuotientGrid* grid_;
    std::vector<coef> sums_;
public:
    explicit BlockPrefix(const QuotientGrid& g) : grid_(&g), sums_(g.values().size()) {}
    BlockPrefix(const QuotientGrid& g, const series& points) : BlockPrefix(g) {
        if (std::int64_t(points.size()) <= g.limit() || points[0] != coef(0))
            throw std::invalid_argument("block sieve: points must cover N");
        coef sum = 0;
        std::size_t i = 0;
        for (std::int64_t x = 1; x <= g.limit(); ++x) {
            sum += points[x];
            if (i < sums_.size() && g.values()[i] == x) sums_[i++] = sum;
        }
    }
    const QuotientGrid& grid() const { return *grid_; }
    coef at(std::int64_t x) const { return x == 0 ? coef(0) : sums_[grid_->index(x)]; }
    coef& operator[](std::size_t i) { return sums_[i]; }
    coef operator[](std::size_t i) const { return sums_[i]; }
    std::size_t size() const { return sums_.size(); }
    coef point(int x) const { return at(x) - at(x - 1); } // x <= root only
};

// Canonical mutable representation: sums over consecutive quotient intervals.
// Static additions are O(1); when prefix queries are needed, a Fenwick tree
// is built once, then updates/queries are O(log |D(N)|). Storage is O(sqrt N).
class BlockSums {
    const QuotientGrid* grid_;
    std::vector<coef> values_, tree_;
    struct FastSlab {
        std::size_t begin, size;
        int base;
        coef total = 0;
        std::vector<std::vector<coef>> levels;
    };
    std::vector<FastSlab> fast_slabs_;
    bool fast_ready_ = false;
    bool dirty_ = true;
    void rebuild() {
        if (!dirty_) return;
        tree_.assign(values_.size() + 1, coef(0));
        for (std::size_t i = 1; i < tree_.size(); ++i) {
            tree_[i] += values_[i - 1];
            std::size_t parent = i + (i & -i);
            if (parent < tree_.size()) tree_[parent] += tree_[i];
        }
        dirty_ = false;
    }
    void build_fast() {
        if (fast_ready_) return;
        fast_slabs_.clear();
        fast_slabs_.reserve(32);
        for (std::size_t begin = 0, width = 1; begin < values_.size();
             begin += width, width *= 2) {
            FastSlab slab;
            slab.begin = begin;
            slab.size = std::min(width, values_.size() - begin);
            slab.base = 2;
            auto eighth_power = [](int b) {
                std::int64_t product = 1;
                for (int i = 0; i < 8; ++i) product *= b;
                return product;
            };
            while (eighth_power(slab.base) < std::int64_t(slab.size))
                ++slab.base;
            slab.levels.reserve(12);
            slab.levels.emplace_back(values_.begin() + begin,
                                     values_.begin() + begin + slab.size);
            for (coef v : slab.levels.front()) slab.total += v;
            while (slab.levels.back().size() > std::size_t(slab.base)) {
                const auto& current = slab.levels.back();
                std::vector<coef> next((current.size() + slab.base - 1) / slab.base);
                for (std::size_t i = 0; i < current.size(); ++i)
                    next[i / slab.base] += current[i];
                slab.levels.push_back(std::move(next));
            }
            fast_slabs_.push_back(std::move(slab));
        }
        fast_ready_ = true;
    }
    void update_fast(std::size_t index, coef value) {
        if (!fast_ready_) return;
        unsigned slab_index = 63U - unsigned(__builtin_clzll(index + 1));
        auto& slab = fast_slabs_[slab_index];
        std::size_t offset = index - slab.begin;
        slab.total += value;
        for (auto& level : slab.levels) {
            level[offset] += value;
            offset /= slab.base;
        }
    }
public:
    explicit BlockSums(const QuotientGrid& grid)
        : grid_(&grid), values_(grid.values().size()) {}
    explicit BlockSums(const BlockPrefix& prefix) : BlockSums(prefix.grid()) {
        coef previous = 0;
        for (std::size_t i = 0; i < values_.size(); ++i) {
            values_[i] = prefix[i] - previous;
            previous = prefix[i];
        }
    }
    const QuotientGrid& grid() const { return *grid_; }
    std::size_t size() const { return values_.size(); }
    coef block(std::size_t index) const { return values_.at(index); }
    coef small_point(int x) const {
        if (x < 1 || x > grid_->root())
            throw std::out_of_range("block sums: point not in singleton range");
        return values_[x - 1];
    }
    void add_static(std::size_t index, coef value) {
        values_.at(index) += value;
        dirty_ = true;
        fast_slabs_.clear();
        fast_ready_ = false;
    }
    void add_block(std::size_t index, coef value) {
        if (dirty_) rebuild();
        values_.at(index) += value;
        update_fast(index, value);
        for (++index; index < tree_.size(); index += index & -index)
            tree_[index] += value;
    }
    void add_fast(std::size_t index, coef value) {
        if (!fast_ready_) build_fast();
        values_.at(index) += value;
        update_fast(index, value);
        dirty_ = true; // Fenwick sums are rebuilt if the other API queries.
    }
    coef prefix_at(std::int64_t x) {
        if (x == 0) return 0;
        if (dirty_) rebuild();
        std::size_t index = grid_->index(x) + 1;
        coef result = 0;
        for (; index; index -= index & -index) result += tree_[index];
        return result;
    }
    // Dyadic slabs, each with eight bounded-degree levels. Updates touch
    // O(1) aggregates; prefix x visits O(x^(1/8)+log x) cells since the
    // index of x in D(N) is O(x). This is the same block-inner-sums class.
    coef prefix_fast(std::int64_t x) {
        if (x == 0) return 0;
        build_fast();
        std::size_t remaining = grid_->index(x) + 1;
        coef result = 0;
        for (const auto& slab : fast_slabs_) {
            if (remaining >= slab.size) {
                result += slab.total;
                remaining -= slab.size;
                if (!remaining) break;
                continue;
            }
            for (const auto& level : slab.levels) {
                if (remaining <= std::size_t(slab.base)) {
                    for (std::size_t i = 0; i < remaining; ++i)
                        result += level[i];
                    break;
                }
                std::size_t tail = remaining % slab.base;
                for (std::size_t i = remaining - tail; i < remaining; ++i)
                    result += level[i];
                remaining /= slab.base;
            }
            break;
        }
        return result;
    }
    BlockPrefix to_prefix() const {
        BlockPrefix result(*grid_);
        coef running = 0;
        for (std::size_t i = 0; i < values_.size(); ++i)
            result[i] = (running += values_[i]);
        return result;
    }
};

inline void same_grid(const BlockPrefix& a, const BlockPrefix& b) {
    if (&a.grid() != &b.grid()) throw std::invalid_argument("block sieve: different grids");
}
inline int block_threshold(const QuotientGrid& g, const series& dense) {
    int t = int(dense.size()) - 1;
    if (dense.empty() || dense[0] != coef(0) || t < g.root() || t > g.limit())
        throw std::invalid_argument("block sieve: dense prefix must cover sqrt(N)");
    return t;
}

// Exact Dirichlet-hyperbola block convolution using only D(N) prefixes.
// O(N^(2/3) + sqrt(N)*log N) field operations, O(sqrt(N)) storage.
// The small-small quadrant uses a<=b and groups consecutive b whose product
// a*b lies in the same quotient interval. The groups per row total
// sum_{a<=N^(1/3)} O(sqrt(N/a)) + sum_{a>N^(1/3)} O(N/a^2)
// = O(N^(2/3)). The two large-small quadrants cost O(sqrt(N) log N).
inline BlockPrefix block_convolve(const BlockPrefix& f, const BlockPrefix& g) {
    same_grid(f, g);
    const QuotientGrid& grid = f.grid();
    std::int64_t n = grid.limit();
    int root = grid.root();
    const auto& xs = grid.values();
    BlockSums blocks(grid);
    std::vector<coef> fp(root + 1), gp(root + 1);
    for (int a = 1; a <= root; ++a) {
        fp[a] = f.point(a);
        gp[a] = g.point(a);
    }
    auto target_block = [&](std::int64_t product) -> int {
        if (product <= root) return int(product - 1);
        return grid.index(n / (n / product));
    };
    for (int a = 1; a <= root; ++a) {
        // The diagonal is counted once. The strictly upper triangle includes
        // the swapped pair, even when neither operand is multiplicative.
        blocks.add_static(target_block(1LL * a * a), fp[a] * gp[a]);
        for (int b = a + 1; b <= root; ) {
            int j = target_block(1LL * a * b);
            int end = int(std::min<std::int64_t>(root, xs[j] / a));
            blocks.add_static(j, fp[a] * (g.at(end) - g.at(b - 1))
                               + gp[a] * (f.at(end) - f.at(b - 1)));
            b = end + 1;
        }
        // If the other factor exceeds sqrt(N), the first factor is small.
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
    return blocks.to_prefix();
}

// Dirichlet hyperbola. dense_h contains h=f*g through T. For arbitrary
// nonmultiplicative h, the caller can supply any O(T) point sieve; this
// kernel then takes O(N/sqrt(T)) time and O(sqrt(N)) extra space.
inline BlockPrefix block_convolve(const BlockPrefix& f, const BlockPrefix& g,
                                 const series& dense_h) {
    same_grid(f, g);
    const QuotientGrid& grid = f.grid();
    int t = block_threshold(grid, dense_h);
    BlockPrefix h(grid);
    coef prefix = 0;
    for (std::size_t i = 0; i < h.size(); ++i) {
        std::int64_t x = grid.values()[i];
        if (x <= t) { // Only values in D(N) are needed here.
            int previous = i ? grid.values()[i - 1] : 0;
            for (int j = previous + 1; j <= x; ++j) prefix += dense_h[j];
            h[i] = prefix;
            continue;
        }
        int r = int(std::sqrt((long double)x));
        while (1LL * (r + 1) * (r + 1) <= x) ++r;
        while (1LL * r * r > x) --r;
        coef sum = coef(0) - f.at(r) * g.at(r);
        for (int j = 1; j <= r; ++j)
            sum += f.point(j) * g.at(x / j) + g.point(j) * f.at(x / j);
        h[i] = sum;
    }
    return h;
}

inline BlockPrefix block_convolve(const BlockPrefix& f, const BlockPrefix& g,
                                 const series& dense_f, const series& dense_g) {
    same_grid(f, g);
    int t = block_threshold(f.grid(), dense_f);
    if (int(dense_g.size()) != t + 1 || dense_g[0] != coef(0))
        throw std::invalid_argument("block sieve: different dense limits");
    // Generic dense preparation costs O(T log T). Supplying dense_h to the
    // overload above removes this step (even for nonmultiplicative h).
    return block_convolve(f, g, mul(dense_f, dense_g));
}

// h need not be multiplicative: the second operand alone is multiplicative.
// Dense preparation O(T log log T), followed by O(N/sqrt(T)) block work.
inline BlockPrefix block_convolve_one_multiplicative(
    const BlockPrefix& f, const BlockPrefix& g,
    const series& dense_f, const series& dense_g) {
    same_grid(f, g);
    int t = block_threshold(f.grid(), dense_f);
    if (int(dense_g.size()) != t + 1 || dense_g[0] != coef(0) || dense_g[1] != coef(1))
        throw std::invalid_argument("block sieve: incompatible dense multiplicative operand");
    // Caller promises that the G block sieve and its dense values agree and
    // that g is multiplicative; validating the entire promise would be O(T).
    return block_convolve(f, g, mul_multiplicative(dense_f, dense_g));
}

// Both operands may be arbitrary. EI's ranked convolution avoids a log(T)
// factor in dense preparation; its constants are substantially larger than
// the ordinary convolution on typical OI-sized inputs.
inline BlockPrefix block_convolve_ranked(
    const BlockPrefix& f, const BlockPrefix& g,
    const series& dense_f, const series& dense_g) {
    same_grid(f, g);
    int t = block_threshold(f.grid(), dense_f);
    if (int(dense_g.size()) != t + 1 || dense_g[0] != coef(0))
        throw std::invalid_argument("block sieve: different dense limits");
    return block_convolve(f, g, fast_mul(dense_f, dense_g));
}

// Traditional quotient-grouped Du Jiao recurrence; dense_a is A=C/B on
// [1,T]. It only needs block prefixes of B and C, including nonmultiplicative.
inline BlockPrefix dujiao_direct(const BlockPrefix& b, const BlockPrefix& c,
                                const series& dense_a) {
    same_grid(b, c);
    const QuotientGrid& grid = b.grid();
    int t = block_threshold(grid, dense_a);
    coef b1 = b.at(1);
    if (b1 == coef(0)) throw std::invalid_argument("block sieve: B(1) must be nonzero");
    coef inv1 = b1.inv(), prefix = 0;
    BlockPrefix a(grid);
    for (std::size_t i = 0; i < a.size(); ++i) {
        std::int64_t x = grid.values()[i];
        if (x <= t) {
            int previous = i ? grid.values()[i - 1] : 0;
            for (int j = previous + 1; j <= x; ++j) prefix += dense_a[j];
            a[i] = prefix;
            continue;
        }
        coef sum = c.at(x);
        for (std::int64_t l = 2, r; l <= x; l = r + 1) {
            std::int64_t q = x / l;
            r = x / q;
            sum -= (b.at(r) - b.at(l - 1)) * a.at(q);
        }
        a[i] = sum * inv1;
    }
    return a;
}

// Zak's short-tail quotient: compute B*A0 by a block convolution, where
// A0=A on [1,T] and zero afterwards. The residual A1 starts after T;
// its recurrence visits only k <= x/T rather than all quotient groups.
inline BlockPrefix dujiao_zak(const BlockPrefix& b, const BlockPrefix& c,
                             const series& dense_b, const series& dense_c) {
    same_grid(b, c);
    const QuotientGrid& grid = b.grid();
    int t = block_threshold(grid, dense_b);
    if (int(dense_c.size()) != t + 1 || dense_c[0] != coef(0))
        throw std::invalid_argument("block sieve: different dense limits");
    if (dense_b[1] == coef(0)) throw std::invalid_argument("block sieve: B(1) must be nonzero");
    series a0 = div(dense_c, dense_b);
    BlockPrefix small(grid);
    coef sum = 0;
    for (std::size_t i = 0; i < small.size(); ++i) {
        std::int64_t x = grid.values()[i];
        if (x <= t) {
            int previous = i ? grid.values()[i - 1] : 0;
            for (int j = previous + 1; j <= x; ++j) sum += a0[j];
        }
        small[i] = sum;
    }
    BlockPrefix product = block_convolve(b, small, dense_c);
    BlockPrefix tail(grid), answer(grid);
    coef inv1 = dense_b[1].inv();
    for (std::size_t i = 0; i < answer.size(); ++i) {
        std::int64_t x = grid.values()[i];
        if (x > t) {
            coef v = c[i] - product[i];
            for (int k = 2; k <= x / (t + 1); ++k)
                v -= dense_b[k] * tail.at(x / k);
            tail[i] = v * inv1;
        }
        answer[i] = small[i] + tail[i];
    }
    return answer;
}

// Direct quotient-group recurrence using ONLY the block sieves of B and C.
// Its O(N^(3/4)) time bound is intentionally separate from the faster
// short-tail method below; both have O(sqrt N) working space.
inline BlockPrefix dujiao_direct(const BlockPrefix& b, const BlockPrefix& c) {
    same_grid(b, c);
    const QuotientGrid& grid = b.grid();
    int root = grid.root();
    series db(root + 1), dc(root + 1);
    for (int i = 1; i <= root; ++i) {
        db[i] = b.point(i);
        dc[i] = c.point(i);
    }
    if (db[1] == coef(0)) throw std::invalid_argument("block sieve: B(1) must be nonzero");
    series da = div(dc, db);
    BlockPrefix a(grid);
    coef prefix = 0, inv1 = db[1].inv();
    for (std::size_t j = 0; j < a.size(); ++j) {
        std::int64_t x = grid.values()[j];
        if (x <= root) {
            a[j] = (prefix += da[x]);
            continue;
        }
        coef value = c[j];
        for (std::int64_t l = 2, r; l <= x; l = r + 1) {
            std::int64_t q = x / l;
            r = x / q;
            value -= (b.at(r) - b.at(l - 1)) * a.at(q);
        }
        a[j] = value * inv1;
    }
    return a;
}

// Short-tail inversion: B*A0 is obtained with the O(N^(2/3)) exact block
// convolution. A1=A-A0 vanishes through sqrt(N); the remaining recurrence
// costs O(sqrt(N) log N), and every temporary has O(sqrt(N)) entries.
inline BlockPrefix dujiao_zak(const BlockPrefix& b, const BlockPrefix& c) {
    same_grid(b, c);
    const QuotientGrid& grid = b.grid();
    int root = grid.root();
    series db(root + 1), dc(root + 1);
    for (int i = 1; i <= root; ++i) {
        db[i] = b.point(i);
        dc[i] = c.point(i);
    }
    if (db[1] == coef(0)) throw std::invalid_argument("block sieve: B(1) must be nonzero");
    series a0 = div(dc, db);
    BlockPrefix small(grid);
    coef prefix = 0;
    for (std::size_t j = 0; j < small.size(); ++j) {
        std::int64_t x = grid.values()[j];
        if (x <= root) prefix += a0[x];
        small[j] = prefix;
    }
    BlockPrefix product = block_convolve(b, small);
    BlockPrefix tail(grid), answer(grid);
    coef inv1 = db[1].inv();
    for (std::size_t j = 0; j < answer.size(); ++j) {
        std::int64_t x = grid.values()[j];
        if (x > root) {
            coef value = c[j] - product[j];
            for (int k = 2; k <= x / (root + 1); ++k)
                value -= db[k] * tail.at(x / k);
            tail[j] = value * inv1;
        }
        answer[j] = small[j] + tail[j];
    }
    return answer;
}

} } // namespace mal::dgf
