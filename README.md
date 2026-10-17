# eagler-common

[English](README.en.md) · [维护指南](docs/MAINTENANCE.md)

多部 eagler-touhou 游戏共用的 C++ Runtime 基础和浏览器/testkit 支持。许可见 [LICENSE](LICENSE)。

## 职责

公共库拥有联机协议与会话算法、输入安排和修复、回滚存储原语、固定步调度辅助，以及独立于作品的数据验证工具。各游戏保留对象布局、完整快照清单、ECL/ANM/关卡规则、原生 Replay 语义和生命周期适配。

共享算法不等于共享全部游戏状态。根 eagler-touhou 的 TypeScript contracts 服务于 Web、Node 和服务消息，不能替代本库的 C++ 实现。

## 构建与验证

```text
cmake -S . -B build/tests -DEAGLER_COMMON_BUILD_TESTS=ON
cmake --build build/tests --config Release
ctest --test-dir build/tests -C Release --output-on-failure
```

Windows 原生构建已执行 34 项 CTest，包括图形状态与移动记录格式；通过范围以各测试实现为准。消费者的浏览器、游戏规则和资源验收需要在对应游戏单独执行。

## 文档

- [维护指南](docs/MAINTENANCE.md)：消费者版本、状态边界、输入与会话生命周期。
- [浏览器桥接](browser/README.md)：键盘所有权与生成代码。
- [输入 Replay 格式](docs/input-replay.md)：确认输入的有界事务格式。
- [Presentation Lab](testkit/presentation-lab/README.md)：固定步、绘制观察和覆盖声明。
- [Replay Verifier](testkit/replay-verifier/README.md)：逐步状态比较与独立基准。
- [Windows Present 观察器](testkit/replay-verifier/windows-present-observer/README.md)：原版观察的实际能力和限制。

游戏子模块锁定各自的 common 提交。升级时先核对接口和消费者测试，再提交 gitlink，不以根目录 common 较新为由覆盖所有游戏依赖。
