//
// Created by AivenLau on 2017/12/21.
//

#include <cstring>

#include "MyMediaCoder.h"
#include "Defines.h"
//#include "FFMpegPlayerCore.h"

//extern  C_FFMpegPlayer m_FFMpegPlayer;





MyMediaCoder::MyMediaCoder()//:encoder(NULL),decoder(NULL)
{


}

MyMediaCoder::~MyMediaCoder()
{
     F_CloseDecoder();
    F_CloseEncoder();
}

//int encord_colorformat = OMX_COLOR_FormatYUV420SemiPlanar;

void F_InitEncoder(int w,int h,int bitfrate,int fps );

bool MyMediaCoder::F_InitEncoder(int width,int height,int32_t bitrate,int fps)
{

    ::F_InitEncoder(width,height,bitrate,fps);
    return true;

/*
    m_nRecWidth  = width;
    m_nRecHeight = height;
    m_nBitrate = framerate;
    m_nFps = fps;
    F_CloseEncoder();
    pts=0;

    encoder = AMediaCodec_createEncoderByType("video/avc");
    if(encoder == NULL)
        return false;
    AMediaFormat *mediaFormat =  AMediaFormat_new();
    if(mediaFormat==NULL)
    {
        F_CloseEncoder();
        return false;
    }

    if (mediaFormat != NULL)
    {

        uint32_t flags = AMEDIACODEC_CONFIGURE_FLAG_ENCODE;
        AMediaFormat_setString(mediaFormat, AMEDIAFORMAT_KEY_MIME, "video/avc");
        AMediaFormat_setInt32(mediaFormat, AMEDIAFORMAT_KEY_WIDTH, m_nRecWidth);
        AMediaFormat_setInt32(mediaFormat, AMEDIAFORMAT_KEY_HEIGHT, m_nRecHeight);

        AMediaFormat_setInt32(mediaFormat, AMEDIAFORMAT_KEY_BIT_RATE, m_nBitrate);
        AMediaFormat_setInt32(mediaFormat, AMEDIAFORMAT_KEY_FRAME_RATE,  m_nFps);

        AMediaFormat_setInt32(mediaFormat, AMEDIAFORMAT_KEY_COLOR_FORMAT, encord_colorformat);      // #21 COLOR_FormatYUV420SemiPlanar (NV12)
        AMediaFormat_setInt32(mediaFormat, AMEDIAFORMAT_KEY_I_FRAME_INTERVAL, 1);
        media_status_t rc = AMediaCodec_configure(encoder, mediaFormat, NULL, NULL, flags);
        if (AMEDIA_OK == rc) {
            AMediaCodec_start(encoder);
            AMediaFormat_delete(mediaFormat);
            return true;
        } else
        {
            encord_colorformat = OMX_COLOR_FormatYUV420Planar;
            AMediaFormat_setString(mediaFormat, AMEDIAFORMAT_KEY_MIME, "video/avc");
            AMediaFormat_setInt32(mediaFormat, AMEDIAFORMAT_KEY_WIDTH, m_nRecWidth);
            AMediaFormat_setInt32(mediaFormat, AMEDIAFORMAT_KEY_HEIGHT, m_nRecHeight);

            AMediaFormat_setInt32(mediaFormat, AMEDIAFORMAT_KEY_BIT_RATE, m_nBitrate);
            AMediaFormat_setInt32(mediaFormat, AMEDIAFORMAT_KEY_FRAME_RATE, m_nFps);
            AMediaFormat_setInt32(mediaFormat, AMEDIAFORMAT_KEY_COLOR_FORMAT, encord_colorformat);      // #21 COLOR_FormatYUV420SemiPlanar (NV12)
            AMediaFormat_setInt32(mediaFormat, AMEDIAFORMAT_KEY_I_FRAME_INTERVAL, 1);
            rc = AMediaCodec_configure(encoder, mediaFormat, NULL, NULL, flags);
            if (AMEDIA_OK == rc) {
                AMediaCodec_start(encoder);
                AMediaFormat_delete(mediaFormat);
                return true;
            }
            LOGE("set encord 0");
        }
    }
    AMediaFormat_delete(mediaFormat);
    return false;
    */
    return true;
}


bool MyMediaCoder::F_InitDecoder(int width,int height,int8_t *sps,int spsLen,int8_t *pps,int ppsLen,int nfps)
{
#if 0
    m_nDecWidth  = width;
    m_nDecHeight = height;
    decpts = 0;
    m_nFps = nfps;

    if(decoder!=NULL)
        return true;

    F_CloseDecoder();
    if(sps==NULL || pps == NULL || spsLen<=0 || ppsLen <=0)
    {
        return false;
    }
    const char* mine = "video/avc";
    AMediaFormat* videoFormat = AMediaFormat_new();
    if(videoFormat==NULL)
    {
        return false;
    }
    decoder =  AMediaCodec_createDecoderByType(mine);
    AMediaFormat_setString(videoFormat, "mime", "video/avc");
    AMediaFormat_setInt32(videoFormat, AMEDIAFORMAT_KEY_WIDTH, m_nDecWidth); // 视频宽度
    AMediaFormat_setInt32(videoFormat, AMEDIAFORMAT_KEY_HEIGHT, m_nDecHeight); // 视频高度
    AMediaFormat_setInt32(videoFormat,AMEDIAFORMAT_KEY_COLOR_FORMAT,OMX_COLOR_FormatYUV420SemiPlanar);
    AMediaFormat_setBuffer(videoFormat, "csd-0", sps, spsLen); // sps
    AMediaFormat_setBuffer(videoFormat, "csd-1", pps, ppsLen); // pps
    media_status_t rc = AMediaCodec_configure(decoder, videoFormat, NULL, NULL, 0);
    if (AMEDIA_OK == rc)
    {
        AMediaCodec_start(decoder);
        AMediaFormat_delete(videoFormat);
        return true;
    }
    AMediaFormat_delete(videoFormat);
    return false;
#else
    return true;
#endif

}
//extern C_FFMpegPlayer m_FFMpegPlayer;



