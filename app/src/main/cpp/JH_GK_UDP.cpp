//
// Created by AivenLau on 2017/12/14.
//

#include <sys/socket.h>
#include <sstream>
#include "JH_GK_UDP.h"
#include "phone_rl_protocol.h"


JH_GK_UDP::JH_GK_UDP():video_fd(-1),notice_fd(-1),cmd_fd(-1),bInited(false),isCancelled(false),video_id(-1),cmd_id(-1),notic_id(-1),nStep(-1),session_id(-1),sentCmd_fd(-1)
{
    readBuff = new uint8[GK_UDP_ReadBuffLen];
    bzero(readBuff,GK_UDP_ReadBuffLen);
    noticBuff = new uint8[100];
    cmdBuff = new uint8[100];
    UDT::startup();
}
JH_GK_UDP::~JH_GK_UDP()
{
    isCancelled = true;
    usleep(1000*10);
    if(video_fd!=-1)
    {
        close(video_fd);
    }

    if(notice_fd!=-1)
    {
        close(notice_fd);
    }
    if(cmd_fd!=-1)
    {
        close(cmd_fd);
    }

    delete []readBuff;
    delete []noticBuff;
    delete []cmdBuff;
    UDT::cleanup();
}




bool JH_GK_UDP::sentcmd(uint8 *data,int nLen,int port,const char *server)
{
    unsigned int size;


    int udp = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);


    struct sockaddr_in servaddr; /* the server's full addr */
    bzero((char *) &servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = inet_addr(server);
    size = sizeof(servaddr);
    ssize_t res = sendto(udp, data, nLen, 0, (struct sockaddr *) &servaddr, size);
    if (res < 0)
    {
        LOGE("write error");
    }
    close(udp);
    return true;

}


int JH_GK_UDP::F_Login(const char *sCustomer) {
    MySocketData data;
    T_NET_CMD_MSG_UDP Cmd;
    Cmd.checksum = UDP_CHECKSUM;
    Cmd.session_id = 0;
    Cmd.type = CMD_LOGIN;

    data.AppendData((uint8_t *) &Cmd, sizeof(T_NET_CMD_MSG_UDP));
    T_NET_LOGIN user;
    memset(user.passwd, 0, 100);
    memset(user.user, 0, 100);
    int nLen = strlen(sCustomer);
    if (nLen >1) {
        memcpy(user.user, sCustomer, nLen);
    }
    data.AppendData((uint8_t *) &user, sizeof(T_NET_LOGIN));
    nStep = CMD_LOGIN;
    sentcmd(data.data,data.nLen,GK_UDP_CmdPort,sServerIP.c_str());

    return 0;
}

int JH_GK_UDP::F_GetSDInfo(void ) {
    MySocketData data;
    data.nLen = 0;
    T_NET_CMD_MSG_UDP Cmd;
    Cmd.checksum = UDP_CHECKSUM;
    T_NET_CONFIG config;
    Cmd.session_id = session_id;
    Cmd.type = CMD_GET_CONFIG;
    config.type = CONFIG_SD_CARD;
    config.res = 0;
    data.AppendData((uint8_t *) &Cmd, sizeof(T_NET_CMD_MSG_UDP));
    data.AppendData((uint8_t *) &config, sizeof(T_NET_CONFIG));
    nStep = CMD_GET_CONFIG;
    sentcmd(data.data,data.nLen,GK_UDP_CmdPort,sServerIP.c_str());
    return 0;
}
int JH_GK_UDP::F_GetAdjTime(void )
{

        T_NET_CMD_MSG_UDP Cmd;
        Cmd.checksum = UDP_CHECKSUM;
        MySocketData data;
        if (session_id == 0) {
            return -1;
        }
        Cmd.session_id = session_id;
        Cmd.type = CMD_ADJUST_TIME;
        data.AppendData((uint8_t *) &Cmd, sizeof(T_NET_CMD_MSG_UDP));

        T_NET_DATE_TIME date;
        time_t tt = time(NULL);//这句返回的只是一个时间cuo
        tm *t = localtime(&tt);
        date.usYear = (uint16_t) (t->tm_year + 1900);
        date.usMonth = (uint16_t) (t->tm_mon + 1);
        date.usDay = (uint16_t) t->tm_mday;
        date.ucHour = (unsigned char) t->tm_hour;
        date.ucMin = (unsigned char) t->tm_min;
        date.ucSec = (unsigned char) t->tm_sec;
        nStep = CMD_ADJUST_TIME;
        data.AppendData((uint8_t *) &date, sizeof(T_NET_DATE_TIME));
        sentcmd(data.data,data.nLen,GK_UDP_CmdPort,sServerIP.c_str());
        return 0;


}

