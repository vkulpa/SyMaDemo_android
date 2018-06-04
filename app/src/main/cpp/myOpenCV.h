//
// Created by AivenLau on 2018/6/4.
//

#ifndef SYMADEMO_ANDROID_JH_OPENCV_H
#define SYMADEMO_ANDROID_JH_OPENCV_H


#include <cstdint>
#include<opencv2/opencv.hpp>
using namespace cv;
using namespace std;


class myOpenCV {

public:
    static bool  bIsbusing;
    static void F_Bitmap2Grey(uint8_t * cbuf, int h, int w);
};


#endif //SYMADEMO_ANDROID_JH_OPENCV_H
