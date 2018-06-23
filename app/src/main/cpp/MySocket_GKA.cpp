//
// Created by AivenLau on 2016/12/1.
//

#include <linux/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include <unistd.h>
#include <linux/in.h>
#include <linux/tcp.h>
#include <errno.h>
#include <sys/endian.h>
#include <arpa/inet.h>
#include <asm/ioctls.h>
#include <sys/socket.h>
#include "Defines.h"
#include "MySocket_GKA.h"
#include "phone_rl_protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/pixfmt.h"
#include "libavutil/imgutils.h"
#include "libavutil/time.h"
#include "libavutil/error.h"
#include "libavutil/frame.h"

#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>

#ifdef __cplusplus
}
#endif


MySocket_GKA::MySocket_GKA() : socketfd(-1), bConnected(false), readid(-1), bFindHead(false),bNotice(false),
                               buffLen(1000) {
    pthread_mutex_init(&m_mutex, NULL);
    fuc_getData = NULL;
    bConnected = false;
};

MySocket_GKA::~MySocket_GKA() {
    DisConnect();
};

int MySocket_GKA::DisConnect() {
    if (bConnected) {
        bConnected = false;
        usleep(1000*5);
        shutdown(socketfd,2);
        close(socketfd);
        socketfd = -1;
        this->host = "";
        this->port = -1;
    }

    if (readid != -1) {
        void *ret = NULL;
        pthread_join(readid, &ret);
        readid = -1;
    }

    return 0;
}

int MySocket_GKA::Connect(string host, int port)
{
     if(ConnectA(host,port)<0)
     {
         usleep(1000*10);
         DisConnect();
         return  ConnectA(host,port);
     }
    return 0;
}

int MySocket_GKA::ConnectA(string host, int port)
{
    const char *df = host.c_str();
    struct sockaddr_in dest_addr; //destnation ip info
    if (socketfd != -1 && bConnected) {
        return 1;
    }
    this->host = host;
    this->port = port;
    socketfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socketfd == -1) {
        bConnected = false;
        return -1;
    }
    int ret = 0;
    unsigned long ul = 1;
    struct timeval timeout;
    fd_set readset, writeset;
    int error = -1, len = sizeof(int);

    int bTimeoutFlag = 0;

    int nRecvBufLen = 200 * 1024; //设置为32K
    int status = setsockopt(socketfd, SOL_SOCKET, SO_RCVBUF, (char *) &nRecvBufLen, sizeof(int));

    int nAddr = 1;
    setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &nAddr, sizeof(int));
    int nPorta = 1;
    setsockopt(socketfd, SOL_SOCKET, SO_REUSEPORT, &nPorta, sizeof(int));

