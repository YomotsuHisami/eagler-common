# 公共库维护

## 公共与游戏边界

`include/` 和 `src/` 维护作品无关的实现；游戏通过自己的协议、输入提交和传输配置接入。CMake 提供 `eagler::netplay_headers`、`input_replay`、`rollback`、`browser_peer_transport`、`netplay_input`、`netplay_base` 和 `session_channel` 目标。

游戏拥有完整状态快照及恢复。common 不枚举 Bullet、Enemy、ECL、Stage 等对象，也不因状态摘要相等就自行认定两个历史输入等价。抽取公共代码应先核对实际消费者，升级分别验证普通和多人行为。

## 输入与回滚

`RollbackCore::ApplyInputPacket` 先验证完整包，再修改输入、ACK 和回滚前沿。矛盾冗余样本、无效模拟量、不可能 ACK 和破坏未来环槽的包必须事务性拒绝。过期重传和非法输入是不同结果。

游戏恢复目标帧之前的状态后，才调用 `RewindSimulationTo(firstFrame)`。它撤销该帧后的模拟决定并降低模拟前沿，同时保留已捕获本地输入、已确认远端输入及重传/ACK 历史；它不恢复游戏内存。

`AcknowledgedLocalThroughAllRemotes()` 表示远端已确认的本地输入；`ConfirmedThroughAllRemotes()` 表示本地已收到的远端输入。两方向不可混用，缺失前沿不得当作很大的已确认帧。

DirectTouch 位移由生产者捕获一次，未应用位移留在可回滚状态中；重模拟不重新采样设备。缺失 delta 不凭空增加位移。等价预测提交需要游戏提供历史轨迹并修补其快照，不能仅由输入数值接近推断。

## 会话与传输

`SessionChannel` 在 `PeerTransport` 上处理 HELLO/READY、输入/ACK、可靠尾部修复和确认活性。游戏仍拥有 SessionGate、RollbackCore、输入捕获、世界恢复和完整逻辑更新。

终止运行前先完成协调，并使输入双向前沿都到终点。新代次握手期间仍须能应答旧代次最后 ACK；旧流量不能推进新核心。`Clear()` 是完整销毁，不是普通代次切换。

可靠修复补发已捕获输入，不成为第二套采样系统。健康 ACK 进度不应持续触发修复；可靠通道背压应跳过本次补发，不能直接破坏正常游戏传输。

## 调度与游戏时间

FrameBudget、FramePacingPolicy 和确认活性超时影响浏览器调度，不属于确定性游戏状态。追帧限制应保留尚未消化的时间债，不能通过无声丢逻辑步来换取流畅指标。

成员管理、遥测、隐藏窗口策略和可见错误属于消费者。修改公共常量需要说明对应实际问题并验证目标链路，不能把某次耗时测量永久变成所有设备必须满足的阈值。

## 测试

CTest 执行实际 C++ 实现及已声明的 Node 行为测试。`simulation-frontier-test.cpp`、`packet-transaction-test.cpp`、`session-channel-test.cpp` 分别覆盖回退前沿、非法包事务及有损多端会话；其他测试名与对象见 `tests/CMakeLists.txt`。

源码同步检查只证明生成代码来自当前输入。它不能证明运行协议或浏览器交互。已移除仅搜索 BrowserPeerTransport 源码 token 的检查，实际 EM_JS 行为测试保留。

[输入 Replay](input-replay.md)、[Presentation Lab](../testkit/presentation-lab/README.md)和[Replay Verifier](../testkit/replay-verifier/README.md)有各自证据边界。不得用历史性能报告替代当前消费者运行，也不得用候选结果覆盖原版基准。
