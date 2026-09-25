#include "../../include/dgf/dynamic_bell.hpp"
#include <cstdio>
#include <string>

class FastInput {
    char buffer[1 << 16];
    int position = 0, length = 0;
    int next() {
        if (position == length) {
            length = int(std::fread(buffer, 1, sizeof(buffer), stdin));
            position = 0;
            if (!length) return -1;
        }
        return buffer[position++];
    }
public:
    long long integer() {
        int c = next();
        while (c >= 0 && c <= ' ') c = next();
        long long value = 0;
        while (c >= '0' && c <= '9') { value = value * 10 + c - '0'; c = next(); }
        return value;
    }
};

int main() {
    FastInput input;
    int n = input.integer(), operations = input.integer();
    mal::number_theory::Sieve sieve(n);
    mal::dgf::series bell(n + 1);
    for (int p : sieve.primes)
        for (int power = p; power <= n; ) {
            bell[power] = input.integer();
            if (power > n / p) break;
            power *= p;
        }
    mal::dgf::DynamicBellBlock blocks(std::move(sieve), std::move(bell));
    std::string output;
    for (int i = 0; i < operations; ++i) {
        int op = input.integer();
        if (op == 2) {
            output += std::to_string(blocks.answer());
            output += '\n';
        } else {
            int p = input.integer();
            std::vector<mal::dgf::coef> values;
            for (int power = p; power <= n; ) {
                values.emplace_back(input.integer());
                if (power > n / p) break;
                power *= p;
            }
            blocks.update(p, values);
        }
    }
    std::fwrite(output.data(), 1, output.size(), stdout);
}