/*
    int keepAlive = 1;   // 开启keepalive属性. 缺省值: 0(关闭)
    int keepIdle = 5;   // 如果在60秒内没有任何数据交互,则进行探测. 缺省值:7200(s)
    int keepInterval = 1;   // 探测时发探测包的时间间隔为5秒. 缺省值:75(s)
    int keepCount = 2;   // 探测重试的次数. 全部超时则认定连接失效..缺省值:9(次)
    setsockopt(socketfd, SOL_SOCKET, SO_KEEPALIVE, (void*)&keepAlive, sizeof(keepAlive));
    setsockopt(socketfd, SOL_TCP, TCP_KEEPIDLE, (void*)&keepIdle, sizeof(keepIdle));
    setsockopt(socketfd, SOL_TCP, TCP_KEEPINTVL, (void*)&keepInterval, sizeof(keepInterval));
    setsockopt(socketfd, SOL_TCP, TCP_KEEPCNT, (void*)&keepCount, sizeof(keepCount));
*/

    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(this->port);
    dest_addr.sin_addr.s_addr = inet_addr(this->host.c_str());

    /*Setting socket to non-blocking mode */
    ioctl(socketfd, FIONBIO, &ul);
    bConnected = false;
    if (-1 == connect(socketfd, (struct sockaddr *) &dest_addr, sizeof(struct sockaddr))) {
        timeout.tv_sec = 1;
        timeout.tv_usec = 1000 * 0;  //500ms
        FD_ZERO(&writeset);
        FD_SET(socketfd, &writeset);

        ret = select(socketfd + 1, NULL, &writeset, NULL, &timeout);
        if (ret == 0)              //返回0，代表在描述词状态改变已超过timeout时间
        {
            getsockopt(socketfd, SOL_SOCKET, SO_ERROR, &error, (socklen_t *) &len);
            if (error == 0)          // 超时，可以做更进一步的处理，如重试等
            {
                bTimeoutFlag = 1;
                LOGE("Not Connect timeout");
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
    int statusa=0;

    ul = 0;
    ioctl(socketfd, FIONBIO, &ul); //重新将socket设置成阻塞模式
    int on = 1;
    statusa =  setsockopt(socketfd, IPPROTO_TCP, TCP_QUICKACK, (void *)&on, sizeof(on));
  //  LOGE("TCP_QUICKACK status = %d",statusa);
    on = 0;
    statusa =  setsockopt(socketfd, IPPROTO_TCP, TCP_NODELAY, (void *)&on, sizeof(on));
   // LOGE("TCP_NODELAY status = %d",statusa);
    on = 1;
    statusa =  setsockopt(socketfd, IPPROTO_TCP, TCP_DEFER_ACCEPT, (void *)&on, sizeof(on));



    int set = 1;

    if (bConnected) {
        setsockopt(socketfd, SOL_SOCKET, MSG_NOSIGNAL, (void *)&set, sizeof(int));
        return 0;
    } else {
        LOGE("not connect!!!!!");
        close(socketfd);
        socketfd = -1;
        bConnected = false;
        this->host = "";
        this->port = -1;
        return -1;
    }
}

int MySocket_GKA::Write(MySocketData *data) {
    struct timeval timeoutA = {0, 1000 * 10};     //10ms
    if (!bConnected)
        return -1;
    setsockopt(socketfd, SOL_SOCKET, SO_SNDTIMEO, (char *) &timeoutA, sizeof(struct timeval));
    ssize_t ret = send(socketfd, data->data, (size_t) data->nLen, 0);
    if (ret < data->nLen)
        return -1;
    return 0;
}

int MySocket_GKA::Read(MySocketData *data, int msTimeout) {
    ssize_t nRet;

    uint8_t *bufferA = data->data;
    int nLen = data->nLen;
    uint32_t nCount = 0;
    int64_t start = av_gettime();
    while (nLen > 0) {
        struct timeval timeoutA = {0, 1000 * 20};     //20ms
        setsockopt(socketfd, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeoutA, sizeof(struct timeval));
        nRet = recv(socketfd, bufferA, nLen, 0);
        if (nRet > 0) {
            nLen -= nRet;
            bufferA += nRet;
            nCount += nRet;
        }
        else if(nRet == 0)
        {
            int errs = errno;
            if (errs == EWOULDBLOCK)
            {
                ;
            } else {
                data->nLen = 0;       //已经断开连接
                return -1;
            }
        }
        if (msTimeout > 0) {
            int64_t current = av_gettime();
            if (current - start >= (msTimeout * 1000)) {
                break;
            }
        }
    }
    data->nLen = nCount;
    return data->nLen;
}

int MySocket_GKA::Read(MySocketData *data) {
    int nRet;
    struct timeval timeoutA = {0, 1000 * 50};     //20ms
    setsockopt(socketfd, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeoutA, sizeof(struct timeval));
    if ((nRet = recv(socketfd, data->data, data->nLen, 0)) != -1)
    {
        if (nRet <= 0) {
            int errs = errno;
            if (errs == EWOULDBLOCK) {
                ;
            } else {
                data->nLen = 0;       //已经断开连接
                return -1;
            }
        } else {
            return nRet;
        }
    }
    return -1;
}
//0XF3B0A4B8

int MySocket_GKA::FindHead(MySocketData *dat, int pos) {

    int ix;
    if (dat->nLen < 4)
        return -1;
    uint8_t  *pdata = dat->data;
    uint32_t magic = 0x01000000;
    uint32_t  data=0;
    for (ix = pos; ix <= dat->nLen - 4; ix++)
    {
        data = *((uint32_t *) (pdata + ix));
        if (magic == data)
        {
                return ix;
        }
    }
    return -1;
}

void F_ResetRelinker();

extern  int64_t  nCheckT_pre;




extern MySocket_GKA GK_tcp_NoticeSocket;


uint8_t pat[]={0,0,0,1};
void getNext(int next[])   //lengthP为模式串P的长度
{
    int j=0,k=-1;//j为P串的下标，k用来记录该下标对应的next数组的值
    next[0]=-1;//初始化0下标下的next数组值为-1
    while(j<4){ //对模式串进行扫描
        if(k==-1||pat[j]==pat[k]){//串后缀与前缀没有相等的子串或者此时j下标下的字符与k下的字符相等。
            j++;k++;
            next[j]=k;//设置next数组j下标的值为k
            }
        else
            k=next[k];//缩小子串的范围继续比较
        }
}
int kmp(int k,int next[],int nTlen,uint8_t *T)
{
    int posP=0,posT=k;              //posP和posT分别是模式串pat和目标串T的下标，先初始化它们的起始位置
    //int lengthP= pat.length();    //lengthP是模式串pat长
    int lengthT=nTlen;// T.length();        //lengthT是目标串T长
    while(posP<4 &&posT<lengthT)    //对两串扫描
    {
        if(posP==-1||pat[posP]==T[posT])
        {                           //对应字符匹配
            posP++;
            posT++;
        }
        else
            posP=next[posP];        //失配时，用next数组值选择下一次匹配的位置
    }
    if(posP<4)
        return -1;
    else
        return posT-4;//匹配成功
}

void F_SetRelinkerT(int nMs);
int nNexPos = 0;
void *MySocket_GKA::ReadData(void *dat) {
    MySocket_GKA *self = (MySocket_GKA *) dat;
    int nRet;
    int Bufferlen = self->buffLen;

    uint8_t *buffer = new uint8_t[Bufferlen];
    self->RevData.nLen = 0;
    MySocketData mydat;
    MySocketData mydatA;
    fd_set   set;
    bool  bStartab=false;
    int on = 1;

    //F_SetRelinkerT(3000);
    nNexPos = 0;
    int next[4]={0};
    while (self->bConnected)
    {
        struct timeval timeoutA = {0, 500};
        int nError;
        FD_ZERO(&set); // 在使用之前总是要清空
        // 开始使用select
        FD_SET(self->socketfd, &set); // 把socka放入要测试的描述符集中
        nRet = select(self->socketfd+1, &set, NULL, NULL, &timeoutA);  // 检测是否有套接口是否可读+1, &rfd, NULL, NULL, &timeoutA);// 检测是否有套接口是否可读
        if (nRet <=0) // 超时
        {
            continue;
        }
        if (!(FD_ISSET(self->socketfd, &set)))
        {
            continue;
        }

        bzero(buffer,Bufferlen);
        nRet = recv(self->socketfd, buffer, Bufferlen, 0);
        setsockopt(self->socketfd, IPPROTO_TCP, TCP_QUICKACK, (int[]){1}, sizeof(int));
        if (nRet <= 0)
        {
            nError = errno;
            if (nError == EWOULDBLOCK && nRet <0)
            {
                //self->RevData.nLen = 0;
            } else
            {
                continue;//break;
            }
        }
        if (nRet > 0)
        {
            F_ResetRelinker();
            if(self->bNotice)
            {
                if (nRet > self->RevData.nSize)
                {
                    LOGE("errorA");
                }
                self->RevData.nLen = nRet;
                memcpy(self->RevData.data,buffer,nRet);
                self->GetData(&self->RevData);
                usleep(1000);
                continue;
            }

#if 1
            self->RevData.AppendData(buffer,nRet);
            int headpos = self->FindHead(&self->RevData,nNexPos);//nSerchPos);
            if(headpos<0)
            {
                if(self->RevData.nLen>4) {
                    nNexPos = self->RevData.nLen-4;
                } else
                {
                    nNexPos = 0;
                }

            }
            else
            {
                nNexPos = 0;
                if(!bStartab)
                {
                    bStartab = true;
                    LOGE("GetFrame Data!!!!");
                    continue;
                }

                mydat.nLen=0;
                mydat.AppendData(self->RevData.data,headpos+4);
                self->GetData(&mydat);
                mydatA.nLen = 0;
                mydatA.AppendData(self->RevData.data+headpos+4,self->RevData.nLen-headpos-4);
                self->RevData.nLen=0;
                self->RevData.AppendData(mydatA.data,mydatA.nLen);
            }

#else
            if (nRet > self->RevData.nSize)
            {
                LOGE("errorA");
            }
            self->RevData.nLen = nRet;
            memcpy(self->RevData.data,buffer,nRet);
            //self->RevData.data = buffer;
            self->GetData(&self->RevData);
#endif
        }

    }
    if(self == &GK_tcp_NoticeSocket) {
        LOGE("Exit NoticeSocket  Read Thread!!!");
    }
    else
    {
        LOGE("Exit DataSocket  Read Thread!!!");
    }
    self->readid = -1;
    delete[]buffer;
    return NULL;

}

void MySocket_GKA::GetData(MySocketData *dat) {
    if (fuc_getData != NULL) {
        fuc_getData(dat);
    }
}

void MySocket_GKA::StartReadThread(void) {
    int ret = 0;
    if (readid == -1) {
        bFindHead = true;
        RevData.nLen = 0;
        ret = pthread_create(&readid,  NULL, ReadData, (void *) this); // 成功返回0，错误返回错误编号

    }
    if (ret != 0) {
        readid = -1;
    }

}