#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/select.h>

#include "uart.h"
#include "util.h"

int main(int argc, char *argv[])
{
    if (argc<7)
    {
        printf("用法: %s <设备> <波特率> <数据位> <校验> <停止位> <流控>\n", argv[0]);
        return 1;

    }
    int appStatus=0;
    int loopCount=3;
    int UARTfd_TX=0;
    int UARTfd_RX=0;
    uart_cfg_t cfg;
    
    unsigned char *buf;
    ssize_t count=1024;
    ssize_t rcvStatus=0;

    const char *msg = "hello loopback";

    int selectStatus=0;
    struct timeval timeout;
    int maxfds=0;
    fd_set readfds;

    buf=(unsigned char *)malloc(sizeof(unsigned char)*(count+1));

    if (!buf)
    {
        printf("资源分配失败\n");
        return -1;
    }
    

    cfg.baud=str_to_int(UART_BAUD, argv[2]);
    cfg.data_bits=str_to_int(UART_DATA_BIT, argv[3]);
    cfg.parity=str_to_parity(argv[4]);
    cfg.stop_bits=str_to_int(UART_STOP_BIT, argv[5]);
    cfg.flow=str_to_FLOWCTL(argv[6]);
    
    UARTfd_RX=uart_open("/tmp/ttyV1",&cfg);

    UARTfd_TX=uart_open(argv[1],&cfg);


    if(UARTfd_RX<0)
    {
        printf("串口 /tmp/ttyV1 打开失败\n");
        return 2;
    }

    maxfds=UARTfd_RX+1;

    if(UARTfd_TX<0)
    {
        printf("串口 %s 打开失败\n",argv[1]);
        return 2;
    }
    printf("串口打开成功，fd: %d\n",UARTfd_TX);
    printf("串口打开成功，fd: %d\n",UARTfd_RX);


    uart_send(UARTfd_TX,msg,strlen(msg));
    
    do{
        if(loopCount-- <0)
        {
            printf("UARTfd_RX is loop to limited\n");
            break;
        }
        FD_ZERO(&readfds);
        FD_SET(UARTfd_RX,&readfds);

        timeout.tv_sec=1;
        timeout.tv_usec=0;

        selectStatus=select(maxfds, &readfds,NULL,NULL,&timeout);

        if(selectStatus>0)
        {
            if(!FD_ISSET(UARTfd_RX, &readfds)){
                printf("UARTfd_RX is not found in select SET\n");
                appStatus=5;
                break;
            }

            rcvStatus=uart_recv(UARTfd_RX,buf,count);

            if (rcvStatus>0)
            {
                buf[rcvStatus]='\0';
                fprintf(stdout,"%s\n",buf);
            }else if(rcvStatus==0){
                printf("UARTfd_RX is closed\n");
                appStatus=6;
                break;
            }else{
                if(errno==EINTR)
                {
                    printf("UARTfd_RX read error\n");
                    appStatus=7;
                    continue;
                }
                printf("UARTfd_RX read error,not EINTR \n");
                appStatus=8;
                break;
            }
        }else if(selectStatus==0){
            printf("select timeout\n");
            appStatus=3;
            break;
        }else if(selectStatus == -1 && errno != EINTR){
            printf("select error, errno is %d errno\n",errno);
            appStatus=4;
            break;
        }
    }while (selectStatus == -1 && errno == EINTR);
    
    free(buf);

    return appStatus;
}