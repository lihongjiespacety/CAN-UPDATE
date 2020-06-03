#include <stdio.h>
#include <Windows.h>
#include <process.h>

static HANDLE portHandle = INVALID_HANDLE_VALUE;

int uart_open(const char *name) 
{
    portHandle = CreateFileA(name, 
        GENERIC_READ|GENERIC_WRITE,  // ע�⴮�ں��������COM9Ӧ����ǰ�����\\.\������COM10��ʾΪ"\\\\.\\COM10"
        0,                           //�������д  
        NULL,                        //��ռ��ʽ  
        OPEN_EXISTING,               //�򿪶����Ǵ���  
        0,                           //0ͬ����ʽ ; FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED�첽��ʽ
        NULL);

    if( portHandle == INVALID_HANDLE_VALUE ) 
	{
        DWORD errorCode = GetLastError();
        if( errorCode == ERROR_FILE_NOT_FOUND ) 
		{
            //���ڲ�����
        }
        else
		{
            //���ڴ��ڴ�ʧ��
		}
        return errorCode;
    }
    return 0;
}

int uart_config(DWORD baudrate,BYTE bytesize, BYTE parity, BYTE stopbits,UINT timeout)
{
	/*1.��ȡ���ڵĳ�ʼ����,���޸�ָ������*/
	DCB dcb; 
	COMMTIMEOUTS TimeOuts;
	GetCommState(portHandle,&dcb); 
	dcb.BaudRate=baudrate;     
	dcb.ByteSize=bytesize; 
	dcb.Parity=parity; 
	dcb.StopBits=stopbits; 
	SetCommState(portHandle,&dcb); 
	/*2.���û�������С*/
	SetupComm(portHandle,4096+128,4096+128); //���뻺����������������Ĵ�С����1024
	/*3.���ó�ʱ*/
	//GetCommTimeouts(portHandle,&TimeOuts);
	TimeOuts.ReadIntervalTimeout=1200;         //���ַ������ʱʱ��  ���ReadIntervalTimeout=MAXDWORD����ReadTotalTimeoutMultiplier=0��ReadTotalTimeoutConstant=0 �򲻵ȴ��������ء� 
	TimeOuts.ReadTotalTimeoutMultiplier=100;   //���ܳ�ʱ��ReadTotalTimeoutMultiplier���ֽ�����ReadTotalTimeoutConstant
	TimeOuts.ReadTotalTimeoutConstant=timeout; 
	TimeOuts.WriteTotalTimeoutMultiplier=100;  //д�ܳ�ʱ 
	TimeOuts.WriteTotalTimeoutConstant=timeout; 
	SetCommTimeouts(portHandle,&TimeOuts);  
    /*6.��ջ�����*/
	PurgeComm(portHandle,PURGE_TXCLEAR | PURGE_RXCLEAR); 
	return 0;
}

int uart_flush(void)
{
	PurgeComm(portHandle,PURGE_TXCLEAR | PURGE_RXCLEAR);
	return 0;
}

int uart_close(const char *name) 
{
    CloseHandle(portHandle);
	portHandle = INVALID_HANDLE_VALUE;
	return 0;
}


DWORD uart_read(char* buff, DWORD len, BOOL* readstat)
{
	/*ͬ����*/
	DWORD wCount;     //��ȡ���ֽ��� 
	BOOL bReadStat; 

	DWORD dwErrorFlags; 
	COMSTAT ComStat; 
	ClearCommError(portHandle,&dwErrorFlags,&ComStat); 

	bReadStat=ReadFile(portHandle,buff,len,&wCount,NULL); 
	*readstat = bReadStat;
	return wCount;
}

int uart_send(char* buff, DWORD len, BOOL* writestat)
{
	BOOL bWriteStat; 
	DWORD dwBytesWrite; 
	//PurgeComm(portHandle, PURGE_TXABORT| PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR); 
	bWriteStat=WriteFile(portHandle,buff,len,&dwBytesWrite,NULL); 
	*writestat = bWriteStat;
	return dwBytesWrite;
}

int uart_trans(BYTE* outbuff, UINT outlen, BYTE* inbuff, UINT inlen)
{
	BOOL writestat;
	DWORD writebytes;
	BOOL readstat;
	DWORD readbytes;
	uart_flush();
	writebytes = uart_send(outbuff, outlen, &writestat);
	if((writebytes != outlen) || (writestat != TRUE))
	{
		return 0;
	}
	else
	{
		readbytes = uart_read(inbuff, inlen, &readstat);
		if((readbytes != inlen) || (readstat != TRUE))
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}
}