# Replay Verifier

本工具比较每个完整 Replay 逻辑步后的、由游戏声明的权威状态。common 拥有 trace 校验、严格对齐、流式比较与报告；游戏拥有原生 Replay 语义、生命周期、字段和原版/候选执行环境。

## Trace 和比较

JSONL 从 `run-start` 开始，包含明确 segment 生命周期和逐 tick 记录，以完整 `run-end` 结束。序列缺口、提前 EOF、身份不兼容、缺少必需类别或额外 tick 都失败，不能裁剪尾部后宣告通过。

`adapter.mjs` 处理通用生命周期框架；`capture-file.mjs`、`golden-files.mjs`、`file-suite.mjs`、`join-traces.mjs` 支持文件型适配器。拼接关卡保留每个来源 tick。

```text
node testkit/replay-verifier/cli.mjs compare --expected expected.jsonl --actual actual.jsonl --report result.json
```

## 原版基准

原版捕获、候选捕获和接受后的 golden 分开保存。接受基准必须显式执行，并拒绝覆盖现有清单；不能拿候选输出充当独立原版结果。`fetch-fixtures.mjs` 只取得语料清单中有 SHA-256 的输入。

TH08/TH10 使用其声明的 JIT/捕获适配；TH11/TH15 使用 WASI 诊断核心与文件提供器。字段覆盖和完整终止由各作定义。诊断字段一致不证明全部内存、渲染、音频、浏览器存档或设备兼容。

普通 CTest 包含本工具的已登记行为测试。消费者验收见各游戏 `tools/replay-verifier/README.md`。本轮没有重新生成或替换原版 golden。
