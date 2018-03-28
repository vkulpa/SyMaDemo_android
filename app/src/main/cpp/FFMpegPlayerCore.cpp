//
// Created by generalplus_sa1 on 4/29/16.
//
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
#include <src/main/cpp/h264prase/h264_rbsp.h>
#include <src/main/cpp/h264prase/sps_parse.h>
#include <sstream>

#include "MyMediaCoder.h"
#include "phone_rl_protocol.h"

extern MyMediaCoder myMediaCoder;


#if 0
#include <SoftwareRenderer.h>
#include <android_runtime/AndroidRuntime.h>
#include <android_runtime/android_view_Surface.h>
#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <media/stagefright/foundation/ADebug.h>
#include <surfaceflinger/Surface.h>
#include <surfaceflinger/SurfaceComposerClient.h>
#include <surfaceflinger/ISurfaceComposer.h>
#include <ui/DisplayInfo.h>
#include <ui/GraphicBuffer.h>
#include <hardware/gralloc.h>
#include <ui/GraphicBufferMapper.h>

#include <android/native_window.h>
#include <system/window.h>
#include <ui/GraphicBufferMapper.h>
#include <assert.h>
using namespace android;

#define TAG "DisplayYUV"
#define BUFSIZE 176*144
static sp<Surface> native_surface;
static android::Surface::SurfaceInfo info;
static android::Region dirtyRegion;
char buf[BUFSIZE] = {0};
char * bufYUV = NULL;
FILE *fp = NULL;
jsize len = 0;



static android::Surface* getNativeSurface(JNIEnv* env, jobject jsurface, jint version)
{
    jclass clazz = env->FindClass("android/view/Surface");
    jfieldID field_surface;
    if(version <=8)
    {
        field_surface = env->GetFieldID(clazz, "mSurface", "I");
    }
    else
        field_surface = env->GetFieldID(clazz, ANDROID_VIEW_SURFACE_JNI_ID, "I");

    if (field_surface == NULL)
    {
        return NULL;
    }
    return (android::Surface *) env->GetIntField(jsurface, field_surface);
}
int setSurface(JNIEnv *env, jobject jsurface, jint version)
{
    native_surface = getNativeSurface(env, jsurface, version);

    if(android::Surface::isValid(native_surface))
    {
        __android_log_print(ANDROID_LOG_INFO, "libjni", "native_surface is valid");
        return 1;
    }
    else
        __android_log_print(ANDROID_LOG_ERROR, "libjni", "native_surface is invalid");

    return 0;
}

void Java_com_example_displayyuv_MainActivity_test(JNIEnv *env, jobject thiz, jobject jsurface)
{
    __android_log_print(ANDROID_LOG_INFO, TAG, "get good cpp test surface!");
    setSurface(env, jsurface, 9);
    //dirtyRegion.set(android::Rect(0x3fff, 0x3fff));
    dirtyRegion.set(android::Rect(0, 0, 176, 144));
    status_t err = native_surface->lock(&info, &dirtyRegion);
    int sformat;
    switch(info.format)
    {
        case PIXEL_FORMAT_RGBA_8888:
        case PIXEL_FORMAT_RGBX_8888:
        case PIXEL_FORMAT_BGRA_8888:
            sformat = 4;
            break;
        case PIXEL_FORMAT_RGB_888:
            sformat = 3;
            break;

        case PIXEL_FORMAT_RGB_565:
        case PIXEL_FORMAT_RGBA_5551:
        case PIXEL_FORMAT_RGBA_4444:
            sformat = 2;
            break;

        default:
            sformat = -1;
    }
    sprintf(buf, "width - %d -- height %d-- format %d---Locked -- %d", info.w, info.h, sformat, err);
    __android_log_print(ANDROID_LOG_INFO, TAG, buf);
    memset(buf, 0x77, BUFSIZE);
    memcpy(info.bits, buf, BUFSIZE);
    //memcpy(info.bits, bufYUV, len);
    native_surface->unlockAndPost();
}


#endif
using namespace std;


extern uint8_t nSDStatus;
extern bool     bRealRecording;
extern int32_t  nRecTime;

extern int nFrameCount;


extern int PlatformDisplay();

extern void F_CloseSocket();


typedef std::queue<AVFrame *> WriteFrameQueue;
typedef std::vector<AVFrame *>::iterator WriteFrameQueue_iterator;


typedef std::queue<MyFrame *> WriteFrameQueue_h264;

typedef std::vector<MyFrame *>::iterator WriteFrameQueue_iterator_h264;


typedef std::queue<MySocketData *> WriteFrameQueue_Disp;

WriteFrameQueue m_FrameQueue;

WriteFrameQueue_Disp m_FrameQueue_Display;

WriteFrameQueue_h264 m_FrameQueue_H264;


int64_t time_out = 0;
int64_t time_out_dat = 5000;


bool firsttimeplay = true;

void frame_rotate_180(AVFrame *src, AVFrame *des);

void Adj23D(AVFrame *src, AVFrame *des);


