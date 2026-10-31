#ifndef UART_H
#define UART_H

#include <sys/types.h>
#include <termios.h>
/**
 * 
 */
typedef enum {
    FLOW_NONE,  /* 无流控 */
    FLOW_HW,    /* 硬件流控 RTS/CTS */
    FLOW_SW     /* 软件流控 XON/XOFF */
}FLOWCTL;

/**
 * 
 */
typedef struct{
    int baud;           //波特率
    int data_bits;      //数据位
    char parity;        //校验位
    int stop_bits;      //停止位
    FLOWCTL flow;       //流控
} uart_cfg_t;

/* 成功时，saved_tio 保存原配置，由调用者保留以便恢复。 */
int uart_open(const char *dev, const uart_cfg_t *prag,
              struct termios *saved_tio);
ssize_t uart_send();
#endif
