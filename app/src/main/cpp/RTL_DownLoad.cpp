//
// Created by AivenLau on 2019-05-13.
//

#include "RTL_DownLoad.h"
#include <linux/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include <unistd.h>
#include <linux/in.h>
#include <linux/tcp.h>

#include <sys/endian.h>
#include <arpa/inet.h>
#include <asm/ioctls.h>
#include <sys/socket.h>
#include <string>
#include "Defines.h"

#include <errno.h>

#include <pthread.h>
#include <cerrno>
#include <errno.h>


using namespace std;




RTL_DownLoad::RTL_DownLoad():socketfd(-1),bConnected(false),readid(-1),fuc_getData(NULL)
{



}
RTL_DownLoad::~RTL_DownLoad()
{
    disConnected();

}


void RTL_DownLoad::disConnected()
{
    if(bConnected)
    {
        bConnected = false;
        usleep(1000*50);
    }
    if(socketfd>0)
    {
        close(socketfd);
        socketfd=-1;
        bConnected = false;
    }
}


int RTL_DownLoad::Connect()
{

    struct sockaddr_in dest_addr; //destnation ip info
    if (socketfd != -1 && bConnected) {
        return 0;
    }

    socketfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socketfd <0) {
        bConnected = false;
        return -1;
    }
    int ret = 0;
    unsigned long ul = 1;
    struct timeval timeout;
    fd_set readset, writeset;
    int error = -1, len = sizeof(int);

    int bTimeoutFlag = 0;

    int nRecvBufLen = 200 * 1024; //设置为200K
    int status = setsockopt(socketfd, SOL_SOCKET, SO_RCVBUF, (char *) &nRecvBufLen, sizeof(int));
    int nAddr = 1;
    setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &nAddr, sizeof(int));
    int nPorta = 1;
    setsockopt(socketfd, SOL_SOCKET, SO_REUSEPORT, &nPorta, sizeof(int));

    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(10001);
    dest_addr.sin_addr.s_addr = inet_addr("192.168.32.1");
    ioctl(socketfd, FIONBIO, &ul);
    usleep(1000);
    bConnected = false;
    if (-1 == connect(socketfd, (struct sockaddr *) &dest_addr, sizeof(struct sockaddr)))
    {
        timeout.tv_sec = 1;
        timeout.tv_usec = 1000 * 500;  //1.500sec
        FD_ZERO(&writeset);
        FD_SET(socketfd, &writeset);
        ret = select(socketfd + 1, NULL, &writeset, NULL, &timeout);
        if (ret == 0)              //返回0，代表在描述词状态改变已超过timeout时间
        {
            getsockopt(socketfd, SOL_SOCKET, SO_ERROR, &error, (socklen_t *) &len);
            if (error == 0)          // 超时，可以做更进一步的处理，如重试等
            {
                bTimeoutFlag = 1;
                LOGE("Not Connect timeout RTL");
            } else {
                LOGE("Not Connect host error");
            }
        } else if (ret == -1)      // 返回-1， 有错误发生，错误原因存在于errno
        {
            LOGE("Not Connect host error1");

        } else {
            bConnected = true;
        }
    } else {
        bConnected = true;
    }
    int statusa = 0;

    ul = 0;
    ioctl(socketfd, FIONBIO, &ul); //重新将socket设置成阻塞模式
    int on = 1;
    statusa = setsockopt(socketfd, IPPROTO_TCP, TCP_QUICKACK, (void *) &on, sizeof(on));
    on = 0;
    statusa = setsockopt(socketfd, IPPROTO_TCP, TCP_NODELAY, (void *) &on, sizeof(on));
    on = 1;
    statusa = setsockopt(socketfd, IPPROTO_TCP, TCP_DEFER_ACCEPT, (void *) &on, sizeof(on));
    int set = 1;
    if (bConnected)
    {
        setsockopt(socketfd, SOL_SOCKET, MSG_NOSIGNAL, (void *) &set, sizeof(int));
        StartReadThread();
        return 0;
    } else {
        socketfd = -1;
        bConnected = false;
        return -1;
    }

}

