# Windows D3D8 Present 观察器

这是 Windows 原版观察工具。32 位代理 DLL 转发系统 Direct3DCreate8，在 Present 边界读取固定地址的 Replay 状态。它不修改游戏内存，不合成输入，不改变 RNG/计时，也不自动选择 Demo 或 Replay。

用 `build.ps1` 构建对应游戏的 DLL，只把产物放到隔离的原版副本旁。此入口依赖 Windows 原生工具，属于保留 PowerShell 的平台适配。必要时可链到未修改的 d3d8to9 兼容二进制。

输出 `replay-verifier-present.bin` 为追加的 64 字节记录流，保留非活跃记录以确认 Demo 边界。只加载 DLL 不代表 hook 成功，捕获必须观察到有效且前进的记录。

`windows-present-capture.py` 在启动前校验配置大小、版本、范围和窗口模式。TH06 ANSI 配置名按 Windows 代码页解析；缺失配置不能静默退回全屏。Locale 包装器需要已验证的子进程代码页配置。

TH07 尚未实现 DllMain 晚注入；支持路径是启动时加载 D3D8 代理。普通原版 Replay 的输入自动化和完成判定仍未完成，向隐藏窗口发送键盘消息不构成已验证输入提供器。

该观察器本轮未执行原版捕获。报告只能描述实际读到的边界和字段，不能据此宣称完整游戏一致。
