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

## 构建与验证

```bash
gcc -Wall -Wextra -o uart_app main.c uart.c
./uart_app /tmp/ttyV0