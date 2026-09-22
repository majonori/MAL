#include "../../bundles/hp/main.cpp"
#include <iostream>
#include <string>

using mal::BigInt;

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    unsigned long long m;
    std::string s;
    if (!(std::cin >> m >> s)) return 0;
    BigInt n(s);
    std::cout << n.nroot(m).to_string() << '\n';
    return 0;
}