int64_t getCurrentTime() {
    return av_gettime() / 1000;
    //struct timeval tv;
    //gettimeofday(&tv, NULL);
    //return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}


void F_ResetCheckT(int n) {
    time_out_dat = n;
    time_out = getCurrentTime();

}

static int interrupt_cb(void *ctx) {
    int64_t currentTime = getCurrentTime();
    if (time_out_dat == 0)
        return 0;
    int64_t da = currentTime - time_out;

    if (da >= time_out_dat) {
        LOGE("error-------------%lld", time_out_dat);
        return 1;
    }
    return 0;

}

//----------------------------------------------------------------------


int C_FFMpegPlayer::SaveSnapshot(const char *path) {
    if (m_bSaveSnapshot)
        return -1;
    strcpy(m_snapShotPath, path);
    m_bSaveSnapshot = true;
    return FFMPEGPLAYER_NOERROR;
}

int C_FFMpegPlayer::GetStatus() {
    return m_Status;
}

C_FFMpegPlayer::C_FFMpegPlayer() :
        m_Status(E_PlayerStatus_Stoped), nRecFps(25),
        m_formatCtx(NULL), m_videoStream(0), m_codecCtx(NULL), m_decodedFrame(NULL),
        My_EncodecodecCtx(NULL),
        m_sws_ctx(NULL), m_width(0), m_height(0), fmt(NULL), pFormatCtx(NULL), pFrameRGB(NULL),
        pCodecCtx(NULL), video_st(NULL), m_bSaveSnapshot(false), m_bSaveVideo(false),
        img_convert_ctxBmp(NULL),
        m_writeThread(0), m_outsws_ctx(NULL), m_EncodeID(AV_CODEC_ID_MPEG4), m_bOpenOK(false),  //AV_CODEC_ID_MPEG4
        bPause(true), nIC_Type(IC_GK),
        buffer_a(NULL), frame_c(NULL),
        out_buffer(NULL), out_bufferBmp(NULL), m_parser(NULL), codec(NULL), bFlip(false),
        nDisplayWidth(640), nDisplayHeight(360), nNeedRedraw(false), nativeWindow(NULL), b3D(false),
        frame_b(NULL), frame_a(NULL), b480(false), bFollow(false), Rgbabuffer(NULL), video(MP4_INVALID_TRACK_ID), fileHandle(MP4_INVALID_FILE_HANDLE), bIsH264(false), nSpsSet(0), nSecT(0),
        bContinue(true), b3DA(false), nfps(20), nErrorFrame(0), h64fileHandle(-1), bStatWrite(false), YUVbuffer(NULL), img_convert_ctxRecord(NULL), pFrameRecord(NULL), nRecordWidth(1280),
        nRecordHeight(720) {
    m_snapShotPath[0] = 0;  //= m_VideoPath[0]
    pthread_mutex_init(&m_Frame_Queuelock, NULL);
    pthread_cond_init(&m_Frame_condition, NULL);

    pthread_mutex_init(&m_Frame_Queuelock_Display, NULL);
    pthread_cond_init(&m_Frame_condition_disp, NULL);
    sRecordFileName = "";
}


void C_FFMpegPlayer::F_ReSetRecordWH(int w, int h) {

    nRecordWidth = w;
    nRecordHeight = h;
    if (pFrameRecord != NULL) {
        av_free(pFrameRecord->data[0]);
        av_frame_free(&pFrameRecord);
        pFrameRecord = NULL;
    }
    pFrameRecord = av_frame_alloc();
    pFrameRecord->format = AV_PIX_FMT_YUVJ420P;
    pFrameRecord->width = nRecordWidth;
    pFrameRecord->height = nRecordHeight;
    int ret = av_image_alloc(
            pFrameRecord->data, pFrameRecord->linesize, nRecordWidth,
            nRecordHeight,
            AV_PIX_FMT_YUVJ420P, 4);

    if (img_convert_ctxRecord != NULL) {
        sws_freeContext(img_convert_ctxRecord);
        img_convert_ctxRecord = NULL;
    }

    img_convert_ctxRecord = sws_getContext(m_codecCtx->width, m_codecCtx->height, AV_PIX_FMT_YUVJ420P,
                                           nRecordWidth, nRecordHeight, AV_PIX_FMT_YUVJ420P,
                                           SWS_AREA, NULL, NULL, NULL); //

}

//----------------------------------------------------------------------
C_FFMpegPlayer::~C_FFMpegPlayer() {
    LOGE("~C_FFMpegPlayer()");
    Stop();
    CloseVideo();
    if (frame_a != NULL) {
        av_frame_free(&frame_a);
        frame_a = NULL;
    }
    if (frame_b != NULL) {
        av_frame_free(&frame_b);
        frame_b = NULL;
    }

}
//----------------------------------------------------------------------

int C_FFMpegPlayer::InitMediaSN(void) {
    AVCodec *pCodec = NULL;
    AVDictionary *optionsDict = NULL;
    int err_code = 0;

    if (m_decodedFrame == NULL) {
        avcodec_register_all();
        av_register_all();
        avformat_network_init();
        av_log_set_level(AV_LOG_QUIET);

        m_decodedFrame = av_frame_alloc();
        codec = avcodec_find_decoder(AV_CODEC_ID_MJPEG);
        m_codecCtx = avcodec_alloc_context3(codec);
        m_codecCtx->codec_id = AV_CODEC_ID_MJPEG;
        //m_codecCtx->width = nDisplayWidth;
        //m_codecCtx->height = nDisplayHeight;
        m_codecCtx->pix_fmt = AV_PIX_FMT_YUVJ422P;
        //m_codecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
        m_codecCtx->time_base.den = 1;
        m_codecCtx->time_base.num = 1;
        int ret = avcodec_open2(m_codecCtx, codec, NULL);
        if (ret != 0) {
            m_bOpenOK = false;
            LOGE("open codec failed :%d", ret);
        }
        return 0;
    }

    if (pFrameYUV != NULL)
        return 0;
    if (m_codecCtx->width <= 0 || m_codecCtx->height <= 0) {
        return 0;
    }


    pix_format = AV_PIX_FMT_YUV420P;
    disp_pix_format = AV_PIX_FMT_RGBA;
    dispCodeID = AV_CODEC_ID_BMP;// AV_CODEC_ID_MJPEG;

    pFrameYUV = av_frame_alloc();
    pFrameRGB = av_frame_alloc();


    frame_SnapBuffer = av_frame_alloc();

    frame_SnapBuffer->format = pix_format;
    frame_SnapBuffer->width = m_codecCtx->width;
    frame_SnapBuffer->height = m_codecCtx->height;
    int ret = av_image_alloc(
            frame_SnapBuffer->data, frame_SnapBuffer->linesize, m_codecCtx->width,
            m_codecCtx->height,
            pix_format, 4);

    pFrameYUV->format = pix_format;
    pFrameYUV->width = m_codecCtx->width;
    pFrameYUV->height = m_codecCtx->height;
    ret = av_image_alloc(
            pFrameYUV->data, pFrameYUV->linesize, m_codecCtx->width,
            m_codecCtx->height,
            pix_format, 4);


    pFrameRecord = av_frame_alloc();
    pFrameRecord->format = pix_format;
    pFrameRecord->width = nRecordWidth;
    pFrameRecord->height = nRecordHeight;
    ret = av_image_alloc(
            pFrameRecord->data, pFrameRecord->linesize, nRecordWidth,
            nRecordHeight,
            pix_format, 4);

    pFrameRGB->format = disp_pix_format;//AV_PIX_FMT_BGR24;
    pFrameRGB->width = m_codecCtx->width;
    pFrameRGB->height = m_codecCtx->height;
    ret = av_image_alloc(
            pFrameRGB->data, pFrameRGB->linesize, nDisplayWidth,
            nDisplayHeight,
            disp_pix_format, 4);

    img_convert_ctx = sws_getContext(m_codecCtx->width, m_codecCtx->height, m_codecCtx->pix_fmt,
                                     m_codecCtx->width, m_codecCtx->height, pix_format,
                                     SWS_POINT, NULL, NULL, NULL); //

    img_convert_ctxBmp = sws_getContext(m_codecCtx->width, m_codecCtx->height, pix_format,
                                        nDisplayWidth, nDisplayHeight, disp_pix_format,
                                        SWS_POINT, NULL, NULL, NULL); //

    img_convert_ctxRecord = sws_getContext(m_codecCtx->width, m_codecCtx->height, pix_format,
                                           nRecordWidth, nRecordHeight, pix_format,
                                           SWS_AREA, NULL, NULL, NULL); //


    img_convert_ctx_half = sws_getContext(m_codecCtx->width, m_codecCtx->height, pix_format,
                                          m_codecCtx->width / 2, m_codecCtx->height / 2, pix_format,
                                          SWS_POINT, NULL, NULL, NULL); //


    if (frame_a == NULL) {
        frame_a = av_frame_alloc();
        frame_a->format = AV_PIX_FMT_YUV420P;
        frame_a->width = m_codecCtx->width;
        frame_a->height = m_codecCtx->height;
        av_image_alloc(frame_a->data, frame_a->linesize, m_codecCtx->width,
                       m_codecCtx->height,
                       AV_PIX_FMT_YUV420P, 4);
    }


    if (frame_b == NULL) {
        frame_b = av_frame_alloc();
        frame_b->format = AV_PIX_FMT_YUV420P;
        frame_b->width = m_codecCtx->width / 2;
        frame_b->height = m_codecCtx->height / 2;
        av_image_alloc(frame_b->data, frame_b->linesize, frame_b->width,
                       frame_b->height,
                       AV_PIX_FMT_YUV420P, 4);
    }
    m_bOpenOK = true;
    return 0;
}

int C_FFMpegPlayer::InitMediaGPRTP(void) {
    AVCodec *pCodec = NULL;
    AVDictionary *optionsDict = NULL;
    int err_code = 0;
    m_bOpenOK = false;


    if (m_decodedFrame == NULL) {
        avcodec_register_all();
        av_register_all();
        avformat_network_init();
        av_log_set_level(AV_LOG_QUIET);
        m_decodedFrame = av_frame_alloc();
        codec = avcodec_find_decoder_by_name("h264_mediacodec");//寻找指定解码器
        if (codec == NULL)
            codec = avcodec_find_decoder(AV_CODEC_ID_H264);


        m_codecCtx = avcodec_alloc_context3(codec);
        m_codecCtx->codec_id = codec->id;//AV_CODEC_ID_H264;

        m_codecCtx->width = nDisplayWidth;
        m_codecCtx->height = nDisplayHeight;

        m_parser = av_parser_init(codec->id); //AV_CODEC_ID_H264);

        int ret = avcodec_open2(m_codecCtx, codec, NULL);
        if (ret != 0) {
            LOGE("open codec failed :%d", ret);
        }

        return 0;
    }
    if (pFrameYUV != NULL)
        return 0;
    if (m_codecCtx == NULL)
        return 0;

    if (m_codecCtx->width <= 0 || m_codecCtx->height <= 0) {
        return 0;
    }

    pix_format = AV_PIX_FMT_YUV420P;
    disp_pix_format = AV_PIX_FMT_RGBA;
    dispCodeID = AV_CODEC_ID_BMP;// AV_CODEC_ID_MJPEG;

    pFrameYUV = av_frame_alloc();
    pFrameRGB = av_frame_alloc();


    pFrameYUV->format = pix_format;
    pFrameYUV->width = m_codecCtx->width;
    pFrameYUV->height = m_codecCtx->height;
    int ret = av_image_alloc(
            pFrameYUV->data, pFrameYUV->linesize, m_codecCtx->width,
            m_codecCtx->height,
            pix_format, 4);

    pFrameRGB->format = disp_pix_format;//AV_PIX_FMT_BGR24;
    pFrameRGB->width = m_codecCtx->width;
    pFrameRGB->height = m_codecCtx->height;
    ret = av_image_alloc(
            pFrameRGB->data, pFrameRGB->linesize, nDisplayWidth,
            nDisplayHeight,
            disp_pix_format, 4);


    pFrameRecord = av_frame_alloc();

    pFrameRecord->format = pix_format;
    pFrameRecord->width = nRecordWidth;
    pFrameRecord->height = nRecordHeight;
    ret = av_image_alloc(
            pFrameRecord->data, pFrameRecord->linesize, nRecordWidth,
            nRecordHeight,
            pix_format, 4);

    img_convert_ctx = sws_getContext(m_codecCtx->width, m_codecCtx->height, m_codecCtx->pix_fmt,
                                     m_codecCtx->width, m_codecCtx->height, pix_format,
                                     SWS_BILINEAR, NULL, NULL, NULL); //

    img_convert_ctxBmp = sws_getContext(m_codecCtx->width, m_codecCtx->height, pix_format,
                                        nDisplayWidth, nDisplayHeight, disp_pix_format,
                                        SWS_BILINEAR, NULL, NULL, NULL); //

    img_convert_ctxRecord = sws_getContext(m_codecCtx->width, m_codecCtx->height, pix_format,
                                           nRecordWidth, nRecordHeight, pix_format,
                                           SWS_AREA, NULL, NULL, NULL); //



    img_convert_ctx_half = sws_getContext(m_codecCtx->width, m_codecCtx->height, pix_format,
                                          m_codecCtx->width / 2, m_codecCtx->height / 2, pix_format,
                                          SWS_BILINEAR, NULL, NULL, NULL); //


    if (frame_a == NULL) {
        frame_a = av_frame_alloc();
        frame_a->format = AV_PIX_FMT_YUV420P;
        frame_a->width = m_codecCtx->width;
        frame_a->height = m_codecCtx->height;
        av_image_alloc(frame_a->data, frame_a->linesize, m_codecCtx->width,
                       m_codecCtx->height,
                       AV_PIX_FMT_YUV420P, 4);
    }


    if (frame_b == NULL) {
        frame_b = av_frame_alloc();
        frame_b->format = AV_PIX_FMT_YUV420P;
        frame_b->width = m_codecCtx->width / 2;
        frame_b->height = m_codecCtx->height / 2;
        av_image_alloc(frame_b->data, frame_b->linesize, frame_b->width,
                       frame_b->height,
                       AV_PIX_FMT_YUV420P, 4);
    }

    m_bOpenOK = true;

    return 0;

}


int C_FFMpegPlayer::InitMediaGK(void) {
    AVCodec *pCodec = NULL;
    AVDictionary *optionsDict = NULL;
    int err_code = 0;
    m_bOpenOK = false;

    AVDictionary *avCodecOptions = NULL;

    if (m_decodedFrame == NULL) {
        avcodec_register_all();
        av_register_all();
        avformat_network_init();
        av_log_set_level(AV_LOG_QUIET);
        m_decodedFrame = av_frame_alloc();
        // codec = avcodec_find_decoder_by_name("h264_mediacodec");//寻找指定解码器
        // if(codec == NULL)
        codec = avcodec_find_decoder(AV_CODEC_ID_H264);

        m_codecCtx = avcodec_alloc_context3(codec);
        //m_codecCtx = avcodec_alloc_context3(NULL);
        char buffer[1024];
        buffer[1023] = 0;
        int ret = avcodec_open2(m_codecCtx, codec, &avCodecOptions);
        if (ret != 0) {
            av_strerror(ret, buffer, 1023);
            LOGE("open codec failed :%d", ret);
        }
        m_codecCtx->width = nDisplayWidth;
        m_codecCtx->height = nDisplayHeight;
        m_parser = av_parser_init(codec->id);//AV_CODEC_ID_H264);
        return 0;
    }
    if (pFrameYUV != NULL)
        return 0;
    if (m_codecCtx == NULL)
        return 0;

    if (m_codecCtx->width <= 0 || m_codecCtx->height <= 0) {
        return 0;
    }


    pix_format = AV_PIX_FMT_YUV420P;
    disp_pix_format = AV_PIX_FMT_RGBA;
    dispCodeID = AV_CODEC_ID_BMP;// AV_CODEC_ID_MJPEG;

    pFrameYUV = av_frame_alloc();
    pFrameRGB = av_frame_alloc();


    frame_SnapBuffer = av_frame_alloc();
    frame_SnapBuffer->format = pix_format;
    frame_SnapBuffer->width = m_codecCtx->width;
    frame_SnapBuffer->height = m_codecCtx->height;
    int ret = av_image_alloc(
            frame_SnapBuffer->data, frame_SnapBuffer->linesize, m_codecCtx->width,
            m_codecCtx->height,
            pix_format, 4);


    pFrameYUV->format = pix_format;
    pFrameYUV->width = m_codecCtx->width;
    pFrameYUV->height = m_codecCtx->height;
    ret = av_image_alloc(
            pFrameYUV->data, pFrameYUV->linesize, m_codecCtx->width,
            m_codecCtx->height,
            pix_format, 4);

    pFrameRGB->format = disp_pix_format;//AV_PIX_FMT_BGR24;
    pFrameRGB->width = m_codecCtx->width;
    pFrameRGB->height = m_codecCtx->height;
    ret = av_image_alloc(
            pFrameRGB->data, pFrameRGB->linesize, nDisplayWidth,
            nDisplayHeight,
            disp_pix_format, 4);


    pFrameRecord = av_frame_alloc();
    pFrameRecord->format = pix_format;
    pFrameRecord->width = nRecordWidth;
    pFrameRecord->height = nRecordHeight;
    ret = av_image_alloc(
            pFrameRecord->data, pFrameRecord->linesize, nRecordWidth,
            nRecordHeight,
            pix_format, 4);

    img_convert_ctx = sws_getContext(m_codecCtx->width, m_codecCtx->height, m_codecCtx->pix_fmt,
                                     m_codecCtx->width, m_codecCtx->height, pix_format,
                                     SWS_BILINEAR, NULL, NULL, NULL); //

    img_convert_ctxBmp = sws_getContext(m_codecCtx->width, m_codecCtx->height, pix_format,
                                        nDisplayWidth, nDisplayHeight, disp_pix_format,
                                        SWS_BILINEAR, NULL, NULL, NULL); //

    img_convert_ctxRecord = sws_getContext(m_codecCtx->width, m_codecCtx->height, pix_format,
                                           nRecordWidth, nRecordHeight, pix_format,
                                           SWS_AREA, NULL, NULL, NULL); //


    img_convert_ctx_half = sws_getContext(m_codecCtx->width, m_codecCtx->height, pix_format,
                                          m_codecCtx->width / 2, m_codecCtx->height / 2, pix_format,
                                          SWS_BILINEAR, NULL, NULL, NULL); //


    if (frame_a == NULL) {
        frame_a = av_frame_alloc();
        frame_a->format = AV_PIX_FMT_YUV420P;
        frame_a->width = m_codecCtx->width;
        frame_a->height = m_codecCtx->height;
        av_image_alloc(frame_a->data, frame_a->linesize, m_codecCtx->width,
                       m_codecCtx->height,
                       AV_PIX_FMT_YUV420P, 4);
    }


    if (frame_b == NULL) {
        frame_b = av_frame_alloc();
        frame_b->format = AV_PIX_FMT_YUV420P;
        frame_b->width = m_codecCtx->width / 2;
        frame_b->height = m_codecCtx->height / 2;
        av_image_alloc(frame_b->data, frame_b->linesize, frame_b->width,
                       frame_b->height,
                       AV_PIX_FMT_YUV420P, 4);
    }

    m_bOpenOK = true;


    return 0;
}

bool C_FFMpegPlayer::F_RecreateEnv(void) {
    if (nNeedRedraw) {
        nNeedRedraw = false;
        if (pFrameRGB != NULL) {
            av_freep(pFrameRGB->data);
            pFrameRGB->format = AV_PIX_FMT_BGR24;
            pFrameRGB->width = m_codecCtx->width;
            pFrameRGB->height = m_codecCtx->height;
            av_image_alloc(
                    pFrameRGB->data, pFrameRGB->linesize, nDisplayWidth,
                    nDisplayHeight,
                    AV_PIX_FMT_BGR24, 4);

        }
        if (img_convert_ctxBmp != NULL) {
            sws_freeContext(img_convert_ctxBmp);
            img_convert_ctxBmp = sws_getContext(m_codecCtx->width, m_codecCtx->height,
                                                m_codecCtx->pix_fmt,
                                                nDisplayWidth, nDisplayHeight, AV_PIX_FMT_BGR24,
                                                SWS_BILINEAR, NULL, NULL, NULL); //
        }
    }
    return true;
}


int C_FFMpegPlayer::InitMedia(const char *path) {

#ifdef  D_H264file
    sH264FileName ="/storage/emulated/0/com.joyhonest.jh_demo/Video/2017-11-21-001.h264";
    h64fileHandle = open(sH264FileName.c_str(), O_CREAT  | O_RDWR,0666);
#endif

    if (nIC_Type == IC_GK_UDP) {

        InitMediaGK();
        F_RecreateEnv();
        return 0;
    }

    if (nIC_Type == IC_GKA) {

        InitMediaGK();
        F_RecreateEnv();
        return 0;
    }
    if (nIC_Type == IC_GPH264A) {
        InitMediaGK();
        F_RecreateEnv();
        return 0;
    }
    if (nIC_Type == IC_SN) {
        InitMediaSN();
        F_RecreateEnv();
        return 0;
    }
    if (nIC_Type == IC_GPRTP || nIC_Type == IC_GPRTPB) {
        //SN_Width = 640;
        //SN_Height = 480;
        nDisplayHeight = 480;
        nDisplayWidth = 640;
        InitMediaSN();
        F_RecreateEnv();
        return 0;
    }

    if (m_bOpenOK) {
        LOGE("Is Opened! ");
        return 0;
    }

    AVCodec *pCodec = NULL;
    AVDictionary *optionsDict = NULL;
    int err_code = 0;
    char buffer[1024];
    buffer[1023] = 0;
    m_bOpenOK = false;

    if (m_formatCtx != NULL) {
        Stop();
        //Releaseffmpeg();
    }

    ClearQueue();

    //pthread_mutex_init(&m_Frame_Queuelock, NULL);


    avcodec_register_all();
    av_register_all();
    avformat_network_init();
    av_log_set_level(AV_LOG_QUIET);

    time_out = getCurrentTime();
    firsttimeplay = true;

    int nVer = avcodec_version();


    m_formatCtx = avformat_alloc_context();

    m_formatCtx->interrupt_callback.callback = interrupt_cb; //--------注册回调函数
    m_formatCtx->interrupt_callback.opaque = m_formatCtx;

    if (nIC_Type == IC_GPH264)
        m_formatCtx->probesize = 600 * 1024;
    else
        m_formatCtx->probesize = 600 * 1024;

    AVDictionary *opts = NULL;
    bNeedCheck = false;
    AVDictionary *options = NULL;

    av_dict_set(&options, "rtsp_transport", "tcp", 0);

    F_ResetCheckT(1000 * 10);
    if ((err_code = avformat_open_input(&m_formatCtx, path, NULL, &options)) != 0) {
        av_strerror(err_code, buffer, 1023);
        DEBUG_PRINT("avformat_open_input %s failed!", path);
        DEBUG_PRINT("%s", buffer);
        avformat_free_context(m_formatCtx);
        m_formatCtx = NULL;
        return FFMPEGPLAYER_INITMEDIAFAILED;
    }
    strcpy(m_MediaPath, path);
    std::string str(m_MediaPath);
    std::string per = "";
    int is = str.find(':', 0);
    if (is >= 0) {
        per = str.substr(0, is);
    }
    transform(per.begin(), per.end(), per.begin(), ::tolower);
    F_ResetCheckT(5000);
    {
        if (nIC_Type == IC_GPH264) {
            m_formatCtx->probesize = 8 * 1024;
            m_formatCtx->max_analyze_duration = 50 * AV_TIME_BASE;
        } else {
            m_formatCtx->probesize = 30 * 1024;
            m_formatCtx->max_analyze_duration = 100 * AV_TIME_BASE;
        }

        if (avformat_find_stream_info(m_formatCtx, NULL) < 0) {
            DEBUG_PRINT("avformat_find_stream_info failed!\n");
            avformat_close_input(&m_formatCtx);
            avformat_free_context(m_formatCtx);
            m_formatCtx = NULL;
            return FFMPEGPLAYER_INITMEDIAFAILED;
        }
    }
    /*
        else
        {
            if (init_decode(m_formatCtx) != 0) {
                LOGE("Start 1-1");
                return FFMPEGPLAYER_INITMEDIAFAILED;
            }
        }
        */
    F_ResetCheckT(5000);
    int videoindex = -1;
    int i = 0;
    for (i = 0; i < m_formatCtx->nb_streams; i++)
        //if (m_formatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)  //ByAiven
        if (m_formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoindex = i;
            m_videoStream = i;
            break;
        }
    if (videoindex == -1) {
        LOGE("Didn't find a video stream.\n");
        avformat_close_input(&m_formatCtx);
        avformat_free_context(m_formatCtx);
        m_formatCtx = NULL;
        return -1;
    }

    AVStream *stream = m_formatCtx->streams[videoindex];

    //m_codecCtx = stream->codec;  //ByAiven

    {
        AVCodec *codecA = avcodec_find_decoder(stream->codecpar->codec_id);
        m_codecCtx = avcodec_alloc_context3(codecA);
    }

    // Find the decoder for the video stream
    // pCodec = avcodec_find_decoder(m_codecCtx->codec_id);  Aiven
    pCodec = avcodec_find_decoder(stream->codecpar->codec_id);
    if (pCodec == NULL) {
        DEBUG_PRINT("Unsupported codec!\n");
        avformat_close_input(&m_formatCtx);
        avformat_free_context(m_formatCtx);
        m_formatCtx = NULL;
        return FFMPEGPLAYER_INITMEDIAFAILED; // Codec not found
    }

    m_codecCtx = avcodec_alloc_context3(pCodec);
    avcodec_parameters_to_context(m_codecCtx, stream->codecpar);
    av_codec_set_pkt_timebase(m_codecCtx, stream->time_base);


    // Open codec
    F_ResetCheckT(5000);
    if (avcodec_open2(m_codecCtx, pCodec, &optionsDict) < 0) {
        DEBUG_PRINT("avcodec_open2 failed!\n");
        avformat_close_input(&m_formatCtx);
        avformat_free_context(m_formatCtx);
        m_formatCtx = NULL;
        return FFMPEGPLAYER_INITMEDIAFAILED;
    }


    if (m_codecCtx->width <= 0 || m_codecCtx->height <= 0) {
        avformat_close_input(&m_formatCtx);
        avformat_free_context(m_formatCtx);
        m_formatCtx = NULL;
        return FFMPEGPLAYER_INITMEDIAFAILED;
    }


    buffer[1023] = 0;
    m_bOpenOK = false;


    if (m_decodedFrame == NULL) {
        m_decodedFrame = av_frame_alloc();

    }

    pix_format = AV_PIX_FMT_YUV420P;
    disp_pix_format = AV_PIX_FMT_RGBA;
    dispCodeID = AV_CODEC_ID_BMP;// AV_CODEC_ID_MJPEG;

    pFrameYUV = av_frame_alloc();
    pFrameRGB = av_frame_alloc();


    pFrameYUV->format = pix_format;
    pFrameYUV->width = m_codecCtx->width;
    pFrameYUV->height = m_codecCtx->height;
    int ret = av_image_alloc(
            pFrameYUV->data, pFrameYUV->linesize, m_codecCtx->width,
            m_codecCtx->height,
            pix_format, 4);


    frame_SnapBuffer = av_frame_alloc();

    frame_SnapBuffer->format = pix_format;
    frame_SnapBuffer->width = m_codecCtx->width;
    frame_SnapBuffer->height = m_codecCtx->height;
    ret = av_image_alloc(
            frame_SnapBuffer->data, frame_SnapBuffer->linesize, m_codecCtx->width,
            m_codecCtx->height,
            pix_format, 4);


    pFrameRGB->format = disp_pix_format;//AV_PIX_FMT_BGR24;
    pFrameRGB->width = m_codecCtx->width;
    pFrameRGB->height = m_codecCtx->height;
    ret = av_image_alloc(
            pFrameRGB->data, pFrameRGB->linesize, nDisplayWidth,
            nDisplayHeight,
            disp_pix_format, 4);


    pFrameRecord = av_frame_alloc();

    pFrameRecord->format = pix_format;
    pFrameRecord->width = nRecordWidth;
    pFrameRecord->height = nRecordHeight;
    ret = av_image_alloc(
            pFrameRecord->data, pFrameRecord->linesize, nRecordWidth,
            nRecordHeight,
            pix_format, 4);

    img_convert_ctx = sws_getContext(m_codecCtx->width, m_codecCtx->height, m_codecCtx->pix_fmt,
                                     m_codecCtx->width, m_codecCtx->height, pix_format,
                                     SWS_FAST_BILINEAR, NULL, NULL, NULL); //

    img_convert_ctxBmp = sws_getContext(m_codecCtx->width, m_codecCtx->height, pix_format,
                                        nDisplayWidth, nDisplayHeight, disp_pix_format,
                                        SWS_FAST_BILINEAR, NULL, NULL, NULL); //
    img_convert_ctxRecord = sws_getContext(m_codecCtx->width, m_codecCtx->height, pix_format,
                                           nRecordWidth, nRecordHeight, pix_format,
                                           SWS_FAST_BILINEAR, NULL, NULL, NULL); //



    img_convert_ctx_half = sws_getContext(m_codecCtx->width, m_codecCtx->height, pix_format,
                                          m_codecCtx->width / 2, m_codecCtx->height / 2, pix_format,
                                          SWS_FAST_BILINEAR, NULL, NULL, NULL); //


    if (frame_a == NULL) {
        frame_a = av_frame_alloc();
        frame_a->format = AV_PIX_FMT_YUV420P;
        frame_a->width = m_codecCtx->width;
        frame_a->height = m_codecCtx->height;
        av_image_alloc(frame_a->data, frame_a->linesize, m_codecCtx->width,
                       m_codecCtx->height,
                       AV_PIX_FMT_YUV420P, 4);
    }


    if (frame_b == NULL) {
        frame_b = av_frame_alloc();
        frame_b->format = AV_PIX_FMT_YUV420P;
        frame_b->width = m_codecCtx->width / 2;
        frame_b->height = m_codecCtx->height / 2;
        av_image_alloc(frame_b->data, frame_b->linesize, frame_b->width,
                       frame_b->height,
                       AV_PIX_FMT_YUV420P, 4);
    }
    m_bOpenOK = true;

    return FFMPEGPLAYER_NOERROR;
}
//----------------------------------------------------------------------

int C_FFMpegPlayer::PlayMedia() {
    if (nIC_Type == IC_GKA)
        return 0;
    if (nIC_Type == IC_SN) {
        //m_Status = E_PlayerStatus_Playing;
        return 0;
    }
    if (m_Status == E_PlayerStatus_Playing)
        return FFMPEGPLAYER_MEDIAISPLAYING;
    else if (m_Status == E_PlayerStatus_Stoping) {
        void *ret = NULL;
        pthread_join(m_decodeThread, &ret);
    }

    if (m_formatCtx == NULL) {
        int i32Ret = InitMedia(m_MediaPath);
        if (i32Ret != 0)
            return i32Ret;
    }
    if (!m_bOpenOK)
        return -1;

    m_Status = E_PlayerStatus_Playing;
    F_ResetCheckT(0);
    pthread_create(&m_decodeThread, NULL, decodeThreadFunction, (void *) this);
    return FFMPEGPLAYER_NOERROR;
}

//----------------------------------------------------------------------
int C_FFMpegPlayer::Stop() {
#ifdef D_H264file
    if(h64fileHandle!=-1)
        {
            close(h64fileHandle);
            h64fileHandle = -1;
        }
#endif

    m_Status = E_PlayerStatus_Stoping;
    m_bSaveVideo = false;
    usleep(1000 * 20);
    F_ResetCheckT(5);
    usleep(1000 * 10);
    //if (nIC_Type == IC_GKA || nIC_Type == IC_SN || nIC_Type == IC_GPRTP || nIC_Type == IC_GPH264A || nIC_Type == IC_GPRTPB || nIC_Type == IC_GK_UDP)
    if (nIC_Type == IC_GPH264 ||
        nIC_Type == IC_GPRTSP) { ;
    } else {
        Releaseffmpeg();
    }
    m_bOpenOK = false;
    return FFMPEGPLAYER_NOERROR;
}

//----------------------------------------------------------------------

int C_FFMpegPlayer::WriteMp4Frame(uint8_t *data, int nLen, bool b) {

    if (nLen == 0)
        return -1;
    int ds = 90000 / nfps;  //   (1000/nfps*90000)/1000
    if (F_IsNeed2WriteH264Stream()) {
        ds = 90000 / nRecFps;
    }

    if (fileHandle != MP4_INVALID_FILE_HANDLE && video != MP4_INVALID_TRACK_ID) {
        int nL = nLen - 4;
        data[0] = (uint8_t) ((nL & 0xFF000000) >> 24);
        data[1] = (uint8_t) ((nL & 0x00FF0000) >> 16);
        data[2] = (uint8_t) ((nL & 0x0000FF00) >> 8);
        data[3] = (uint8_t) ((nL & 0x000000FF));
        {
            MP4WriteSample(fileHandle, video, data, nLen, ds, 0, b);
        }
        return 0;
    }
    return -1;
}


int C_FFMpegPlayer::AddMp4Video(uint8_t *sps, int len1, uint8_t *pps, int len2) {

    if (fileHandle != MP4_INVALID_FILE_HANDLE) {
        video = MP4AddH264VideoTrack(fileHandle, 90000, 90000 / nRecFps, nRecordWidth, nRecordHeight, sps[1], sps[2], sps[3], 3);
        if (video == MP4_INVALID_TRACK_ID) {
            MP4Close(fileHandle, 0);
            fileHandle = MP4_INVALID_FILE_HANDLE;
            return -1;
        } else {
            MP4AddH264SequenceParameterSet(fileHandle, video, sps, len1);
            MP4AddH264PictureParameterSet(fileHandle, video, pps, len2);
            //MP4SetVideoProfileLevel(fileHandle, 0x7F);
            LOGE("SPS PPS");
        }
        return 0;
    }
    return -1;

}

int C_FFMpegPlayer::SaveVideo(const char *path, bool bisH264) {

    bisH264 = true;
    bRealRecording = false;
    nRecTime = 0;
    bStatWrite = false;
    this->bIsH264 = bisH264;
    sRecordFileName = path;
    sRecordFileName_tmp = path;
    sRecordFileName_tmp = sRecordFileName_tmp + "_.tmp";
    if (m_codecCtx == NULL) {
        this->bIsH264 = false;
        fileHandle = MP4_INVALID_FILE_HANDLE;
        return -1;
    }
    if (m_codecCtx->width <= 0 || m_codecCtx->height <= 0) {
        this->bIsH264 = false;
        fileHandle = MP4_INVALID_FILE_HANDLE;
        return -1;
    }

    if (bisH264) {
        if (fileHandle != MP4_INVALID_FILE_HANDLE) {
            MP4Close(fileHandle, 0);
            fileHandle = MP4_INVALID_FILE_HANDLE;
        }
        //创建mp4文件
        fileHandle = MP4Create(sRecordFileName_tmp.c_str(), 0);
        if (fileHandle == MP4_INVALID_FILE_HANDLE) {
            return -1;
        }
        MP4SetTimeScale(fileHandle, 90000);

        pthread_create(&m_writeThread, NULL, WriteThreadFunction, this);


        return 0;
    }
    return FFMPEGPLAYER_NOERROR;
}


//----------------------------------------------------------------------
void C_FFMpegPlayer::ClearQueue() {
    pthread_mutex_lock(&m_Frame_Queuelock);

    WriteFrameQueue_iterator iterator;
    /*
    for (iterator = m_FrameQueue.begin(); iterator != m_FrameQueue.end(); ++iterator)
        delete (*iterator);

    m_FrameQueue.clear();
     */

    AVFrame *pFrame = NULL;
    while (m_FrameQueue.size()) {
        pFrame = m_FrameQueue.front();
        m_FrameQueue.pop();
        av_free(pFrame->data[0]);
        av_frame_free(&pFrame);

    }
    while (m_FrameQueue_H264.size()) {
        MyFrame *fa = m_FrameQueue_H264.front();
        m_FrameQueue_H264.pop();
        delete fa;
    }

    pthread_mutex_unlock(&m_Frame_Queuelock);
}

//----------------------------------------------------------------------
int C_FFMpegPlayer::SetResolution(int Width, int Height) {
    m_width = Width;
    m_height = Height;

    sws_freeContext(m_sws_ctx);

    // Setup scaler
    m_sws_ctx = sws_getContext(
            m_codecCtx->width,
            m_codecCtx->height,
            m_codecCtx->pix_fmt,
            m_width,
            m_height,
            AV_PIX_FMT_RGBA,
            SWS_BILINEAR,
            NULL,
            NULL,
            NULL
    );

    return FFMPEGPLAYER_NOERROR;
}

//----------------------------------------------------------------------
void *C_FFMpegPlayer::decodeThreadFunction(void *param) {
    C_FFMpegPlayer *pPlayer = (C_FFMpegPlayer *) param;

    pPlayer->decodeAndRender();

    return FFMPEGPLAYER_NOERROR;
}

//----------------------------------------------------------------------

void *C_FFMpegPlayer::WriteThreadFunction(void *param) {
    C_FFMpegPlayer *pPlayer = (C_FFMpegPlayer *) param;
    pPlayer->writeVideo();
    return NULL;
}


//----------------------------------------------------------------------

void F_ResetRelinker();

char typeOfNalu(char *data) {
    char first = *data;
    return first & 0x1f;
}

uint16_t ixxxx = 0;


int64_t nPre = 0;


#ifndef D_BufferData

void C_FFMpegPlayer::F_DispH264NoBuffer(MySocketData *data) {

    //if (data != NULL)
    {
        bCanDisplay = false;
        decodeAndRender_GKA_B(data);
#ifdef D_BufferData
        delete data;
#endif
    }
}

#endif

extern int64_t disp_no;
extern int64_t start_time;
extern int nDelayDisplayT;
extern FILE *testFile;
//GKA,GPH264A
string getstring ( const int n )
{
    std::stringstream newstr;
    newstr<<n;
    return newstr.str();
}

int C_FFMpegPlayer::decodeAndRender_GKA(MySocketData *data) {

    F_ResetRelinker();
    if (data->nLen < 4)
        return 0;




    int64_t T0 = av_gettime() / 1000;
#if 1

#ifdef  D_BufferData
    MySocketData *pd = NULL;
    pthread_mutex_lock(&m_Frame_Queuelock_Display);
    pd =  new MySocketData();
    pd->disp_no=disp_no;
    pd->AppendData(data->data,data->nLen);
//#endif
//#ifdef D_BufferData

    /*
    char type = typeOfNalu((char *)data->data);
    if(type == 5)
    {
        while(!m_FrameQueue_Display.empty()) {
            MySocketData *dataA = m_FrameQueue_Display.front();
            m_FrameQueue_Display.pop();
            delete dataA;
        }
    }
    */


    m_FrameQueue_Display.push(pd);
    if(m_FrameQueue_Display.size()<4)
    {
        nDelayDisplayT = 150;
    } else if(m_FrameQueue_Display.size()<5)
    {
        nDelayDisplayT=50;
    } else
    {
        nDelayDisplayT=20;
    }
    pthread_cond_signal(&m_Frame_condition_disp);
    pthread_mutex_unlock(&m_Frame_Queuelock_Display);
#else
    //pd = data;
    F_DispH264NoBuffer(data);
#endif

#endif
    /*
    static int64_t T1 = av_gettime() / 1000;
    int64_t T2 = av_gettime() / 1000;
    int daT = (int) (T2 - T1);
    //  LOGE("Data Size = %00000008d  DispFrame T = %003d, Decord = %003d",data->nLen, daT,(int)(T2-T0));
    T1 = T2;
     */
    return 0;

}


void *C_FFMpegPlayer::F_DispThread(void *dat) {
#ifdef D_BufferData
    C_FFMpegPlayer *self = (C_FFMpegPlayer *)dat;
    MySocketData *data;
    bool bdisp=true;
    int dab = (1000 / (self->nfps+5));
    struct timespec timespec1;

    int ndelay = 1000/self->nfps - 8;


    while (self->m_Status == E_PlayerStatus_Playing)
    {

        data = NULL;
        timespec1.tv_sec = 0;
        timespec1.tv_nsec = 1000*1000*10;
        pthread_mutex_lock(&(self->m_Frame_Queuelock_Display));
        pthread_cond_timedwait(&(self->m_Frame_condition_disp), &(self->m_Frame_Queuelock_Display),&timespec1);
        if(!m_FrameQueue_Display.empty()) {
            data = m_FrameQueue_Display.front();
            m_FrameQueue_Display.pop();

        }
        pthread_mutex_unlock(&(self->m_Frame_Queuelock_Display));
        if(data!=NULL)
        {
            self->bCanDisplay=false;
            self->decodeAndRender_GKA_B(data);
            delete data;

            if(self->bCanDisplay)
            {
                if(self->m_Status == E_PlayerStatus_Playing) {
                    PlatformDisplay();
                }
                if(nDelayDisplayT!=0)
                {
                    usleep(nDelayDisplayT*1000);
                    if(nDelayDisplayT==150) {
                        ;
                        LOGE("Delay Display 150ms!");
                    }
                    if(nDelayDisplayT==50)
                    {
                        ;
                        LOGE("Delay Display 50ms!");
                    } else
                    {
                        ;
                        LOGE("Delay Display 20ms!");
                    }
                }

/*
                int64_t  nc = av_gettime()/1000;

                if(data->disp_no == 1)
                {
                    start_time = nc;
                }
                else
                {
                    int doa = data->disp_no-1;
                    int64_t da = doa*ndelay+start_time;
                    if(da>nc)
                    {
                        da = da-nc;
                        usleep(da*1000);
                    }
                }
                nc = av_gettime()/1000;
               // LOGE("Disp Frame :== %d   count=%d",(int)(nc-self->nPreDiptime),m_FrameQueue_Display.size());

                if(self->m_Status == E_PlayerStatus_Playing) {
                        PlatformDisplay();
                }
                    //self->nPreDiptime = av_gettime() / 1000;
                    */

            }

        }

    }
    while(!m_FrameQueue_Display.empty()) {
        data = m_FrameQueue_Display.front();
        m_FrameQueue_Display.pop();
        delete data;
    }

    LOGE("Exit GKA DisplayThread");
#endif
    return NULL;

}


int F_Decord_JAVA(uint8 *data, int nLen);

void C_FFMpegPlayer::F_StartDispThread() {

    m_Status = E_PlayerStatus_Playing;
    int ret = pthread_create(&dispThreadid, NULL, F_DispThread, (void *) this); // 成功返回0，错误返回错误编号

}

int F_SendKey2Jave(int nKey);

extern jbyte TestInfo[1024];

int F_SentTestInfo(void);

//extern uint8_t nICType;

//int F_GetH264Frame(byte *data,int nLen);


int sps_prase(char *i_sps, int sps_length, int *pwidth, int *pheight) {
    if (!i_sps || sps_length <= 0)
        return -1;

    char rbspbuffer[512] = {0,};
    rbsp spsrsbp(i_sps, sps_length, rbspbuffer, 512);
    int bufl = spsrsbp.torbsp();
    sps tempsps((unsigned char *) rbspbuffer, bufl);
    tempsps.sps_parse();
    *pwidth = (int) tempsps.getWidth();
    *pheight = (int) tempsps.getHeight();
    int fps = tempsps.get_separate_colour_plane_flag();
    return 0;

    /*
    unsigned char frame_mbs_only_flag = tempsps.get_frame_mbs_only_flag();
    unsigned int log2_max_frame_num_minus4 = tempsps.get_log2_max_frame_num_minus4();
    unsigned int pic_order_cnt_type = tempsps.get_pic_order_cnt_type();
    unsigned int num_units_in_tick = tempsps.get_num_units_in_tick();
    unsigned int time_scale = tempsps.get_time_scale();
    unsigned char separate_colour_plane_flag = tempsps.get_separate_colour_plane_flag();
    unsigned int log2_max_pic_order_cnt_lsb_minus4 = tempsps.get_log2_max_pic_order_cnt_lsb_minus4();
    printf("----------------------------------------------------------------\n");
    printf("@@            frame_mbs_only_flag =%d             \n",frame_mbs_only_flag);
    printf("@@            log2_max_frame_num_minus4 =%d             \n",log2_max_frame_num_minus4);
    printf("@@            pic_order_cnt_type =%d             \n",pic_order_cnt_type);
    printf("@@            num_units_in_tick =%d             \n",num_units_in_tick);
    printf("@@            time_scale =%d             \n",time_scale);
    printf("@@            separate_colour_plane_flag =%d             \n",separate_colour_plane_flag);
    printf("@@            log2_max_pic_order_cnt_lsb_minus4 =%d             \n",log2_max_pic_order_cnt_lsb_minus4);
    printf("----------------------------------------------------------------\n");
     */
}

extern bool bInitMedia;

int64_t iTime = 0;

int nPIndex = 0;


void F_InserFrame(int nFps) {

    int8_t P13[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 11, -1};
    int8_t P14[] = {1, 2, 3, 5, 4, 6, 7, 8, 9, 11, 12, -1};
    int8_t P15[] = {1, 2, 3, 5, 7, 8, 9, 10, 11, 13, -1};
    int8_t P16[] = {1, 3, 5, 7, 8, 9, 11, 13, 14, -1};
    int8_t P17[] = {2, 3, 5, 7, 8, 9, 14, 15, -1};
    int8_t P18[] = {2, 4, 6, 8, 10, 12, 14, -1};
    int8_t P19[] = {2, 6, 10, 14, 15, 16, -1};
    int8_t P20[] = {2, 8, 12, 16, 18, -1};
    int8_t P21[] = {8, 12, 16, 18, -1};
    int8_t P22[] = {12, 16, 19, -1};
    int8_t P23[] = {16, 20, -1};
    int8_t P24[] = {12, -1};
    int8_t *PP = NULL;

    if (nFps == 13) {
        PP = P13;
    }

    if (nFps == 14) {
        PP = P14;

    }

    if (nFps == 15) {
        PP = P15;
    }
    if (nFps == 16) {
        PP = P16;
    }
    if (nFps == 17) {
        PP = P17;
    }
    if (nFps == 18) {
        PP = P18;
    }
    if (nFps == 19) {
        PP = P19;
    }
    if (nFps == 20) {
        PP = P20;
    }
    if (nFps == 21) {
        PP = P21;
    }
    if (nFps == 22) {
        PP = P22;
    }
    if (nFps == 23) {
        PP = P23;
    }

    if (nFps == 24) {
        PP = P24;
    }

    if (PP == NULL)
        return;
    for (int x = 0; x < 20; x++) {
        if (PP[x] == -1) {
            break;
        }

        if (PP[x] == nPIndex) {
            MyFrame *f1 = new MyFrame();


            f1->data = new uint8_t[12];
            f1->nLen = 12;
            f1->data[0] = 0;
            f1->data[1] = 0;
            f1->data[2] = 0;
            f1->data[3] = 8;
            f1->data[4] = 0x0B;//0x01;
            f1->data[5] = 0x9A;
            f1->data[6] = 0x0E;
            f1->data[7] = 0x0F;
            f1->data[8] = 0x8A;
            f1->data[9] = 0x00;
            f1->data[10] = 0x38;
            f1->data[11] = 0x46;

            m_FrameQueue_H264.push(f1);
        }

    }

}

int nAddCount = 0;

int Head_size = sizeof(T_NET_FRAME_HEADER);
extern int g_sdkVersion;
int nCount_fps = -1;
extern int64_t  nPreTimeA;

int C_FFMpegPlayer::decodeAndRender_GKA_B(MySocketData *data) {

    F_ResetRelinker();
    if (data->nLen < 4)
        return 0;
    char head[] = {0x00, 0x00, 0x00, 0x01};
    char *pData = (char *) (data->data);
    char type = typeOfNalu(pData);
    if (nIC_Type != IC_GKA) {
        Head_size = 0;
    } else {
        Head_size = sizeof(T_NET_FRAME_HEADER);
    }
    if (type == 5 || type == 1) {
        data->nLen -= Head_size;
    }
    uint32_t nMySize = data->nLen - 4;


    if(testFile!=NULL)
    {
        //char type = typeOfNalu((char *)data->data);
        if(type ==5 || type ==1 || type ==7 || type ==8) {
            //char *LF = "\n";
            time_t timep;
            time(&timep);
            int ds123=-1;
            if(type == 5 || type ==1)
            {
                if(type == 5 && nPreTimeA==0)
                {
                    nPreTimeA=av_gettime()/1000;
                }
                if(nPreTimeA !=0)
                {
                    int64_t  nCur = av_gettime()/1000;
                    ds123= nCur - nPreTimeA;
                    nPreTimeA = nCur;
                }
            }



            struct tm *p;
            p = localtime(&timep);
            int64_t tv = av_gettime() / 1000;
            int ms = tv % 1000;
            char bfa[200];
            if(ds123>=0)
            {
                sprintf(bfa, "%02d:%02d:%02d:%003d  FrameType:%d  Frame Len:%000006ld   DA = %003d", p->tm_hour, p->tm_min, p->tm_sec, ms, (int) type,
                        (long)(data->nLen),(int)ds123);
            } else {
                sprintf(bfa, "%02d:%02d:%02d:%003d  FrameType:%d  Frame Len:%000006ld", p->tm_hour, p->tm_min, p->tm_sec, ms, (int) type,
                        (long)(data->nLen));
            }
            string str = (char *) bfa;
            if(type ==7 || type ==8)
            {
                str+="\n";
            }


            fwrite(str.c_str(), 1, strlen(str.c_str()), testFile);
        }

    }

    if (type == 7 || type == 8 || type == 6 || type == 5)  //SPS PPS SEI IDR
    {
        keyFrame.AppendData((uint8_t *) head, 4);
        keyFrame.AppendData(data->data, nMySize);

        // if(F_IsNeed2WriteH264Stream())
        {
            int nLLL = nMySize;
            if (nLLL > 1024)
                nLLL = 1024;
            if (type == 7) {
                memcpy(sps, data->data, nLLL);
                sps_len = nLLL;
                nSpsSet++;
                if (nSpsSet > 2) {
                    nSpsSet = 2;
                }
            }
            if (type == 8) {
                memcpy(pps, data->data, nLLL);
                pps_len = nLLL;
                nSpsSet++;
                if (nSpsSet > 2) {
                    nSpsSet = 2;
                }
            }
            if (nSpsSet == 2) {
                if (nIC_Type == IC_GKA) {
                    sps[15] = 0x03;
                    sps[16] = 0x0D;
                    sps[17] = 0x42;
                }


                if (g_sdkVersion < 21)
                {
                    if (!bInitMedia)
                    {
                        bInitMedia = true;
                        int width = 0;
                        int height = 0;
                        sps_prase((char *) sps, sps_len, &width, &height);
                        width = (width + 1) * 16;
                        height = (height + 1) * 16;
                        myMediaCoder.F_InitDecoder(width, height, (int8_t *) sps, sps_len, (int8_t *) pps, pps_len, nfps);
                        myMediaCoder.offerDecoder(sps, sps_len, 2);//BUFFER_FLAG_CODEC_CONFIG
                        myMediaCoder.offerDecoder(pps, pps_len, 2);
                    }
                }

            }
        }

    }

    if (F_IsNeed2WriteH264Stream()) {
        if (m_bSaveVideo && fileHandle != MP4_INVALID_FILE_HANDLE) {
            //创建视频track //根据ISO/IEC 14496-10 可知sps的第二个，第三个，第四个字节分别是 AVCProfileIndication,profile_compat,AVCLevelIndication     其中90000/25  中的25是fps
            if (nSpsSet == 2 && video == MP4_INVALID_TRACK_ID) {
                nPIndex = 0;
                if (nIC_Type == IC_GKA) {
                    sps[15] = 0x03;
                    sps[16] = 0x0D;
                    sps[17] = 0x42;
                }
                int nSetFps = nRecFps;
                video = MP4AddH264VideoTrack(fileHandle, 90000, 90000 / nSetFps, m_codecCtx->width, m_codecCtx->height, sps[1], sps[2], sps[3], 3);
                if (video == MP4_INVALID_TRACK_ID) {
                    MP4Close(fileHandle, 0);
                    fileHandle = MP4_INVALID_FILE_HANDLE;
                } else {
                    //设置sps和pps
                    MP4AddH264SequenceParameterSet(fileHandle, video, sps, sps_len);
                    MP4AddH264PictureParameterSet(fileHandle, video, pps, pps_len);
                    MP4SetVideoProfileLevel(fileHandle, 0x7F);
                }
            }
        }
    }

    //if(m_codecCtx!=NULL && m_codecCtx->width == nRecordWidth && m_codecCtx->height == nRecordHeight)
    if (F_IsNeed2WriteH264Stream()) {
        if (m_bSaveVideo && fileHandle != MP4_INVALID_FILE_HANDLE && video != -1 && nSpsSet == 2) {
            bool bKey = false;
            if (type == 7 || type == 8 || type == 6 || type == 5 || type == 1)  //SPS PPS SEI IDR
            {

                if (!bStatWrite && type == 5) {
                    bStatWrite = true;
                }
                if (bStatWrite && (type == 5 || type == 1))// || type == 7 || type == 8 || type == 6)
                {
                    MyFrame *myfram = new MyFrame();

                    if (type == 5) {
                        nPIndex = 0;
                        myfram->bKeyFranme = true;
                    } else {
                        nPIndex++;
                        myfram->bKeyFranme = false;
                    }

                    myfram->data = new uint8_t[nMySize + 4];
                    myfram->nLen = nMySize + 4;
                    myfram->data[0] = (nMySize & 0xFF000000) >> 24;
                    myfram->data[1] = (nMySize & 0x00FF0000) >> 16;
                    myfram->data[2] = (nMySize & 0x0000FF00) >> 8;
                    myfram->data[3] = (nMySize & 0xFF0000FF);
                    memcpy(myfram->data + 4, data->data, nMySize);
                    pthread_mutex_lock(&m_Frame_Queuelock);
                    if (m_FrameQueue_H264.size() > 30) {
                        while (!m_FrameQueue_H264.empty()) {
                            MyFrame *pMyFrame = m_FrameQueue_H264.front();
                            m_FrameQueue.pop();
                            delete pMyFrame;
                            bStatWrite = false;
                        }
                    } else {
                        m_FrameQueue_H264.push(myfram);
                        if(nIC_Type==IC_GKA)
                        {
                            if (type == 1) {
                                F_InserFrame(nfps);
                            }
                        }
                    }
                    pthread_cond_signal(&m_Frame_condition);
                    pthread_mutex_unlock(&m_Frame_Queuelock);
                }
            }
        }
    }


    int nPackLen = 0;
    int rr = -1;



    int ret = -1;
    if (type == 5 || type == 1) {
        if (g_sdkVersion >= 21)
        {
            static int64_t  nT1 = av_gettime()/1000;
            int64_t  nT2 = av_gettime()/1000;
            // LOGE("Decord type = %d Time = %d",type,(int)(nT2-nT1));
            nT1 = nT2;
            if (type == 5)    // I frame
            {
                rr = av_new_packet(&packet, keyFrame.nLen);
                if (rr == 0) {
                    memcpy(packet.data, keyFrame.data, keyFrame.nLen);
                    keyFrame.nLen = 0;
                }
            } else {
                rr = av_new_packet(&packet, nMySize + 4);
                if (rr == 0) {
                    memcpy(packet.data, head, 4);
                    memcpy(packet.data + 4, data->data, nMySize);
                }
            }
            if (rr == 0) {
                if (avcodec_send_packet(m_codecCtx, &packet) == 0) {
                    if (avcodec_receive_frame(m_codecCtx, m_decodedFrame) != 0) {
                        ret = -1;
                    } else {
                        ret = 0;
                    }
                } else {
                    ret = -1;
                }
            }

        }


        else
        {
            int flag = 0;
            if (type == 5)
                flag = 1;  //BUFFER_FLAG_KEY_FRAME
            if (!bStarDecord) {
                if (type == 5) {
                    bStarDecord = true;

                    myMediaCoder.offerDecoder((uint8_t *) (data->data), nMySize, flag);
                }
            } else {
                myMediaCoder.offerDecoder((uint8_t *) (data->data), nMySize, flag);

            }

            return 0;
        }
        if(testFile!=NULL) {
            string sok = " Decord    OK ";
            if (ret == 0) {
                time_t timep;
                time(&timep);


                struct tm *p;
                p = localtime(&timep);
                int64_t tv = av_gettime() / 1000;
                int ms = tv % 1000;
                char bfa[200];
                sprintf(bfa, "%02d:%02d:%02d:%003d  ", p->tm_hour, p->tm_min, p->tm_sec, ms);
                sok += (char *) bfa;
            } else {
                sok = " Decord Error XX:XX:XX:XXX";
            }
            sok += "\n";
            fwrite(sok.c_str(), 1, strlen(sok.c_str()), testFile);
            fflush(testFile);
        }

        if (ret == 0) {
            _DispDecordData();
            av_packet_unref(&packet);
        }


    }
    return 0;
}


void C_FFMpegPlayer::_DispDecordData(void) {
    int ret = 0;
    if (ret != 0) {
        nErrorFrame++;
    }
    if (ret == 0) {
        nDisplayWidth = m_codecCtx->width;
        nDisplayHeight = m_codecCtx->height;
       // LOGE("H264 W = %d H = %d", nDisplayWidth, nDisplayHeight);

        InitMediaGK();
        F_RecreateEnv();
        if (m_codecCtx->pix_fmt != AV_PIX_FMT_YUV420P) {
            libyuv::I422ToI420(m_decodedFrame->data[0], m_decodedFrame->linesize[0],
                               m_decodedFrame->data[1], m_decodedFrame->linesize[1],
                               m_decodedFrame->data[2], m_decodedFrame->linesize[2],
                               pFrameYUV->data[0], pFrameYUV->linesize[0],
                               pFrameYUV->data[1], pFrameYUV->linesize[1],
                               pFrameYUV->data[2], pFrameYUV->linesize[2],
                               m_decodedFrame->width, m_decodedFrame->height
            );
            /*
            sws_scale(img_convert_ctx,
                      (const uint8_t *const *) m_decodedFrame->data,
                      m_decodedFrame->linesize, 0,
                      m_codecCtx->height,
                      pFrameYUV->data, pFrameYUV->linesize);
                      */
        } else {
            //av_frame_copy(pFrameYUV, m_decodedFrame);
            libyuv::I420Copy(m_decodedFrame->data[0], m_decodedFrame->linesize[0],
                             m_decodedFrame->data[1], m_decodedFrame->linesize[1],
                             m_decodedFrame->data[2], m_decodedFrame->linesize[2],
                             pFrameYUV->data[0], pFrameYUV->linesize[0],
                             pFrameYUV->data[1], pFrameYUV->linesize[1],
                             pFrameYUV->data[2], pFrameYUV->linesize[2],
                             m_decodedFrame->width, m_decodedFrame->height);
        }
        if (bFlip) {

            if (frame_a == NULL) {
                frame_a = av_frame_alloc();
                frame_a->format = AV_PIX_FMT_YUV420P;
                frame_a->width = m_codecCtx->width;
                frame_a->height = m_codecCtx->height;
                av_image_alloc(frame_a->data, frame_a->linesize, m_codecCtx->width,
                               m_codecCtx->height,
                               AV_PIX_FMT_YUV420P, 4);
            }
            /*
            if(frame_c == NULL)
            {
                frame_c = av_frame_alloc();
                frame_c->format = AV_PIX_FMT_YUV420P;
                frame_c->width = m_codecCtx->width/2;
                frame_c->height = m_codecCtx->height/2;
                av_image_alloc(frame_c->data, frame_c->linesize, m_codecCtx->width/2,
                               m_codecCtx->height/2,
                               AV_PIX_FMT_YUV420P, 4);
            }
        */


            //frame_rotate_180(pFrameYUV, frame_a);
            //av_frame_copy(pFrameYUV, frame_a);

            libyuv::I420Mirror(pFrameYUV->data[0], pFrameYUV->linesize[0],
                               pFrameYUV->data[1], pFrameYUV->linesize[1],
                               pFrameYUV->data[2], pFrameYUV->linesize[2],
                               frame_a->data[0], frame_a->linesize[0],
                               frame_a->data[1], frame_a->linesize[1],
                               frame_a->data[2], frame_a->linesize[2],
                               frame_a->width, frame_a->height);

            ret = libyuv::I420Copy(frame_a->data[0], frame_a->linesize[0],
                                   frame_a->data[1], frame_a->linesize[1],
                                   frame_a->data[2], frame_a->linesize[2],
                                   pFrameYUV->data[0], frame_a->linesize[0],
                                   pFrameYUV->data[1], frame_a->linesize[1],
                                   pFrameYUV->data[2], frame_a->linesize[2],
                                   frame_a->width, frame_a->height);
        }
        /*
        if (b3D) {

            if (m_decodedFrame != NULL) {
                if (m_bSaveSnapshot) {
                    if (m_decodedFrame->key_frame != 0) {
                        av_frame_copy(frame_SnapBuffer, pFrameYUV);
                        EncodeSnapshot();
                    }
                }
            }

            libyuv::I420Scale(pFrameYUV->data[0], pFrameYUV->linesize[0],
                              pFrameYUV->data[1], pFrameYUV->linesize[1],
                              pFrameYUV->data[2], pFrameYUV->linesize[2],
                              m_codecCtx->width, m_codecCtx->height,
                              frame_b->data[0], frame_b->linesize[0],
                              frame_b->data[1], frame_b->linesize[1],
                              frame_b->data[2], frame_b->linesize[2],
                              frame_b->width, frame_b->height,
                              libyuv::kFilterLinear);

            Adj23D(frame_b, pFrameYUV);

        } else
         */
        {
            if (m_decodedFrame != NULL) {
                if (m_decodedFrame->key_frame != 0) {
                    av_frame_copy(frame_SnapBuffer, pFrameYUV);
                }
            }
            if (m_bSaveSnapshot) {
                EncodeSnapshot();
            }

        }
        bCanDisplay = true;

    }
    av_packet_unref(&packet);

    if (bCanDisplay) {
#ifndef D_BufferData
        PlatformDisplay();
#endif
    }

}


int PlatformDisplaySN(char *dat, int nLen);


int C_FFMpegPlayer::decodeAndRender_RTP(char *data, int nLen) {


    return decodeAndRender_SN(data, nLen);

}


int C_FFMpegPlayer::decodeAndRender_SN(char *data, int nLen) {
    if (nLen <= 0) {
        return 0;
    }
    uint8_t *in_data = (uint8_t *) data;// new uint8_t[in_len];
    int ret;
    int frameFinished = 0;
    AVPacket packet = {0};
    av_init_packet(&packet);
    packet.data = in_data;
    packet.size = nLen;

    if (avcodec_send_packet(m_codecCtx, &packet) == 0) {
        if (avcodec_receive_frame(m_codecCtx, m_decodedFrame) != 0) {
            ret = -1;
        } else {
            ret = 0;
        }
    } else {
        ret = -1;
    }

    if (ret == 0) {

        static int64_t nPreA = av_gettime() / 1000;
        int64_t nCur = av_gettime() / 1000;
        int dp = (int) (nCur - nPreA);

        nPreA = nCur;
        m_Status = E_PlayerStatus_Playing;
        nDisplayWidth = m_codecCtx->width;
        nDisplayHeight = m_codecCtx->height;
        InitMediaSN();
        /*
        ret = sws_scale(img_convert_ctx,
                        (const uint8_t *const *) m_decodedFrame->data,
                        m_decodedFrame->linesize, 0,
                        m_codecCtx->height,
                        pFrameYUV->data, pFrameYUV->linesize);
        */

        if (m_decodedFrame->format == AV_PIX_FMT_YUVJ420P || m_decodedFrame->format == AV_PIX_FMT_YUV420P) {
            libyuv::I420Copy(m_decodedFrame->data[0], m_decodedFrame->linesize[0],
                             m_decodedFrame->data[1], m_decodedFrame->linesize[1],
                             m_decodedFrame->data[2], m_decodedFrame->linesize[2],
                             pFrameYUV->data[0], pFrameYUV->linesize[0],
                             pFrameYUV->data[1], pFrameYUV->linesize[1],
                             pFrameYUV->data[2], pFrameYUV->linesize[2],
                             m_decodedFrame->width, m_decodedFrame->height);
        } else {
            libyuv::I422ToI420(m_decodedFrame->data[0], m_decodedFrame->linesize[0],
                               m_decodedFrame->data[1], m_decodedFrame->linesize[1],
                               m_decodedFrame->data[2], m_decodedFrame->linesize[2],
                               pFrameYUV->data[0], pFrameYUV->linesize[0],
                               pFrameYUV->data[1], pFrameYUV->linesize[1],
                               pFrameYUV->data[2], pFrameYUV->linesize[2],
                               m_decodedFrame->width, m_decodedFrame->height);
        }

        m_decodedFrame->key_frame = 1;
#if 0
        if (bFlip)
        {
            if (frame_a == NULL) {
                frame_a = av_frame_alloc();
                frame_a->format = pix_format;
                frame_a->width = m_codecCtx->width;
                frame_a->height = m_codecCtx->height;
                av_image_alloc(frame_a->data, frame_a->linesize, m_codecCtx->width,
                               m_codecCtx->height,
                               pix_format, 4);
            }
            frame_rotate_180(pFrameYUV, frame_a);
            av_frame_copy(pFrameYUV, frame_a);
        }

        if (b3D) {
            ret = sws_scale(img_convert_ctx_half,
                            (const uint8_t *const *) pFrameYUV->data,
                            pFrameYUV->linesize, 0,
                            m_codecCtx->height,
                            frame_b->data, frame_b->linesize);

            Adj23D(frame_b, pFrameYUV);

        }
        if (m_decodedFrame != NULL) {

            if (m_decodedFrame->key_frame != 0) {
                av_frame_copy(frame_SnapBuffer, pFrameYUV);
            }
        }

        if (m_bSaveSnapshot) {
            {
                EncodeSnapshot();
            }
        }
#endif
        PlatformDisplay();
    }
    av_packet_unref(&packet);

    return 0;

}


int C_FFMpegPlayer::decodeAndRender() {
    if (nIC_Type == IC_SN)
        return 0;
    int frameFinished = 0;
    AVPacket packet;
    int ret = 0;
    time_out = getCurrentTime();
    firsttimeplay = true;
    //F_ResetCheckT(4000);  //2sec TimeOut
    F_ResetCheckT(0);
    while (m_Status == E_PlayerStatus_Playing) {
        if (av_read_frame(m_formatCtx, &packet) < 0)
            break;
        time_out = getCurrentTime();
        firsttimeplay = true;
        if (packet.stream_index == m_videoStream) {

            if (avcodec_send_packet(m_codecCtx, &packet) == 0) {
                if (avcodec_receive_frame(m_codecCtx, m_decodedFrame) != 0) {
                    ret = -1;
                } else {
                    ret = 0;
                }
            } else {
                ret = -1;
            }
            nDisplayWidth = m_codecCtx->width;
            nDisplayHeight = m_codecCtx->height;
            /*
            if(nRecordWidth !=m_codecCtx->width  ||
                       nRecordHeight != m_codecCtx->height) {
                F_ReSetRecordWH(m_codecCtx->width, m_codecCtx->height);
            }
             */


            if (ret == 0) {
                ret = sws_scale(img_convert_ctx,
                                (const uint8_t *const *) m_decodedFrame->data,
                                m_decodedFrame->linesize, 0,
                                m_codecCtx->height,
                                pFrameYUV->data, pFrameYUV->linesize);
                if (bFlip) {
                    if (frame_a == NULL) {
                        frame_a = av_frame_alloc();
                        frame_a->format = pix_format;
                        frame_a->width = m_codecCtx->width;
                        frame_a->height = m_codecCtx->height;
                        av_image_alloc(frame_a->data, frame_a->linesize, m_codecCtx->width,
                                       m_codecCtx->height,
                                       pix_format, 4);
                    }
                    frame_rotate_180(pFrameYUV, frame_a);
                    av_frame_copy(pFrameYUV, frame_a);
                }

                if (b3D) {
                    if (b3DA) {
                        ret = sws_scale(img_convert_ctx_half,
                                        (const uint8_t *const *) pFrameYUV->data,
                                        pFrameYUV->linesize, 0,
                                        m_codecCtx->height,
                                        frame_b->data, frame_b->linesize);

                        Adj23D(frame_b, pFrameYUV);

                        if (m_decodedFrame != NULL) {
                            if (m_decodedFrame->key_frame != 0) {
                                av_frame_copy(frame_SnapBuffer, pFrameYUV);
                            }
                        }

                        if (m_bSaveSnapshot) {

                            EncodeSnapshot();
                        }
                        if (m_bSaveVideo) {
/*
                            AVFrame *pFrame = av_frame_alloc();
                            pFrame->format = AV_PIX_FMT_YUV420P;
                            pFrame->width = m_codecCtx->width;
                            pFrame->height = m_codecCtx->height;
                            int ret = av_image_alloc(
                                    pFrame->data, pFrame->linesize, m_codecCtx->width,
                                    m_codecCtx->height,
                                    AV_PIX_FMT_YUV420P, 4);
                            av_frame_copy(pFrame, pFrameYUV);
                            pthread_mutex_lock(&m_Frame_Queuelock);
                            m_FrameQueue.push(pFrame);
                            pthread_cond_signal(&m_Frame_condition);
                            pthread_mutex_unlock(&m_Frame_Queuelock);
                            */

                        }
                    } else {
                        if (m_decodedFrame != NULL) {
                            if (m_decodedFrame->key_frame != 0) {
                                av_frame_copy(frame_SnapBuffer, pFrameYUV);
                            }
                        }
                        if (m_bSaveSnapshot) {

                            EncodeSnapshot();
                        }
                        /*
                        if (m_bSaveVideo) {
                            AVFrame *pFrame = av_frame_alloc();
                            pFrame->format = AV_PIX_FMT_YUV420P;
                            pFrame->width = m_codecCtx->width;
                            pFrame->height = m_codecCtx->height;
                            int ret = av_image_alloc(
                                    pFrame->data, pFrame->linesize, m_codecCtx->width,
                                    m_codecCtx->height,
                                    AV_PIX_FMT_YUV420P, 4);
                            av_frame_copy(pFrame, pFrameYUV);
                            pthread_mutex_lock(&m_Frame_Queuelock);
                            m_FrameQueue.push(pFrame);
                            pthread_cond_signal(&m_Frame_condition);
                            pthread_mutex_unlock(&m_Frame_Queuelock);

                        }
                         */
                        ret = sws_scale(img_convert_ctx_half,
                                        (const uint8_t *const *) pFrameYUV->data,
                                        pFrameYUV->linesize, 0,
                                        m_codecCtx->height,
                                        frame_b->data, frame_b->linesize);

                        Adj23D(frame_b, pFrameYUV);
                    }
                } else {
                    if (m_decodedFrame != NULL) {
                        if (m_decodedFrame->key_frame != 0) {
                            av_frame_copy(frame_SnapBuffer, pFrameYUV);
                        }
                    }
                    if (m_bSaveSnapshot) {

                        EncodeSnapshot();
                    }
                    /*
                    if (m_bSaveVideo) {
                        AVFrame *pFrame = av_frame_alloc();
                        pFrame->format = AV_PIX_FMT_YUV420P;
                        pFrame->width = m_codecCtx->width;
                        pFrame->height = m_codecCtx->height;
                        int ret = av_image_alloc(
                                pFrame->data, pFrame->linesize, m_codecCtx->width,
                                m_codecCtx->height,
                                AV_PIX_FMT_YUV420P, 4);
                        av_frame_copy(pFrame, pFrameYUV);
                        pthread_mutex_lock(&m_Frame_Queuelock);
                        m_FrameQueue.push(pFrame);
                        pthread_cond_signal(&m_Frame_condition);
                        pthread_mutex_unlock(&m_Frame_Queuelock);
                    }
                     */
                }
                PlatformDisplay();
            }

        }
        av_packet_unref(&packet);
    }

    m_bOpenOK = false;
    F_CloseSocket();
    LOGE("decodeAndRender End");
    m_bSaveVideo = false;
    Releaseffmpeg();
    m_Status = E_PlayerStatus_Stoped;
    return FFMPEGPLAYER_NOERROR;
}

void F_OnSave2ToGallery_mid(int i);

//----------------------------------------------------------------------
int C_FFMpegPlayer::StopSaveVideo() {
    void *ret = NULL;
    m_bSaveVideo = false;
    bRealRecording = false;

    usleep(1000 * 50);
    if (m_writeThread != 0) {
        pthread_join(m_writeThread, &ret);
        m_writeThread = 0;
    }

    if (buffer_a != NULL) {
        av_free(buffer_a);
        buffer_a = NULL;
    }

    if (pthread_mutex_trylock(&m_Frame_Queuelock) == 0) {
        pthread_cond_signal(&m_Frame_condition);
        pthread_mutex_unlock(&m_Frame_Queuelock);
    }

    if (bIsH264) {
        MP4Close(fileHandle, 0);
        fileHandle = MP4_INVALID_FILE_HANDLE;
        video = MP4_INVALID_TRACK_ID;
    }

    if (access(sRecordFileName_tmp.c_str(), F_OK) == 0) {
        rename(sRecordFileName_tmp.c_str(), sRecordFileName.c_str());
        F_OnSave2ToGallery_mid(1);
    }
    if (h64fileHandle != -1) {
        close(h64fileHandle);
        h64fileHandle = -1;
    }

    return FFMPEGPLAYER_NOERROR;
}

void F_SentRevYUV(int32_t wh);


#define LOGE_A(...) __android_log_print(ANDROID_LOG_ERROR ,"ffmpegJNI—",__VA_ARGS__)

int C_FFMpegPlayer::writeVideo() {
    m_bSaveVideo = true;
    int64_t T1 = av_gettime() / 1000;

    if (bIsH264) {

        int nLen = 0;
        MyFrame *pMyFrame = NULL;
        while (m_bSaveVideo && m_Status == E_PlayerStatus_Playing) {
            pMyFrame = NULL;
            struct timespec timespec1;
            timespec1.tv_sec = 0;
            timespec1.tv_nsec = 1000 * 1000 * 10;
            pthread_mutex_lock(&m_Frame_Queuelock);
            pthread_cond_timedwait(&m_Frame_condition, &m_Frame_Queuelock, &timespec1);
            if (!m_FrameQueue_H264.empty()) {
                pMyFrame = m_FrameQueue_H264.front();
                m_FrameQueue_H264.pop();

                //LOGE("count Frame= %d", m_FrameQueue_H264.size());
            }
            pthread_mutex_unlock(&m_Frame_Queuelock);
            if (pMyFrame == NULL) {
                usleep(1000 * 2);
                continue;
            }

            if (F_IsNeed2WriteH264Stream()) {
                WriteMp4Frame(pMyFrame->data, pMyFrame->nLen, pMyFrame->bKeyFranme);
            } else {
                myMediaCoder.offerEncoder(pMyFrame->data, pMyFrame->nLen);
            }


            if (pMyFrame != NULL) {
                delete pMyFrame;
            }

            usleep(2*1000); //delay 10ms  让出CPU给其他线程
        }


        CloseVideo();
        while (!m_FrameQueue.empty()) {
            AVFrame *pFrame = NULL;
            pFrame = m_FrameQueue.front();
            m_FrameQueue.pop();
            if (pFrame == NULL) {
                continue;
            }
            if (pFrame != NULL) {
                if (pFrame->data[0] != NULL)
                    av_free(pFrame->data[0]);
                av_frame_free(&pFrame);
            }
        }


        while (!m_FrameQueue_H264.empty()) {
            pMyFrame = m_FrameQueue_H264.front();
            m_FrameQueue.pop();
            delete pMyFrame;
        }
        LOGE("Exit Write Frame!");


        return 0;
    }

    return FFMPEGPLAYER_NOERROR;
}

//----------------------------------------------------------------------
#if 0
int C_FFMpegPlayer::writeFrame(AVFrame *pOutFrame) {
    int ret;
    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;

    int got_output;
    pOutFrame->pts = nSaveInx * (video_st->time_base.den) / ((video_st->time_base.num) * 25);
    nSaveInx++;
    ret = avcodec_encode_video2(pCodecCtx, &pkt, pOutFrame, &got_output);
    if (ret < 0) {
        return FFMPEGPLAYER_SAVEVIDEOFAILED;
    }
    if (got_output) {
        pkt.stream_index = video_st->index;
        writePacket(&pkt);
    }


    for (got_output = 1; got_output;) {

        ret = avcodec_encode_video2(pCodecCtx, &pkt, NULL, &got_output);
        if (ret < 0) {
            //DEBUG_PRINT("Error encoding frame\n");
            //return FFMPEGPLAYER_SAVEVIDEOFAILED;
        }

        if (got_output) {
            writePacket(&pkt);
        }
    }

    return FFMPEGPLAYER_NOERROR;
}
#endif

//----------------------------------------------------------------------
int C_FFMpegPlayer::writePacket(AVPacket *pPacket) {


    if (pCodecCtx->coded_frame->key_frame)
        pPacket->flags |= AV_PKT_FLAG_KEY;
    int64_t now = av_gettime();
    pPacket->stream_index = video_st->index;
    if (m_prevTime != 0) {
        float frameTime =
                ((float) pCodecCtx->time_base.num / (float) pCodecCtx->time_base.den) *
                1000;
        int timeEsc = ((now - m_prevTime) / 1000) + m_prevLeft;
        m_prevLeft = timeEsc % (int) frameTime;
        int frameNeed = (timeEsc / frameTime) - 1;
        if (frameNeed >= 1) {
            for (int i = 0; i < frameNeed; i++) {
                AVPacket tempkt;
                av_init_packet(&tempkt);
                av_packet_ref(&tempkt, &m_prevPkt);
                writePacketDirectly(&tempkt);
            }
        }
    }
    if (m_EncodeID == AV_CODEC_ID_MJPEG) {
        av_packet_unref(&m_prevPkt);
        av_init_packet(&m_prevPkt);
        av_packet_ref(&m_prevPkt, pPacket);
    }
    pPacket->stream_index = video_st->index;
    writePacketDirectly(pPacket);
    m_prevTime = now;

    return FFMPEGPLAYER_NOERROR;
}

//----------------------------------------------------------------------
int C_FFMpegPlayer::writePacketDirectly(AVPacket *pPacket) {

    pPacket->pts = m_outFrameCnt;
    pPacket->dts = pPacket->pts;

    if (pPacket->pts != AV_NOPTS_VALUE) {
        pPacket->pts = av_rescale_q(pPacket->pts, video_st->codec->time_base,
                                    video_st->time_base);
    }
    //video_st->codecpar->sample_aspect_ratio;
    if (pPacket->dts != AV_NOPTS_VALUE)
        pPacket->dts = av_rescale_q(pPacket->dts, video_st->codec->time_base,
                                    video_st->time_base);
    m_outFrameCnt++;
    int i32Ret = av_interleaved_write_frame(pFormatCtx, pPacket);
    if (i32Ret != 0) {
        av_packet_unref(pPacket);
        return i32Ret;
    }

    return FFMPEGPLAYER_NOERROR;
}

int C_FFMpegPlayer::CreateEncodeStream() {


    fmt = av_guess_format("mp4", NULL, NULL);
    if (!fmt) {
        DEBUG_PRINT("av_guess_format failed\n");
        return FFMPEGPLAYER_CREATESAVESTREAMFAILED;
    }

    fmt->video_codec = m_EncodeID;
    int err = avformat_alloc_output_context2(&pFormatCtx, fmt, NULL, NULL);

    if (err < 0 || !pFormatCtx) {
        DEBUG_PRINT("avformat_alloc_output_context2 failed\n");
        return FFMPEGPLAYER_CREATESAVESTREAMFAILED;
    }

    AVCodec *codec;
    codec = avcodec_find_encoder(m_EncodeID);//  m_EncodeID);
    if (!codec) {
        DEBUG_PRINT("Codec not found\n");
        return FFMPEGPLAYER_CREATESAVESTREAMFAILED;
    }

    pCodecCtx = avcodec_alloc_context3(codec);
    if (!pCodecCtx) {
        DEBUG_PRINT("Could not allocate video codec context\n");
        return FFMPEGPLAYER_CREATESAVESTREAMFAILED;
    }


    if (nIC_Type == IC_SN) {
        pCodecCtx->width = m_codecCtx->width;
        pCodecCtx->height = m_codecCtx->height;
        pCodecCtx->time_base.num = 1;
        pCodecCtx->time_base.den = 25;
    } else {
        //streaming format
        pCodecCtx->width = m_codecCtx->width;
        pCodecCtx->height = m_codecCtx->height;
        pCodecCtx->time_base.num = 1;//m_codecCtx->time_base.num;
        pCodecCtx->time_base.den = 25;//m_codecCtx->time_base.den;
    }


    if (m_EncodeID == AV_CODEC_ID_MJPEG) {
        pCodecCtx->bit_rate = m_codecCtx->bit_rate;
        pCodecCtx->pix_fmt = m_codecCtx->pix_fmt;

    } else {
        if (m_codecCtx->height >= 720) {
            pCodecCtx->bit_rate = 4 * 1000 * 1000;//m_codecCtx->bit_rate;
        } else {
            pCodecCtx->bit_rate = 2 * 1000 * 1000;// m_codecCtx->bit_rate;
        }

        pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
        pCodecCtx->gop_size = 30;
        pCodecCtx->qmin = 8;
        pCodecCtx->qmax = 41;
        pCodecCtx->thread_count = 2;
        //pCodecCtx->max_b_frames=0;
        pCodecCtx->profile = FF_PROFILE_MPEG4_SIMPLE;


    }

    //Optional Param
    pCodecCtx->has_b_frames = 0;
    pCodecCtx->max_b_frames = 0;

    if (pFormatCtx->oformat->flags & AVFMT_GLOBALHEADER)
        pCodecCtx->flags |= CODEC_FLAG_GLOBAL_HEADER;


    int ret = avcodec_open2(pCodecCtx, codec, NULL);
    if (ret < 0) {
        DEBUG_PRINT("Could not open codeca\n");
        return FFMPEGPLAYER_CREATESAVESTREAMFAILED;
    }

    //header format

    video_st = avformat_new_stream(pFormatCtx, codec);
    //AVCodecContext *tmpcodeContext=video_st->codec;  //aivenlau
    AVCodecContext *tmpcodeContext = pCodecCtx;

    //tmpcodeContext->thread_count =  pCodecCtx->thread_count;
    //tmpcodeContext->gop_size=pCodecCtx->gop_size;
    //tmpcodeContext->qmin = pCodecCtx->qmin;
    //tmpcodeContext->qmax = pCodecCtx->qmax;

    //video_st->codec->coder_type = AVMEDIA_TYPE_VIDEO;


    //avienlau
    pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    if (pFormatCtx->oformat->flags & AVFMT_GLOBALHEADER)
        pCodecCtx->flags |= CODEC_FLAG_GLOBAL_HEADER;

//    tmpcodeContext->sample_aspect_ratio = video_st->sample_aspect_ratio = pCodecCtx->sample_aspect_ratio;// = m_codecCtx->sample_aspect_ratio;
    //   tmpcodeContext->bits_per_raw_sample = pCodecCtx->bits_per_raw_sample;// = m_codecCtx->bits_per_raw_sample;
    pCodecCtx->chroma_sample_location = AVCHROMA_LOC_LEFT;// = m_codecCtx->chroma_sample_location;
    pCodecCtx->codec_id = m_EncodeID;
    //tmpcodeContext->width = pCodecCtx->width;
    //tmpcodeContext->height = pCodecCtx->height;
    //tmpcodeContext->time_base = pCodecCtx->time_base;


    avcodec_parameters_from_context(video_st->codecpar, pCodecCtx);

    av_stream_set_r_frame_rate(video_st, (AVRational) {1, 25});

    err = avio_open(&pFormatCtx->pb, sRecordFileName_tmp.c_str(), AVIO_FLAG_WRITE);
    if (err < 0) {
        return FFMPEGPLAYER_CREATESAVESTREAMFAILED;
    }

    if (pCodecCtx->extradata) {
        unsigned char *newData = (unsigned char *) malloc(pCodecCtx->extradata_size);
        memcpy(newData, pCodecCtx->extradata, pCodecCtx->extradata_size);
        tmpcodeContext->extradata = newData;
        tmpcodeContext->extradata_size = pCodecCtx->extradata_size;
    }

    int rea = avformat_write_header(pFormatCtx, NULL);
    if (rea < 0) {
        LOGE("Write Video error1");
        return FFMPEGPLAYER_CREATESAVESTREAMFAILED;
    }


    return FFMPEGPLAYER_NOERROR;
}

//----------------------------------------------------------------------
int C_FFMpegPlayer::flush_encoder(AVFormatContext *fmt_ctx, unsigned int stream_index) {
    return 0;
}

int C_FFMpegPlayer::CloseVideo() {
    DEBUG_PRINT("CloseVideo\n");

    if (fmt != NULL) {
        av_write_trailer(pFormatCtx);
        if (!(pFormatCtx->oformat->flags & AVFMT_NOFILE) && pFormatCtx->pb)
            avio_close(pFormatCtx->pb);

        /*
        if (video_st)
        {
            avcodec_close(video_st->codec);
        }
         */
        if (pCodecCtx) {
            avcodec_free_context(&pCodecCtx);
            pCodecCtx = NULL;
        }
        avformat_free_context(pFormatCtx);
        video_st = NULL;
        fmt = NULL;
        pFormatCtx = NULL;
        if (m_EncodeID == AV_CODEC_ID_MJPEG)
            av_packet_unref(&m_prevPkt);
    }
    ClearQueue();
    return FFMPEGPLAYER_NOERROR;
}

//----------------------------------------------------------------------
AVFrame *C_FFMpegPlayer::F_GetFrame() {
    int ret = sws_scale(img_convert_ctxBmp,
                        (const uint8_t *const *) m_decodedFrame->data,
                        m_decodedFrame->linesize, 0, m_codecCtx->height,
                        pFrameRGB->data, pFrameRGB->linesize);
    if (ret == 0)
        return pFrameRGB;
    else
        return NULL;
}


int nSteppp = 0;

void F_SentRevBmp(int32_t wh);


extern int nBufferLen;


//extern uint8_t nICType;

bool C_FFMpegPlayer::F_IsNeed2WriteH264Stream(void) {

    //if (nICType == IC_GKA || nICType == IC_GPH264A || nICType == IC_GPH264)
    if (nIC_Type == IC_GKA || nIC_Type == IC_GPH264A) {
        if (m_codecCtx != NULL && m_codecCtx->width == nRecordWidth && m_codecCtx->height == nRecordHeight) {
            return true;
        }
    }
    return false;

}

extern int encord_colorformat;

AVPacket *C_FFMpegPlayer::F_GetPacket() {
    int ret, got_output;
    if (pFrameYUV == NULL)
        return NULL;
    if (m_codecCtx == NULL)
        return NULL;

#if 1
    if (!F_IsNeed2WriteH264Stream()) {
        if (m_FrameQueue_H264.size() < 15) {
            if (m_bSaveVideo) {
#if 1
                AVFrame *frame_rec;
                if (pFrameYUV->width != nRecordWidth || pFrameYUV->height != nRecordHeight) {

                    int psrc_w = pFrameYUV->width;
                    int psrc_h = pFrameYUV->height;
                    int pdst_w = nRecordWidth;
                    int pdst_h = nRecordHeight;

                    uint8 *i420_buf1 = pFrameYUV->data[0];
                    uint8 *i420_buf2 = pFrameRecord->data[0];
                    libyuv::I420Scale(&i420_buf1[0], psrc_w,
                                      &i420_buf1[psrc_w * psrc_h], psrc_w >> 1,
                                      &i420_buf1[(psrc_w * psrc_h * 5) >> 2], psrc_w >> 1,
                                      psrc_w, psrc_h,
                                      &i420_buf2[0], pdst_w,
                                      &i420_buf2[pdst_w * pdst_h], pdst_w >> 1,
                                      &i420_buf2[(pdst_w * pdst_h * 5) >> 2], pdst_w >> 1,
                                      pdst_w, pdst_h,
                                      libyuv::kFilterBilinear);

                    frame_rec = pFrameRecord;

                } else {
                    frame_rec = pFrameYUV;
                }

                int HH = nRecordHeight;
                int H2 = HH >> 1;
                int WW = nRecordWidth;
                int W2 = WW >> 1;
                uint8_t *srcY = frame_rec->data[0];
                uint8_t *srcU = frame_rec->data[1];
                uint8_t *srcV = frame_rec->data[2];
                uint32_t nCount = 0;

                MyFrame *myfram = new MyFrame();

                nCount += frame_rec->linesize[0] * HH;
                nCount += frame_rec->linesize[1] * H2;
                nCount += frame_rec->linesize[2] * H2;
                myfram->data = new uint8_t[nCount];
                myfram->nLen = nCount;
                unsigned char *bufY = myfram->data;
                unsigned char *bufU = bufY + WW * HH;
                unsigned char *bufV = bufU + (W2 * H2);
                //unsigned char *bufV = bufU + frame_rec->linesize[1] * H2;

                if (encord_colorformat == OMX_COLOR_FormatYUV420SemiPlanar) {

                    libyuv::I420ToNV12(srcY, frame_rec->linesize[0],
                                       srcU, frame_rec->linesize[1],
                                       srcV, frame_rec->linesize[2],
                                       bufY, WW,
                                       bufU, WW,
                                       WW, HH);
                } else {
                    libyuv::I420Copy(srcY, frame_rec->linesize[0],
                                     srcU, frame_rec->linesize[1],
                                     srcV, frame_rec->linesize[2],
                                     bufY, WW,
                                     bufU, W2,
                                     bufV, W2,
                                     WW, HH);
                }

                pthread_mutex_lock(&m_Frame_Queuelock);
                while (m_FrameQueue_H264.size()) {
                    MyFrame *fa = m_FrameQueue_H264.front();
                    m_FrameQueue_H264.pop();
                    delete fa;
                }
                m_FrameQueue_H264.push(myfram);
                pthread_cond_signal(&m_Frame_condition);
                pthread_mutex_unlock(&m_Frame_Queuelock);

#endif

            }
        }
    }
    F_DispSurface();
#endif
    return NULL;

}

void F_OnSave2ToGallery_mid(int n);

int C_FFMpegPlayer::EncodeSnapshot() {


    AVCodec *codec;
    AVCodecContext *cSnap = NULL;
    int ret, got_output;
    AVPacket pkt;
    DEBUG_PRINT("Save Snapshot\n");

    m_bSaveSnapshot = false;

    codec = avcodec_find_encoder(AV_CODEC_ID_MJPEG);
    if (!codec) {
        DEBUG_PRINT("Codec not found\n");
        return FFMPEGPLAYER_SAVESNAPSHOTFAILED;
    }
    cSnap = avcodec_alloc_context3(codec);
    if (!cSnap) {
        DEBUG_PRINT("Could not allocate video codec context\n");
        return FFMPEGPLAYER_SAVESNAPSHOTFAILED;
    }
    /* put sample parameters */

    cSnap->codec_type = AVMEDIA_TYPE_VIDEO;
    cSnap->width = nRecordWidth;   // 1280;//m_codecCtx->width;
    cSnap->height = nRecordHeight; //720;//m_codecCtx->height;
    cSnap->time_base.num = 1;//m_codecCtx->time_base.num;
    cSnap->time_base.den = 1;//m_codecCtx->time_base.den;
    cSnap->pix_fmt = AV_PIX_FMT_YUVJ420P;  //  pix_format;//m_codecCtx->pix_fmt;


    /* open it */
    if ((ret = avcodec_open2(cSnap, codec, NULL)) < 0) {
        DEBUG_PRINT("Could not open codecc\n");
        return FFMPEGPLAYER_SAVESNAPSHOTFAILED;
    }

    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;
    ret = 0;
    if (ret != 0) {
        return FFMPEGPLAYER_SAVESNAPSHOTFAILED;
    }
    AVFrame *frame_Snap;
    frame_Snap = av_frame_alloc();

    frame_Snap->format = pix_format;
    frame_Snap->width = m_codecCtx->width;
    frame_Snap->height = m_codecCtx->height;
    ret = av_image_alloc(
            frame_Snap->data, frame_Snap->linesize, nRecordWidth,
            nRecordHeight,
            pix_format, 4);


    int psrc_w = frame_SnapBuffer->width;
    int psrc_h = frame_SnapBuffer->height;
    int pdst_w = nRecordWidth;
    int pdst_h = nRecordHeight;

    uint8 *i420_buf1 = frame_SnapBuffer->data[0];
    uint8 *i420_buf2 = frame_Snap->data[0];

    libyuv::I420Scale(&i420_buf1[0], psrc_w,
                      &i420_buf1[psrc_w * psrc_h], psrc_w >> 1,
                      &i420_buf1[(psrc_w * psrc_h * 5) >> 2], psrc_w >> 1,
                      psrc_w, psrc_h,
                      &i420_buf2[0], pdst_w,
                      &i420_buf2[pdst_w * pdst_h], pdst_w >> 1,
                      &i420_buf2[(pdst_w * pdst_h * 5) >> 2], pdst_w >> 1,
                      pdst_w, pdst_h,
                      libyuv::kFilterBilinear);


    ret = avcodec_encode_video2(cSnap, &pkt, frame_Snap, &got_output);

    if (frame_Snap != NULL) {
        av_free(frame_Snap->data[0]);
        av_frame_free(&frame_Snap);
        frame_Snap = 0;
    }


    if (ret < 0) {
        DEBUG_PRINT("Error encoding frame\n");
        return FFMPEGPLAYER_SAVESNAPSHOTFAILED;
    }
    if (got_output) {
        DEBUG_PRINT("Write frame (size=%5d)\n", pkt.size);
        FILE *fp = fopen(m_snapShotPath, "wb");
        if (fp) {
            fwrite(pkt.data, 1, pkt.size, fp);
            fclose(fp);
        } else {
            DEBUG_PRINT("Failed to open file: %s\n", m_snapShotPath);
            return FFMPEGPLAYER_SAVESNAPSHOTFAILED;
        }
    }

    av_packet_unref(&pkt);
    avcodec_close(cSnap);
    avcodec_free_context(&cSnap);
    F_OnSave2ToGallery_mid(0);
    return FFMPEGPLAYER_NOERROR;
}

//----------------------------------------------------------------------
int C_FFMpegPlayer::Releaseffmpeg() {

    //avpicture_free(&m_frameRGBA);
    // Free the YUV frame
    m_Status = E_PlayerStatus_Stoped;
    av_free(m_decodedFrame);

    // Close the codec
    avcodec_close(m_codecCtx);
    // Close the video file
    avformat_close_input(&m_formatCtx);

    sws_freeContext(img_convert_ctx);
    if (img_convert_ctxBmp != NULL) {
        sws_freeContext(img_convert_ctxBmp);
        img_convert_ctxBmp = NULL;
    }

    if (pFrameYUV != NULL) {
        av_free(pFrameYUV->data[0]);
        av_frame_free(&pFrameYUV);
        pFrameYUV = 0;
    }

    if (pFrameYUV_Disp != NULL) {
        av_free(pFrameYUV_Disp->data[0]);
        av_frame_free(&pFrameYUV_Disp);
        pFrameYUV_Disp = NULL;
    }


    img_convert_ctx = NULL;
    img_convert_ctxBmp = NULL;
    m_formatCtx = NULL;
    m_codecCtx = NULL;
    m_decodedFrame = NULL;
    if (My_EncodecodecCtx != NULL) {
        avcodec_close(My_EncodecodecCtx);
        av_free(My_EncodecodecCtx);
        My_EncodecodecCtx = NULL;
    }

    if (buffer_a != NULL) {
        av_free(buffer_a);
        buffer_a = NULL;

    }
    if (out_buffer != NULL) {
        av_free(out_buffer);
        out_buffer = NULL;
    }
    if (out_bufferBmp != NULL) {
        av_free(out_bufferBmp);
        out_bufferBmp = NULL;
    }
    if (frame_a != NULL) {
        av_free(frame_a->data[0]);
        av_frame_free(&frame_a);
        frame_a = NULL;
    }
    if (frame_b != NULL) {
        av_free(frame_b->data[0]);
        av_frame_free(&frame_b);
        frame_b = NULL;
    }
    if (pFrameRGB != NULL) {
        av_free(pFrameRGB->data[0]);
        av_frame_free(&pFrameRGB);
        pFrameRGB = NULL;
    }
    if (nativeWindow != NULL) {
        ANativeWindow_release(nativeWindow);
        nativeWindow = NULL;
    }
    if (frame_SnapBuffer != NULL) {
        av_free(frame_SnapBuffer->data[0]);
        av_frame_free(&frame_SnapBuffer);
        frame_SnapBuffer = NULL;

    }
    if (pFrameRecord != NULL) {
        av_free(pFrameRecord->data[0]);
        av_frame_free(&pFrameRecord);
        pFrameRecord = NULL;
    }
    if (img_convert_ctxRecord != NULL) {
        sws_freeContext(img_convert_ctxRecord);
        img_convert_ctxRecord = NULL;
    }
    if (bInitMediaA) {
        bInitMediaA = false;
        if (m_decodedFrame != NULL) {
            av_free(m_decodedFrame->data[0]);
            av_frame_free(&m_decodedFrame);
            m_decodedFrame = NULL;
        }
    }

    LOGE("Exit PlayB...");
    return FFMPEGPLAYER_NOERROR;
}

//----------------------------------------------------------------------
#if 0
int C_FFMpegPlayer::init_decode(AVFormatContext *s) {
int video_index = -1;
int audio_index = -1;
int ret = -1;
if (s == NULL) {
LOGE("error!");
return -1;
}
/*
Get video stream index, if no video stream then create it.
And audio so on.
*/
if (0 == s->nb_streams) {
create_stream(s, AVMEDIA_TYPE_VIDEO);
create_stream(s, AVMEDIA_TYPE_AUDIO);
video_index = 0;
audio_index = 1;
LOGE("intiD:1");
} else if (1 == s->nb_streams) {
if (AVMEDIA_TYPE_VIDEO ==
    s->streams[0]->codec->codec_type)//  s->streams[0]->codec->codec_type)
{
    create_stream(s, AVMEDIA_TYPE_AUDIO);
    video_index = 0;
    audio_index = 1;
    LOGE("intiD:2");
} else if (AVMEDIA_TYPE_AUDIO ==
           s->streams[0]->codec->codec_type) {  //s->streams[0]->codec->codec_type
    create_stream(s, AVMEDIA_TYPE_VIDEO);
    video_index = 1;
    audio_index = 0;
    LOGE("intiD:3");
}
} else if (2 == s->nb_streams) {
if (AVMEDIA_TYPE_VIDEO == s->streams[0]->codec->codec_type) {
    video_index = 0;
    audio_index = 1;
    LOGE("intiD:4");
} else if (AVMEDIA_TYPE_VIDEO == s->streams[1]->codec->codec_type) {
    video_index = 1;
    audio_index = 0;
    LOGE("intiD:5");
}
}
/*Error. I can't find video stream.*/
if (video_index != 0 && video_index != 1) {
LOGE("intiD:6");
return ret;
}
//Init the audio codec(AAC).
#if 1
s->streams[audio_index]->codec->codec_id = AV_CODEC_ID_PCM_ALAW;
s->streams[audio_index]->codec->sample_rate = 8000;
s->streams[audio_index]->codec->time_base.den = 8000;
s->streams[audio_index]->codec->time_base.num = 1;
s->streams[audio_index]->codec->bits_per_coded_sample = 8;
s->streams[audio_index]->codec->channels = 1;
s->streams[audio_index]->codec->channel_layout = 0;
s->streams[audio_index]->pts_wrap_bits = 32;
s->streams[audio_index]->time_base.den = 8000;
s->streams[audio_index]->time_base.num = 1;
#endif
#if 1
//Init the video codec(H264).
s->streams[video_index]->codec->codec_id = AV_CODEC_ID_H264;
s->streams[video_index]->codec->width = 640;
if (b480)
s->streams[video_index]->codec->height = 480;
else
s->streams[video_index]->codec->height = 360;
s->streams[video_index]->codec->ticks_per_frame = 2;
s->streams[video_index]->codec->bit_rate_tolerance = 1024000;
s->streams[video_index]->codec->pix_fmt = AV_PIX_FMT_YUV420P;
s->streams[video_index]->pts_wrap_bits = 32;
s->streams[video_index]->time_base.den = 90000;
s->streams[video_index]->time_base.num = 1;
s->streams[video_index]->avg_frame_rate.den = 0;
s->streams[video_index]->avg_frame_rate.num = 0;
/*Need to change, different condition has different frame_rate. 'r_frame_rate' is new in ffmepg2.3.3*/
s->streams[video_index]->r_frame_rate.den = 2;
s->streams[video_index]->r_frame_rate.num = 40;
LOGE("intiD:7");
#else
s->streams[video_index]->codec->codec_id = AV_CODEC_ID_H264;
s->streams[video_index]->codec->width = 1280;
s->streams[video_index]->codec->height = 720;
s->streams[video_index]->codec->ticks_per_frame = 2;
s->streams[video_index]->codec->bit_rate_tolerance = 1000000;
s->streams[video_index]->codec->pix_fmt = AV_PIX_FMT_YUV420P;
s->streams[video_index]->pts_wrap_bits = 32;
s->streams[video_index]->time_base.den = 90000;
s->streams[video_index]->time_base.num = 1;
s->streams[video_index]->avg_frame_rate.den = 0;
s->streams[video_index]->avg_frame_rate.num = 0;
/*Need to change, different condition has different frame_rate. 'r_frame_rate' is new in ffmepg2.3.3*/
s->streams[video_index]->r_frame_rate.den = 2;
s->streams[video_index]->r_frame_rate.num = 40;

#endif

/* H264 need sps/pps for decoding, so read it from the first video tag.*/
ret = get_video_extradata(s, video_index);
LOGE("intiD:8");
/*Update the AVFormatContext Info*/
//s->nb_streams = 2;
/*empty the buffer.*/
//  s->pb->buf_ptr = s->pb->buf_end;
/*
something wrong.
TODO: find out the 'pos' means what.
then set it.
*/
//s->pb->pos = s->pb->buf_end;
return 0;
}
#endif

uint8_t *bufferA;
//uint32_t   nLenA;
//uint32_t   nInxA;


//unsigned char buff[5000];
int bufflen;
int pos;

int read_packet(void *opaque, uint8_t *buf, int buf_size) {
    int size = buf_size;
    if (bufflen - pos < buf_size)
        size = bufflen - pos;
    if (size > 0) {
        memcpy(buf, bufferA + pos, size);
        pos += size;
    }
    return size;
}

int64_t seek(void *opaque, int64_t offset, int whence) {
    switch (whence) {
        case SEEK_SET:
            pos = offset;
            break;
        case SEEK_CUR:
            pos += offset;
            break;
        case SEEK_END:
            pos = bufflen - offset;
            break;
        case AVSEEK_SIZE:
            return bufflen;
            break;
    }
    return pos;
}

#if 0
void C_FFMpegPlayer::F_SendFrame(uint8_t *buffer, int nLen) {
    if (nIC_Type != IC_SN)
        return;
    int err_code;


    int i;
    int ret;
    av_register_all();
    AVCodec *pCodec = NULL;
    AVCodecContext *pCodecCtx;
    AVFormatContext *pFormatCtxA;
    pFormatCtxA = avformat_alloc_context();

    AVInputFormat *inf = av_find_input_format("mjpeg");

    pos = 0;
    bufferA = buffer;
    bufflen = nLen;

    unsigned char *aviobuffer = (unsigned char *) av_malloc(1024 * 200);
    AVIOContext *avio = avio_alloc_context(aviobuffer, 1024 * 200, 0, &pos, read_packet, NULL, seek);
    pFormatCtxA->pb = avio;
    if ((err_code = avformat_open_input(&pFormatCtxA, "nofile.jpg", inf, NULL)) != 0) {
        LOGE("AVFrame Error-1");
        av_free(aviobuffer);
        return;
    }
    if (avformat_find_stream_info(pFormatCtxA, NULL) < 0) {
        LOGE("Couldn't find stream information.");
        av_free(aviobuffer);
        return;
    }
    /* Find the first video stream */
    int videoStream = -1;
    //printf("==========> pFormatCtxA->nb_streams = %dn", pFormatCtxA->nb_streams);

    for (i = 0; i < pFormatCtxA->nb_streams; i++) {
        //if (pFormatCtxA->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        if (pFormatCtxA->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStream = i;
            break;
        }
    }

    if (videoStream == -1) {
        LOGE("No videoStream!");
        av_free(aviobuffer);
        return;        // Didn't find a video stream
    }

    // pCodecCtx = pFormatCtxA->streams[videoStream]->codec;  by aivenlau

    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if (pCodec == NULL) {
        LOGE("can not find decoder!n");
        av_free(aviobuffer);
        return;
    }

    pCodecCtx = avcodec_alloc_context3(pCodec);
    avcodec_parameters_to_context(pCodecCtx, pFormatCtxA->streams[i]->codecpar);

    LOGE("W=%d H=%d Fomrt = %d", pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt);

    /* Open codec */
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        LOGE("cannot open software codecn");
        av_free(aviobuffer);
        return; // Could not open codec
    }
    AVFrame *pFrame = av_frame_alloc();
    AVPacket packet;
    av_init_packet(&packet);
    if (av_read_frame(pFormatCtxA, &packet) == 0) {
        int frameFinished = 0;
        //ret = avcodec_send_packet(pCodecCtx,&packet);
        ret = 0;
        if (ret == 0) {
#ifndef FFMPEG3_3_3
            ret = avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);
    if (ret < 0 || frameFinished == 0)
        ret = -1;
    else
        ret = 0;
#else
            ret = avcodec_receive_frame(pCodecCtx, pFrame);
            if (avcodec_send_packet(pCodecCtx, &packet) == 0) {
                if (avcodec_receive_frame(pCodecCtx, pFrame) != 0) {
                    ret = -1;
                } else {
                    ret = 0;
                }
            } else {
                ret = -1;
            }
#endif
            if (ret == 0) {

                AVFrame *pFrameA = av_frame_alloc();


                int ret = av_image_alloc(
                        pFrameA->data, pFrameA->linesize, pCodecCtx->width,
                        pCodecCtx->height,
                        AV_PIX_FMT_YUV420P,
                        32);  //pCodecCtx->pix_fmt,

                {
                    ret = sws_scale(m_outsws_ctx,
                                    (const uint8_t *const *) (pFrame->data),
                                    pFrame->linesize,
                                    0,
                                    pCodecCtx->height,
                                    pFrameA->data,
                                    pFrameA->linesize);
                }

                pthread_mutex_lock(&m_Frame_Queuelock);
                //m_FrameQueue.push_back(pFrameA);
                m_FrameQueue.push(pFrameA);
                pthread_cond_signal(&m_Frame_condition);
                pthread_mutex_unlock(&m_Frame_Queuelock);
            } else {
                LOGE("Record error!");
            }
            av_packet_unref(&packet);
        }
    } else {
        LOGE("ERROR!!!!");
    }
    av_frame_free(&pFrame);
    av_free(aviobuffer);
    avcodec_close(pCodecCtx);
    avcodec_free_context(&pCodecCtx);
    avformat_close_input(&pFormatCtxA);
}
#endif