int JH_GK_UDP::F_OpenStream(void) {


    MySocketData data;
    T_NET_CMD_MSG_UDP Cmd;
    Cmd.checksum = UDP_CHECKSUM;
    Cmd.type = CMD_OPEN_STREAM;
    Cmd.session_id = session_id;

    T_NET_STREAM_CONTROL control;
    control.stream_type = nGKA_StreamNo;

    data.AppendData((uint8_t *) &Cmd, sizeof(T_NET_CMD_MSG_UDP));
    data.AppendData((uint8_t *) &control, sizeof(T_NET_STREAM_CONTROL));

    nStep = CMD_OPEN_STREAM;
    sentcmd(data.data,data.nLen,GK_UDP_CmdPort,sServerIP.c_str());



    //    GK_tcp_DataSocket.Write(&data);
    //    data.nLen = sizeof(T_REQ_MSG);
    //    GK_tcp_DataSocket.Read(&data);
    //    return 0;

    return 0;


}

int  JH_GK_UDP::F_CMD_FORCE_I(void) {
    MySocketData data;
    data.nLen = 0;
    T_NET_CMD_MSG_UDP Cmd;
    Cmd.checksum = UDP_CHECKSUM;
    Cmd.session_id = session_id;
    Cmd.type = CMD_FORCE_I;

    T_NET_STREAM_CONTROL streeam;
    streeam.stream_type = nGKA_StreamNo;
    data.AppendData((uint8_t *) &Cmd, sizeof(T_NET_CMD_MSG_UDP));
    data.AppendData((uint8_t *) &streeam, sizeof(T_NET_STREAM_CONTROL));
    nStep = CMD_FORCE_I;
    sentcmd(data.data,data.nLen,GK_UDP_CmdPort,sServerIP.c_str());
    return 0;

}

bool JH_GK_UDP::Stop(void)
{
    isCancelled=true;
    usleep(1000*20);
    if(video_fd>0) {
        close(video_fd);
        video_fd = -1;
    }
    if(notice_fd>0) {
        close(notice_fd);
        notice_fd = -1;
    }
    if(cmd_fd>0) {
        close(cmd_fd);
        cmd_fd = -1;
    }
    bInited = false;
    return true;


}

bool JH_GK_UDP::InitByUDT(void)
{

    pthread_create(&video_id, NULL, doReceiveVideo_UDT, (void *) this); // 成功返回0，错误返回错误编号


    return 0;




}

bool JH_GK_UDP::Init(void)
{
     if(bInited)
     {
         return true;
     }

    bInited = false;

    bzero(&myaddr_video,sizeof(struct sockaddr_in));
    bzero(&myaddr_cmd,sizeof(struct sockaddr_in));
    bzero(&myaddr_notic,sizeof(struct sockaddr_in));

    int nAddr = 1;
    int nPorta = 1;
#ifdef USE_UDT
    InitByUDT();
#else

    video_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(video_fd<0)
    {
        return false;
    }

    nAddr = 1;
    setsockopt(video_fd, SOL_SOCKET, SO_REUSEADDR, &nAddr, sizeof(int));
    nPorta = 1;
    setsockopt(video_fd, SOL_SOCKET, SO_REUSEPORT, &nPorta, sizeof(int));



    int nRecvBufLen = 1000 * 1000; //设置为32K
    int nSize = sizeof(int);
    int status = setsockopt(video_fd, SOL_SOCKET, SO_RCVBUF, (char *) &nRecvBufLen, sizeof(int));
    nRecvBufLen = 0;
    status = getsockopt(video_fd,SOL_SOCKET,SO_RCVBUF,&nRecvBufLen,&nSize);

    if (bind(video_fd, (struct sockaddr *) &myaddr_video, sizeof(myaddr_video)) < 0) {
        LOGE("Bind GK_UDP video_fd error!");
        return false;
    }

#endif

    notice_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(notice_fd<0)
    {
        return false;
    }
    cmd_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(cmd_fd<0)
    {
        return false;
    }







    nAddr = 1;
    setsockopt(notice_fd, SOL_SOCKET, SO_REUSEADDR, &nAddr, sizeof(int));
    nPorta = 1;
    setsockopt(notice_fd, SOL_SOCKET, SO_REUSEPORT, &nPorta, sizeof(int));

    nAddr = 1;
    setsockopt(cmd_fd, SOL_SOCKET, SO_REUSEADDR, &nAddr, sizeof(int));
    nPorta = 1;
    setsockopt(cmd_fd, SOL_SOCKET, SO_REUSEPORT, &nPorta, sizeof(int));



    myaddr_notic.sin_family = AF_INET;
    myaddr_notic.sin_addr.s_addr = htonl(INADDR_ANY);
    myaddr_notic.sin_port = htons(GK_UDP_Notic_RevPort);

    if (bind(notice_fd, (struct sockaddr *) &myaddr_notic, sizeof(myaddr_notic)) < 0) {
        LOGE("Bind GK_UDP notic_fd error!");
        return false;
    }
    myaddr_cmd.sin_family = AF_INET;
    myaddr_cmd.sin_addr.s_addr = htonl(INADDR_ANY);
    myaddr_cmd.sin_port = htons(GK_UDP_Cmd_RevPort);
    if (bind(cmd_fd, (struct sockaddr *) &myaddr_cmd, sizeof(myaddr_cmd)) < 0) {
        LOGE("Bind GK_UDP cmd_fd error!");
        return false;
    }




    isCancelled = false;
    int ret = 0;
#ifndef USE_UDT
    if (video_id == -1) {
        pthread_create(&video_id, NULL, doReceiveVideo, (void *) this); // 成功返回0，错误返回错误编号
    }
#endif
    if (cmd_id == -1) {
        pthread_create(&cmd_id, NULL, doReceiveCmd, (void *) this); // 成功返回0，错误返回错误编号
    }
    if (notic_id == -1) {
        pthread_create(&notic_id, NULL, doReceiveNotic, (void *) this); // 成功返回0，错误返回错误编号
    }

    F_Login("sima");

    bInited = true;

    return true;
}


