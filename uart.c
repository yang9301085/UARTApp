#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>

#include "uart.h"

int uart_open(const char *dev, const uart_cfg_t *prag,
              struct termios *saved_tio)
{
    if (prag == NULL || saved_tio == NULL) {
        errno = EINVAL;
        return -1;
    }

    if (prag->data_bits!=8 || 
        prag->stop_bits!=1 || 
        prag->flow!=FLOW_NONE ||  
        prag->parity!='N')
    {
        errno = EINVAL;
        return -1;
    }
    

    speed_t speed;

    switch (prag->baud){
        case 115200:{
            speed=B115200;
            break;
        }
        case 1000000:{
            speed=B1000000;
            break;
        }
        default:{
            errno = EINVAL;
            return -1;
        }
    }

    int fd = open(dev, O_RDWR | O_NOCTTY | O_NONBLOCK);

    if (fd < 0) {
        return -1;
    }

    if (tcgetattr(fd, saved_tio) < 0)
    {
        int saved_errno = errno;
        close(fd);
        errno = saved_errno;
        return -1;
    }

    struct termios new_tio = *saved_tio;

    cfmakeraw(&new_tio);

    if (cfsetispeed(&new_tio, speed) < 0)
    {
        int saved_errno = errno;
        close(fd);
        errno = saved_errno;
        return -1;
    }
    if (cfsetospeed(&new_tio, speed) < 0)
    {
        int saved_errno = errno;
        close(fd);
        errno = saved_errno;
        return -1;
    }

    new_tio.c_cflag|=CLOCAL | CREAD;
    new_tio.c_cflag &= ~(CSIZE | PARENB | CSTOPB | CRTSCTS);
    new_tio.c_cflag |= CS8;

    new_tio.c_iflag&=~(IXON | IXOFF | IXANY);

    if (tcsetattr(fd, TCSANOW, &new_tio) < 0)
    {
        int saved_errno = errno;
        close(fd);
        errno = saved_errno;
        return -1;
    }

    return fd;
}
