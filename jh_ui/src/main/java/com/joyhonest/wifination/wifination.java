package com.joyhonest.wifination;


import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Matrix;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.util.Log;




import org.simple.eventbus.EventBus;


import java.nio.ByteBuffer;

import static android.content.Context.WIFI_SERVICE;


/**
 * Created by aivenlau on 16/7/13.
 *
 */


public class wifination {

    public final static int IC_NO = -1;
    public final static int IC_GK = 0;
    public final static int IC_GP = 1;
    public final static int IC_SN = 2;
    public final static int IC_GKA = 3;
    public final static int IC_GPRTSP = 4;
    public final static int IC_GPH264 = 5;
    public final static int IC_GPRTP = 6;
    public final static int IC_GPH264A = 7;
    public final static int IC_GPRTPB = 8;
    public final static int IC_GK_UDP = 9;


    public  static  AudioEncoder  AudioEncoder;

    public final static int TYPE_ONLY_PHONE = 0;
    public final static int TYPE_ONLY_SD = 1;
    public final static int TYPE_BOTH_PHONE_SD = 3;
    public final static int TYPE_PHOTOS = 0;
    public final static int TYPE_VIDEOS = 1;
    public static ByteBuffer mDirectBuffer;
    //public static ByteBuffer mDirectBufferYUV;
    public static boolean bDisping = false;


    private final static String TAG = "wifination";
    private static final wifination m_Instance = new wifination();
    private static final int BMP_Len = (((1280 + 3) / 4) * 4) * 4 * 720 + 1024;

    public  static Context  appContext=null;

    static {
        try {
            System.loadLibrary("jh_wifi");
            AudioEncoder = new AudioEncoder();
        } catch (UnsatisfiedLinkError Ule) {
            Log.e(TAG, "Cannot load jh_wifi.so ...");
            Ule.printStackTrace();
        } finally {

            mDirectBuffer = ByteBuffer.allocateDirect(BMP_Len + 50);     //获取每帧数据，主要根据实际情况，分配足够的空间。
            naSetDirectBuffer(mDirectBuffer, BMP_Len + 50);
        }
    }

    private wifination()
    {

    }

    //静态工厂方法
    public static wifination getInstance() {
        return m_Instance;
    }

    private static native void naSetDirectBuffer(Object buffer, int nLen);

    private static native void naSetDirectBufferYUV(Object buffer, int nLen);

    //Display
    public static native int naInit(String pFileName);

    public static native int naPlay();

    public static native int naStop();

    public static native int naSentCmd(byte[] cmd, int nLen);

    public static native int naStartCheckSDStatus(boolean bStart);

    public static native void naSetIcType(int nICType);

    public static native void naSetFlip(boolean b);

    public static native void naSet3D(boolean b);

    public static native void naSet3DA(boolean b);

    //public static native boolean naSetVideoSurface(Object surface);

    public static native void naSetCustomer(String sCustomer);

    //获取SD卡列表 (针对  IC_GK)A
    public static native int naGetPhotoDir();

    public static native int naGetVideoDir();

    public static native int naGetFiles(int nType);

    public static native int naDownloadFile(String sPath, String dPath);

    public static native int naCancelDownload();

    public static native int naDeleteSDFile(String fileName);

    public static native int naSetGPFps(int nFps);


    //获取SD卡中视频的的缩略图(针对  IC_GKA),一般建议如果已经下载到手机的视频文件,利用系统函数来获取缩略图,本函数主要是用于获取没有下载到手机
//的SD卡中的视频文件缩略图,调用次函数后,SDK会回调 GetThumb(byte[] data,String sFilename), data 是缩略图数据,filename是表明是哪个视频文件
//一般,我们在调用naGetVideoDir()时, 在回调函数GetFiles(byte[] filesname)得到文件名,在调用此函数来获取缩略图

    public static native int naGetThumb(String filename);

    public static native int naCancelGetThumb();

    //拍照和录像
    public static native int naSnapPhoto(String pFileName, int PhoneOrSD);

    public static native int naStartRecord(String pFileName, int PhoneOrSD);

    public static native int naGetRecordTime();

    public static native void naStopRecord(int PhoneOrSD);

    public static native int naStopRecord_All();

    public static native int naGkASetRecordResolution(boolean b20P);


    public static native int naGetSessionId();

    public static native boolean isPhoneRecording();

    public static native void naSetGKA_SentCmdByUDP(boolean bUdp);


    //GP_RTSP

    public static native int naGetGP_RTSP_Status();

    ///旧接口,不建议用....
    public static native void naSetdispRect(int w, int h);

    public static native int naRemoteSnapshot();