void *JH_GK_UDP::doReceiveCmd(void *dat)
{
    JH_GK_UDP *self = (JH_GK_UDP *)dat;


    struct sockaddr_in servaddr;
    int size = (int)(sizeof(struct sockaddr_in));
    bzero(&servaddr,size);

    fd_set rfd;     // 读描述符集
    int nbytes=0;

    while(!self->isCancelled)
    {
        struct timeval timeoutA = {0, 1000};     //
        FD_ZERO(&rfd); // 在使用之前总是要清空
        FD_SET(self->cmd_fd, &rfd); // 把socka放入要测试的描述符集中
        int nRet = select(self->cmd_fd+1, &rfd, NULL, NULL, &timeoutA);// 检测是否有套接口是否可读
        if (nRet >0) {
            int nRecEcho = 0;
            if (FD_ISSET(self->cmd_fd, &rfd))
            {
                nbytes = recvfrom(self->cmd_fd, self->cmdBuff, 100, 0,
                                  (struct sockaddr *) &servaddr, (socklen_t *) &size);
                if(nbytes>=sizeof(T_REQ_MSG_UDP))
                {
                    T_REQ_MSG_UDP *requdp=(T_REQ_MSG_UDP *)self->cmdBuff;
                    if(self->nStep == CMD_LOGIN && requdp->type == CMD_LOGIN)
                    {
                        if(requdp->ret == 0)
                        {
                            self->session_id = requdp->session_id;
                           // self->F_GetSDInfo();
                            self->F_GetAdjTime();
                        }
                    }

                    if(self->nStep == CMD_GET_CONFIG && requdp->type == CMD_GET_CONFIG)
                    {
                        if(requdp->ret == 0)
                        {
                            self->F_GetAdjTime();
                        }
                    }

                    if(self->nStep == CMD_ADJUST_TIME && requdp->type == CMD_ADJUST_TIME)
                    {
                        if(requdp->ret == 0)
                        {
                            self->F_OpenStream();
                        }
                    }
                    if(self->nStep == CMD_OPEN_STREAM && requdp->type == CMD_OPEN_STREAM)
                    {
                        if(requdp->ret == 0)
                        {
                            self->F_CMD_FORCE_I();
                        }
                    }
                }
            }
        }
        else
        {
            continue;
        }
    }
    LOGE("Exit CmdThread KG_UDP!");
    return NULL;
}
void *JH_GK_UDP::doReceiveNotic(void *dat)
{

    JH_GK_UDP *self = (JH_GK_UDP *)dat;


    struct sockaddr_in servaddr;
    int size = (int)(sizeof(struct sockaddr_in));
    bzero(&servaddr,size);

    fd_set rfd;     // 读描述符集
    int nbytes=0;
    while(!self->isCancelled)
    {
        struct timeval timeoutA = {0, 1000};     //
        FD_ZERO(&rfd); // 在使用之前总是要清空
        FD_SET(self->notice_fd, &rfd); // 把socka放入要测试的描述符集中
        int nRet = select(self->notice_fd+1, &rfd, NULL, NULL, &timeoutA);// 检测是否有套接口是否可读
        if (nRet >0) {
            int nRecEcho = 0;
            if (FD_ISSET(self->notice_fd, &rfd))
            {
                nbytes = recvfrom(self->notice_fd, self->noticBuff, 100, 0,
                                  (struct sockaddr *) &servaddr, (socklen_t *) &size);
                if(nbytes>0){

                }
            }
        }
        else
        {
            continue;
        }
    }
    LOGE("Exit NoticThread KG_UDP!");
    return NULL;
}
JPEG_BUFFER * JH_GK_UDP::F_ClearJpegBuffer(void)
{

    int ix0= jpg0.nJpegInx;
    int ix1= jpg1.nJpegInx;
    int ix2= jpg2.nJpegInx;

    if(ix0>ix1)
    {
        if(ix1>ix2)
        {
            jpg2.Clear();
            return &jpg2;
        }
        else
        {
            jpg1.Clear();
            return &jpg1;
        }
    }
    else
    {
        if(ix0<ix2)
        {
            jpg0.Clear();
            return &jpg0;
        }
        else
        {
            jpg2.Clear();
            return &jpg2;
        }
    }
}

