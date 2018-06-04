//
// Created by AivenLau on 2018/6/4.
//

#include "myOpenCV.h"

bool myOpenCV::bIsbusing = false;

void myOpenCV::F_Bitmap2Grey(uint8_t * cbuf, int h, int w)
{
    if(bIsbusing)
        return;

    if(cbuf!=NULL) {
        bIsbusing=true;
        //Mat imgData(h, w, CV_8UC4, cbuf);
        Mat imgData(h + h/2,w,CV_8UC1,(unsigned char *)cbuf);
        cvtColor(imgData, imgData, CV_YUV420p2GRAY);
        cvtColor(imgData, imgData, CV_GRAY2BGRA);
        cvtColor(imgData, imgData, CV_BGRA2YUV_I420);
        memcpy(cbuf,imgData.data,(size_t)(w*h*1.5));
        imgData.release();
        bIsbusing=false;
    }

}
