#include     <stdio.h>      /*标准输入输出定义*/
#include     <stdlib.h>     /*标准函数库定义*/
#include     <unistd.h>     /*Unix 标准函数定义*/
#include     <sys/types.h>  
#include     <sys/stat.h>   
#include     <fcntl.h>      /*文件控制定义*/
#include     <termios.h>    /*PPSIX 终端控制定义*/
#include     <errno.h>      /*错误号定义*/

int uart_open(const char * name)
{
    int fd;
    /*以读写方式打开串口*/
    fd = open( name, O_RDWR| O_NOCTTY | O_NDELAY);
    if (-1 == fd)
    { 
        /* 不能打开串口一*/ 
         perror(" 打开串口失败！");
    }
    return fd;
}

int uart_set(const char * name，int baud)
{
    struct termios options;
    tcgetattr(fd, &options);
    cfsetispeed(&options, baud);
    cfsetospeed(&options, baud);
    options.c_cflag |= (CLOCAL | CREAD);

    /*8N1*/
    options.c_cflag &= ~PARENB
    options.c_cflag &= ~CSTOPB
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;

    options.c_cflag &= ~CNEW_RTSCTS;
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_oflag &= ~OPOST;

    tcsetattr(fd, TCSANOW, &options);
}

int uart_send(const char * name，uint_t buff, int len,int timeout)
{

}

int uart_send(const char * name，uint_t buff, int len,int timeout)
{

}