JPEG_BUFFER *JH_GK_UDP::F_FindOKBuffer(uint32_t nIndex)
{
    if(jpg0.bOK && jpg0.nJpegInx == nIndex)
    {
            return &jpg0;
    }
    if(jpg1.bOK && jpg1.nJpegInx == nIndex)
    {
        return &jpg1;
    }
    if(jpg2.bOK && jpg2.nJpegInx == nIndex)
    {
        return &jpg2;
    }
/*
    if(jpg3.bOK && jpg3.nJpegInx == nIndex)
    {
        return &jpg3;
    }

    if(jpg4.bOK && jpg4.nJpegInx == nIndex)
    {
        return &jpg4;
    }
    if(jpg5.bOK && jpg5.nJpegInx == nIndex)
    {
        return &jpg5;
    }
*/
    return NULL;


}

JPEG_BUFFER *JH_GK_UDP::F_FindJpegBuffer(int njpginx)
{

    if(jpg0.nJpegInx == njpginx) {
        return &jpg0;
    }
    else if(jpg1.nJpegInx == njpginx) {
        return &jpg1;
    }
    else if(jpg2.nJpegInx == njpginx) {
        return &jpg2;
    }
    else if(jpg0.nJpegInx == 0)
    {
        jpg0.Clear();
        return &jpg0;
    }
    else if(jpg1.nJpegInx == 0) {
        jpg1.Clear();
        return &jpg1;
    }
    else if(jpg2.nJpegInx == 0) {
        jpg2.Clear();
        return &jpg2;
    }
    else
        return F_ClearJpegBuffer(); //清除最下inx的JPEG

}

