//
// Created by generalplus_sa1 on 4/29/16.
//
#ifndef FFMPEGTEST_C_FFMPEGPLAYER_H
#define FFMPEGTEST_C_FFMPEGPLAYER_H
//----------------------------------------------------------------------
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

#include "mp4v2.h"
#include "MyFrame.h"
#include "libyuv.h"


#include "h264_rbsp.h"
#include "sps_parse.h"
#include "pps_parse.h"
//#include "myOpenCV.h"


#define  FFMPEG3_3_3


//----------------------------------------------------------------------
#define FFMPEGPLAYER_NOERROR                     0
#define FFMPEGPLAYER_INITMEDIAFAILED             1
#define FFMPEGPLAYER_MEDIAISPLAYING              2
#define FFMPEGPLAYER_CREATESAVESTREAMFAILED      3
#define FFMPEGPLAYER_SAVESNAPSHOTFAILED          4
#define FFMPEGPLAYER_SAVEVIDEOFAILED             5


typedef unsigned char byte;
#define   IC_NO         -1
#define   IC_GK         0
#define   IC_GP         1
#define   IC_SN         2
#define   IC_GKA        3
#define   IC_GPRTSP     4
#define   IC_GPH264     5
#define   IC_GPRTP      6
#define   IC_GPH264A    7
#define   IC_GPRTPB      8
#define   IC_GK_UDP      9


#define  bit0_OnLine            1
#define  bit1_LocalRecording    2
//#define  SD_Ready               4
//#define  SD_Recroding           8
#define  SD_Photo               0x10




//IC_GK = 0,      //192.168.234.X
//IC_GP,          //192.168.25.X
//IC_SN,          //192.168.123.X
//IC_GKA,        //175.16.10.X
//IC_GPRTSP,   //192.168.26.X
//IC_GPH264,   //192.168.27.X
//IC_GPRTP,    //192.168.28.X   GP
//IC_GPRTPB,   //192.168.29.X   JH
//IC_GPH264A,   //192.168.30.X


//#define   D_H264file



extern bool bInitMediaA;

typedef enum {
    E_PlayerStatus_Stoped,
    E_PlayerStatus_Playing,
    E_PlayerStatus_Stoping,

} E_PlayerStatus;


typedef struct
{
    uint8  r;
    uint8  g;
    uint8  b;
    uint8  a;
}RGBA_STRUCT;

//----------------------------------------------------------------------
class C_FFMpegPlayer {


    bool MP4AacGetConfiguration(uint8_t** ppConfig,
                                                uint32_t* pConfigLength,
                                                uint8_t profile,
                                                uint32_t samplingRate,
                                                uint8_t channels);
    uint8_t MP4AdtsFindSamplingRateIndex(uint32_t samplingRate);

public:

    E_PlayerStatus m_Status;
    uint64_t    nErrorFrame;
    C_FFMpegPlayer();
    ~C_FFMpegPlayer();

    MP4TrackId video;
    MP4TrackId  music;
    MP4FileHandle fileHandle;

    bool  bIsH264;
    int  nSpsSet;
    bool  bStatWrite;
    bool  bStarDecord;
    std::string  sRecordFileName;
    std::string  sRecordFileName_tmp;

#ifdef  D_H264file
    int     h64fileHandle;
    std::string  sH264FileName;
#endif


    int   nRecordWidth;
    int   nRecordHeight;

    void F_ReSetRecordWH(int w,int h);
    void F_InitFFfmpeg(void);

    bool b480;
    bool bFollow;
    bool bContinue;
    unsigned char *Rgbabuffer;
    unsigned char *YUVbuffer;
    int      nSecT;


    int nDisplayWidth;
    int nDisplayHeight;
    bool nNeedRedraw;

    AVFrame *pFrameYUV_D;

    bool F_RecreateEnv(void);
    void F_DispSurface();


    int InitMediaGK(void);
    int InitMediaSN(void);

    AVPacket *F_GetPacket();
    int decodeAndRender_GKA(MySocketData *dat);
    int decodeAndRender_GKA_B(MySocketData *dat);

    void F_StartDispThread();


    int decodeAndRender_SN(char *data, int nLen);
    int decodeAndRender_RTP(char *data,int nLen);


    uint8_t nIC_Type;

    int InitMedia(const char *path);
    int PlayMedia();
    int Stop();
    int SaveSnapshot(const char *path);
    int GetStatus(void);
    int SaveVideo(const char *path,bool bisH264=false);
    int StopSaveVideo();
    int AddMp4Video(uint8_t *sps,int len1,uint8_t * pps,int len2);
    int WriteMp4Frame(uint8_t * data,int nLen,bool b);
    static void *decodeThreadFunction(void *param);
    static void *WriteThreadFunction(void *param);
    void ClearQueue();

    AVFrame *pFrameRecord;
    AVCodecContext *m_codecCtx;
    AVFrame *m_decodedFrame;
    void _DispDecordData(void);
private:



    int CloseVideo();

    int EncodeSnapshot();

    int decodeAndRender();

    int writeVideo();


    int  SetYUVFrame(AVFrame *yunframe);

    AVCodecParserContext *m_parser;
    AVCodec *codec;

    bool bPause;


    AVFormatContext *m_formatCtx;
    int m_videoStream;


    AVFrame *frame_a;
    AVFrame *frame_b;
    AVFrame *frame_c;

    AVFrame *frame_SnapBuffer;


    struct SwsContext *img_convert_ctx;
    //struct SwsContext *m_sws_ctx;

    int m_width;
    int m_height;

    pthread_t m_decodeThread;
    pthread_t m_writeThread;




    AVPixelFormat pix_format;

    pthread_mutex_t m_Frame_Queuelock;

    pthread_cond_t m_Frame_condition;

    bool   bCanDisplay;
    bool m_bSaveSnapshot;

    bool m_bSaveVideo;
    void F_DispH264NoBuffer(MySocketData *data);

    void F_AdjUV(int8_t u,int8_t v);

public:

    AVFrame *pFrameYUV;

    bool  F_WriteAudio(jbyte * data,int nLen);
    int Releaseffmpeg(void);



    char m_snapShotPath[256];
    char m_MediaPath[256];

    int  nfps;
    int  nRecFps;
    bool m_bOpenOK;
    bool bFlip;
    bool b3D;
    bool b3DA;

    bool bMirror;

    MySocketData keyFrame;
    AVPacket packet;



};


#endif //FFMPEGTEST_C_FFMPEGPLAYER_H
