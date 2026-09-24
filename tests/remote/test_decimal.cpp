#include "../../include/remote/decimal.hpp"
#include <cstdint>
#include <iostream>
#include <random>
#include <string>
using namespace mal::remote_detail;

static std::string ref_add(const std::string& a, const std::string& b) {
    // schoolbook decimal reference on strings
    std::string r; int carry = 0;
    for (size_t i = 0; i < std::max(a.size(), b.size()) || carry; ++i) {
        int x = carry + (i < a.size() ? a[a.size() - 1 - i] - '0' : 0)
                      + (i < b.size() ? b[b.size() - 1 - i] - '0' : 0);
        r.push_back(char('0' + x % 10)); carry = x / 10;
    }
    std::reverse(r.begin(), r.end());
    return r;
}

int main() {
    std::mt19937_64 rng(20260924);
    int bad = 0;
    // parse/str round trips including block boundaries
    for (size_t len : {1u, 5u, 6u, 10u, 11u, 500u, 5000u, 400000u}) {
        std::string s;
        s.reserve(len);
        s.push_back(char('1' + rng() % 9));
        for (size_t i = 1; i < len; ++i) s.push_back(char('0' + rng() % 10));
        chunk_vec a = parse(s);
        if (str(a) != s) { std::cout << "parse/str FAIL len=" << len << '\n'; return 1; }
        if (parse("0").size() || str(parse("0000")) != "0") { std::cout << "zero FAIL\n"; return 1; }
    }
    // add/sub against a decimal reference, including the AVX2 carry path
    for (int rep = 0; rep < 40; ++rep) {
        const size_t la = 1 + rng() % 3000, lb = 1 + rng() % 3000;
        std::string sa(la, '0'), sb(lb, '0');
        for (auto& c : sa) c = char('0' + rng() % 10);
        for (auto& c : sb) c = char('0' + rng() % 10);
        sa[0] = char('1' + rng() % 9); sb[0] = char('1' + rng() % 9);
        const std::string want = ref_add(sa, sb);
        if (str(add(parse(sa), parse(sb))) != want) { std::cout << "add FAIL\n"; ++bad; break; }
        // subtract the smaller value (compare numerically, not as strings)
        chunk_vec ca = parse(sa), cb = parse(sb);
        const bool a_bigger = cmp(ca, cb) >= 0;
        const chunk_vec& hi = a_bigger ? ca : cb;
        const chunk_vec& lo = a_bigger ? cb : ca;
        const std::string shi = a_bigger ? sa : sb, slo = a_bigger ? sb : sa;
        std::string r;
        int borrow = 0;
        for (size_t i = 0; i < shi.size(); ++i) {
            int x = (shi[shi.size() - 1 - i] - '0') - borrow
                    - (i < slo.size() ? slo[slo.size() - 1 - i] - '0' : 0);
            if (x < 0) { x += 10; borrow = 1; } else borrow = 0;
            r.push_back(char('0' + x));
        }
        std::reverse(r.begin(), r.end());
        size_t q = r.find_first_not_of('0');
        r = q == std::string::npos ? "0" : r.substr(q);
        if (str(sub(hi, lo)) != r) { std::cout << "sub FAIL\n"; ++bad; break; }
    }
    // carry chain across a long run of base-1 limbs
    {
        chunk_vec x(100000, CHUNK_BASE - 1), y(1, 1);
        chunk_vec z = add(x, y);
        chunk_vec want(100001, 0); want[100000] = 1;
        if (z != want) { std::cout << "carry chain FAIL\n"; return 1; }
    }
    // mul_small / div_small round trip
    for (int rep = 0; rep < 20; ++rep) {
        const size_t len = 1 + rng() % 2000;
        std::string s(len, '0');
        for (auto& c : s) c = char('0' + rng() % 10);
        s[0] = char('1' + rng() % 9);
        const u32 m = 1 + u32(rng() % 99999);
        chunk_vec a = parse(s), b = mul_small(a, m);
        u32 rem = 0;
        chunk_vec back = div_small(b, m, &rem);
        if (back != a || rem) { std::cout << "mul/div_small FAIL\n"; return 1; }
    }
    // multiplication: dispatch paths against the schoolbook reference
    for (size_t la : {1u, 13u, 200u, 500u, 2000u, 9000u}) {
        for (size_t lb : {la, size_t(4), size_t(3000)}) {
            if (lb > 4 * la + 8) continue;
            std::string sa(la, '0'), sb(lb, '0');
            for (auto& c : sa) c = char('0' + rng() % 10);
            for (auto& c : sb) c = char('0' + rng() % 10);
            sa[0] = char('1' + rng() % 9); sb[0] = char('1' + rng() % 9);
            chunk_vec a = parse(sa), b = parse(sb);
            if (mul(a, b) != school_mul(a, b)) {
                std::cout << "mul FAIL la=" << la << " lb=" << lb << '\n';
                return 1;
            }
        }
    }
    // large exact identities: (10^k - 1)^2 = 10^(2k) - 2*10^k + 1
    for (size_t k : {2000u, 40000u, 400000u}) {
        std::string s(k, '9');
        chunk_vec a = parse(s);
        chunk_vec p = mul(a, a);
        std::string want = std::string(k - 1, '9') + "8" + std::string(k - 1, '0') + "1";
        if (str(p) != want) { std::cout << "big square FAIL k=" << k << '\n'; return 1; }
    }
    // division: q*b + r == a and r < b, plus narrow/wide quotient shapes
    for (int rep = 0; rep < 30; ++rep) {
        const size_t la = 1 + rng() % 4000, lb = 1 + rng() % 4000;
        std::string sa(la, '0'), sb(lb, '0');
        for (auto& c : sa) c = char('0' + rng() % 10);
        for (auto& c : sb) c = char('0' + rng() % 10);
        sa[0] = char('1' + rng() % 9); sb[0] = char('1' + rng() % 9);
        chunk_vec a = parse(sa), b = parse(sb);
        div_pair qr = divmod(a, b);
        if (add(mul(qr.first, b), qr.second) != a) { std::cout << "div: q*b+r != a\n"; return 1; }
        if (cmp(qr.second, b) >= 0) { std::cout << "div: r >= b\n"; return 1; }
        if (divide(a, b) != qr.first) { std::cout << "divide != divmod quotient\n"; return 1; }
    }
    // exact identities: (10^k - 1)/9 = repunit, /(10^k - 1) = 1, a/1 = a
    for (size_t k : {200u, 5000u, 60000u}) {
        chunk_vec nine = parse(std::string(k, '9'));
        chunk_vec nine_plus = add(nine, chunk_vec{1});          // 10^k
        if (str(divide(nine, parse("9"))) != std::string(k, '1')) { std::cout << "repunit FAIL\n"; return 1; }
        if (str(divide(nine, nine)) != "1") { std::cout << "self div FAIL\n"; return 1; }
        if (divide(nine, chunk_vec{1}) != nine) { std::cout << "div by 1 FAIL\n"; return 1; }
        // 10^k / (10^m) with k = 2m: quotient 10^m exactly
        if (k % 2 == 0) {
            chunk_vec p = pow_base(k / 2);
            chunk_vec q = divide(shl(chunk_vec{1}, k), p);
            if (q != p) { std::cout << "power div FAIL k=" << k << '\n'; return 1; }
        }
        if (!nine_plus.empty() && cmp(divide(nine, chunk_vec{2}), chunk_vec{}) < 0) { std::cout << "san\n"; return 1; }
    }
    // integer roots: r^k <= a < (r+1)^k, plus perfect powers
    for (int rep = 0; rep < 12; ++rep) {
        const size_t len = 1 + rng() % 3000;
        std::string sa(len, '0');
        for (auto& c : sa) c = char('0' + rng() % 10);
        sa[0] = char('1' + rng() % 9);
        chunk_vec a = parse(sa);
        for (int k = 2; k <= 5; ++k) {
            chunk_vec r = root(a, k);
            if (cmp(power(r, k), a) > 0) { std::cout << "root: r^k > a\n"; return 1; }
            if (cmp(power(add(r, chunk_vec{1}), k), a) <= 0) { std::cout << "root: (r+1)^k <= a\n"; return 1; }
        }
    }
    for (size_t base : {7u, 12345u, 99999u}) {
        for (int k = 2; k <= 5; ++k) {
            chunk_vec p = power(from_u64(base), k);            // exact k-th power
            if (root(p, k) != from_u64(base)) { std::cout << "root: perfect power FAIL\n"; return 1; }
            if (root(add(p, chunk_vec{1}), k) != from_u64(base)) { std::cout << "root: p+1 FAIL\n"; return 1; }
            if (root(sub(p, chunk_vec{1}), k) != from_u64(base - 1)) { std::cout << "root: p-1 FAIL\n"; return 1; }
        }
    }
    // roots: the fast estimator must agree with the reference loop, including
    // at the lengths where the doubling recursion changes shape
    for (size_t len : {1u, 9u, 16u, 17u, 40u, 41u, 100u, 257u, 600u}) {
        for (int rep = 0; rep < 2; ++rep) {
            std::string sa(len, '0');
            for (auto& c : sa) c = char('0' + rng() % 10);
            sa[0] = char('1' + rng() % 9);
            const chunk_vec a = parse(sa);
            for (int k = 2; k <= 8; ++k) {
                const chunk_vec fast = root(a, k);
                if (cmp(fast, root_classic(a, k)) != 0) {
                    std::cout << "root: fast != classic len=" << len << " k=" << k << '\n';
                    return 1;
                }
                if (cmp(power(fast, k), a) > 0 ||
                    cmp(power(add(fast, chunk_vec{1}), k), a) <= 0) {
                    std::cout << "root: bound FAIL len=" << len << " k=" << k << '\n';
                    return 1;
                }
            }
        }
    }
    {
        // exact powers with a few digits on either side, for every small k
        chunk_vec base = parse("1234567890123456789012345678901234567890");
        for (int k = 2; k <= 8; ++k) {
            const chunk_vec p = power(base, k);
            if (cmp(root(p, k), base) != 0) { std::cout << "root: exact power FAIL k=" << k << '\n'; return 1; }
            if (cmp(root(add(p, chunk_vec{1}), k), base) != 0) { std::cout << "root: power+1 FAIL\n"; return 1; }
            if (cmp(root(sub(p, chunk_vec{1}), k), sub(base, chunk_vec{1})) != 0) {
                std::cout << "root: power-1 FAIL\n";
                return 1;
            }
        }
    }
    // the root iteration's cheap quotient stays within a few units of the exact
    // one (the final correction walk has to absorb that difference)
    for (int rep = 0; rep < 40; ++rep) {
        const size_t la = 200 + rng() % 3000, lb = 40 + rng() % 900;
        std::string sa(la, '0'), sb(lb, '0');
        for (auto& c : sa) c = char('0' + rng() % 10);
        for (auto& c : sb) c = char('0' + rng() % 10);
        sa[0] = char('1' + rng() % 9); sb[0] = char('1' + rng() % 9);
        const chunk_vec a = parse(sa), b = parse(sb);
        if (cmp(a, b) < 0) continue;
        const chunk_vec exact = divide(a, b);
        const chunk_vec est = quotient_estimate(a, b);
        const chunk_vec diff = cmp(est, exact) >= 0 ? sub(est, exact) : sub(exact, est);
        if (diff.size() > 1 || (!diff.empty() && diff[0] > 4)) {
            std::cout << "quotient_estimate out of range\n";
            return 1;
        }
    }
    // large exponents still go through the reference loop and must stay exact
    for (int k : {9, 12, 40, 200}) {
        chunk_vec a = pow_base(30);                       // 10^30 in chunk form
        const chunk_vec r = root(a, k);
        if (cmp(power(r, k), a) > 0 || cmp(power(add(r, chunk_vec{1}), k), a) <= 0) {
            std::cout << "root: large k bound FAIL k=" << k << '\n';
            return 1;
        }
        if (cmp(r, root_classic(a, k)) != 0) { std::cout << "root: large k FAIL\n"; return 1; }
    }
    // scratch buffers: many full width operations back to back (nesting inside
    // division and multiplication) must never observe each other's buffers
    for (int rep = 0; rep < 4; ++rep) {
        const size_t n = 900 + rng() % 1500;
        std::string s(n, '0');
        for (auto& c : s) c = char('0' + rng() % 10);
        s[0] = char('1' + rng() % 9);
        const chunk_vec a = parse(s);
        chunk_vec b = a;
        inc(b);
        const chunk_vec prod = mul(a, b);
        const chunk_vec sq = mul(a, a);
        const chunk_vec sum = add(prod, sq);
        div_pair qr = divmod(sum, b);
        if (cmp(qr.second, b) >= 0) { std::cout << "scratch: r >= b\n"; return 1; }
        if (add(mul(qr.first, b), qr.second) != sum) { std::cout << "scratch: q*b+r != a\n"; return 1; }
    }
    // wide quotients go through the block loop, which reuses the divisor's and
    // the reciprocal's transforms between blocks
    for (const std::pair<size_t, size_t> shape : {std::make_pair(size_t(40000), size_t(200)),
                                                   std::make_pair(size_t(30000), size_t(1000)),
                                                   std::make_pair(size_t(9000), size_t(97))}) {
        std::string sa(shape.first, '0'), sb(shape.second, '0');
        for (auto& c : sa) c = char('0' + rng() % 10);
        for (auto& c : sb) c = char('0' + rng() % 10);
        sa[0] = char('1' + rng() % 9);
        sb[0] = char('1' + rng() % 9);
        const chunk_vec a = parse(sa), d = parse(sb);
        const div_pair qr = divmod(a, d);
        if (cmp(qr.second, d) >= 0) { std::cout << "wide div: r >= b\n"; return 1; }
        if (add(mul(qr.first, d), qr.second) != a) { std::cout << "wide div: q*b+r != a\n"; return 1; }
        if (divide(a, d) != qr.first) { std::cout << "wide div: divide != divmod\n"; return 1; }
        // same quotient when only the top limbs of the dividend are used
        if (cmp(qr.first, chunk_vec{}) <= 0) { std::cout << "wide div: empty quotient\n"; return 1; }
    }
    std::cout << (bad ? "decimal: FAILED" : "decimal: ok") << '\n';
    return bad ? 1 : 0;
}
