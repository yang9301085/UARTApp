# UARTApp

纯 C 的 Linux 用户态 UART 串口驱动与应用（教学项目，分里程碑逐步实现）。

## 环境

- 开发/编译/运行：WSL2（Ubuntu-22.04），编译验证一律在 WSL 内用 `gcc` 完成
- 编辑器：VS Code（Windows 侧打开 `E:\UARTApp`，WSL 终端执行编译）
- 回环测试设备：socat 虚拟串口对 `/tmp/ttyV0` ↔ `/tmp/ttyV1`
- 目标帧格式：115200 8N1
- 语言：C11，仅 POSIX API，无第三方依赖

### 已知环境坑（已解决）

| 现象 | 原因 | 处理 |
|------|------|------|
| VS Code 编辑器里 Linux 头文件报红，gcc 却能编过 | Windows 侧 IntelliSense 用了 Windows 头文件，与 WSL 编译环境无关 | 只信 WSL 里 `gcc` 的结果；`.vscode/c_cpp_properties.json` 设 `intelliSenseMode: linux-gcc-x64` |
| Copilot / 网络在 WSL 内报 `socket connection to proxies: 127.0.0.1:15490` | WSL2 NAT 网络独立，WSL 的 `127.0.0.1` 不等于 Windows 的 `127.0.0.1`；代理只监听 Windows 回环 | 关闭该代理/走直连，或改用 Windows 主机 IP（`ip route show` 取默认网关）并开启代理的 Allow LAN |

## 进度里程碑

| 里程碑 | 内容 | 状态 |
|--------|------|------|
| M0 | WSL 环境搭建 + socat 虚拟串口回环验证 | ✅ 2026-10-10 |
| M1 | 骨架 uart.h/uart.c/main.c + 打开设备 | ✅ 2026-10-12 |
| M2 | termios 配置（115200 8N1） | ✅ 2026-10-13 |
| M3 | 发送（循环 write + tcdrain） | ✅ 2026-10-14 |
| M4 | 接收（uart_recv + 回环测试；select 超时待做） | 🔶 2026-10-15 |
| M5 | 关闭时恢复旧配置 + 完整回环测试 | ⬜ |

## 教学知识点地图（里程碑 → 知识点）

| 里程碑 | 知识点编号 | 内容 |
|--------|-----------|------|
| **M1 骨架+打开** | K1.1 ✅ | 头文件保护宏 + 需要包含哪些头文件（`#include <sys/types.h>` 提供 `ssize_t`） |
| | K1.2 ✅ | 函数原型：为什么 `uart_open` 只收 2 个参数（设备路径 + 配置结构体指针）；`uart_open` 已定稿，`uart_send/recv/close` 原型随 M3/M4/M5 补全 |
| | K1.3 ✅ | `open()` 的三个 flag 各自防什么坑 |
| | K1.4 ✅ | `fcntl` 恢复阻塞模式 |
| | K1.5 ✅ | 错误处理：`errno` / `perror` / `strerror` |
| | K1.6 ✅ | `main` 里从 `argv` 拿设备路径 |
| **M2 配置** | K2.1 ✅ | `termios` 是什么、为什么先 `tcgetattr` |
| | K2.2 ✅ | `cfmakeraw`：不设会出什么怪现象 |
| | K2.3 ✅ | 波特率：`B115200` 宏是什么、为何要用 `cfsetispeed/cfsetospeed` |
| | K2.4 ✅ | `c_cflag` 帧格式位：CS8/校验/停止位 |
| | K2.5 ✅ | `CLOCAL`/`CREAD`：漏掉会发生什么 |
| | K2.6 ✅ | `tcsetattr` + `stty` 反向验证（`tcflush` 遗留，M5 收尾补） |
| **M3 发送** | K3.1 ✅ | `write` 为什么可能"只写一半" |
| | K3.2 ✅ | 循环写 + `tcdrain` 的意义 |
| **M4 接收** | K4.1 ✅ | 串口是字节流：没有"消息"概念 |
| | K4.2 ✅ | VMIN/VTIME 三种组合语义 |
| | K4.3 ⬜ | `select` 超时接收 + `EINTR` |
| **M5 收尾** | K5.1 | 为什么要保存并恢复旧 `termios` |
| | K5.2 | 完整回环测试 + README 更新 |

## 构建与验证

```bash
gcc -Wall -Wextra -o uart_app main.c uart.c util.c
./uart_app /tmp/ttyV0 115200 8 N 1 0
```

## 当前进行中（M4）

### M3 已完成（2026-10-14）

- `uart_send` 循环写：处理部分写、`EINTR` 重试、错误返回 -1
- `uart.h` 补 `uart_send` 原型；`main.c` 用上 `uart_cfg_t` 并解析命令行参数

### M4 已完成（2026-10-15，commit 15e9d30）

- `uart_recv`：`do-while` 处理 `EINTR` 重试，返回实际字节数
- `uart.h` 补 `uart_recv` 原型
- `main.c` 回环测试跑通：`/tmp/ttyV0`（发）→ `/tmp/ttyV1`（收），`uart_send` 发 "hello loopback" → `uart_recv` 收回 → 打印

### 待完成（下一步，M4 后半段）

- K4.3：`select`/`poll` 超时接收，解决"read 一次收不全"的问题
- 接收返回值完整处理（`rcvStatus <= 0` 边界）

### 遗留债务（不阻塞，随对应里程碑处理）

- `uart.c`：`fcntl(F_GETFL)` 失败未检查；`tcdrain` 未加（M5 决定）；`tcflush` 未加（M5）
- `uart.h`：缺 `uart_close` 原型（M5）
- `main.c`：`count` 用 `ssize_t` 而非 `size_t`（洁癖，不改也行）

### 待决设计问题（下次先定）

1. `uart_recv` 超时：由 `select`/`poll` 处理（已倾向，M4 后半段做）
2. 缓冲区指针类型：已定稿 `void *`（`uart_recv` 已用）