    public static native int naRemoteSaveVideo();

    //public static native void naSN_WriteFrame(byte[] data, int nLen);

    public static native int naGetSettings();

    public static native boolean naCheckDevice();

    public static native int naSaveSnapshot(String pFileName);

    public static native int naSaveVideo(String pFileName);

    public static native int naStopSaveVideo();

    public static native int naStatus();

    //跟随
    public static native void naSetFollow(boolean bFollow);

    public static native void naSetContinue();

    //Sunbplus
    public static native int naSetMenuFilelanguage(int nLanguage);

    public static native int naSetRecordWH(int ww, int hh);

    public static native void naFillFlyCmdByC(int nType);

    public static native int naSave2FrameMp4(byte[] data, int nLen, int tyoe, boolean bKeyframe);

    public static native int naGetFps();

    public static native int naGetwifiFps();

    public static native String naGetControlType(); //获取飞控型号，主要针对 SYMA  国科

    public static native boolean naSetBackground(byte[] data, int width, int height);

    public static native void naSetAdjFps(boolean b); //对应国科IC，有些早期固件不支持调整FPS，所以需要增加这一条命令


    public static native  void naSetRevBmp(boolean b); //是否把解码到的图像发送到JAVA，有APP自己来显示而不是通过SDK内部来渲染显示

    public static native  void naSetVrBackground(boolean b);
    public static native  void naRotation(int n);  //N = 0  90    -90   //画面转90 度 显示


    public  static  native  boolean naSetWifiPassword(String sPassword);


    public  static native  void naSetScal(float fScal); //设定放大显示倍数


    public static native  void naSetRecordAudio(boolean b);






    public static native void init();
    public static native void release();
    public static native void changeLayout(int width, int height);
    public static native void drawFrame();


    private static void G_StartAudio(int b)
    {
        if(b!=0)
        {
            AudioEncoder.start();
        }
        else
        {
            AudioEncoder.stop();
        }
    }


/*
    private static String intToIp(int i) {
        return (i & 0xFF) + "." + ((i >> 8) & 0xFF) + "." + ((i >> 16) & 0xFF) + "." + ((i >> 24) & 0xFF);
    }

    private static int G_getIP()
    {
          if(appContext==null)
            return IC_NO;
        WifiManager wifi_service = (WifiManager) appContext.getSystemService(WIFI_SERVICE);

        WifiInfo info = wifi_service.getConnectionInfo();

        String wifiId;
        wifiId = (info != null ? info.getSSID() : null);
        if (wifiId != null) {
            wifiId = wifiId.replace("\"", "");
            if (wifiId.length() > 4)
                wifiId = wifiId.substring(wifiId.length() - 4);
        } else {
            wifiId = "nowifi";
        }

        int ip = info.getIpAddress();
        return ip;

    }
    */

    public static void F_AdjBackGround(Context context, int bakid) {
        Bitmap bmp = null;
        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inJustDecodeBounds = true;
        BitmapFactory.decodeResource(context.getResources(), bakid, options);
        int imageHeight = options.outHeight;
        int imageWidth = options.outWidth;
        if (imageWidth <= 640 && imageHeight <= 480)
        {
            bmp = BitmapFactory.decodeResource(context.getResources(), bakid);
        }
        else
        {
            int scale = imageWidth / 640;
            if (scale <= 0) {
                scale = 2;
            }
            options.inSampleSize = scale;
            options.inJustDecodeBounds = false;
            bmp = BitmapFactory.decodeResource(context.getResources(), bakid, options);
        }

        if (bmp == null)
            return;

        int ww = bmp.getWidth();
        int hh = bmp.getHeight();
        if (ww > 1280 || hh > 720)
        {
            //获得图片的宽高
            int width = bmp.getWidth();
            int height = bmp.getHeight();
            // 设置想要的大小
            int newWidth = 1280;
            int newHeight = 720;
            // 计算缩放比例
            float scaleWidth = ((float) newWidth) / width;
            float scaleHeight = ((float) newHeight) / height;
            // 取得想要缩放的matrix参数
            Matrix matrix = new Matrix();
            matrix.postScale(scaleWidth, scaleHeight);
            // 得到新的图片
            Bitmap newbm = Bitmap.createBitmap(bmp, 0, 0, width, height, matrix,
                    true);
            bmp.recycle();
            bmp = newbm;
        }
        ww = bmp.getWidth();
        hh = bmp.getHeight();
        int bytes = bmp.getByteCount();
        ByteBuffer buf = ByteBuffer.allocate(bytes);
        bmp.copyPixelsToBuffer(buf);
        byte[] byteArray = buf.array();
        naSetBackground(byteArray, ww, hh);
        bmp.recycle();
    }


