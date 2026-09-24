# DGF 模块接入说明

此压缩包基于 MAL 提交 `24bfc0fe6a341dd07b044c49ac9ae5e2f18fef81` 制作。
解压后将 `MAL/` 下的文件按相同路径放进项目根目录：

- `include/dgf/`：可读、可维护的 C++14 实现；
- `include/number_theory/`：线性筛、欧拉/Möbius 表与 64 位素性测试；
- `bundles/dgf/main.cpp`：构建脚本自动生成的单模块压行文件；
- `bundles/interactive_lib.cpp`：加入 DGF 的完整交互库；
- `bundles/dgf/README.md`：选手可以直接复制的声明块与完整接口说明；
- `tests/dgf/test_dgf.cpp`：运算正确性与随机对拍；
- `tests/number_theory/test_number_theory.cpp`：数论算法正确性测试；
- `tests/docs/check_declarations.py`、顶层 README：接入测试与索引。

如果现有仓库的其他模块已有更新，可只覆盖 `include/dgf/`、
`include/number_theory/`、`bundles/dgf/`、`bundles/number_theory/`、
`tests/dgf/` 和 `tests/number_theory/`，再在仓库根目录运行：

```bash
g++ -std=c++14 -O2 scripts/build.cpp -o /tmp/mal_build
/tmp/mal_build --all include bundles
g++ -std=c++14 -O2 tests/dgf/test_dgf.cpp -o /tmp/mal_dgf_test
/tmp/mal_dgf_test
g++ -std=c++14 -O2 tests/number_theory/test_number_theory.cpp -o /tmp/mal_nt_test
/tmp/mal_nt_test
python3 tests/docs/check_declarations.py
```

定义域：模 `998244353` 的形式 Dirichlet 级数，下标从 1 开始、
`f[0]=0`。普通卷积及基本形式运算是任意数论函数的通用实现；
`fast_mul` 独立提供 EI 正交卷积拆分；`DGF` 类经验证的积性标签
让乘法、除法、求逆、对数、指数、幂自动采用线性或近线性路径。
一般卷积默认保留实测较快的低常数版本；`fast_mul` 可显式选择。

这里的“全家桶”覆盖文稿 DGF 章节中的运算。文稿后半部分
“块筛全家桶”还是待写小节，此包不声称包含未定义的块筛算法。
