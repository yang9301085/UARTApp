# 会话交接（Handoff）

> 供下一个教学会话 / 其他分支开发快速接手。最后更新：2026-10-15。

## 一句话现状

M4 基础版已完成：`uart_recv`（EINTR 重试）+ 回环测试跑通（commit `15e9d30`）；下一步 **M4 后半段：`select`/`poll` 超时接收**，解决"read 一次收不全"。

---

## 本次会话进展（2026-10-14 ~ 10-15）

### M3 发送（2026-10-14，commit `566f564`）

- `uart_send` 循环写：处理部分写（`write` 返回小于请求量）、`EINTR` 重试、错误返回 -1
- `uart.h` 补 `uart_send` 原型；`main.c` 用上 `uart_cfg_t` 并解析命令行参数（util 模块）
- 带用户追完 write() 内核链路：`vfs_write`→`new_sync_write`→`call_write_iter`→`tty_write`→`do_tty_write`→`n_tty_write`→`serial8250_write`

### M4 接收（2026-10-15，commit `15e9d30`）

- `uart_recv`：`do-while` 处理 `EINTR` 重试，返回实际字节数
- `uart.h` 补 `uart_recv` 原型
- `main.c` 回环测试跑通：`/tmp/ttyV0`（发）→ socat → `/tmp/ttyV1`（收），`uart_send` 发 "hello loopback" → `uart_recv` 收回 → 打印
- 带用户追完 read() 内核链路：`ksys_read`→`vfs_read`→`tty_read`→`iterate_tty_read`→`n_tty_read`；讲透 VMIN/VTIME（`MIN_CHAR`/`TIME_CHAR`）、cookie 续读、环形缓冲 read_buf

### 本次澄清的关键概念（M4 相关）

- `read` 返回多少不由你控制：`VMIN=1` 时收到 1 字节就返回，可能一次收不全一条数据（经典坑，M4 后半段用 select/poll 解决）
- `sizeof(指针)` = 8（64 位），不是字符串长度；字符串长度用 `strlen`
- 接收缓冲区必须分配内存（数组或 `malloc`），否则野指针段错误
- 收到数据后要补 `\0` 才能用 `%s` 打印，否则越界

---

## 当前代码状态

| 文件 | 状态 |
|------|------|
| `uart.h` | 保护宏、`<sys/types.h>`、`FLOWCTL`、`uart_cfg_t`；`uart_open`/`baud_to_speed`/`uart_send`/`uart_recv` 原型齐；缺 `uart_close`（M5） |
| `uart.c` | `uart_open`（termios 配置）、`uart_send`（循环写）、`uart_recv`（EINTR 重试）、`baud_to_speed` 均已实现 |
| `main.c` | 命令行参数解析 + 双串口（TX/RX）回环测试，跑通 "hello loopback" 收发 |

---

## 待决设计问题（下次先定）

1. `uart_recv` 超时：由 `select`/`poll` 处理（已定倾向，M4 后半段做），解决"read 一次收不全"
2. 缓冲区指针类型：**已定稿 `void *`**（`uart_recv` 已用 `void *`）
3. `tcdrain` 是否加入 `uart_send`：推迟到 M5 决定（发送后是否需要等待数据真正从 FIFO 发完）

---

## 遗留债务（不阻塞，随对应里程碑处理）

- `uart.h`：缺 `uart_close` 原型 → M5 补；末尾缺换行
- `uart.c`：`fcntl(F_GETFL)` 失败未检查；`tcdrain` 未加（M5 决定）；`tcflush` 未加（M5 收尾补）
- `main.c`：`count` 用 `ssize_t` 而非 `size_t`（洁癖，不改也行）；回环测试无"比对"逻辑（收到内容 == 发出内容，待补）

---

## 下一步（M4 后半段起点）

1. K4.3：`select`/`poll` 超时接收 —— 解决"read 一次收不全"，实现可靠接收
2. 回环测试加"比对"逻辑（收到内容 == 发出内容），让测试有结论
3. 之后进 M5：`uart_close`（恢复旧 termios）+ `tcdrain` + `tcflush` + 完整回环测试

知识点：K4.3（见 README 知识点地图）。

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
- 编译命令：`gcc -Wall -Wextra -o uart_app main.c uart.c util.c`

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
