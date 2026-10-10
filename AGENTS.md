# AGENTS.md — UARTApp 项目约定

面向 AI 编码代理的项目级说明。项目目标：纯 C 的 Linux 用户态 UART 驱动程序与应用。

## 核心约束

- **默认只指导、不写代码**：用户只要求实现思路、API 说明、宏/结构体用法和注意事项。不要直接创建或修改源文件，除非用户明确要求。
- 输出风格：步骤化指导（打开设备 → 配置 termios → 收发 → 关闭），中文讲解，代码以简短片段示意。

## 技术背景

- 语言：纯 C（C11 / GNU C），仅用 POSIX API，无第三方依赖
- 目标平台：Linux（开发机是 Windows，编译与运行验证必须在 Linux 目标机或 WSL 上完成，不要在 Windows 本地尝试编译）
- 设备节点：`/dev/ttyS0`、`/dev/ttyUSB0` 等
- 建议文件结构：`uart.h`（接口声明）/ `uart.c`（实现）/ `main.c`（测试）

## 关键实现约定

- 打开设备用 `O_RDWR | O_NOCTTY | O_NONBLOCK`，打开后再用 `fcntl(fd, F_SETFL, 0)` 恢复阻塞模式
- 串口配置走 `termios`：先 `cfmakeraw()`；`CLOCAL | CREAD` 必须置位；数据位 `CS8`
- 接收语义由 `c_cc[VMIN]` / `c_cc[VTIME]` 决定；带超时接收用 `select()` 或 `poll()`
- `write()` 必须循环写直到写完；发送后可用 `tcdrain()`
- 非标准波特率用 `struct termios2` + `ioctl(TCSETS2)`
- 退出前恢复保存的 `termios` 旧值，避免串口配置残留

## 构建与验证

- 编译：`gcc -Wall -Wextra -o uart_app main.c uart.c`
- 回环测试：短接 TX/RX，发送字符串应原样收到
- 配置检查：`stty -F /dev/ttyS0 -a`
- 权限问题：将用户加入 `dialout` 组或 `sudo` 运行

## 自定义代理

- UART 编程指导问题优先委派给 `UART导师` 代理（`.github/agents/uart-mentor.agent.md`）：只指导不写代码，内置 termios 知识速查与常见坑，可读用户代码但无编辑能力