void *JH_GK_UDP::doReceiveVideo_UDT(void *dat)
{


    JH_GK_UDP *self = (JH_GK_UDP *)dat;


    self->jpg0.Release();
    self->jpg0.buffer =(uint8_t   *) malloc(GK_UDP_PackMax);


    self->jpg1.Release();
    self->jpg1.buffer =(uint8_t   *) malloc(GK_UDP_PackMax);

    self->jpg2.Release();
    self->jpg2.buffer =(uint8_t   *) malloc(GK_UDP_PackMax);




    addrinfo hints;
    addrinfo* res;

    memset(&hints, 0, sizeof(struct addrinfo));

    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;//SOCK_STREAM;

    string service("34952"); //0x8888;

    if (0 != getaddrinfo(NULL, service.c_str(), &hints, &res))
    {
        //cout << "illegal port number or port is busy.\n" << endl;
        return NULL;
    }
    self->serv = UDT::socket(res->ai_family, res->ai_socktype, res->ai_protocol);

     //UDT Options
    //UDT::setsockopt(self->serv, 0, UDT_CC, new CCCFactory<CUDPBlast>, sizeof(CCCFactory<CUDPBlast>));
    UDT::setsockopt(self->serv, 0, UDT_MSS, new int(1470), sizeof(int));
    UDT::setsockopt(self->serv, 0, UDT_RCVBUF, new int(1000000), sizeof(int));
    UDT::setsockopt(self->serv, 0, UDP_RCVBUF, new int(1000000), sizeof(int));



    if (UDT::ERROR == UDT::bind(self->serv, res->ai_addr, res->ai_addrlen))
    {
        return NULL;
    }
    freeaddrinfo(res);


    if (UDT::ERROR == UDT::listen(self->serv, 10))
    {
        return NULL;
    }
    sockaddr_storage clientaddr;
    int addrlen = sizeof(clientaddr);
    UDTSOCKET recver;
    {
        if (UDT::INVALID_SOCK == (recver = UDT::accept(self->serv, (sockaddr*)&clientaddr, &addrlen)))
        {
            return NULL;
        }
        int nHeadSize = (int)sizeof(T_NET_FRAME_HEADER_UDP);
        T_NET_FRAME_HEADER_UDP  *pHeader;
        int32_t  nFrameNo=0;
        int     nDaF=0;
        bool bKeyFrame;
        int32_t jpginx;
        int jpg_pack_count;
        int jpg_udp_inx;
        int data_len;
        uint8 *pH264Data=NULL;
        int nPreFrame=-1;
        MySocketData  frameData;
        while(!self->isCancelled)
        {
            int rcvd = UDT::recvmsg(recver, (char *) (self->readBuff), GK_UDP_ReadBuffLen);
            if (rcvd > 0) {
                pHeader = (T_NET_FRAME_HEADER_UDP *)self->readBuff;
                nDaF = (int)(pHeader->frame_no - nFrameNo);

                if(nDaF>1 || nDaF<0)
                {
                    LOGE(">>>>>>>>>>>>>>>>>>>>>>2 %00000008X - %00000008X = %d ",pHeader->frame_no,nFrameNo,nDaF);
                }
                else
                {
                //    LOGE(">>>>>>>>>>>>>>>2 %00000008X - %00000008X = %d ",pHeader->frame_no,nFrameNo,nDaF);
                    ;
                }
                nFrameNo = pHeader->frame_no;
                data_len = rcvd-nHeadSize;
                pHeader->pack_no--;
                jpginx = pHeader->frame_no;
                //nCurFrame = jpginx;
                jpg_pack_count = pHeader->pack_count;
                jpg_udp_inx = pHeader->pack_no;
                pH264Data = self->readBuff+nHeadSize;

                if(pHeader->frame_type == NET_FRAME_TYPE_I)
                {
                    bKeyFrame = true;
                } else
                {
                    bKeyFrame = false;
                }
              //  LOGE("len = %d ,frameno= %00000008X ,Pack No= %00000008X  PackCount = %00000008X, KeyFrame = %d",rcvd,pHeader->frame_no,pHeader->pack_no,pHeader->pack_count,bKeyFrame);
                JPEG_BUFFER *jpg = self->F_FindJpegBuffer(jpginx);
                if(jpg!=NULL)
                {
                    if (jpg->nJpegInx == 0 || jpg->nJpegInx == jpginx) {
                        jpg->nJpegInx = jpginx;
                        jpg->bKeyFrame = bKeyFrame;
                    }
                }
                bool bOK = false;
                if (jpg->nSize + data_len <= GK_UDP_PackMax)
                {
                    if (jpg->mInx[jpg_udp_inx] == 0)
                    {
                        jpg->mInx[jpg_udp_inx] = 1;
                        memcpy(jpg->buffer + jpg->nSize, pH264Data, data_len);
                        jpg->nCount++;
                        jpg->nSize += data_len;
                    }
                    else
                    {
                        LOGE("Duplicate Recivied  packet  %d of %d", jpg_udp_inx, jpginx);
                    }
                    if (jpg->nCount >= jpg_pack_count)
                    {
                        jpg->nCount = jpg_pack_count;
                        bOK = true;
                        for (int ix = 0; ix < jpg_pack_count; ix++) {
                            if (jpg->mInx[ix] == 0) {
                                bOK = false;
                                break;
                            }
                        }
                        jpg->bOK = bOK;
                    }
                }
                JPEG_BUFFER *minok = NULL;
                if(jpg->bOK)
                {
                    if(jpg->bKeyFrame)
                    {
                        minok = jpg;
                        nPreFrame = minok->nJpegInx;
                        int sps = 0;
                        int pps = 0;
                        for (sps = 0; sps < minok->nSize;)
                        {
                            if (minok->buffer[sps++] == 0x00 && minok->buffer[sps++] == 0x00 && minok->buffer[sps++] == 0x00
                                && minok->buffer[sps++] == 0x01) {
                                break;
                            }
                        }
                        for (pps = sps; pps < minok->nSize;)
                        {
                            if (minok->buffer[pps++] == 0x00 && minok->buffer[pps++] == 0x00 && minok->buffer[pps++] == 0x00
                                && minok->buffer[pps++] == 0x01) {
                                break;
                            }
                        }
                        if (sps >= minok->nSize || pps >= minok->nSize) {
                            ;
                        } else {
                            frameData.nLen = 0;
                            frameData.AppendData(minok->buffer + sps, pps - sps - 4);
                            frameData.AppendData((void *) "abcd", 4);
                            m_FFMpegPlayer.decodeAndRender_GKA(&frameData);
                            frameData.nLen = 0;
                            frameData.AppendData(minok->buffer + pps, 4);
                            frameData.AppendData((void *) "abcd", 4);
                            m_FFMpegPlayer.decodeAndRender_GKA(&frameData);
                            frameData.nLen = 0;
                            frameData.AppendData(minok->buffer + pps + 4 + 4, minok->nSize - pps - 4);
                            m_FFMpegPlayer.decodeAndRender_GKA(&frameData);
                        }
                        minok->Clear();
                    }
                    else
                    {
                        minok = jpg;
                        //if(nPreFrame>0)
                        {
                          //  minok = self->F_FindOKBuffer(nPreFrame + 1);
                            if(minok!=NULL)
                            {
                                nPreFrame = minok->nJpegInx;
                                frameData.nLen = 0;
                                frameData.AppendData(minok->buffer + 4, minok->nSize - 4);
                                frameData.AppendData((void *) "abcd", 4);
                                m_FFMpegPlayer.decodeAndRender_GKA(&frameData);
                                minok->Clear();
                            }
                            else
                            {
                                ;
                            }
                        }
                    }
                }
            }
        }
    }
    LOGE("Exit Rev_UDT Thread!!!!");
    close(self->serv);
    self->jpg0.Release();
    self->jpg1.Release();
    self->jpg2.Release();

    return NULL;

}


