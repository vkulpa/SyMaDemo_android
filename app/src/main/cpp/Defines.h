//
// Created by generalplus_sa1 on 4/29/16.
//

#ifndef FFMPEGTEST_DEFINES_H
#define FFMPEGTEST_DEFINES_H



//#ifdef DEBUG
#include <android/log.h>
#ifdef DEBUG

#if defined(ANDROID) || defined(__ANDROID__)

#define DEBUG_PRINT(...) __android_log_print(ANDROID_LOG_ERROR  , "ffmpegJNI",__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR ,"ffmpegJNI—",__VA_ARGS__)
#define M_LOGE(Var, ...) __android_log_print(ANDROID_LOG_ERROR  , ""+Var,__VA_ARGS__)




#else
#define DEBUG_PRINT(format, ...)  printf( \
        "%*.*s(%.5d): " \
        format, 40, 40 , __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__ \
        )
#endif
#else
#define DEBUG_PRINT(format, ...)  ((void)0)
#define LOGE(...) ((void)0)

#endif

#define LOGE_B(...) __android_log_print(ANDROID_LOG_ERROR ,"ffmpegJNI—",__VA_ARGS__)

#endif //FFMPEGTEST_DEFINES_H