//void F_ProcessDecordData(uint8_t *data,int32_t nLen,int w,int h,int nColor);


//int nDecColor = OMX_COLOR_FormatYUV420SemiPlanar;

uint8_t  head[]={0x00,0x00,0x00,0x01};

int64_t  nT = av_gettime()/1000;
int64_t  nCurrent;
bool MyMediaCoder::offerDecoder(uint8_t *data,int32_t nLen,int flag)
{
#if 0
    if(decoder== NULL)
        return  false;

#if 1
    size_t inoutLen;
    ssize_t inputIndex = AMediaCodec_dequeueInputBuffer( decoder, 50000);
    if(inputIndex>=0)
    {
        nCurrent = av_gettime()/1000;
        LOGE("set Decord 1 = %d",(int)(nCurrent-nT));
        nT = av_gettime()/1000;
        uint8_t* inputbuffer =  AMediaCodec_getInputBuffer(decoder,inputIndex,&inoutLen);
        if(nLen+4>inoutLen)
        {
            nLen = inoutLen-4;
        }
        memcpy(inputbuffer,head,4);
        memcpy(inputbuffer+4,data,nLen);
        uint64_t ppp = 1+ decpts * 1000000 / m_nFps;
        media_status_t sta = AMediaCodec_queueInputBuffer(decoder,inputIndex,0,nLen+4,ppp,flag);
        decpts++;
        AMediaCodecBufferInfo info;
        ssize_t outbufidx;
        size_t outsize;
        while(true)
        {
            outbufidx = AMediaCodec_dequeueOutputBuffer(decoder, &info, 10);//40000
            if(outbufidx>=0)
            {
                uint8_t *outputBuf = AMediaCodec_getOutputBuffer(decoder, outbufidx, &outsize);
                F_ProcessDecordData(outputBuf, info.size, m_nDecWidth, m_nDecHeight, nDecColor);
                AMediaCodec_releaseOutputBuffer(decoder, outbufidx, 0);// info.size != 0);
                //return true;
            }
            else if(outbufidx == AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED)
            {
                AMediaFormat *format  = AMediaCodec_getOutputFormat(decoder);
                AMediaFormat_getInt32(format,AMEDIAFORMAT_KEY_COLOR_FORMAT,&nDecColor);
                AMediaFormat_delete(format);
                LOGE("Format changed");
            }
            else if(outbufidx == AMEDIACODEC_INFO_OUTPUT_BUFFERS_CHANGED)
            {
                LOGE("Buffer changed");
            }
            else
            {
                break;
            }
        }
    } else
    {
        LOGE("Decord NoInputbuffer!");
    }
#endif
#endif
    return false;
}

void offerEncoder(uint8_t *data,int nLen);
bool MyMediaCoder::offerEncoder(uint8_t *data,int32_t nLen)
{
    ::offerEncoder(data,nLen);
    return true;
#if 0
    int pos = 0;
    size_t inoutLen;
    ssize_t inputIndex = AMediaCodec_dequeueInputBuffer( encoder, 5000);
    bool re = false;
    if(inputIndex>=0)
    {
        uint8_t* inputbuffer =  AMediaCodec_getInputBuffer(encoder,(size_t)inputIndex,&inoutLen);
        if(nLen>inoutLen)
        {
            nLen = inoutLen;
        }
        memcpy(inputbuffer,data,(size_t)nLen);
        uint64_t ppp = (uint64_t)(pts * 1000000 / m_nFps);
        pts++;
        media_status_t sta = AMediaCodec_queueInputBuffer(encoder,(size_t)inputIndex,0,inoutLen,ppp,0);
        AMediaCodecBufferInfo info;
        ssize_t outbufidx;
        while(true)
        {
            outbufidx = AMediaCodec_dequeueOutputBuffer(encoder, &info, 2000); //2000
            if(outbufidx == AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED)
            {
                continue;
            }
            else if(outbufidx == AMEDIACODEC_INFO_OUTPUT_BUFFERS_CHANGED)
            {
                continue;
            }
            else if(outbufidx>=0)
            {
                size_t outsize;
                uint8_t *outputBuf = AMediaCodec_getOutputBuffer(encoder,(size_t)outbufidx, &outsize);
                bool bKeyframe = false;
                if (info.flags == 2)
                {
                    _naSave2FrameMp4(outputBuf, info.size, 0, bKeyframe);
                } else if (info.flags == 1) //BUFFER_FLAG_KEY_FRAME) I Frame
                {
                    bKeyframe = true;
                    _naSave2FrameMp4(outputBuf, info.size, 1, bKeyframe);
                    re = true;
                } else {
                    _naSave2FrameMp4(outputBuf, info.size, 1, bKeyframe);
                    re = true;
                }
                AMediaCodec_releaseOutputBuffer(encoder, (size_t)outbufidx, 0);// info.size != 0);
                break;
            }
            else
            {
                break;
            }
        }
    }
    return re;
#endif

}


void F_CloseEncoder();

void MyMediaCoder::F_CloseDecoder(void)
{

/*
    if(decoder !=NULL)
    {
        AMediaCodec_stop(decoder);
        AMediaCodec_delete(decoder);
        decoder=NULL;
    }
*/
}

void MyMediaCoder::F_CloseEncoder(void)
{
    ::F_CloseEncoder();
/*
    if(encoder!=NULL)
    {
        AMediaCodec_stop(encoder);
        AMediaCodec_delete(encoder);

        encoder=NULL;
    }
*/
}