bool blocked = false;

extern  bool bSentRevBMP;

extern AVFrame *gl_Frame;
extern int64_t  nRecStartTime;
void C_FFMpegPlayer::F_DispSurface() {

    if(bSentRevBMP)
    {
        if(Rgbabuffer!=NULL)
        {
            libyuv::I420ToBGRA(pFrameYUV->data[0], pFrameYUV->linesize[0],
                               pFrameYUV->data[1], pFrameYUV->linesize[1],
                               pFrameYUV->data[2], pFrameYUV->linesize[2],
                               Rgbabuffer, pFrameYUV->width * 4,
                               pFrameYUV->width, pFrameYUV->height);
            F_SentRevBmp(pFrameYUV->width + pFrameYUV->height * 0x10000);
        }

    } else {

        if (gl_Frame != NULL) {
            if ((nSDStatus & bit1_LocalRecording) && !bRealRecording) {
                bRealRecording = true;
                nRecStartTime = av_gettime() / 1000;

            }

            gl_Frame->width = pFrameYUV->width;
            gl_Frame->height = pFrameYUV->height;

            gl_Frame->linesize[0] = pFrameYUV->linesize[0];
            gl_Frame->linesize[1] = pFrameYUV->linesize[1];
            gl_Frame->linesize[2] = pFrameYUV->linesize[2];


            libyuv::I420Copy(pFrameYUV->data[0], pFrameYUV->linesize[0],
                             pFrameYUV->data[1], pFrameYUV->linesize[1],
                             pFrameYUV->data[2], pFrameYUV->linesize[2],
                             gl_Frame->data[0], pFrameYUV->linesize[0],
                             gl_Frame->data[1], pFrameYUV->linesize[1],
                             gl_Frame->data[2], pFrameYUV->linesize[2],
                             pFrameYUV->width, pFrameYUV->height);

        }
    }

}


