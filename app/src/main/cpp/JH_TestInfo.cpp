//
// Created by AivenLau on 2018/3/5.
//

#include <cstring>

#include <cstdio>
#include <src/main/cpp/ffmpeginclude/libavutil/time.h>

#include "JH_TestInfo.h"

JH_TestInfo::JH_TestInfo()
{
    TestInfo_vector.clear();
}
JH_TestInfo::~JH_TestInfo()
{
    vector<TestInfo_ *>::iterator  it;
    for(it = TestInfo_vector.begin();it!=TestInfo_vector.end();it++)
    {
        delete *it;
    }
    vector<TestInfo_ *>().swap(TestInfo_vector);
}

void  JH_TestInfo::F_InsertInof(int nInxFrame,int nCount,int nNo)
{
    vector<TestInfo_ *>::iterator  it;
    bool   bFind=false;
    for(it = TestInfo_vector.begin();it!=TestInfo_vector.end();it++)
    {
        if((*it)->nFrameNo == nInxFrame)
        {
            bFind = true;
            (*it)->nCount = nCount;
            if(nNo<2000)
            {
                if((*it)->nArray[nNo]==-1)
                {
                    (*it)->nNormalCount++;
                }
                else
                {
                    (*it)->nRepeatCount++;
                }
            }
            break;
        }
    }
    if(!bFind)
    {
        TestInfo_ *info = new TestInfo_();
        /*
        for(int i=0;i<2000;i++)
        {
            info->nArray[i]=-1;
        }
        info->nRepeatCount = 0;
        info->nNormalCount = 0;

        info->nFrameNo = nInxFrame;
        info->nCount = nCount;

        int64_t  msTime = av_gettime()/1000;
        int64_t  secTime = msTime/1000;

        timespec time;
        time.tv_sec = secTime;
        time.tv_nsec = 0;

        tm nowTime;
        localtime_r(&time.tv_sec, &nowTime);
        char current[1024];
        sprintf(current, "%04d-%02d-%02d %02d:%02d:%02d %04dms", nowTime.tm_year + 1900, nowTime.tm_mon+1, nowTime.tm_mday,
                nowTime.tm_hour, nowTime.tm_min, nowTime.tm_sec,msTime);
        info->sStartTime=current;
        if(nNo<2000)
        {
            (*it)->nArray[nNo]=nNo;
            (*it)->nNormalCount++;
        }
        TestInfo_vector.push_back(info);
         */
    }
}

void JH_TestInfo::F_SaveTestInfo(void)
{


}