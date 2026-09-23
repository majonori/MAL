# T793310 MAL 接口测试

本目录使用**方案 B**：

- MAL 核心实现留在 `interactive_lib.cpp` 内部；
- 向选手暴露一个很薄的值包装接口；
- 选手不需要 include 交互库，也不需要复制 MAL 实现；
- 选手可以直接使用 `mal::BigInt`、`mal::BigFloat` 的 `+ - * /`。

## 选手代码接口

洛谷题目模板中预置以下声明，选手只写 `main()`：

```cpp
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

} // namespace remote

using remote::BigInt;
using remote::BigFloat;

} // namespace mal
```

注意：必须在选手源码中看到这段声明。`interactive_lib.cpp` 只参与链接，
不会出现在选手源码目录。如果提交时只写 `main()`，就会报：

```text
错误：'mal' 未声明
```

如果题目没有配置代码模板，请把下面的接口块一起复制到提交代码中。

## 完整提交模板

```cpp
#include <bits/stdc++.h>

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
    std::cout << (x + y).to_string() << '\n';
    return 0;
}
```

## 选手示例

```cpp
#include <bits/stdc++.h>

// 这里放入上面的接口声明块

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::string a, b;
    std::cin >> a >> b;

    mal::BigInt x(a), y(b);
    std::cout << (x + y).to_string() << '\n';
    return 0;
}
```

也可以直接使用 BigFloat：

```cpp
mal::BigFloat x("1.5", 256), y("2.5", 256);
mal::BigFloat z = (x + y) * y / x - y;
std::cout << z.to_string(20) << '\n';
```

## 交互库

`interactive_lib.cpp` 由 `generate_interface.py` 复制 MAL 的标准发布产物：

- MAL 核心实现仍在 `namespace mal` 中；
- `include/remote/` 提供 `mal::remote` 薄包装类型；
- `mal::remote::operator+ - * /` 是非 inline 导出符号；
- 选手通过 `using remote::BigInt` / `using remote::BigFloat` 使用。

重新生成：

```bash
python3 examples/T793310/generate_interface.py
```

检查符号：

```bash
g++ -std=c++14 -O2 -c examples/T793310/interactive_lib.cpp \
    -o /tmp/t793310.o
nm -C /tmp/t793310.o | grep 'mal::remote::operator'
```

## 数据

本目录包含 10 组数据：

```text
1.in  1.ans
2.in  2.ans
...
10.in 10.ans
```

打包后的数据文件为 `data.zip`，上传洛谷即可。压缩包根目录只包含：

```text
interactive_lib.cpp
1.in 1.ans
...
10.in 10.ans
```

## 本地测试

```bash
cd examples/T793310

g++ -std=c++14 -O2 interactive_lib.cpp main.cpp -o main
for i in $(seq 1 10); do
    ./main < "$i.in" | diff - "$i.ans"
done
```
