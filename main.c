#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "uart.h"

int main()
{
    int exit_status = 0;
    uart_cfg_t prag;

    prag.baud=1000000;
    prag.data_bits=8;
    prag.flow=FLOW_NONE;
    prag.parity='N';
    prag.stop_bits=1;

    struct termios old_tio;
    struct termios current_tio;
    int fd=uart_open("/dev/ttyS9", &prag, &old_tio);
    if (fd<0)
    {
        perror("uart open");
        return 1;
    }
    printf("uart open successed fd id is %d.\n",fd);

    if (tcgetattr(fd, &current_tio) < 0)
    {
        perror("uart config");
        exit_status = 1;
    }
    else
    {

        tcflag_t current_cflag = current_tio.c_cflag;
        tcflag_t current_c_iflag = current_tio.c_iflag;

        if ((current_cflag & (CLOCAL | CREAD)) != (CLOCAL | CREAD))
        {
            fprintf(stderr, "uart config: CLOCAL or CREAD is not set\n");
            exit_status = 1;
        }

        if((current_cflag & CSIZE) != CS8)
        {
            fprintf(stderr, "uart config: data bits are not CS8\n");
            exit_status = 1;
        }

        if ((current_cflag & (PARENB | CSTOPB | CRTSCTS)) != 0)
        {
            fprintf(stderr, "uart config: PARENB, CSTOPB or CRTSCTS is unexpectedly set\n");
            exit_status = 1;
        }

        if((current_c_iflag & (IXON | IXOFF | IXANY))!=0){
            fprintf(stderr, "uart config: IXON, IXOFF or IXANY is unexpectedly set\n");
            exit_status = 1;
        }

    }

    if (tcsetattr(fd, TCSANOW, &old_tio) < 0)
    {
        perror("uart restore");
        exit_status=1;
    }
    
    fd=close(fd);
    if (fd<0)
    {
        perror("uart close");
        return 1;
    }
    
    return exit_status;
}
