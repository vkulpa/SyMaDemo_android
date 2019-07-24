package com.joyhonest.wifination;

import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaCodecList;
import android.media.MediaFormat;
import android.util.Log;

import java.io.IOException;
import java.nio.ByteBuffer;


public class VideoMediaCoder {
    long pts;
    int fps;
    private MediaCodec mMediaCodec;

    private final  static String VCODEC="video/avc";
    private final  static String TAG="MediaCoder";

    private  boolean  bGetPPS=false;


    public VideoMediaCoder() {
        mMediaCodec=null;
    }

    private  MediaFormat F_GetMediaFormat(int width,int height,int bitrate,int fps,int color)
    {


        MediaFormat mediaFormat = MediaFormat.createVideoFormat(VCODEC, width,height); //height和width一般都是照相机的height和width。

        //描述平均位速率（以位/秒为单位）的键。 关联的值是一个整数
        mediaFormat.setInteger(MediaFormat.KEY_WIDTH, width);
        mediaFormat.setInteger(MediaFormat.KEY_HEIGHT, height);
        mediaFormat.setInteger(MediaFormat.KEY_BIT_RATE, bitrate);
        //描述视频格式的帧速率（以帧/秒为单位）的键。
        mediaFormat.setInteger(MediaFormat.KEY_FRAME_RATE, fps);//帧率，一般在15至30之内，太小容易造成视频卡顿。
        mediaFormat.setInteger(MediaFormat.KEY_COLOR_FORMAT, color);//色彩格式，具体查看相关API，不同设备支持的色彩格式不尽相同
        //关键帧间隔时间，单位是秒
        mediaFormat.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, 2);

        try {
            mMediaCodec.configure(mediaFormat, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
        }
        catch (Exception e)
        {
            e.printStackTrace();
            mediaFormat = null;
        }
        return mediaFormat;

    }



    public int initMediaCodec(int width,int height,int bitrate,int fps) {

        //chooseVideoEncoder();

        if (mMediaCodec != null) {
            mMediaCodec.stop();
            mMediaCodec.release();
            mMediaCodec = null;
            pts = 0;
        }
        bGetPPS = false;
        pts = 0;
        this.fps = fps;
        boolean bOK = true;
        try {
            mMediaCodec = MediaCodec.createEncoderByType(VCODEC);
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
            mMediaCodec = null;
            return 0x7FFFF;
        }



        int nColor;

        nColor = MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420SemiPlanar;
        if(F_GetMediaFormat(width,height,bitrate,fps,nColor) ==null)
        {

            //if(F_ReCreateMediaCoder()<0)
            //    return 0x7FFFF;


            if (mMediaCodec != null) {
                mMediaCodec.stop();
                mMediaCodec.release();
                mMediaCodec = null;
                pts = 0;
            }
            bGetPPS = false;
            pts = 0;
            this.fps = fps;
            bOK = true;
            try {
                mMediaCodec = MediaCodec.createEncoderByType(VCODEC);
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
                mMediaCodec = null;
                return 0x7FFFF;
            }
            nColor = MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420Planar;


            if(F_GetMediaFormat(width,height,bitrate,fps,nColor) ==null)
            {
                nColor = 0;
            }
        }
        if(nColor!=0)
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
        bGetPPS = false;
    }

