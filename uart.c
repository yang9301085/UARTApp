#include <stdio.h>
#include <fcntl.h>
#include <termios.h>

#include "uart.h"

int uart_open(const char *dev, const uart_cfg_t *cfg)
{
    int fd=0;
    int flag=0;

    struct termios tio;

    fd=open(dev,O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd<0)
    {
        perror("uart_open: open failed");
        return -1;
    }

    flag=fcntl(fd,F_GETFL);
    flag&=(~O_NONBLOCK);
    fcntl(fd,F_SETFL,flag);
    
    if(tcgetattr(fd,&tio)<0)
    {
        perror("uart_open: termios error");
        return -1;
    }

    cfmakeraw(&tio);

    cfsetispeed(&tio, B115200);
    cfsetospeed(&tio, B115200);

    tio.c_cflag|=(CLOCAL|CREAD);

    if(tcsetattr(fd, TCSANOW, &tio)<0)
    {
        perror("uart_open: tcsetattr failed");
        return -1;        
    }

    return fd;
}
