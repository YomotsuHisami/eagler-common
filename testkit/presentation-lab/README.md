# Presentation Lab

本工具冻结游戏于固定步边界，执行单步、快速 Replay 遍历和只绘制的 alpha 扫描，报告原生状态及提交变化。生产 Runtime 不应暴露这些诊断控制。

## 接口与状态所有权

`contracts.d.ts`/`contracts.mjs` 定义 RuntimeDriverV1 和 ObservationAdapterV1；`controller-core.mjs` 拥有调度。游戏适配器提供完整逻辑步、绘制入口、输入、对象身份、状态指纹与覆盖声明。

freeze 返回代次 token；过期 token 不操作新运行。恢复正常运行时不把暂停期间墙钟时间变成追帧债。advanceOneTick 必须覆盖所有真实固定步所有者，不能只调用一个局部 calc 函数。drawOnly 不得推进输入、RNG、计时器、ANM 脚本或音频事件队列。

创建、回收和脚本切换必须初始化正确的前后态。对象序号相同不代表对象还是同一代，观察记录需要生命周期身份。

## 证据解释

观察覆盖为 none 或缺失时输出 unknown；没有发现差异不等于纯绘制通过。原版绘制参考必须独立于正在检查的插值算法，不能用待测输出构造期望值。报告区分“没有观察能力”“覆盖范围内一致”和“发现差异”。

TH08 有适配器；TH10 是有限覆盖适配器。TH06/TH07 的 draw 链仍有兼容性副作用，不宣称已支持 Lab。各游戏本轮普通构建不代表诊断构建已重跑。

```text
node --test testkit/presentation-lab/controller-core.test.mjs testkit/presentation-lab/release-contract.test.mjs
```

消费者锁定 common 提交，升级驱动或观察 ABI 后一起验证。图像、状态和重模拟报告保存在源码外，记录准确输入及版本。
