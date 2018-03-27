//
// Created by AivenLau on 2017/12/14.
//

#ifndef SYMADEMO_ANDROID_JH_GK_UDP_H
#define SYMADEMO_ANDROID_JH_GK_UDP_H


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
#include "libavutil/opt.h"

#ifdef __cplusplus
}
#endif


#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include "MySocketData.h"

#include <android/native_window.h>
#include <android/native_window_jni.h>
#include "media/NdkMediaCodec.h"


#include <algorithm>
#include <string>
#include <iostream>
#include <cctype>

#include "FFMpegPlayerCore.h"
#include "Defines.h"
#include <vector>
#include <queue>


#include <jni.h>
#include <unistd.h>
#include <fcntl.h>


#include "mp4v2.h"
#include "MyFrame.h"
#include "libyuv.h"

#include <unistd.h>

#include <endian.h>
#include "MySonix.h"
#include "Defines.h"
#include "JPEG_BUFFER.h"
#include <netdb.h>
#include <arpa/inet.h>
#include <set>

#include <udt.h>

#define GK_UDP_CmdPort      0x8887
#define GK_UDP_Cmd_RevPort   0x8886
#define GK_UDP_Video_RevPort   0x8888
#define GK_UDP_Notic_RevPort   0x8889


#define  GK_UDP_PackMax  1024*500


#define  GK_UDP_ReadBuffLen  1024*100


#define UDP_CHECKSUM 0x12345678

typedef  multiset<JPEG_BUFFER *> My_multiset;
typedef  multiset<JPEG_BUFFER *>::iterator My_multiset_iterator;

//typedef  vector<JPEG_BUFFER *>  My_Vector;
//typedef  vector<JPEG_BUFFER *>::iterator  My_Vector_iterator;


#define   USE_UDT

extern C_FFMpegPlayer m_FFMpegPlayer;
extern std::string    sServerIP;

void F_ResetRelinker();

class JH_GK_UDP {

public:
    JH_GK_UDP();
    ~JH_GK_UDP();


    bool Init(void);
    bool Stop(void);
    bool isCancelled;
    pthread_t video_id;
    pthread_t cmd_id;
    pthread_t notic_id;


    UDTSOCKET   serv;

    UDTSOCKET   *clicent_udt;

    int   video_fd;
    int   notice_fd;
    int   cmd_fd;

    int    sentCmd_fd;

    uint8 *readBuff;
    uint8 *noticBuff;
    uint8 *cmdBuff;

    JPEG_BUFFER   jpg0;
    JPEG_BUFFER   jpg1;
    JPEG_BUFFER   jpg2;
    //JPEG_BUFFER   jpg3;
    //JPEG_BUFFER   jpg4;
    //JPEG_BUFFER   jpg5;

    bool sentcmd(uint8 *data,int nLen,int port,const char *server);
    int nStep;
    int nGKA_StreamNo;
    My_multiset myMultiset;
    //My_Vector  myVector;


    bool InitByUDT(void);



private:


    struct sockaddr_in myaddr_video;
    struct sockaddr_in myaddr_cmd;
    struct sockaddr_in myaddr_notic;

    int session_id;
    int F_Login(const char *sCustomer);
    int F_GetSDInfo(void);
    int F_GetAdjTime(void);
    int F_OpenStream(void);
    int F_CMD_FORCE_I(void);

    JPEG_BUFFER *F_FindOKBuffer(uint32_t nIndex);


    static void *doReceiveVideo_UDT(void *dat);

    static void *doReceiveVideo(void *dat);
    static void *doReceiveCmd(void *dat);
    static void *doReceiveNotic(void *dat);

    int  FindHead(MySocketData *dat, int pos);


    bool   bInited;


    JPEG_BUFFER *F_FindJpegBuffer(int njpginx);
    JPEG_BUFFER * F_ClearJpegBuffer(void);





};


#endif //SYMADEMO_ANDROID_JH_GK_UDP_H
