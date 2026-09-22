#include "../../include/hp/bigfloat.hpp"
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <boost/multiprecision/number.hpp>
#include <cmath>
#include <iostream>
#include <string>

using boost::multiprecision::cpp_dec_float;
using dec = boost::multiprecision::number<cpp_dec_float<220>>;
using dec_wide = boost::multiprecision::number<cpp_dec_float<12000>>;
using mal::BigFloat;

static dec to_dec(const BigFloat& x) {
    return dec(x.to_string(180));
}

static bool close(const BigFloat& got, const dec& want, const char* what, double eps = 1e-140) {
    dec g = to_dec(got);
    dec d = abs(g - want);
    dec scale = abs(want) > 1 ? abs(want) : dec(1);
    if (d / scale <= eps) return true;
    std::cerr << "FAIL " << what << "\n got " << g << "\n want " << want
              << "\n err " << (d / scale) << "\n";
    return false;
}

int main() {
    const int p = 512;
    bool ok = true;

    const char* exps[] = {"-10", "-2.5", "-0.125", "-0.001", "0.001", "0.5", "1", "3.75", "10"};
    for (const char* s : exps) {
        BigFloat x(s, p);
        BigFloat y = BigFloat::exp(x);
        if (!close(y, exp(dec(s)), "exp")) ok = false;
    }

    const char* logs[] = {"0.001", "0.125", "0.5", "1", "1.5", "2", "10", "12345.6789"};
    for (const char* s : logs) {
        BigFloat x(s, p);
        BigFloat y = BigFloat::log(x);
        if (!close(y, log(dec(s)), "log")) ok = false;
    }

    for (const char* s : exps) {
        BigFloat x(s, p);
        BigFloat y = BigFloat::log(BigFloat::exp(x));
        if (!close(y, dec(s), "log(exp(x))", 1e-135)) ok = false;
    }
    for (const char* s : logs) {
        BigFloat x(s, p);
        BigFloat y = BigFloat::exp(BigFloat::log(x));
        if (!close(y, dec(s), "exp(log(x))", 1e-135)) ok = false;
    }

    BigFloat a("123456789.25", p), b("-98765.125", p);
    if (!close(a + b, dec("123456789.25") + dec("-98765.125"), "add")) ok = false;
    if (!close(a - b, dec("123456789.25") - dec("-98765.125"), "sub")) ok = false;
    if (!close(a * b, dec("123456789.25") * dec("-98765.125"), "mul")) ok = false;
    if (!close(a / b, dec("123456789.25") / dec("-98765.125"), "div")) ok = false;
    if (!close(BigFloat::pow(a, 5), pow(dec("123456789.25"), 5), "pow+")) ok = false;
    if (!close(BigFloat::pow(a, -3), pow(dec("123456789.25"), -3), "pow-")) ok = false;

    {
        dec pi_want = acos(dec(-1));
        dec ln2_want = log(dec(2));
        if (!close(BigFloat::pi(p), pi_want, "pi", 1e-140)) ok = false;
        if (!close(BigFloat::ln2(p), ln2_want, "ln2", 1e-140)) ok = false;
    }

    // Newton reciprocal path for large precision.
    BigFloat a_big("123456789.25", 16384), b_big("-98765.125", 16384);
    BigFloat q_big = a_big / b_big;
    dec_wide g_big(q_big.to_string(4200));
    dec_wide w_big = dec_wide("123456789.25") / dec_wide("-98765.125");
    if (abs(g_big - w_big) / abs(w_big) > dec_wide("1e-4100")) {
        std::cerr << "FAIL newton div\n";
        ok = false;
    }

    // Fast decimal string conversion path.
    {
        std::string s = "1.";
        for (int i = 0; i < 5000; ++i) s.push_back(char('0' + (i * 7 + 3) % 10));
        BigFloat x(s, 17000);
        dec_wide g(x.to_string(5000));
        dec_wide w(s);
        if (abs(g - w) / abs(w) > dec_wide("1e-4900")) {
            std::cerr << "FAIL fast decimal string\n";
            ok = false;
        }
    }

    // AGM exp/log path identity at large precision.
    {
        BigFloat x("1.25", 1 << 16);
        BigFloat y = BigFloat::log(BigFloat::exp(x));
        BigFloat d = y - x;
        if (!d.is_zero() && d.order() > -60000) {
            std::cerr << "FAIL agm exp/log identity\n";
            ok = false;
        }
    }

    std::cout << (ok ? "bigfloat: ok" : "bigfloat: FAILED") << '\n';
    return ok ? 0 : 1;
}
