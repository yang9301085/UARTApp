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

int uart_open(const char *dev,const uart_cfg_t *cfg);

speed_t baud_to_speed (int baud);

ssize_t uart_send(int fd, const void *buf, size_t count);
#endif