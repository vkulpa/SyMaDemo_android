//
// Created by AivenLau on 2017/9/7.
//

#include <string.h>
#include "JPEG_BUFFER.h"
JPEG_BUFFER::JPEG_BUFFER():nSize(0)
{
    buffer=new uint8_t[500*1024];
    nCount = 0;
    nJpegInx= 0;
    bOK = false;
    bzero(mInx,sizeof(mInx));
}
JPEG_BUFFER::~JPEG_BUFFER()
{
    if(buffer!=NULL)
    {
        delete  []buffer;
        buffer = NULL;
    }
}
void JPEG_BUFFER::Clear(void)
{
    if(buffer==NULL)
    {
        buffer=new uint8_t[500*1024];
    }
    nCount = 0;
    nJpegInx= 0;
    nSize = 0;
    nJpegInx= 0;
    bOK = false;
    bzero(mInx,sizeof(mInx));

}
void JPEG_BUFFER::Release(void)
{
    if(buffer!=NULL)
    {
        delete  []buffer;
        buffer = NULL;
    }
    nCount = 0;
    nJpegInx= 0;
    nSize = 0;
    bOK = false;
    bzero(mInx,sizeof(mInx));

}

bool  JPEG_BUFFER::AppendData(uint8_t *data,int nLen)
{
    if(nCount+nLen>500*1024)
    {
        Clear();
        return false;
    }
    memcpy(buffer+nCount,data,nLen);
    nCount+=nLen;
    return true;
}