    public static void naInitgl(Context context, int backid) {
        init();
    }



    private static void OnSave2ToGallery(String sName, int nPhoto)     //拍照或者录像完成。可以把它加入到系统图库中去
    {
        String Sn = String.format("%02d%s", nPhoto, sName);
        EventBus.getDefault().post(Sn, "SavePhotoOK");
    }


    private static void OnGetWifiData(byte[] data)        //GKA  返回 模块透传数据
    {
        JH_Tools.AdjData(data);
        JH_Tools.FindCmd();
        JH_Tools.F_ClearData();
    }


    private static void OnGetGP_Status(int nStatus)
    {
        if ((nStatus&0xFFFFFF00) == 0x55AA5500)  // wifi模块透传回来的数据
        {
            //String s = "";
            int nLen = (nStatus & 0xFF);
            if(nLen>50)
                nLen=50;

            byte[] cmd = new byte[nLen];

            ByteBuffer buf = wifination.mDirectBuffer;
            buf.rewind();
            for (int i = 0; i < nLen; i++) {
                cmd[i] = buf.get(i + BMP_Len);
            }
            EventBus.getDefault().post(cmd, "GetWifiSendData");
        }else if ((nStatus&0xFFFFFF00) == 0xAA55AA00)    //GP RTPB  回传 模块信息数据
        {
            int nLen = (nStatus & 0xFF);
            if(nLen>50)
                nLen=50;
            byte[] cmd = new byte[nLen];
            ByteBuffer buf = wifination.mDirectBuffer;
            buf.rewind();
            for (int i = 0; i < nLen; i++) {
                cmd[i] = buf.get(i + BMP_Len);
            }
            EventBus.getDefault().post(cmd, "GetWifiInfoData");
        }
        else {
            Integer ix = nStatus;                //返回 模块按键
            Log.e(TAG,"Get data = "+nStatus);
            EventBus.getDefault().post(ix, "OnGetGP_Status");
        }
    }

    //// 测试信息。。。。。
    private static void RevTestInfo(byte[] info) {


    }

    // IC_GKA  获取SD卡文件列表回调函数
    private static void GetFiles(byte[] filesname)
    {
        String s1 = null;
        s1 = new String(filesname);
        EventBus.getDefault().post(s1, "GetFiles");      //调用第三方库来发消息。
    }


    //  当模块状态改变时回调函数
    private static void OnStatusChamnge(int nStatus) {
        Integer n = nStatus;
        EventBus.getDefault().post(n, "SDStatus_Changed");      //调用第三方库来发送消图片显示消息。

        //#define  bit0_OnLine            1
        //#define  bit1_LocalRecording    2
        //#define  SD_Ready               4
        //#define  SD_Recroding           8
        //#define  SD_Photo               0x10
    }


    //下载文件回调 nError =1 表示有错误。
    private static void DownloadFile_callback(int nPercentage, String sFileName, int nError) {
        if (nError == 0) {
            Log.e("downloading", "downloading  " + nPercentage + "%     " + sFileName);
        }
        jh_dowload_callback jh_dowload_callback = new jh_dowload_callback(nPercentage, sFileName, nError);
        EventBus.getDefault().post(jh_dowload_callback, "DownloadFile");
    }

    //获取缩略图 回调函数 data，160*90 的图像数据
    private static void GetThumb(byte[] data, String sFilename) {
        if (data != null) {
            MyThumb thumb = new MyThumb(data, sFilename);
            EventBus.getDefault().post(thumb, "GetThumb");      //调用第三方库来发送消息。
        }
    }

    /////// 以下 SYMA 不使用 --------
    private static void OnKeyPress(int nStatus)
    {
        Integer n = nStatus;
        Log.e(TAG,"Get Key = "+nStatus);
        EventBus.getDefault().post(n, "key_Press");
        EventBus.getDefault().post(n, "Key_Pressed");
    }


    // 获取一帧图像
    private static void ReceiveBmp(int i) {
        //其中，i:bit00-bit15   为图像宽度
        //      i:bit16-bit31  为图像高度
        // 图像数据保存在mDirectBuffer中，格式为ARGB_8888
        Bitmap bmp = Bitmap.createBitmap(i&0xFFFF,(i&0xFFFF0000)>>16,Bitmap.Config.ARGB_8888);
        ByteBuffer buf = wifination.mDirectBuffer;
        buf.rewind();
        bmp.copyPixelsFromBuffer(buf);    //
        //Integer iw=i;
        EventBus.getDefault().post(bmp, "ReviceBMP");
    }




}
