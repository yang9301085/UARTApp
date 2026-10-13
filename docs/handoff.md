# 会话交接（Handoff）

> 供下一个教学会话 / 其他分支开发快速接手。最后更新：2026-10-13。

## 一句话现状

M2（termios 配置 115200 8N1）已完成并通过 `stty` 反向验证；下一步进入 **M3（发送：循环 write + tcdrain）**。

---

## 本次会话进展（2026-10-13）

- 讲透并实现 `uart_open` 的 termios 配置（M2）：
  - `tcgetattr(fd, &tio)` 读当前配置
  - `cfmakeraw(&tio)` 原始模式（关回显/行编辑/`\r\n` 转换）
  - `cfsetispeed(&tio, B115200)` + `cfsetospeed(&tio, B115200)` 设波特率
  - `tio.c_cflag |= (CLOCAL | CREAD)` 置忽略 modem 控制线 + 使能接收
  - `tcsetattr(fd, TCSANOW, &tio)` 写回内核立即生效（含返回值检查）
- 验证通过：`stty -F /tmp/ttyV0 -a` 显示 `speed 115200 baud`、`cs8`、`-parenb`、`-cstopb`、`cread clocal`、`min=1; time=0`，全部命中 115200 8N1

### 本次额外深入（源码追查）

- 下载并解压了 **glibc 2.35 源码**（`~/glibc-src/usr/src/glibc/glibc-2.35/`）与 **Linux 6.5 内核源码**（`~/linux-src/usr/src/linux-source-6.5.0/`），追通了 `tcgetattr/tcsetattr` 的完整调用链：
  - 用户函数 → glibc 包装（`__tcgetattr`/`__tcsetattr`，`weak_alias` 别名）→ `INLINE_SYSCALL` → `INTERNAL_SYSCALL` → x86_64 内联汇编 `syscall` 指令 → 内核 `tty_ioctl`/`tty_set_termios` → 8250 驱动 `serial8250_do_set_termios` → `serial_port_out(port, UART_LCR, ...)` 写硬件寄存器
- 配置了 `.vscode/c_cpp_properties.json`：加了 `browse.path`（指向 glibc 源码）+ `limitSymbolsToIncludedHeaders: false`，用于"跳转到实现"

### 本次澄清的关键概念

- `struct termios` 4 个 flag（`c_iflag/oflag/cflag/lflag`）+ `c_cc[]` 数组；波特率/帧格式几乎都在 `c_cflag`
- `cfmakeraw` 是 BSD 扩展（非 POSIX），受 `__USE_MISC` 保护；`cfsetispeed/ospeed` 才是 POSIX 标准
- 波特率必须用 `B115200` 宏（值 `0010002` 八进制 = 4098，是内核编号，非 115200）
- `CLOCAL`（忽略 modem 控制线/DCD）、`CREAD`（使能接收）
- `TCSANOW`/`TCSADRAIN`/`TCSAFLUSH` 三种生效时机
- `/tmp` 会被 WSL 重启清空，长期保留的文件要放家目录 `~/`
- 内联汇编：`asm volatile("syscall")` 里只有 `syscall` 是汇编，`=a`/`0`/`memory` 是 GCC 约束语法

---

## 当前代码状态

| 文件 | 状态 |
|------|------|
| `uart.h` | 保护宏、`<sys/types.h>`、`FLOWCTL`、`uart_cfg_t`、`uart_open` 原型；`uart_send()` 空括号待修（M3） |
| `uart.c` | `#include <stdio.h><fcntl.h><termios.h>`；`uart_open` 已实现完整 termios 配置（M2） |
| `main.c` | `#include <stdio.h>`；`main` 参数校验 + 打开 + 打印；`uart_open(argv[1], NULL)` 中 `NULL` 待换为实际 `uart_cfg_t`（M3） |

---

## 待决设计问题（下次先定）

1. `uart_recv` 超时：参数携带，还是交给 `select`/`poll`？（倾向后者，职责单一，M4 本就规划 select）
2. 缓冲区指针类型：`void *` 还是 `unsigned char *`？（倾向后者，明确"字节流"语义）

---

## 遗留债务（不阻塞，随对应里程碑处理）

- `uart.h`：`ssize_t uart_send()` 空括号 → M3 改 `(void)` 或补参数
- `uart.h`：缺 `uart_recv` / `uart_close` 原型 → M4 / M5 补
- `uart.c`：`cfg` 参数未使用（M3 用上后消除）；`fcntl(F_GETFL)` 失败未检查（收尾补）；`tcflush` 未加（M5 收尾补）

---

## 下一步（M3 起点）

1. M3 发送：`uart_send` 实现循环 `write` + 发送后 `tcdrain`
2. `main.c` 创建并填充 `uart_cfg_t`（含波特率 115200），替换 `NULL` 传入，让 `cfg` 参数用起来
3. 补 `uart.h` 里 `uart_send` 原型参数

知识点：K3.1 ~ K3.2（见 README 知识点地图）。

---

## 跨会话 / 跨分支通用信息

> 以下内容**不依赖 main 分支的代码进度**，切到其他分支（如 `RK3568_UART`）开发时同样适用。

### 环境与约定

- 编译/运行一律在 WSL2（Ubuntu-22.04）内用 `gcc`，**不要在 Windows 本地编译**；编辑器在 Windows 侧打开 `/mnt/e/UARTApp`
- 回环测试设备：socat 虚拟串口对 `/tmp/ttyV0` ↔ `/tmp/ttyV1`
  ```bash
  socat -d -d pty,raw,echo=0,link=/tmp/ttyV0 pty,raw,echo=0,link=/tmp/ttyV1
  ```
- 目标帧格式 115200 8N1；C11 + POSIX，无第三方依赖
- 编译命令：`gcc -Wall -Wextra -o uart_app main.c uart.c`

### 已解决的环境坑

| 现象 | 处理 |
|------|------|
| 编辑器 Linux 头文件报红，gcc 却能编过 | Windows IntelliSense 与 WSL 环境分离，只信 WSL 里 `gcc` 结果 |
| WSL 内代理报 `127.0.0.1:15490` | WSL2 NAT 网络，WSL 的 127.0.0.1 ≠ Windows 的 127.0.0.1；走直连/关代理 |
| WSL 手动搬 ext4.vhdx 损坏 | 用 `wsl --export` / `--import` |

### 关键实现约定（来自 AGENTS.md）

- 打开：`O_RDWR | O_NOCTTY | O_NONBLOCK`，开完 `fcntl(F_SETFL, 0)` 恢复阻塞
- 配置：`cfmakeraw()`；`CLOCAL | CREAD` 必置位；`CS8`
- 接收语义由 `c_cc[VMIN]` / `c_cc[VTIME]` 决定；超时用 `select()`/`poll()`
- `write` 循环写满；发送后 `tcdrain()`
- 非标准波特率用 `termios2` + `TCSETS2`
- 退出前恢复旧 `termios`

### 教学知识点进度（通用）

- M1 全部知识点 K1.1~K1.6 ✅
- M2 全部知识点 K2.1~K2.6 ✅
- M3 ~ M5 见 README「教学知识点地图」

### 教学方式备忘

- 只指导不写代码，先讲"为什么需要、解决什么问题"，再逐步引导用户自己写；代码只用短片段示意
- 用户说"继续"时接最近未完成知识点；问概念就先答概念，不强行拉回课程
- 新术语第一次出现用"中文含义 + 英文名 + 作用"解释；每轮聚焦一个知识点
- 画图必须 Mermaid UML，禁手绘 ASCII 字符画
