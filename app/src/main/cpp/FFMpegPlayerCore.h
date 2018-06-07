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
#include "myOpenCV.h"


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


//#define D_BufferData

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
    bool  F_IsNeed2WriteH264Stream(void);
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


    int     h64fileHandle;
    std::string  sH264FileName;
    std::string  sH264FileName_tmp;

    int   nRecordWidth;
    int   nRecordHeight;

    void F_ReSetRecordWH(int w,int h);

    bool b480;
    bool bFollow;
    bool bContinue;
    unsigned char *Rgbabuffer;
    unsigned char *YUVbuffer;
    uint8_t sps[1024];
    int      sps_len;
    uint8_t pps[1024];
    int      pps_len;
    int      nSecT;
    int64_t  nStartTime;

    int nDisplayWidth;
    int nDisplayHeight;
    bool nNeedRedraw;

    bool F_RecreateEnv(void);


    void F_DispSurface();

    int InitMediaGPRTP(void);

    int InitMediaGK(void);

    int InitMediaSN(void);

    //void F_SendFrame(uint8_t *buffer, int nLen);


    AVPacket *F_GetPacket();


    AVFrame *F_GetFrame();

    int decodeAndRender_GKA(MySocketData *dat);

    int decodeAndRender_GKA_B(MySocketData *dat);

    static  void* F_DispThread(void *dat);
    void F_StartDispThread();
    pthread_t dispThreadid;

    int decodeAndRender_SN(char *data, int nLen);

    int decodeAndRender_RTP(char *data,int nLen);


  //  int SN_Width;
  //  int SN_Height;
    uint8_t nIC_Type;


    unsigned char *out_bufferBmp;
    int nDataCount;

    int InitMedia(const char *path);

    int PlayMedia();

    int Stop();


    int SaveSnapshot(const char *path);

    int GetStatus(void);

    int SaveVideo(const char *path,bool bisH264=false);

    int StopSaveVideo();
/*(
    int GetWidth() {
        if (m_codecCtx == NULL)
            return 0;

        return m_codecCtx->width;
    }

    int GetHeight() {
        if (m_codecCtx == NULL)
            return 0;

        return m_codecCtx->height;
    }
*/
/*
    AVPicture* GetDecodePicture()
    {
        return &m_frameRGBA;
    }
*/
    int SetResolution(int Width, int Height);

    int convertFrame();
    int AddMp4Video(uint8_t *sps,int len1,uint8_t * pps,int len2);
    int WriteMp4Frame(uint8_t * data,int nLen,bool b);



    static void *decodeThreadFunction(void *param);

    static void *WriteThreadFunction(void *param);

    //static void *WriteThreadFunctionA(void *param);

    int init_decode(AVFormatContext *s);
#if 0
    AVStream *create_stream(AVFormatContext *s, AVMediaType codec_type) {
        AVStream *st = avformat_new_stream(s, NULL);
        if (!st)
            return NULL;
        st->codec->codec_type = codec_type;
        return st;
    }

    int get_video_extradata(AVFormatContext *s, int video_index) {

        int size;//, type, flags, pos, stream_type;
        int ret = -1;
        //    int64_t dts;
        //    bool got_extradata = false;

        if (!s || video_index < 0 || video_index > 2)
            return ret;
#if 1
        size = 32;
        s->streams[video_index]->codec->extradata = (uint8_t *) malloc(
                size + FF_INPUT_BUFFER_PADDING_SIZE);
        if (NULL == s->streams[video_index]->codec->extradata)
            return ret;
        memset(s->streams[video_index]->codec->extradata, 0, size + FF_INPUT_BUFFER_PADDING_SIZE);
        s->streams[video_index]->codec->extradata_size = size;
        return 1;
#else
        for (;; avio_skip(s->pb, 4))
    {
        pos  = (int)avio_tell(s->pb);
        type = avio_r8(s->pb);
        size = avio_rb24(s->pb);
        dts  = avio_rb24(s->pb);
        dts |= avio_r8(s->pb) << 24;
        avio_skip(s->pb, 3);
        if (0 == size)
            break;
        if (FLV_TAG_TYPE_AUDIO == type || FLV_TAG_TYPE_META == type) {
            /*if audio or meta tags, skip them.*/
            avio_seek(s->pb, size, SEEK_CUR);
        } else if (type == FLV_TAG_TYPE_VIDEO) {
            /*if the first video tag, read the sps/pps info from it. then break.*/
            size -= 5;
            s->streams[video_index]->codec->extradata = malloc(size + FF_INPUT_BUFFER_PADDING_SIZE);
            if (NULL == s->streams[video_index]->codec->extradata)
                break;
            memset(s->streams[video_index]->codec->extradata, 0, size + FF_INPUT_BUFFER_PADDING_SIZE);
            memcpy(s->streams[video_index]->codec->extradata, s->pb->buf_ptr + 5, size);
            s->streams[video_index]->codec->extradata_size = size;
            ret = 0;
            got_extradata = true;
        } else  {
            /*The type unknown,something wrong.*/
            break;
        }
        if (got_extradata)
            break;
    }
    return ret;
#endif
    }
