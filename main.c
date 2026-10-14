#include <stdio.h>
#include "uart.h"
#include "util.h"

int main(int argc, char *argv[])
{
    if (argc<7)
    {
        printf("用法: %s <设备> <波特率> <数据位> <校验> <停止位> <流控>\n", argv[0]);
        return 1;

    }
    int fd=0;
    uart_cfg_t cfg;
    
    cfg.baud=str_to_int(UART_BAUD, argv[2]);
    cfg.data_bits=str_to_int(UART_DATA_BIT, argv[3]);
    cfg.parity=str_to_parity(argv[4]);
    cfg.stop_bits=str_to_int(UART_STOP_BIT, argv[5]);
    cfg.flow=str_to_FLOWCTL(argv[6]);
    
    fd=uart_open(argv[1],&cfg);

    if(fd<0)
    {
        printf("串口打开失败\n");
        return 2;
    }
    printf("串口打开成功，fd: %d\n",fd);

    return 0;
}