string RTL_DownLoad::string_format(const char* format, ...)
{
    char buff[1024] = {0};
    va_list args;
    va_start(args, format);
    vsprintf(buff,format,args);
    //vsprintf_s(buff, sizeof(buff), format, args);
    va_end(args);
    std::string str(buff);
    return str;

}

int RTL_DownLoad::F_GetMode(void)
{
    string sgetmode="GETMODE";
    if(!bConnected || socketfd<0)
        return -2;
    struct timeval timeoutA = {0, 1000 * 10};     //10ms
    setsockopt(socketfd, SOL_SOCKET, SO_SNDTIMEO, (char *) &timeoutA, sizeof(struct timeval));
    ssize_t ret = send(socketfd, sgetmode.c_str(), (size_t)sgetmode.length(),0);
    if (ret < sgetmode.length())
        return -1;
    usleep(1000*50);
    return 0;

}

int  RTL_DownLoad::F_ReadList(int  nImage,int inx)
{
    if(inx<0)
        return -1;
    if(!bConnected || socketfd<0)
        return -2;
    struct timeval timeoutA = {0, 1000 * 10};     //10ms
    string sformat="";
    if(nImage == 0)
    {
        sformat="BROWSE;img;%003d";
    }
    else if(nImage == 1)
    {
        sformat="BROWSE;vid;%003d";
    }
    else if(nImage == 2)
    {
        sformat="BROWSE;pir;%003d";
    } else
    {
        return -2;
    }
    string sdata= string_format(sformat.c_str(),inx);
    setsockopt(socketfd, SOL_SOCKET, SO_SNDTIMEO, (char *) &timeoutA, sizeof(struct timeval));
    ssize_t ret = send(socketfd, sdata.c_str(), (size_t)sdata.length(),0);
    if (ret < sdata.length())
        return -1;
    return 0;
}

void RTL_DownLoad::StartReadThread(void) {
    int ret = 0;
    if (readid == -1) {
        ret = pthread_create(&readid, NULL, ReadData, (void *) this); // 成功返回0，错误返回错误编号
    }
    if (ret != 0) {
        readid = -1;
    }

}

void F_onReadRtlData(uint8_t *data,int nLen);

void *RTL_DownLoad::ReadData(void *dat)
{
    RTL_DownLoad *self = (RTL_DownLoad *)dat;

    int Bufferlen = 2048;

    uint8_t *buffer = new uint8_t[Bufferlen];

    fd_set set;
    int nRet;

    while (self->bConnected)
    {
        struct timeval timeoutA = {0, 1000*30}; //5ms
        int nError;
        FD_ZERO(&set); // 在使用之前总是要清空
        FD_SET(self->socketfd, &set); // 把socka放入要测试的描述符集中
        nRet = select(self->socketfd + 1, &set, NULL, NULL, &timeoutA);  // 检测是否有套接口是否可读+1, &rfd, NULL, NULL, &timeoutA);// 检测是否有套接口是否可读
        if (nRet <= 0) // 超时
        {
            continue;
        }
        if (!(FD_ISSET(self->socketfd, &set)))
        {
            continue;
        }
        bzero(buffer, Bufferlen);
        nRet = recv(self->socketfd, buffer, Bufferlen, 0);
        if (nRet <= 0) {
            nError = errno;
            if (nError == EWOULDBLOCK && nRet < 0) {
                //self->RevData.nLen = 0;
            } else {
                continue;//break;
            }
        } else{
            F_onReadRtlData(buffer,nRet);
        }
    }
    delete []buffer;
    self->readid=-1;
    return NULL;
}

void RTL_DownLoad::GetData(uint8_t *data,int nLen)
{
    if (fuc_getData != NULL) {
        fuc_getData(data,nLen);
    }
}

int RTL_DownLoad::DownLoadFile(const char *sFile)
{
     if(socketfd<0 || !bConnected )
         return -1;
    struct timeval timeoutA = {0, 1000 * 100};     //10ms
    string sformat="DOWN;%s";
    string sdata= string_format(sformat.c_str(),sFile);
    setsockopt(socketfd, SOL_SOCKET, SO_SNDTIMEO, (char *) &timeoutA, sizeof(struct timeval));
    ssize_t ret = send(socketfd, sdata.c_str(), (size_t)sdata.length(),0);
    if (ret < sdata.length())
        return -1;
    return 0;

}