#endif
    void ClearQueue();

    AVCodecContext *My_EncodecodecCtx;
    AVFrame *pFrameRGB;
    AVFrame *pFrameRecord;
    AVCodecContext *m_codecCtx;
    AVFrame *m_decodedFrame;
    void _DispDecordData(void);
private:


    bool bNeedCheck;

    int CreateEncodeStream();

    int CloseVideo();

    int EncodeSnapshot();

    int decodeAndRender();

    int writeVideo();

   // int writeFrame(AVFrame *pOutFrame);


    int writePacket(AVPacket *pPacket);

    int writePacketDirectly(AVPacket *pPacket);



    //AVCodecContext *codecCtx;
    AVCodecParserContext *m_parser;
    AVCodec *codec;


   // AVFrame *frame_rec;
    uint8_t *buffer_a;




    bool bPause;

    //Decode Media
    AVPacket Mypkt;

    AVFormatContext *m_formatCtx;
    int m_videoStream;





    AVFrame *frame_a;
    AVFrame *frame_b;
    AVFrame *frame_c;


    AVFrame *frame_SnapBuffer;


    struct SwsContext *img_convert_ctx;
    struct SwsContext *img_convert_ctx_half;
    struct SwsContext *img_convert_ctxBmp;
    struct SwsContext *img_convert_ctxRecord;
    unsigned char *out_buffer;

    //AVPicture         	m_frameRGBA;
    struct SwsContext *m_sws_ctx;
    int m_width;
    int m_height;
    pthread_t m_decodeThread;
    pthread_t m_writeThread;


    //Encode AVI
    AVCodecID m_EncodeID;
    AVOutputFormat *fmt;
    AVFormatContext *pFormatCtx;
    AVCodecContext *pCodecCtx;
    AVStream *video_st;
    uint64_t  nSaveInx;
    struct SwsContext *m_outsws_ctx;

    int64_t m_outFrameCnt;

    int64_t m_prevTime;
    int m_prevLeft;
    AVPacket m_prevPkt;

    AVPixelFormat pix_format;
    AVPixelFormat disp_pix_format;
    AVCodecID dispCodeID;

    pthread_mutex_t m_Frame_Queuelock;
    pthread_mutex_t m_Frame_Queuelock_Display;
    pthread_cond_t m_Frame_condition;

    pthread_cond_t m_Frame_condition_disp;




    bool   bCanDisplay;
    bool m_bSaveSnapshot;


    bool m_bSaveVideo;


public:

    bool  F_WriteAudio(jbyte * data,int nLen);

    AVFrame *pFrameYUV;//=av_frame_alloc();//av_frame_alloc();
    int Releaseffmpeg();

    AVFrame *pFrameYUV_Disp;

    char m_snapShotPath[256];
    //   char m_VideoPath[256];
    char m_MediaPath[256];

    int  nfps;
    int  nRecFps;
    bool m_bOpenOK;
    bool bFlip;
    bool b3D;
    bool b3DA;


    MySocketData keyFrame;
    AVPacket packet;

    bool    bReleaseSurface;


    ANativeWindow *nativeWindow;
    ANativeWindow_Buffer windowBuffer;


    int flush_encoder(AVFormatContext *fmt_ctx,unsigned int stream_index);

#ifndef D_BufferData
    void F_DispH264NoBuffer(MySocketData *data);
#endif


};


#endif //FFMPEGTEST_C_FFMPEGPLAYER_H
