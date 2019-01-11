package com.joyhonest.wifination;


import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Matrix;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.support.v4.app.NavUtils;
import android.util.Log;

import org.simple.eventbus.EventBus;

import java.nio.ByteBuffer;


/**
 * Created by aivenlau on 16/7/13.
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


    public static AudioEncoder AudioEncoder;

    public final static int TYPE_ONLY_PHONE = 0;
    public final static int TYPE_ONLY_SD = 1;
    public final static int TYPE_BOTH_PHONE_SD = 3;
    public final static int TYPE_PHOTOS = 0;
    public final static int TYPE_VIDEOS = 1;
    public static ByteBuffer mDirectBuffer;
    //public static ByteBuffer mDirectBufferYUV;
    public static boolean bDisping = false;


    private  static VideoMediaCoder  videoMediaCoder;

    //private static  Context  mContext=null;

    private final static String TAG = "wifination";
    private static final wifination m_Instance = new wifination();
    private static final int BMP_Len = (((1280 + 3) / 4) * 4) * 4 * 720 + 1024;

    public static Context appContext = null;

    static {
        try {
            System.loadLibrary("jh_wifi");
            AudioEncoder = new AudioEncoder();
            videoMediaCoder = new VideoMediaCoder();
        } catch (UnsatisfiedLinkError Ule) {
            Log.e(TAG, "Cannot load jh_wifi.so ...");
            Ule.printStackTrace();
        } finally {

            mDirectBuffer = ByteBuffer.allocateDirect(BMP_Len + 200);     //获取每帧数据，主要根据实际情况，分配足够的空间。
            naSetDirectBuffer(mDirectBuffer, BMP_Len + 200);
        }
    }

    private wifination() {

    }

    //静态工厂方法
    public static wifination getInstance() {
        return m_Instance;
    }
    private static native void naSetDirectBuffer(Object buffer, int nLen);
    private static native void naSetDirectBufferYUV(Object buffer, int nLen);


    ////// ------------- -------------------------



    //初始化，开始接受wifi数据，显示图像
    /*
        IC_GKA：  sPath=@“1”  720P   sPath=@“2” VGA
        IC_GP：       sPath=@"http://192.168.25.1:8080/?action=stream"
        IC_GPRTSP     sPath = @"rtsp://192.168.26.1:8080/?action=stream"
        其他模块：      sPath=@“”;
    */
    public static native int naInit(String pFileName);

    //停止播放
    public static native int naStop();
    //向飞控发送命令
    public static native int naSentCmd(byte[] cmd, int nLen);

    //图像是否翻转
    public static native void naSetFlip(boolean b);
    // 是否VR显示
    public static native void naSet3D(boolean b);


    //TYPE_ONLY_PHONE   ==  录像或者拍照到手机
    //TYPE_ONLY_SD     ==  录像或者拍照到模块的SD卡（目前只对应GKA模块有效)
    // TYPE_BOTH_PHONE_SD  ==  录像或者拍照同时到模块的SD卡和手机
    //
    //拍照
    public static native int naSnapPhoto(String pFileName, int PhoneOrSD);
    //录像
    public static native int naStartRecord(String pFileName, int PhoneOrSD);
    // 获取录像时间 ms
    public static native int naGetRecordTime();
    //停止录像
    public static native void naStopRecord(int PhoneOrSD);
    //停止所有录像
    public static native int naStopRecord_All();
    //录像到手机时，是否录音
    public static native void naSetRecordAudio(boolean b);


    //手机是否在录像
    public static native boolean isPhoneRecording();
    //设定录像的分辨率，一般无需设定，默认位模块传回视频分辨率
    public static native int naSetRecordWH(int ww, int hh);

    //设定是否需要SDK内部来显示，b = true， SDK 把解码到的图像发送到JAVA，由APP自己来显示而不是通过SDK内部来渲染显示

    // SDK解码后图像 由 ReceiveBmp 返回

    public static void naSetRevBmp(boolean b)
    {
        bRevBmp = b;
        naSetRevBmpA(b);
    }
    public static void naSetGesture(boolean b,Context appContext)
    {
        bGesture = b;
        if(bGesture)
        {
            if(sig==null)
            {
                sig = ObjectDetector.getInstance();
                sig.SetAppCentext(appContext);
            }
        }
        if(sig!=null)
        {
            sig.F_Start(bGesture);
        }
        naSetGestureA(b);
    }


    private static native void naSetRevBmpA(boolean b);

    //设定是否手势识别， True，每一帧也会由 ReceiveBmp 返回，不同的是 SDK内部还是会显示视频。 如果APP 自己来实现手势识别和显示，
    // 可以用 naSetRevBmp 来替代
    private static native void naSetGestureA(boolean b);


    //设定 客户 只针对 GKA， “sima” 表示 客户是司马 ，目前只有这一个设定
    public static native void naSetCustomer(String sCustomer);
    //获取SD卡列表 (针对  IC_GK_A 以下只对 IC_GKA
    public static native int naGetPhotoDir();
    public static native int naGetVideoDir();
    public static native int naGetFiles(int nType);
    public static native int naDownloadFile(String sPath, String dPath);
    public static native int naCancelDownload();
    public static native int naDeleteSDFile(String fileName);




    //获取SD卡中视频的的缩略图(针对  IC_GKA),一般建议如果已经下载到手机的视频文件,利用系统函数来获取缩略图,本函数主要是用于获取没有下载到手机
