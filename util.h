#ifndef UTIL_H
#define UTIL_H
#include "uart.h"
typedef enum{
    UART_BAUD,
    UART_DATA_BIT,
    UART_STOP_BIT,
    UART_FLCTL
}uart_cfg_type;

int str_to_int(uart_cfg_type type,const char *s);
FLOWCTL str_to_FLOWCTL(const char *s);
char str_to_parity(const char *s);
char setDefaultParitytoN();
FLOWCTL setDefaultFLOWCTLtoFLOW_NONE();
#endif