    int  ddd=0;
    public  void  offerEncoder(byte[] data,int nLen)
    {

        if(mMediaCodec==null)
        {
             return;
        }
        //Log.e("abc","sent 0");
        ByteBuffer[] inputBuffers = mMediaCodec.getInputBuffers();//拿到输入缓冲区,用于传送数据进行编码
        ByteBuffer[] outputBuffers = mMediaCodec.getOutputBuffers();//拿到输出缓冲区,用于取到编码后的数据
        int inputBufferIndex = mMediaCodec.dequeueInputBuffer(5000);
        if (inputBufferIndex >= 0) {//当输入缓冲区有效时,就是>=0

            long  ppp = (pts * (1000000 / fps));
            pts++;
            ByteBuffer inputBuffer = inputBuffers[inputBufferIndex];
            inputBuffer.clear();
            inputBuffer.put(data);//往输入缓冲区写入数据,
            ////五个参数，第一个是输入缓冲区的索引，第二个数据是输入缓冲区起始索引，第三个是放入的数据大小，第四个是时间戳，保证递增就是
            mMediaCodec.queueInputBuffer(inputBufferIndex, 0, data.length, ppp, 0);
            MediaCodec.BufferInfo bufferInfo = new MediaCodec.BufferInfo();
            int outputBufferIndex = mMediaCodec.dequeueOutputBuffer(bufferInfo, 2000);//拿到输出缓冲区的索引
            if (outputBufferIndex >= 0)
            {
                ByteBuffer outputBuffer = outputBuffers[outputBufferIndex];
                byte[] outData = new byte[bufferInfo.size];
                outputBuffer.get(outData);
                //outData就是输出的h264数据
                //outputStream.write(outData, 0, outData.length);//将输出的h264数据保存为文件，用vlc就可以播放
                boolean bKeyframe = false;
                if(bufferInfo.flags==2)
                {
                    if(!bGetPPS) {
                        bGetPPS = true;
                        wifination.naSave2FrameMp4(outData, bufferInfo.size, 0, bKeyframe);
                        //Log.e(TAG,"SPS PPS");
                    }
                }
                else if(bufferInfo.flags==1)
                {
                    bKeyframe = true;
                    wifination.naSave2FrameMp4(outData, bufferInfo.size, 1, bKeyframe);
                    //Log.e(TAG,"KeyFrame");
                }
                else
                {
                    wifination.naSave2FrameMp4(outData, bufferInfo.size, 1, bKeyframe);
                    //Log.e(TAG,"noKeyFrame");

                }

                mMediaCodec.releaseOutputBuffer(outputBufferIndex, false);
                //outputBufferIndex = mMediaCodec.dequeueOutputBuffer(bufferInfo, 2000);

                //break;
            }

        }

    }


    private MediaCodecInfo chooseVideoEncoder(String name, MediaCodecInfo def) {
        int nbCodecs = MediaCodecList.getCodecCount();
        for (int i = 0; i < nbCodecs; i++) {
            MediaCodecInfo mci = MediaCodecList.getCodecInfoAt(i);
            if (!mci.isEncoder()) {
                continue;
            }
            String[] types = mci.getSupportedTypes();
            for (int j = 0; j < types.length; j++) {
                if (types[j].equalsIgnoreCase(VCODEC)) {
                    //Log.i(TAG, String.format("vencoder %s types: %s", mci.getName(), types[j]));
                    if (name == null) {
                        return mci;
                    }

                    if (mci.getName().contains(name)) {
                        return mci;
                    }
                }
            }
        }
        return def;
    }

    private int chooseVideoEncoder() {
        // choose the encoder "video/avc":
        //      1. select one when type matched.
        //      2. perfer google avc.
        //      3. perfer qcom avc.
        MediaCodecInfo vmci = chooseVideoEncoder(null, null);
        //vmci = chooseVideoEncoder("google", vmci);
        //vmci = chooseVideoEncoder("qcom", vmci);

        int matchedColorFormat = 0;
        MediaCodecInfo.CodecCapabilities cc = vmci.getCapabilitiesForType(VCODEC);
        for (int i = 0; i < cc.colorFormats.length; i++) {
            int cf = cc.colorFormats[i];
            Log.i(TAG, String.format("vencoder %s supports color fomart 0x%x(%d)", vmci.getName(), cf, cf));

            // choose YUV for h.264, prefer the bigger one.
            // corresponding to the color space transform in onPreviewFrame
            if ((cf >= cc.COLOR_FormatYUV420Planar && cf <= cc.COLOR_FormatYUV420SemiPlanar)) {
                if (cf > matchedColorFormat) {
                    matchedColorFormat = cf;
                }
            }
        }
        for (int i = 0; i < cc.profileLevels.length; i++) {
            MediaCodecInfo.CodecProfileLevel pl = cc.profileLevels[i];
            Log.i(TAG, String.format("vencoder %s support profile %d, level %d", vmci.getName(), pl.profile, pl.level));
        }
        Log.i(TAG, String.format("vencoder %s choose color format 0x%x(%d)", vmci.getName(), matchedColorFormat, matchedColorFormat));
        return matchedColorFormat;
    }



}
