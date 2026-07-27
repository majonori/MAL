面向信息学竞赛的数学算法库。  
包含大量小常数压行算法模板，赛题的交互式接口的模板就是基于本仓库。

## 规范

```text
MAL/
├── src/                             # 模块化开发源文件
│   ├── common/
│   │   ├── consts.hpp               # 类型别名与常用常量
│   │   └── modint.hpp               # 静态模数类模板（默认 998244353）
│   ├── polynomial/                  # 多项式全家桶
│   │   ├── ntt.hpp                  # NTT
│   │   ├── fft.hpp                  # FFT
│   │   └── polynomial.hpp           # 总头文件，包含上述全部
│   └── ...                          # 未来可扩展的数论、线性代数等
├── snippets/
│   └── embed/                       # 出题人放在交互库里的代码
│       └── polynomial_full.hpp      # 完整多项式库
├── tests/                           # 示例代码
│   ├── polynomial/
│   │   ├── test_ntt.cpp
│   │   ├── test_conv.cpp
│   │   ├── test_inv.cpp
│   │   └── ...
│   └── run_all.sh
├── scripts/
│   └── gen_embed.py                 # 把嵌入交互库的代码压行的脚本
├── README.md
└── README_EN.md
```
