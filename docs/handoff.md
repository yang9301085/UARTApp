# 会话交接（Handoff）

> 供下一个教学会话 / 其他分支开发快速接手。最后更新：2026-10-12。

## 一句话现状

M1（打开设备）已完成并通过 socat 回环验证；下一步进入 **M2（termios 配置 115200 8N1）**。

---

## 本次会话进展（2026-10-12）

- 讲透并实现 `uart_open`：
  - `open(dev, O_RDWR | O_NOCTTY | O_NONBLOCK)`
  - `fcntl(fd, F_GETFL)` → `flag &= ~O_NONBLOCK` → `fcntl(fd, F_SETFL, flag)` 恢复阻塞
  - `fd < 0` 时 `perror` + 返回 -1
- 讲透并实现 `main.c`：`argc < 2` 校验 → `uart_open(argv[1], NULL)` → 打印 fd / 报错退出
- 编译 `gcc -Wall -Wextra -o uart_app main.c uart.c` 通过（仅 `cfg` 未使用警告，良性）
- socat 回环验证通过：无参报用法、未起 socat 报打开失败、起 socat 后打开成功 fd=3

### 本次澄清的关键概念

- 阻塞 vs 非阻塞：`O_NONBLOCK` 只让 `open` 不卡；恢复阻塞是让 `read` 在没数据时乖乖等
- 内核缓冲区：数据先入内核 RX 缓冲区，`read` 从这里取；非阻塞 `read` 不丢数据，只是"来早了空手回"
- `errno` / `perror` / `strerror`：`perror` 只用于系统调用错误，参数校验用 `printf`
- `argc`/`argv`：`argv[0]` 是程序名，设备路径在 `argv[1]`；`argc==1` 表示用户没给路径
- `fcntl` 读-改-写三步；置 1 用 `|`，清 0 用 `& ~`
- 位掩码宏：`1 << n` 表示第 n 位

---

## 当前代码状态

| 文件 | 状态 |
|------|------|
| `uart.h` | 保护宏、`<sys/types.h>`、`FLOWCTL`、`uart_cfg_t`、`uart_open` 原型；`uart_send()` 空括号待修 |
| `uart.c` | `#include <stdio.h><fcntl.h>`；`uart_open` 已实现 |
| `main.c` | `#include <stdio.h>`；`main` 已完成参数校验 + 打开 + 打印 |

---

## 待决设计问题（下次先定）

1. `uart_recv` 超时：参数携带，还是交给 `select`/`poll`？（倾向后者，职责单一，M4 本就规划 select）
2. 缓冲区指针类型：`void *` 还是 `unsigned char *`？（倾向后者，明确"字节流"语义）

---

## 遗留债务（不阻塞，随对应里程碑处理）

- `uart.h`：`ssize_t uart_send()` 空括号 → M3 改 `(void)` 或补参数
- `uart.h`：缺 `uart_recv` / `uart_close` 原型 → M4 / M5 补
- `uart.c`：`cfg` 参数未使用（M2 用上后消除）；`fcntl(F_GETFL)` 失败未检查（收尾补）

---

## 下一步（M2 起点）

1. `uart_open` 内补 termios：`tcgetattr` → `cfmakeraw` → 波特率/帧格式 → `CLOCAL|CREAD` → `tcsetattr` + `tcflush`
2. `main.c` 创建并填充 `uart_cfg_t`，替换 `NULL` 传入
3. `stty -F /tmp/ttyV0 -a` 反向验证

知识点：K2.1 ~ K2.6（见 README 知识点地图）。

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
- M2 ~ M5 见 README「教学知识点地图」

### 教学方式备忘

- 只指导不写代码，先讲"为什么需要、解决什么问题"，再逐步引导用户自己写；代码只用短片段示意
- 用户说"继续"时接最近未完成知识点；问概念就先答概念，不强行拉回课程
- 新术语第一次出现用"中文含义 + 英文名 + 作用"解释；每轮聚焦一个知识点
- 画图必须 Mermaid UML，禁手绘 ASCII 字符画
