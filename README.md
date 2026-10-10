# UARTApp

纯 C 的 Linux 用户态 UART 串口驱动与应用（教学项目，分里程碑逐步实现）。

## 环境

- 开发/编译/运行：WSL2（Ubuntu-22.04）
- 回环测试设备：socat 虚拟串口对 `/tmp/ttyV0` ↔ `/tmp/ttyV1`
- 目标帧格式：115200 8N1
- 语言：C11，仅 POSIX API，无第三方依赖

## 进度里程碑

| 里程碑 | 内容 | 状态 |
|--------|------|------|
| M0 | WSL 环境搭建 + socat 虚拟串口回环验证 | ✅ 2026-10-10 |
| M1 | 骨架 uart.h/uart.c/main.c + 打开设备 | ⬜ |
| M2 | termios 配置（115200 8N1） | ⬜ |
| M3 | 发送（循环 write + tcdrain） | ⬜ |
| M4 | 接收（VMIN/VTIME + select 超时） | ⬜ |
| M5 | 关闭时恢复旧配置 + 完整回环测试 | ⬜ |

## 教学知识点地图（里程碑 → 知识点）

| 里程碑 | 知识点编号 | 内容 |
|--------|-----------|------|
| **M1 骨架+打开** | K1.1 | 头文件保护宏 + 需要包含哪些头文件 |
| | K1.2 | 函数原型：为什么 `uart_open` 只收 2 个参数 |
| | K1.3 | `open()` 的三个 flag 各自防什么坑 |
| | K1.4 | `fcntl` 恢复阻塞模式 |
| | K1.5 | 错误处理：`errno` / `perror` / `strerror` |
| | K1.6 | `main` 里从 `argv` 拿设备路径 |
| **M2 配置** | K2.1 | `termios` 是什么、为什么先 `tcgetattr` |
| | K2.2 | `cfmakeraw`：不设会出什么怪现象 |
| | K2.3 | 波特率：`B115200` 宏是什么、为何要用 `cfsetispeed/cfsetospeed` |
| | K2.4 | `c_cflag` 帧格式位：CS8/校验/停止位 |
| | K2.5 | `CLOCAL`/`CREAD`：漏掉会发生什么 |
| | K2.6 | `tcsetattr` + `tcflush` 收尾 + `stty` 反向验证 |
| **M3 发送** | K3.1 | `write` 为什么可能"只写一半" |
| | K3.2 | 循环写 + `tcdrain` 的意义 |
| **M4 接收** | K4.1 | 串口是字节流：没有"消息"概念 |
| | K4.2 | VMIN/VTIME 三种组合语义 |
| | K4.3 | `select` 超时接收 + `EINTR` |
| **M5 收尾** | K5.1 | 为什么要保存并恢复旧 `termios` |
| | K5.2 | 完整回环测试 + README 更新 |

## 构建与验证

```bash
gcc -Wall -Wextra -o uart_app main.c uart.c
./uart_app /tmp/ttyV0