void *JH_GK_UDP::doReceiveVideo(void *dat)
{

    JH_GK_UDP *self = (JH_GK_UDP *)dat;


    struct sockaddr_in servaddr;
    int size = (int)(sizeof(struct sockaddr_in));
    bzero(&servaddr,size);

    fd_set rfd;     // 读描述符集
    int nbytes=0;



    self->jpg0.Clear();
    self->jpg0.buffer =(uint8_t   *) malloc(GK_UDP_PackMax);


    self->jpg1.Clear();
    self->jpg1.buffer =(uint8_t   *) malloc(GK_UDP_PackMax);

    self->jpg2.Clear();
    self->jpg2.buffer =(uint8_t   *) malloc(GK_UDP_PackMax);




    T_NET_FRAME_HEADER_UDP  *pHeader;
    T_NET_FRAME_HEADER_UDP  *pHeader1;
    int nHeadSize = (int)sizeof(T_NET_FRAME_HEADER_UDP);
    uint8 *pH264Data=NULL;

    int jpginx=0;
    int  jpg_pack_count=0;
    int  jpg_udp_inx = 0;

   // int pak_len = 65507-nHeadSize;
    MySocketData  frameData;
    int  data_len=0;
    int32_t  nPreFrame=-1;
    int32_t  nCurFrame=-1;
    bool     bKeyFrame = false;
    bool    bStart=false;
    int32_t  nFrameNo=0;
    int     nDaF=0;
    while(!self->isCancelled)
    {
        struct timeval timeoutA = {0, 1000};     //
        //FD_ZERO(&rfd); // 在使用之前总是要清空
        //FD_SET(self->video_fd, &rfd); // 把socka放入要测试的描述符集中
        //int nRet = select(self->video_fd+1, &rfd, NULL, NULL, &timeoutA);// 检测是否有套接口是否可读
        // if (nRet >0)
        if(true)
        {

            //if (FD_ISSET(self->video_fd, &rfd))
            {
               // bzero(self->readBuff,GK_UDP_ReadBuffLen);
               // nbytes = recvfrom(self->video_fd, self->readBuff, GK_UDP_ReadBuffLen, 0,
               //                        (struct sockaddr *) &servaddr, (socklen_t *) &size);
                nbytes = UDT::recv(self->serv, (char *)(self->readBuff), GK_UDP_ReadBuffLen,0);
                if(nbytes>0){
                    F_ResetRelinker();
                    if(nbytes>nHeadSize)
                    {
                        data_len = nbytes-nHeadSize;
                        pH264Data = self->readBuff+nHeadSize;
                        pHeader = (T_NET_FRAME_HEADER_UDP *)self->readBuff;
                        nDaF = (int)(pHeader->frame_no - nFrameNo);
                        nFrameNo = pHeader->frame_no;
                        if(nDaF>=2)
                        {
                            LOGE(">>>>>>>>>>>>>2 %d ",nDaF);
                        } else
                        {
                          //  LOGE(">>>>>>1 %d ",nDaF);
                        }

                        pHeader->pack_no--;
                        jpginx = pHeader->frame_no;
                        //nCurFrame = jpginx;
                        jpg_pack_count = pHeader->pack_count;
                        jpg_udp_inx = pHeader->pack_no;
                        if(pHeader->frame_type == NET_FRAME_TYPE_I)
                        {
                            bKeyFrame = true;
                        } else
                        {
                            bKeyFrame = false;
                        }

                        LOGE("len = %d ,frameno= %00000008X ,Pack No= %00000008X  PackCount = %00000008X, KeyFrame = %d",nbytes,pHeader->frame_no,pHeader->pack_no,pHeader->pack_count,bKeyFrame);
#if 0

                        JPEG_BUFFER *jpg = self->F_FindJpegBuffer(jpginx);
                        if(jpg!=NULL)
                        {
                            if (jpg->nJpegInx == 0 || jpg->nJpegInx == jpginx) {
                                jpg->nJpegInx = jpginx;
                                jpg->bKeyFrame = bKeyFrame;
                            }
                        }


                        bool bOK = false;
                        if (jpg->nSize + data_len <= GK_UDP_PackMax)
                        {
                            if (jpg->mInx[jpg_udp_inx] == 0)
                            {
                                jpg->mInx[jpg_udp_inx] = 1;
                                memcpy(jpg->buffer + jpg->nSize, pH264Data, data_len);
                                jpg->nCount++;
                                jpg->nSize += data_len;
                            }
                            else
                            {
                                LOGE("Duplicate Recivied  packet  %d of %d", jpg_udp_inx, jpginx);
                            }
                            if (jpg->nCount >= jpg_pack_count)
                            {
                                bOK = true;
                                for (int ix = 0; ix < jpg_pack_count; ix++) {
                                    if (jpg->mInx[ix] == 0) {
                                        bOK = false;
                                        break;
                                    }
                                }
                                jpg->bOK = bOK;
                            }

                        }

                        JPEG_BUFFER *minok =NULL ;//= self->F_FindOKBuffer();
                        {
                                    if(jpg->bKeyFrame)
                                    {
                                        minok = jpg;
                                      //  LOGE("dA = %d",minok->nJpegInx-nPreFrame);
                                        nPreFrame = minok->nJpegInx;
                                        int sps = 0;
                                        int pps = 0;
                                        for (sps = 0; sps < minok->nSize;) {
                                            if (minok->buffer[sps++] == 0x00 && minok->buffer[sps++] == 0x00 && minok->buffer[sps++] == 0x00
                                                && minok->buffer[sps++] == 0x01) {
                                                break;
                                            }
                                        }
                                        for (pps = sps; pps < minok->nSize;) {
                                            if (minok->buffer[pps++] == 0x00 && minok->buffer[pps++] == 0x00 && minok->buffer[pps++] == 0x00
                                                && minok->buffer[pps++] == 0x01) {
                                                break;
                                            }
                                        }
                                        if (sps >= minok->nSize || pps >= minok->nSize) {
                                            //return 0;
                                        } else {
                                            frameData.nLen = 0;
                                            frameData.AppendData(minok->buffer + sps, pps - sps - 4);
                                            frameData.AppendData((void *) "abcd", 4);
                                            m_FFMpegPlayer.decodeAndRender_GKA(&frameData);
                                            frameData.nLen = 0;
                                            frameData.AppendData(minok->buffer + pps, 4);
                                            frameData.AppendData((void *) "abcd", 4);
                                            m_FFMpegPlayer.decodeAndRender_GKA(&frameData);

                                            frameData.nLen = 0;
                                            frameData.AppendData(minok->buffer + pps + 4 + 4, minok->nSize - pps - 4);
                                            m_FFMpegPlayer.decodeAndRender_GKA(&frameData);
                                            //LOGE("Receive data22!");
                                            minok->Clear();
                                        }
                                    }
                                    else
                                    {
                                        if(nPreFrame>0)
                                        {
                                            minok = self->F_FindOKBuffer(nPreFrame + 1);
                                            if(minok!=NULL) {
                                                    nPreFrame = minok->nJpegInx;
                                                    frameData.nLen = 0;
                                                    frameData.AppendData(minok->buffer + 4, minok->nSize - 4);
                                                    frameData.AppendData((void *) "abcd", 4);
                                                    m_FFMpegPlayer.decodeAndRender_GKA(&frameData);
                                                    minok->Clear();
                                            }
                                            else
                                            {
                                                 ;
                                            }
                                        }
                                    }
                        }



#if 0
                            if (jpg->nCount >= jpg_pack_count)
                            {
                                bool bOK=true;
                                for(int ix=0;ix<jpg_pack_count;ix++)
                                {
                                    if(jpg->mInx[ix]==0)
                                    {
                                        bOK=false;
                                        break;
                                    }
                                }
                                if(bOK)
                                {

                                    if(nPreFrame<0)
                                    {
                                        if(jpg->bKeyFrame)
                                        {
                                            nPreFrame = jpg->nJpegInx;
                                            int sps, pps;
                                            for (sps = 0; sps < jpg->nSize;)
                                                if (jpg->buffer[sps++] == 0x00 && jpg->buffer[sps++] == 0x00 && jpg->buffer[sps++] == 0x00
                                                    && jpg->buffer[sps++] == 0x01)
                                                    break;
                                            for (pps = sps; pps < jpg->nSize;)
                                                if (jpg->buffer[pps++] == 0x00 && jpg->buffer[pps++] == 0x00 && jpg->buffer[pps++] == 0x00
                                                    && jpg->buffer[pps++] == 0x01)
                                                    break;
                                            if (sps >= jpg->nSize || pps >= jpg->nSize)
                                            {
                                                //return 0;
                                            } else
                                            {
                                                frameData.nLen = 0;
                                                frameData.AppendData(jpg->buffer + sps, pps - sps - 4);
                                                frameData.AppendData((void *)"abcd",4);
                                                m_FFMpegPlayer.decodeAndRender_GKA(&frameData);
                                                frameData.nLen = 0;
                                                frameData.AppendData(jpg->buffer + pps, 4);
                                                frameData.AppendData((void *)"abcd",4);
                                                m_FFMpegPlayer.decodeAndRender_GKA(&frameData);

                                                frameData.nLen = 0;
                                                frameData.AppendData(jpg->buffer + pps+4+4, jpg->nSize-pps-4);
                                                m_FFMpegPlayer.decodeAndRender_GKA(&frameData);
                                            }
                                        }
                                    }
                                    else
                                    {
                                         if(jpg->nJpegInx-nPreFrame!=1)
                                         {
                                             LOGE("Receive Error pre = %d, Current = %d",nPreFrame,jpg->nJpegInx);
                                             nPreFrame=-1;
                                         }
                                         else
                                         {
                                             nPreFrame = jpg->nJpegInx;
                                             if(jpg->bKeyFrame) {

                                                 nPreFrame = jpg->nJpegInx;
                                                 int sps, pps;
                                                 for (sps = 0; sps < jpg->nSize;)
                                                     if (jpg->buffer[sps++] == 0x00 && jpg->buffer[sps++] == 0x00 && jpg->buffer[sps++] == 0x00
                                                         && jpg->buffer[sps++] == 0x01)
                                                         break;
                                                 for (pps = sps; pps < jpg->nSize;)
                                                     if (jpg->buffer[pps++] == 0x00 && jpg->buffer[pps++] == 0x00 && jpg->buffer[pps++] == 0x00
                                                         && jpg->buffer[pps++] == 0x01)
                                                         break;
                                                 if (sps >= jpg->nSize || pps >= jpg->nSize)
                                                 {
                                                     //return 0;
                                                 } else
                                                 {
                                                     frameData.nLen = 0;
                                                     frameData.AppendData(jpg->buffer + sps, pps - sps - 4);
                                                     frameData.AppendData((void *)"abcd",4);
                                                     m_FFMpegPlayer.decodeAndRender_GKA(&frameData);
                                                     frameData.nLen = 0;
                                                     frameData.AppendData(jpg->buffer + pps, 4);
                                                     frameData.AppendData((void *)"abcd",4);
                                                     m_FFMpegPlayer.decodeAndRender_GKA(&frameData);

                                                     frameData.nLen = 0;
                                                     frameData.AppendData(jpg->buffer + pps+4+4, jpg->nSize-pps-4);
                                                     m_FFMpegPlayer.decodeAndRender_GKA(&frameData);
                                                     //LOGE("Receive data22!");

                                                 }

                                             } else
                                             {
                                                 frameData.nLen = 0;
                                                 frameData.AppendData(jpg->buffer+4, jpg->nSize-4);
                                                 frameData.AppendData((void *)"abcd",4);
                                                 m_FFMpegPlayer.decodeAndRender_GKA(&frameData);
                                                // LOGE("Receive data33!");
                                             }
                                         }

                                    }

                                }
                                else
                                {
                                    LOGE("receive error!");
                                }
                                jpg->Clear();
                            }
#endif
                        //}
#endif

                    }

                }
            }
        }
        /*
        else
        {
            continue;
        }
         */
    }
    LOGE("Exit ReadVideoThread KG_UDP!");

    self->jpg0.Release();
    self->jpg1.Release();
    self->jpg2.Release();

    return NULL;
}

int JH_GK_UDP::FindHead(MySocketData *dat, int pos) {

    int ix;
    if (dat->nLen < 4)
        return -1;
    uint32_t magic = 0X01000000;
    uint32_t  data=0;
    uint32_t *p;
    for (ix = pos; ix <= dat->nLen - 4; ix++) {
        data = *((uint32_t *) (dat->data + ix));

        if (magic == data) {
            return ix;
        }
    }
    return -1;
}
