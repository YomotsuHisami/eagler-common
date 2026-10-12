# 浏览器桥接

## 键盘所有权

`keyboard-owners.mjs` 保存物理按键身份并协调松键；游戏适配器把事件映射到自己的逻辑位和生命周期。已知左右修饰键、方向键与小键盘别名保持独立。UP 丢失全部物理身份时释放匹配的逻辑所有者，避免永久卡键；没有原始 DOWN 的 repeat 不恢复已取消输入。

`directory-keyboard.mjs` 在目录 Runtime 中复用该算法，合并原生与 hosted 所有者，只发布变化的键码。`clear()` 推进代次。支持 `Module.resetBrowserKeyboard` 的 C++ 适配器使用该桥接并清理 SDL 状态；旧壳读取自己的兼容路径。

```text
node tools/sync-browser-keyboard.mjs TH06_SHELL TH07_SHELL
node tools/sync-browser-keyboard.mjs --check TH06_SHELL TH07_SHELL
node tools/sync-directory-keyboard.mjs DIRECTORY_KEYBOARD_MODULE
node tests/browser-keyboard-owners.test.mjs
```

同步器生成自包含输入，避免旧游戏锁定引入未声明的外部 JS。`--check` 检查生成一致性；事件测试与消费者真实浏览器验证分别证明行为。

## 启动画面

`startup-branding.mjs`/`startup-wordmark.svg` 和 `startup-developers.mjs`/`startup-developers.svg` 保存透明字标及对应生成器。游戏传入真实构建时间，原生渲染器拥有纹理、颜色和显示生命周期。生成字形使用 SVG 轮廓，不依赖构建机器额外下载字体。

```text
node tools/sync-startup-branding.mjs TARGET
node tools/sync-startup-developers.mjs GAME_REPOSITORY
```

同步后重新构建对应 Runtime。构建时间应来自当前构建元数据，不能修改 loader 的任意源码文本来伪造身份；启动动画和菜单准备不得推进游戏逻辑。生成器不修改原版图片，诊断不得靠读回整个画面代替状态边界。
