#include <stdio.h>
#include "uart.h"

int main(int argc, char *argv[])
{
    if (argc<2)
    {
        printf("用法: %s <设备路径>\n", argv[0]);
        return 1;

    }
    int fd=0;
    fd=uart_open(argv[1],NULL);
    if(fd<0)
    {
        printf("串口打开失败\n");
        return 2;
    }
    printf("串口打开成功，fd: %d\n",fd);

    return 0;
}