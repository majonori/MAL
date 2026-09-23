from pathlib import Path
import shutil

HERE = Path(__file__).resolve().parent
ROOT = HERE.parent.parent

# The interactive library is now the normal MAL bundle.  The remote module
# inside it exports the small value interface and all four operators.
shutil.copyfile(ROOT / "bundles" / "interactive_lib.cpp", HERE / "interactive_lib.cpp")

main_source = r'''#include <bits/stdc++.h>

namespace mal {
namespace remote {

struct BigInt {
    std::string s;
    BigInt(const std::string& x = "0") : s(x) {}
    BigInt(long long x) : s(std::to_string(x)) {}
    std::string to_string() const;
};

BigInt operator+(const BigInt& a, const BigInt& b);
BigInt operator-(const BigInt& a, const BigInt& b);
BigInt operator*(const BigInt& a, const BigInt& b);
BigInt operator/(const BigInt& a, const BigInt& b);
std::ostream& operator<<(std::ostream& os, const BigInt& x);

struct BigFloat {
    std::string s;
    int p;
    BigFloat(const std::string& x = "0", int p_ = 256) : s(x), p(p_) {}
    BigFloat(long long x, int p_ = 256) : s(std::to_string(x)), p(p_) {}
    std::string to_string(int digits = -1) const;
};

BigFloat operator+(const BigFloat& a, const BigFloat& b);
BigFloat operator-(const BigFloat& a, const BigFloat& b);
BigFloat operator*(const BigFloat& a, const BigFloat& b);
BigFloat operator/(const BigFloat& a, const BigFloat& b);
std::ostream& operator<<(std::ostream& os, const BigFloat& x);

} // namespace remote

using remote::BigInt;
using remote::BigFloat;

} // namespace mal

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::string a, b;
    std::cin >> a >> b;

    mal::BigInt x(a), y(b);
    std::cout << (x + y) << '\n';
    return 0;
}
'''

(HERE / "main.cpp").write_text(main_source, encoding="utf-8")
