# Task 11 Report: Tracer Bullet — del2d1 e2e 端到端跑通

## 结果：DONE

del2d1 e2e 端到端测试 **PASS**，max|Δ|=0.00e+00。无需重冻结基准。

## 执行序列

### Step 1 — 适配 mingw64 工具链配置

修改 6 个文件：

1. **`test/config.toml`**：
   - 添加 `make_program = "C:/dev/mingw64/bin/mingw32-make.exe"`
   - 添加 `dll_dirs = ["C:/dev/mingw64/bin"]`
   - 修正 TOML 格式：原 `cases = [...]` 内联表数组（跨行 `{}`）违反 TOML 1.0 规范（inline table 必须单行），改为 `[[suite.e2e.cases]]` 标准数组语法

2. **`test/framework/config.py`**：
   - `Toolchain` dataclass 新增 `make_program: str = ""` 和 `dll_dirs: list[str]` 字段
   - `Config` 构造函数从 toml 读取这两个新字段

3. **`test/run_tests.py`**：
   - `build_cases` 中根据 `make_program` 追加 `-DCMAKE_MAKE_PROGRAM=...` 到 cmake 参数
   - E2ECase 构造时传入 `dll_dirs=cfg.toolchain.dll_dirs`

4. **`test/framework/runner.py`**：
   - `run()` 新增 `extra_dll_dirs` 参数
   - 运行前将 exe 所在目录 + extra_dll_dirs 全部加入 PATH（解决 mingw64 运行时 DLL 依赖）

5. **`test/framework/case.py`**：
   - `E2ECase.__init__` 新增 `dll_dirs` 参数
   - `run()` 方法传递 `extra_dll_dirs=self.dll_dirs` 给 runner

6. **`test/framework/tests/test_case.py`**：
   - `fake_run` 签名添加 `**kwargs` 以兼容新增的 `extra_dll_dirs` 关键字参数

### Step 2 — 运行 tracer bullet

```
python test/run_tests.py --suite e2e --case del2d1 -v
```

过程：
- cmake configure（Unix Makefiles, `-DCMAKE_MAKE_PROGRAM=C:/dev/mingw64/bin/mingw32-make.exe`）
- cmake build del2d target → `test/build/output/del2d.exe` + `libCDFEG.dll`
- 隔离工作目录 `test/build/run/e2e.del2d1/` 拷贝输入（排除 .post.res 基准）
- 运行 `del2d.exe del2d .`（PATH 含 output 目录 + mingw64/bin）
- 对比产出 vs 基准 → PASS

### Step 3 — 诊断结果

**首次运行遇到 3 个环境问题（均解决）：**

| # | 问题 | 根因 | 对策 |
|---|------|------|------|
| 1 | TOML 解析失败 "Invalid initial character" | 原格式 `cases = [{ ... }]` 跨行 inline table 违反 TOML 1.0 | 改为 `[[suite.e2e.cases]]` 标准语法 |
| 2 | cmake configure 失败 "no such file or directory" | make_program 用 POSIX 路径 `/c/dev/...`，原生 cmake.exe 不识别 | 改为 Windows 路径 `C:/dev/...` |
| 3 | exe 退出码 0xC0000135 (DLL_NOT_FOUND) | mingw64 运行时 DLL (libgcc_s_seh-1.dll, libstdc++-6.dll) 不在 PATH | 新增 `dll_dirs` 配置 + runner PATH 注入 |

**最终结果：PASS，max|Δ|=0.00e+00**

基准无需重冻结——mingw64 构建的 del2d1 输出与现有基准完全一致（可能是基准本身就是 mingw 产出的，而非 cygwin）。

### Step 4 — 单元测试验证

全部 37 个 framework 测试通过：
- `test_config.py`: 3/3 PASSED
- `test_case.py`: 4/4 PASSED（含 fake_run 签名修复后）
- 其余全部 PASSED

## 变更文件清单

| 文件 | 变更类型 |
|------|----------|
| `test/config.toml` | 修改（新增字段 + 格式修正） |
| `test/framework/config.py` | 修改（新增 make_program/dll_dirs） |
| `test/framework/runner.py` | 修改（DLL PATH 注入） |
| `test/framework/case.py` | 修改（dll_dirs 传递） |
| `test/run_tests.py` | 修改（配置传递） |
| `test/framework/tests/test_case.py` | 修改（fake_run 签名） |

## 自查清单

- [x] 完整链条：configure → build → run → compare → report
- [x] 最终结果 PASS（max|Δ| < 1e-8）
- [x] 无需重冻结基准（mingw 输出与基准完全一致）
- [x] test_config.py 及全部 37 个 framework 测试绿色
- [x] 未修改 CDFEG 核心源码（仅测试框架适配）

## 提交

- **SHA**: `4b714b7`
- **Message**: `test: tracer bullet 跑通 del2d1（mingw64 工具链）`
