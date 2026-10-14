#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>   // write()
#include <errno.h>    // errno 和 EINTR

#include "uart.h"

ssize_t uart_send(int fd, const void *buf, size_t count)
{
    ssize_t sendStatus=0;
    size_t tmp=0;

    while (tmp<count)
    {
        sendStatus=write(fd, (const unsigned char *)buf+tmp,count-tmp);
        if (sendStatus>0)
        {
            tmp+=sendStatus;
        }else if(sendStatus==0){
            return -1;
        }else{
            if(errno==EINTR) {
                continue;
            }
            return -1;
        }
    }
    
    return tmp;
}

int uart_open(const char *dev, const uart_cfg_t *cfg)
{

    if (!cfg)
    {
        fprintf(stderr,"uart_open: uart_cfg is NULL\n");
        return -1;
    }
    

    int fd=0;
    int flag=0;

    struct termios tio;

    speed_t baud=baud_to_speed(cfg->baud);

    if (baud==B0)
    {
        fprintf(stderr,"uart_open: baud error,set to default 115200\n");
        baud=B115200;
    }
    

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


    cfsetispeed(&tio, baud);
    cfsetospeed(&tio, baud);

    tio.c_cflag|=(CLOCAL|CREAD);

    if(tcsetattr(fd, TCSANOW, &tio)<0)
    {
        perror("uart_open: tcsetattr failed");
        return -1;        
    }

    return fd;
}


speed_t baud_to_speed(int baud)
{
    switch(baud)
    {
        case 57600:   return B57600;
        case 115200:  return B115200;
        case 230400:  return B230400;
        case 460800:  return B460800;
        case 500000:  return B500000;
        case 576000:  return B576000;
        case 921600:  return B921600;
        case 1000000: return B1000000;
        case 1152000: return B1152000;
        case 1500000: return B1500000;
        case 2000000: return B2000000;
        case 2500000: return B2500000;
        case 3000000: return B3000000;
        case 3500000: return B3500000;
        case 4000000: return B4000000;
        default:    return B0;
    }
}