//的SD卡中的视频文件缩略图,调用次函数后,SDK会回调 GetThumb(byte[] data,String sFilename), data 是缩略图数据,filename是表明是哪个视频文件
//一般,我们在调用naGetVideoDir()时, 在回调函数GetFiles(byte[] filesname)得到文件名,在调用此函数来获取缩略图
    public static native int naGetThumb(String filename);
    public static native int naCancelGetThumb();


    public static native  void naSetDispStyle(int nType); //0-6

    ///旧接口,不建议用....

    public static  native void naSetDislplayData(byte[]data,int width,int height);

    public static native int naPlay();
    public static native int naStartCheckSDStatus(boolean bStart);
    public static native void naSetIcType(int nICType);
    public static native void naSet3DA(boolean b);
    public static native int naSetGPFps(int nFps);
    public static native int naGkASetRecordResolution(boolean b20P);
    public static native int naGetSessionId();
    public static native void naSetGKA_SentCmdByUDP(boolean bUdp);
    //GP_RTSP
    //获取模块类型
    public static native int naGetGP_RTSP_Status();

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


    public static native void naFillFlyCmdByC(int nType);

    public static native int naSave2FrameMp4(byte[] data, int nLen, int tyoe, boolean bKeyframe);

    public static native int naGetFps();

    public static native int naGetwifiFps();

    public static native String naGetControlType(); //获取飞控型号，主要针对 SYMA  国科

    public static native boolean naSetBackground(byte[] data, int width, int height);

    public static native void naSetAdjFps(boolean b); //对应国科IC，有些早期固件不支持调整FPS，所以需要增加这一条命令




    public static native void naSetVrBackground(boolean b);

    public static native void naRotation(int n);  // n == 0 || n ==90 || n ==-90 || n ==180 || n==270
    public static  native void naSetbRotaHV(boolean b); //b = flase  表示手机是竖屏显示，但因为我们的camera是横屏数据，所以还需调用 naRotation 来转 90度满屏显示
                                                        //b = true,  手机横屏显示，此时如果调用 naRotation， 就只是把 显示画面旋转 ，如果转 90 ，-90 270 ，就会显示有 黑边
    public static native boolean naSetWifiPassword(String sPassword);

    public static native void naSetLedOnOff(boolean bOpenLed);


    public static native void naSetScal(float fScal); //设定放大显示倍数

    public static void naSetCmdResType(int nType)
    {
        JH_Tools.F_SetResType(nType);
    }


    public static native void naSetNoTimeOut(boolean b);    //



    public static native void naSetDebug(boolean b);//串口数据 ，用于与固件调试

    public static native void naWriteport20000(byte[] cmd,int nleng);

    public static native  void naSetMirror(boolean b);



    public static native void init();

    public static native void release();

    public static native void changeLayout(int width, int height);

    public static native void drawFrame();


    public  static boolean  bGesture = false;
    public  static boolean  bRevBmp = false;


    private static ObjectDetector sig=null;






    private static void G_StartAudio(int b) {
        if (b != 0) {
            AudioEncoder.start();
        } else {
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
        if (imageWidth <= 640 && imageHeight <= 480) {
            bmp = BitmapFactory.decodeResource(context.getResources(), bakid);
        } else {
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

        //int ww = bmp.getWidth();
        //int hh = bmp.getHeight();
        //if (ww > 1280 || hh > 720)
        {

            int width = bmp.getWidth();
            int height =bmp.getHeight();
            int newWidth = ((width+7)/8)*8;
            int newHeight = ((height+7)/8)*8;

            Bitmap croppedBitmap = Bitmap.createBitmap(newWidth, newHeight, Bitmap.Config.ARGB_8888);
            Canvas canvas = new Canvas(croppedBitmap);
            Matrix frameToCropTransform;



                frameToCropTransform =
                        ImageUtils.getTransformationMatrix(
                                width, height,
                                newWidth, newHeight,
                                0, false);

                Matrix cropToFrameTransform = new Matrix();
                frameToCropTransform.invert(cropToFrameTransform);
                canvas.drawBitmap(bmp, frameToCropTransform, null);

                /*
            //获得图片的宽高
            int width = bmp.getWidth();
            int height = bmp.getHeight();
            // 设置想要的大小
            int newWidth = ((width+7)/8)*8;
            int newHeight = ((height+7)/8)*8;


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
            */
                bmp.recycle();
                bmp = croppedBitmap;
        }

        int ww = bmp.getWidth();
        int hh = bmp.getHeight();

        int bytes = bmp.getByteCount();
        ByteBuffer buf = ByteBuffer.allocate(bytes);
        bmp.copyPixelsToBuffer(buf);
        byte[] byteArray = buf.array();
        naSetBackground(byteArray, ww, hh);
        bmp.recycle();
    }


    private static int getIP()
    {
        if(appContext!=null)
        {

            WifiManager wm=(WifiManager) appContext.getSystemService(Context.WIFI_SERVICE);
            if(wm!=null)
            {
                WifiInfo wi=wm.getConnectionInfo();
                if(wi!=null)
                {
                    int ipAdd=wi.getIpAddress();
                    return ipAdd;
                }
            }
            return 0;
        }
        return 0;
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


    private static void OnGetGP_Status(int nStatus) {
        if(((nStatus & 0xFFFFFF00) == 0xAABBCC00))   //所有通过串口传过来的数据 ，用于与固件调试时使用
        {
            int nLen = (nStatus & 0xFF);
            if (nLen > 200)
                nLen = 200;

            byte[] cmd = new byte[nLen];

            ByteBuffer buf = wifination.mDirectBuffer;
            buf.rewind();
            for (int i = 0; i < nLen; i++) {
                cmd[i] = buf.get(i + BMP_Len);
            }
            EventBus.getDefault().post(cmd, "GetDataFromRs232");
        }
        else if ((nStatus & 0xFFFFFF00) == 0x55AA5500)  // wifi模块透传回来的数据
        {
            int nLen = (nStatus & 0xFF);
            if (nLen > 200)
                nLen = 200;

            byte[] cmd = new byte[nLen];

            ByteBuffer buf = wifination.mDirectBuffer;
            buf.rewind();
            for (int i = 0; i < nLen; i++) {
                cmd[i] = buf.get(i + BMP_Len);
            }
            EventBus.getDefault().post(cmd, "GetWifiSendData");
        }
        else if ((nStatus & 0xFFFFFF00) == 0xAA55AA00)    //GP RTPB  回传 模块本身信息数据
        {
            int nLen = (nStatus & 0xFF);
            if (nLen > 200)
                nLen = 200;
            byte[] cmd = new byte[nLen];
            ByteBuffer buf = wifination.mDirectBuffer;
            buf.rewind();
            for (int i = 0; i < nLen; i++) {
                cmd[i] = buf.get(i + BMP_Len);
            }
            EventBus.getDefault().post(cmd, "GetWifiInfoData");
        }
        else if ((nStatus & 0xFFFFFF00) == 0xAA555500)    //GP 回传电量
        {
            int nBattery = nStatus &0x0F;
            Integer nB = nBattery;
            EventBus.getDefault().post(nB, "OnGetBatteryLevel");
        }
        else if ((nStatus & 0xFFFFFF00) == 0x11223300)    //回传电量显示nStyle
        {
            int nStyle = nStatus &0x0F;
            Integer nB = nStyle;
            EventBus.getDefault().post(nB, "OnGetSetStyle");
        }

        else {
            Integer ix = nStatus;                //返回 模块按键
            Log.e(TAG, "Get data = " + nStatus);
            EventBus.getDefault().post(ix, "OnGetGP_Status");
        }
    }

    //// 测试信息。。。。。
    private static void RevTestInfo(byte[] info) {


    }

    // IC_GKA  获取SD卡文件列表回调函数
    private static void GetFiles(byte[] filesname) {
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
    private static void OnKeyPress(int nStatus) {
        Integer n = nStatus;
        EventBus.getDefault().post(n, "key_Press");
        EventBus.getDefault().post(n, "Key_Pressed");
    }



    // 获取一帧图像
    private static void ReceiveBmp(int i) {
        //其中，i:bit00-bit15   为图像宽度
        //      i:bit16-bit31  为图像高度
        // 此函数需要把数据尽快处理和保存。
        // 图像数据保存在mDirectBuffer中，格式为ARGB_8888

        Bitmap bmp = Bitmap.createBitmap(i & 0xFFFF, (i & 0xFFFF0000) >> 16, Bitmap.Config.ARGB_8888);
        ByteBuffer buf = wifination.mDirectBuffer;
        buf.rewind();
        bmp.copyPixelsFromBuffer(buf);    //
        if(bRevBmp)
            EventBus.getDefault().post(bmp, "ReviceBMP");
        if(bGesture)
        {
            if(sig!=null)
                sig.GetNumber(bmp);
        }

    }


    ///////////video Media
    private  static int F_InitEncoder(int width,int height,int bitrate,int fps)
    {
        return videoMediaCoder.initMediaCodec(width,height,bitrate,fps);
    }

    private  static  void offerEncoder(byte[] data,int nLen)
    {
         if(videoMediaCoder!=null)
         {
             videoMediaCoder.offerEncoder(data,nLen);
         }
    }

    private  static void F_CloseEncoder()
    {
        if(videoMediaCoder!=null)
        {
            videoMediaCoder.F_CloseEncoder();
        }
    }


}
