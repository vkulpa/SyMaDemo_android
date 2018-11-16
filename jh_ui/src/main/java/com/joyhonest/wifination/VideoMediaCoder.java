package com.joyhonest.wifination;

import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;

import java.io.IOException;
import java.nio.ByteBuffer;

public class VideoMediaCoder {
    int pts;
    int fps;
    private MediaCodec mMediaCodec;
    public VideoMediaCoder() {
        mMediaCodec=null;
    }

    public int initMediaCodec(int width,int height,int bitrate,int fps) {
        if (mMediaCodec != null) {
            mMediaCodec.stop();
            mMediaCodec.release();
            mMediaCodec = null;
            pts = 0;
        }
        pts = 0;
        this.fps = fps;
        boolean bOK = true;
        try {
            mMediaCodec = MediaCodec.createEncoderByType("video/avc");
        } catch (IOException e) {
            e.printStackTrace();
            bOK = false;
        } catch (IllegalArgumentException e) {
            e.printStackTrace();
            bOK = false;
        } catch (NullPointerException e) {
            e.printStackTrace();
            bOK = false;
        }
        if (!bOK) {
            return 0x7FFFF;
        }
        int nColor = 0;
        MediaFormat mediaFormat = MediaFormat.createVideoFormat("video/avc", width,height); //height和width一般都是照相机的height和width。

        //描述平均位速率（以位/秒为单位）的键。 关联的值是一个整数
        mediaFormat.setInteger(MediaFormat.KEY_WIDTH, width);
        mediaFormat.setInteger(MediaFormat.KEY_HEIGHT, height);
        mediaFormat.setInteger(MediaFormat.KEY_BIT_RATE, bitrate);
        //描述视频格式的帧速率（以帧/秒为单位）的键。
        mediaFormat.setInteger(MediaFormat.KEY_FRAME_RATE, fps);//帧率，一般在15至30之内，太小容易造成视频卡顿。
        mediaFormat.setInteger(MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420SemiPlanar);//色彩格式，具体查看相关API，不同设备支持的色彩格式不尽相同
        //关键帧间隔时间，单位是秒
        mediaFormat.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, 2);
        boolean bConfigOK = true;
        try {
            mMediaCodec.configure(mediaFormat, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
            nColor = MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420SemiPlanar;
        }
        catch (Exception e)
        {
            e.printStackTrace();
            bConfigOK=false;
            nColor = 0;
        }
        if(!bConfigOK)
        {
            mediaFormat = MediaFormat.createVideoFormat("video/avc", height, width); //height和width一般都是照相机的height和width。

            //描述平均位速率（以位/秒为单位）的键。 关联的值是一个整数
            mediaFormat.setInteger(MediaFormat.KEY_BIT_RATE, bitrate);
            //描述视频格式的帧速率（以帧/秒为单位）的键。
            mediaFormat.setInteger(MediaFormat.KEY_FRAME_RATE, fps);//帧率，一般在15至30之内，太小容易造成视频卡顿。
            mediaFormat.setInteger(MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420Planar);//色彩格式，具体查看相关API，不同设备支持的色彩格式不尽相同
            //关键帧间隔时间，单位是秒
            mediaFormat.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, 2);
            bConfigOK = true;
            try {
                mMediaCodec.configure(mediaFormat, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
                nColor = MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420Planar;
            }
            catch (Exception e)
            {
                e.printStackTrace();
                bConfigOK=false;
                nColor = 0;
            }
        }
        else
        {
            nColor = MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420SemiPlanar;
        }
        if(bConfigOK)
        {
            mMediaCodec.start();
        }
        else
        {
            mMediaCodec.release();
            mMediaCodec = null;
        }
        return nColor;

    }
    public void F_CloseEncoder()
    {
        if(mMediaCodec==null)
        {
            return;
        }
        mMediaCodec.stop();
        mMediaCodec.release();
        mMediaCodec=null;
    }
    public  void  offerEncoder(byte[] data,int nLen)
    {
        if(mMediaCodec==null)
        {
             return;
        }
        ByteBuffer[] inputBuffers = mMediaCodec.getInputBuffers();//拿到输入缓冲区,用于传送数据进行编码
        ByteBuffer[] outputBuffers = mMediaCodec.getOutputBuffers();//拿到输出缓冲区,用于取到编码后的数据
        int inputBufferIndex = mMediaCodec.dequeueInputBuffer(5000);
        if (inputBufferIndex >= 0) {//当输入缓冲区有效时,就是>=0

            int  ppp = (pts * 1000000 / fps);
            pts++;

            ByteBuffer inputBuffer = inputBuffers[inputBufferIndex];
            inputBuffer.clear();
            inputBuffer.put(data);//往输入缓冲区写入数据,
            ////五个参数，第一个是输入缓冲区的索引，第二个数据是输入缓冲区起始索引，第三个是放入的数据大小，第四个是时间戳，保证递增就是
            mMediaCodec.queueInputBuffer(inputBufferIndex, 0, data.length, ppp, 0);
            MediaCodec.BufferInfo bufferInfo = new MediaCodec.BufferInfo();
            int outputBufferIndex = mMediaCodec.dequeueOutputBuffer(bufferInfo, 2000);//拿到输出缓冲区的索引
            while (outputBufferIndex >= 0) {
                ByteBuffer outputBuffer = outputBuffers[outputBufferIndex];
                byte[] outData = new byte[bufferInfo.size];
                outputBuffer.get(outData);
                //outData就是输出的h264数据
                //outputStream.write(outData, 0, outData.length);//将输出的h264数据保存为文件，用vlc就可以播放
                boolean bKeyframe = false;

                if(bufferInfo.flags==2)
                {
                    wifination.naSave2FrameMp4(outData, bufferInfo.size, 0, bKeyframe);
                }
                else if(bufferInfo.flags==1)
                {
                    bKeyframe = true;
                    wifination.naSave2FrameMp4(outData, bufferInfo.size, 1, bKeyframe);
                }
                else
                {
                    wifination.naSave2FrameMp4(outData, bufferInfo.size, 1, bKeyframe);
                }

                mMediaCodec.releaseOutputBuffer(outputBufferIndex, false);
                //outputBufferIndex = mMediaCodec.dequeueOutputBuffer(bufferInfo, 2000);
                break;
            }
        }

    }
}
