#include <stdio.h>
#include <fcntl.h>
#include "uart.h"

int uart_open(const char *dev, const uart_cfg_t *cfg)
{
    int fd=0;
    int flag=0;
    fd=open(dev,O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd<0)
    {
        perror("uart_open: open failed");
        return -1;
    }

    flag=fcntl(fd,F_GETFL);
    flag&=(~O_NONBLOCK);
    fcntl(fd,F_SETFL,flag);

    return fd;
}
