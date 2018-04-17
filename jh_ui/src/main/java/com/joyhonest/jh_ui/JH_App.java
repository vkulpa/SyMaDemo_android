package com.joyhonest.jh_ui;

import android.app.Activity;
import android.app.Application;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.res.Resources;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.Matrix;
import android.graphics.Point;
import android.location.Criteria;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.media.AudioAttributes;
import android.media.AudioManager;
import android.media.MediaMetadataRetriever;
import android.media.SoundPool;
import android.net.Uri;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.Build;
import android.os.Bundle;
import android.provider.MediaStore;
import android.provider.Settings;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Gravity;
import android.view.Surface;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Toast;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Collections;
import java.util.List;

import static android.content.Context.WIFI_SERVICE;

//import com.autonavi.ae.utils.NaviUtils;
import com.joyhonest.wifination.wifination;

import org.simple.eventbus.EventBus;


/**
 * Created by aiven on 2017/8/31.
 */

public class JH_App {


    public static boolean bSensor = false;
    public static boolean bAdj = false;
    public static boolean bUp = false;
    public static boolean bDn = false;
    public static boolean bStop = false;
    public static boolean bHeadLess = false;
    public static boolean bHiSpeed = false;
    public static boolean bVR = false;


    public static boolean bGPSMode = false;


    public static int nStyle_ui = 0;
    public static int nStyle_fly = 1;

    public static int nType = nStyle_ui;

    public static boolean bIsSyMa = true;
    public static boolean bInitDispCtrol = true;

    public static final int Status_Connected = 1;
    public static final int LocalRecording = 2;
    public static final int SD_Ready = 4;
    public static final int SD_Recording = 8;
    public static final int SD_SNAP = 0x10;
    public static final int SSID_CHANGED = 0x20;

    public static final int mainFragmeng_Value = 0;
    public static final int select_Fragment_Value = 1;
    public static final int Grid_Fragment_Value = 2;


    public static final int DownLoadNormal = 0;
    public static final int DownLoaded = 1;
    public static final int DownLoading = 2;
    public static final int DownLoaded_NO = 3;
    public static final int DownLoadNeed = 4;

    public static boolean bDownLoading = false;
    public static int nSdStatus = 0;

    public static boolean bPhone_SNAP = false;
    public static boolean bPhone_Video = false;

    public static boolean bNeedStartsasyRecord = false;


    //public static long nRecTime = 0;

    public static boolean bisPathMode = false;


    public static boolean bBrowSD = false;
    public static boolean bBrowPhoto = false;

    public static boolean b720P = false;

    // private static JH_App singleton = null;
    public static String sWifi = "";
    public static int nICType = wifination.IC_SN;

    public static List<MyFilesItem> mSD_PhotosList = null;
    public static List<MyFilesItem> mSD_VideosList = null;

    public static List<String> mLocal_PhotosList = null;
    public static List<String> mLocal_VideosList = null;

    public static List<MyItemData> mGridList = null;
    public static List<Integer> mNeedDownLoadList = null;

    public static List<Integer> mDownloadList = null;


    public static List<String> mDispList = null;



    public static boolean   bFlyDisableAll=true;

    public static String sRemotePhoto = null;
    public static String sRemoteVideo = null;
    public static String sLocalPhoto = null;
    public static String sLocalVideo = null;
    public static boolean bUserLgLib = true;

    private static SoundPool soundPool = null;
    private static int music_photo = -1;
    private static int music_mid = -1;
    private static int music_adj = -1;
    private static int music_btn = -1;

    public static int nOrg;

    public static Context mContext = null;
    public static int nCheckt = 0;

    private static boolean bGpsInited = false;

    private static LocationManager locationManager;  //= (LocationManager) getSystemService(Context.LOCATION_SERVICE);


    private static String sVendor = "SyMa_GO_Data";



    public  static  boolean bFlying = false;

    public static int  nAdjRota = 0x80;
    public static int  nAdjLeftRight=0x80;
    public static int  nAdjForwardBack=0x80;

    public static void F_ReadSaveSetting(boolean bSave)
    {
        if(mContext==null)
            return;
        SharedPreferences settings = mContext.getSharedPreferences("AdjSave", 0);
        if(bSave)
        {
            SharedPreferences.Editor editor = settings.edit();
            editor.putInt("nAdjRota",nAdjRota);
            editor.putInt("nAdjLeftRight",nAdjLeftRight);
            editor.putInt("nAdjForwardBack",nAdjForwardBack);

// 提交本次编辑
            editor.commit();
        }
        else {
            nAdjRota = settings.getInt("nAdjRota", 0x80);
            nAdjLeftRight = settings.getInt("nAdjLeftRight", 0x80);
            nAdjForwardBack = settings.getInt("nAdjForwardBack", 0x80);
        }


    }

    public JH_App(Context context) {

    }


    public static void  F_Clear_not_videoFiles()
    {
        File f = new File(JH_App.sLocalVideo);
        File[] files  = f.listFiles();// 列出所有文件
        if(files == null)
            return;
        String fileName=" ";
        for(File  file : files)
        {
            if(file.exists() && !file.isDirectory())
            {
                fileName = file.getAbsolutePath();
                fileName = fileName.toLowerCase();
                if(!fileName.endsWith(".mp4"))
                {
                    file.delete();
                }
            }
        }
    }




    public static void init(Context contextA, String LocalPhoto, String LocalVideo, String SDPhoto, String SDVideo) {


        if (contextA == null) {
            mContext = null;
            if (soundPool != null) {
                if (music_mid > 0)
                    soundPool.unload(music_mid);
                if (music_adj > 0)
                    soundPool.unload(music_adj);
            }
            return;
        }
        if (mContext != null)
            return;
        mContext = contextA.getApplicationContext();
        F_ReadSaveSetting(false);


        if (mGridList == null) {
            mSD_PhotosList = new ArrayList<MyFilesItem>();
            mSD_VideosList = new ArrayList<MyFilesItem>();
            mLocal_PhotosList = new ArrayList<String>();
            mLocal_VideosList = new ArrayList<String>();
            mDownloadList = new ArrayList<Integer>();
            mDispList = new ArrayList<String>();
            mNeedDownLoadList = new ArrayList<Integer>();
            mGridList = new ArrayList<MyItemData>();
        }
        F_CreateLocalDir(LocalPhoto,LocalVideo,SDPhoto,SDVideo);
        F_InitMusic();
    }

    public static void F_CreateLocalDir(String LocalPhoto, String LocalVideo, String SDPhoto, String SDVideo)
    {
        if (mContext == null)
            return;
        boolean bRemote = true;
        String StroragePath = "";

        if (sVendor.length() != 0)
        {

            File fdir;
            String recDir;
            boolean  bCreateOK=false;

            try {
                StroragePath = Storage.getNormalSDCardPath();
            } catch (Exception e) {
                return;
            }
            if (StroragePath.length() == 0) {
                StroragePath = Storage.getNormalSDCardPath();
            }

            bCreateOK=false;
            if (LocalPhoto != null) {
                fdir = new File(LocalPhoto);
                if (!fdir.exists()) {
                    fdir.mkdirs();
                }
                if (fdir.exists()) {
                    sLocalPhoto = LocalPhoto;
                    bCreateOK = true;
                }
            }
            if(!bCreateOK)
            {
                recDir = String.format("%s/%s/Local/SYMA_Photo_JH", StroragePath, sVendor);
                sLocalPhoto = recDir;
                fdir = new File(recDir);
                if (!fdir.exists()) {
                    fdir.mkdirs();
                }

            }


            bCreateOK=false;
            if (LocalVideo != null) {
                fdir = new File(LocalVideo);
                if (!fdir.exists()) {
                    fdir.mkdirs();
                }
                if (fdir.exists()) {
                    sLocalVideo = LocalVideo;
                    bCreateOK = true;
                }
            }
            if(!bCreateOK)
            {
                recDir = String.format("%s/%s/Local/SYMA_Video_JH", StroragePath, sVendor);
                sLocalVideo = recDir;
                fdir = new File(recDir);
                if (!fdir.exists()) {
                    fdir.mkdirs();
                }
            }

            bCreateOK=false;
            if (SDPhoto != null) {
                fdir = new File(SDPhoto);
                if (!fdir.exists()) {
                    fdir.mkdirs();
                }
                if (fdir.exists()) {
                    sRemotePhoto = SDPhoto;
                    bCreateOK = true;
                }
            }
            if(!bCreateOK)
            {
                recDir = String.format("%s/%s/SD/SYMA_SDPhoto_JH", StroragePath, sVendor);
                sRemotePhoto = recDir;
                fdir = new File(recDir);
                if (!fdir.exists()) {
                    fdir.mkdirs();
                }
            }

            bCreateOK=false;
            if (SDVideo != null) {
                fdir = new File(SDVideo);
                if (!fdir.exists()) {
                    fdir.mkdirs();
                }
                if (fdir.exists()) {
                    sRemoteVideo = SDVideo;
                    bCreateOK = true;
                }
            }
            if(!bCreateOK)
            {
                recDir = String.format("%s/%s/SD/SYMA_SDVideo_JH", StroragePath, sVendor);
                sRemoteVideo = recDir;
                fdir = new File(recDir);
                if (!fdir.exists()) {
                    fdir.mkdirs();
                }

            }

        }
    }


    public static void openGPSSettings() {
        if (nType != JH_App.nStyle_fly)
            return;
        if (locationManager == null)
            locationManager = (LocationManager) mContext.getSystemService(Context.LOCATION_SERVICE);
        if (locationManager.isProviderEnabled(LocationManager.GPS_PROVIDER) || locationManager.isProviderEnabled(LocationManager.NETWORK_PROVIDER)) {
            getLocation();
            return;
        }
        Intent intent = new Intent(Settings.ACTION_LOCATION_SOURCE_SETTINGS);// ActionLocationSourceSettings);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        mContext.startActivity(intent);
    }

    /**
     * 定位位置监听器
     */
    private static final LocationListener locationListener = new LocationListener() {
        public void onLocationChanged(Location location) {
            EventBus.getDefault().post(location, "GPS_LocationChanged");
        }
        public void onProviderDisabled(String provider) {
            Location location = new Location(provider);
            location.setLatitude(-400);
            location.setLongitude(-400);
            EventBus.getDefault().post(location, "GPS_LocationChanged");
        }
            public void onProviderEnabled(String provider) {
        }

        @Override
        public void onStatusChanged(String provider, int status, Bundle extras) {
            // 通过GPS获取位置
            Location location = locationManager.getLastKnownLocation(provider);
            EventBus.getDefault().post(location, "GPS_LocationChanged");

        }
    };

    /**
     * 如果开启正常，则会直接进入到显示页面，如果开启不正常，则会进行到GPS设置页面
     */
    protected static void getLocation() {
        //String serviceName = Context.LOCATION_SERVICE;
        // 查找到服务信息
        Criteria criteria = new Criteria();
        // 高精度
        criteria.setAccuracy(Criteria.ACCURACY_FINE);
        criteria.setAltitudeRequired(false);
        criteria.setBearingRequired(false);
        criteria.setCostAllowed(true);
        // 低功耗
        criteria.setPowerRequirement(Criteria.POWER_LOW);
        // 获取GPS信息
        try {
            String provider = locationManager.getBestProvider(criteria, true);
            // 通过GPS获取位置
            Location location = locationManager.getLastKnownLocation(LocationManager.GPS_PROVIDER);
            if (location == null) {
                location = locationManager.getLastKnownLocation(LocationManager.NETWORK_PROVIDER);
                if (location == null) {
                    location = new Location(provider);
                    location.setLatitude(-400);
                    location.setLongitude(-400);
                }
            }
            EventBus.getDefault().post(location, "GPS_LocationChanged");
            // 设置监听*器，自动更新的最小时间为间隔N秒(1秒为1*1000，这样写主要为了方便)或最小位移变化超过N米
            nCheckt = 0;
            if (bGpsInited) {
                locationManager.removeUpdates(locationListener);
            }
            bGpsInited = true;
            locationManager.requestLocationUpdates(LocationManager.GPS_PROVIDER, 1000, 2, locationListener);
            locationManager.requestLocationUpdates(LocationManager.NETWORK_PROVIDER, 1000, 2, locationListener);
        } catch (SecurityException e) {
            e.printStackTrace();
        } catch (RuntimeException e) {
            e.printStackTrace();
        }
    }

    public static void F_InitMusic() {
        if (Build.VERSION.SDK_INT >= 21)
        {
            SoundPool.Builder builder = new SoundPool.Builder();
            builder.setMaxStreams(4);//传入音频数量
            AudioAttributes.Builder attrBuilder = new AudioAttributes.Builder();
            attrBuilder.setLegacyStreamType(AudioManager.STREAM_MUSIC);//设置音频流的合适的属性
            builder.setAudioAttributes(attrBuilder.build());//加载一个AudioAttributes
            soundPool = builder.build();
        }
        else
        {
            soundPool = new SoundPool(2, AudioManager.STREAM_MUSIC, 0);
        }
        music_mid = soundPool.load(mContext, R.raw.center, 1);
        music_adj = soundPool.load(mContext, R.raw.anjian, 1);
    }


    //把图片或者视频添加到系统图库

    public static void F_Save2ToGallery(String filename, boolean bPhoto) {
        //保存图片后发送广播通知更新数据库
        if (mContext == null)
            return;
        try
        {
            ContentResolver contentResolver=mContext.getContentResolver();
            final ContentValues values = new ContentValues();
            if (bPhoto)
            {
                values.put(MediaStore.Images.Media.MIME_TYPE, "image/jpeg");
                values.put(MediaStore.Images.Media.DATA, filename);
                 Uri uri = contentResolver.insert(MediaStore.Images.Media.EXTERNAL_CONTENT_URI,
                        values);
            }
            else
            {
                values.put(MediaStore.Video.Media.MIME_TYPE, "video/mp4");
                values.put(MediaStore.Video.Media.DATA, filename);
                 Uri uri = contentResolver.insert(MediaStore.Video.Media.EXTERNAL_CONTENT_URI,
                        values);
            }
            Uri uri1 = Uri.parse("file://" + filename);
            mContext.sendBroadcast(new Intent(Intent.ACTION_MEDIA_SCANNER_SCAN_FILE, uri1));
        }
        catch (NullPointerException e)
        {

        }
    }


    //删除图片或者视频，并且把它从系统图库中清除
    public static void DeleteImage(String imgPath)
    {
        String stype = imgPath.substring(imgPath.length()-3,imgPath.length());
        stype = stype.toUpperCase();
        ContentResolver resolver = mContext.getContentResolver();
        Cursor cursor;
        {
           // cursor = MediaStore.Images.Media.query(resolver, MediaStore.Images.Media.EXTERNAL_CONTENT_URI, new String[]{MediaStore.Images.Media._ID}, MediaStore.Images.Media.DATA + "=?",
            //        new String[]{imgPath}, null);
            if(stype.equalsIgnoreCase("jpg") || stype.equalsIgnoreCase("png")) {
                cursor = resolver.query(MediaStore.Images.Media.EXTERNAL_CONTENT_URI, new String[]{MediaStore.Images.Media._ID}, MediaStore.Images.Media.DATA + "=?",
                        new String[]{imgPath}, null);
            }
            else
            {
                cursor = resolver.query(MediaStore.Video.Media.EXTERNAL_CONTENT_URI, new String[]{MediaStore.Video.Media._ID}, MediaStore.Video.Media.DATA + "=?",
                        new String[]{imgPath}, null);
            }
        }

        boolean result = false;
        if (cursor.moveToFirst()) {
            long id = cursor.getLong(0);
            Uri contentUri = MediaStore.Images.Media.EXTERNAL_CONTENT_URI;
            Uri uri = ContentUris.withAppendedId(contentUri, id);
            int count = resolver.delete(uri, null, null);
            result = count == 1;
        }
        {
            File file = new File(imgPath);
            if (file.isFile() && file.exists()) {
                file.delete();
            }
        }

    }

    public static void F_PlayCenter() {

        if (music_mid < 0) {
            music_mid = soundPool.load(mContext, R.raw.center, 1);
        }
        if (music_mid != -1)
            soundPool.play(music_mid, 1, 1, 0, 0, 1);

    }

    public static void F_PlayAdj() {
        if (music_adj < 0) {
            music_adj = soundPool.load(mContext, R.raw.anjian, 1);
        }
        if (music_adj != -1)
            soundPool.play(music_adj, 1, 1, 0, 0, 1);

    }

    public static void F_PlayPhoto() {

        if (music_photo != -1)
            soundPool.play(music_photo, 1, 1, 0, 0, 1);
    }

    public static void F_PlayButton() {

        if (music_btn != -1)
            soundPool.play(music_btn, 1, 1, 0, 0, 1);
    }

    public void Clear() {
        if (mGridList == null) {
            mSD_PhotosList.clear();
            mSD_VideosList.clear();
            mLocal_PhotosList.clear();
            mLocal_VideosList.clear();
            mDownloadList.clear();
            mDispList.clear();
            mNeedDownLoadList.clear();
            mGridList.clear();
        }
    }


    private static String intToIp(int i) {
        return (i & 0xFF) + "." + ((i >> 8) & 0xFF) + "." + ((i >> 16) & 0xFF) + "." + ((i >> 24) & 0xFF);
    }

    public static int F_GetWifiRssi() {
        if (mContext == null)
            return -1;
        if(nICType==wifination.IC_NO)
        {
            return -1;
        }
        WifiManager wifi_service = (WifiManager) mContext.getSystemService(WIFI_SERVICE);
        WifiInfo info = wifi_service.getConnectionInfo();

        int level = info.getRssi();
        int nrssi;
        //根据获得的信号强度发送信息
        if (level <= 0 && level >= -50) {
            nrssi = 4;
        } else if (level < -50 && level >= -70) {
            nrssi = 3;
        } else if (level < -70 && level >= -80) {
            nrssi = 2;
        } else if (level < -80 && level >= -90) {
            nrssi = 1;
        } else {
            nrssi = -1;
        }
        return nrssi;

    }

    public static int F_GetWifiType() {
        if (mContext == null)
            return -1;
        WifiManager wifi_service = (WifiManager) mContext.getSystemService(WIFI_SERVICE);

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
        sWifi = wifiId;

        String sIP = intToIp(info.getIpAddress());

        nICType = wifination.IC_NO;
        if (sIP.startsWith("175.16.10")) {
            nICType = wifination.IC_GKA;
        } else if (sIP.startsWith("192.168.234")) {
            nICType = wifination.IC_GK;
        } else if (sIP.startsWith("192.168.25")) {
            nICType = wifination.IC_GP;
        } else if (sIP.startsWith("192.168.26")) {
            nICType = wifination.IC_GPRTSP;
        } else if (sIP.startsWith("192.168.27")) {
            nICType = wifination.IC_GPH264;
        } else if (sIP.startsWith("192.168.28")) {
            nICType = wifination.IC_GPRTP;
        } else if (sIP.startsWith("192.168.29")) {
            nICType = wifination.IC_GPRTPB;
        } else if (sIP.startsWith("192.168.30")) {
            nICType = wifination.IC_GPH264A;
        } else if (sIP.startsWith("192.168.123")) {
            nICType = wifination.IC_SN;
        }
        else if (sIP.startsWith("192.168.16")) {
            nICType = wifination.IC_GK_UDP;
            //nICType = wifination.IC_GPH264;
        }
        return nICType;
    }

    // public static JH_App getInstance() {
    //    return singleton;
    // }


    //获取是否存在NavigationBar
    public static void checkDeviceHasNavigationBar(Context context) {

        /*
        Activity activity = (Activity) context;
        //隐藏虚拟按键，并且全屏
        if (Build.VERSION.SDK_INT > 11 && Build.VERSION.SDK_INT < 19) { // lower api
            View v = activity.getWindow().getDecorView();
            v.setSystemUiVisibility(View.GONE);
        } else if (Build.VERSION.SDK_INT >= 19) {
            //for new api versions.
            View decorView = activity.getWindow().getDecorView();
            int uiOptions = View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                    | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY | View.SYSTEM_UI_FLAG_FULLSCREEN;
            decorView.setSystemUiVisibility(uiOptions);
        }

        Window window =activity.getWindow();
        window.setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON, WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
*/

        if (context == null)
            return;

        boolean hasNavigationBar = false;

        Activity activity = (Activity) context;

        Window window = activity.getWindow();
        window.setFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS, WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS);
        window.setFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_NAVIGATION, WindowManager.LayoutParams.FLAG_TRANSLUCENT_NAVIGATION);

        window.setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON, WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        Resources rs = context.getResources();
        int id = rs.getIdentifier("config_showNavigationBar", "bool", "android");
        if (id > 0) {
            hasNavigationBar = rs.getBoolean(id);
        } else {
            try {
                Class<?> systemPropertiesClass = Class.forName("android.os.SystemProperties");
                Method m = systemPropertiesClass.getMethod("get", String.class);
                String navBarOverride = (String) m.invoke(systemPropertiesClass, "qemu.hw.mainkeys");
                if ("1".equals(navBarOverride)) {
                    hasNavigationBar = false;
                } else if ("0".equals(navBarOverride)) {
                    hasNavigationBar = true;


                }
            } catch (Exception e) {

            }
        }

        if (hasNavigationBar) {

            int uiOptions = View.SYSTEM_UI_FLAG_LAYOUT_STABLE |
                    //布局位于状态栏下方
                    View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION |
                    //全屏
                    View.SYSTEM_UI_FLAG_FULLSCREEN |
                    //隐藏导航栏
                    View.SYSTEM_UI_FLAG_HIDE_NAVIGATION |
                    View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN;
            if (Build.VERSION.SDK_INT >= 19) {
                uiOptions |= 0x00001000;
            } else {
                uiOptions |= View.SYSTEM_UI_FLAG_LOW_PROFILE;
            }
            window.getDecorView().setSystemUiVisibility(uiOptions);


            // activity.getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_HIDE_NAVIGATION);


        }



/*
            activity.getWindow().getDecorView().setSystemUiVisibility(
                    View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                            | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                            | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                            | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION // hide navbar
                            | View.SYSTEM_UI_FLAG_FULLSCREEN // hide status bar
                            | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
                            */

    }

    public static String getFileName(String pathandname) {

        int start = pathandname.lastIndexOf("/");
        int end = pathandname.length();
        if (start == -1)
            return pathandname;
        if (start != -1 && end != 0) {
            return pathandname.substring(start + 1, end);
        } else {
            return null;
        }
    }


    public static void F_ClearDataSDArray() {
        for (MyFilesItem filesItem : mSD_PhotosList) {
            filesItem.sSDPath = null;
            filesItem.sPhonePath = null;
        }
        mSD_PhotosList.clear();

        for (MyFilesItem filesItem : mSD_VideosList) {
            filesItem.sSDPath = null;
            filesItem.sPhonePath = null;
        }
        mSD_VideosList.clear();

        for (MyItemData itemData : mGridList) {
            itemData.image = null;
            itemData.sSDPath = null;
            itemData.sPhonePath = null;
        }
        mGridList.clear();

    }

    public static String getsVendor() {
        return sVendor;
    }







    public static boolean F_CheckFileIsExist(String sFileName, int size) {
        File file = new File(sFileName);
        int nSize = -1;
        if (file.exists() && !file.isDirectory())
        {
            FileInputStream fis = null;
            try {
                fis = new FileInputStream(file);
                nSize = fis.available();
            } catch (FileNotFoundException e) {
                e.printStackTrace();
            } catch (IOException e) {
                e.printStackTrace();
            }
            if (nSize == size)
                return true;
        }
        return false;
    }


    public static String F_GetSaveName(boolean bJpg) {
        Calendar calendar = Calendar.getInstance();  //获取当前时间，作为图标的名字
        int year = calendar.get(Calendar.YEAR);
        int month = calendar.get(Calendar.MONTH) + 1;
        int day = calendar.get(Calendar.DAY_OF_MONTH);
        int hour = calendar.get(Calendar.HOUR_OF_DAY);
        int minute = calendar.get(Calendar.MINUTE);
        int second = calendar.get(Calendar.SECOND);

        String str = String.format("%d-%02d-%02d %02d-%02d-%02d", year, month, day, hour, minute, second);
        if (bJpg)
            return sLocalPhoto + "/" + str + ".jpg";
        else
            return sLocalVideo + "/" + str + ".mp4";
    }


    public static void F_DispMesate(String str) {
        if (mContext == null)
            return;


        WindowManager wm = (WindowManager) (mContext.getSystemService(Context.WINDOW_SERVICE));


        DisplayMetrics metrics = new DisplayMetrics();
/**
 * getRealMetrics - 屏幕的原始尺寸，即包含状态栏。
 * version >= 4.2.2
 */
        int width=0;
        int height = 1;
        if (Build.VERSION.SDK_INT >= 17) {
            wm.getDefaultDisplay().getRealMetrics(metrics);
            width = metrics.widthPixels;
            height = metrics.heightPixels;
        }
        else
        {
            Point outSize = new Point();
            wm.getDefaultDisplay().getSize(outSize);
            width = outSize.x;
            height = outSize.y;
        }

        float dad = width / (float) height;
        Toast toast = Toast.makeText(mContext, str, Toast.LENGTH_SHORT);
        //第一个参数：设置toast在屏幕中显示的位置。我现在的设置是居中靠顶
        //第二个参数：相对于第一个参数设置toast位置的横向X轴的偏移量，正数向右偏移，负数向左偏移
        //第三个参数：同的第二个参数道理一样
        //如果你设置的偏移量超过了屏幕的范围，toast将在屏幕内靠近超出的那个边界显示
        //toast.setGravity(Gravity.TOP|Gravity.CENTER, -50, 100);
        //屏幕居中显示，X轴和Y轴偏移量都是0
        //toast.setGravity(Gravity.BOTTOM, 80, 100);
        if (dad < 1.81) {
            toast.setGravity(Gravity.CENTER, 0, 100);
        } else {
            toast.setGravity(Gravity.CENTER, 80, 100);
        }
        //toast.setGravity(Gravity.BOTTOM|Gravity.CENTER_HORIZONTAL, 0, 100);

        toast.show();
    }

    public static String getAppVersionName() {
        String versionName = "";
        try {
            if (mContext == null) {
                // ---get the package info---
                PackageManager pm = mContext.getPackageManager();
                PackageInfo pi = pm.getPackageInfo(mContext.getPackageName(), 0);
                versionName = pi.versionName;
                //versioncode = pi.versionCode;
                if (versionName == null || versionName.length() <= 0) {
                    return "";
                }
            } else {
                return "";
            }
        } catch (Exception e) {
            Log.e("VersionInfo", "Exception", e);
        }
        return versionName;
    }

    public static void F_GetOrg(Activity activity) {
        int sf = activity.getWindowManager().getDefaultDisplay().getRotation();
        if (sf == Surface.ROTATION_90) {
            JH_App.nOrg = ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE;
        }
        if (sf == Surface.ROTATION_270) {
            JH_App.nOrg = ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE;
        }
    }


    //public static void F_OpenStream(Surface surface) {
    public static void F_OpenStream() {

        int ictype = JH_App.F_GetWifiType();
        wifination.naSetAdjFps(true);


        JH_App.nICType = ictype;
        //JH_App.nICType = wifination.IC_GPH264A;
        //JH_App.nICType = wifination.IC_GKA;
        //JH_App.nICType = wifination.IC_GPH264;
        //JH_App.nICType = wifination.IC_GPRTSP;
        //JH_App.nICType = wifination.IC_SN;
        //JH_App.nICType = wifination.IC_GPRTP;
        wifination.naSetIcType(JH_App.nICType);
        if (JH_App.bIsSyMa)
            wifination.naSetCustomer("sima");
        else
            wifination.naSetCustomer(" ");
       // wifination.naSetVideoSurface(surface); //surfaceHolder.getSurface());
        String str = "";
        int re = -1;
        if (JH_App.nICType == wifination.IC_GK) {
            str = "rtsp://192.168.234.1/12";
            re = wifination.naInit(str);
        }
        if (JH_App.nICType == wifination.IC_GKA) {
            if (JH_App.b720P) {
                str = "1";
            } else {
                str = "2";
            }
            wifination.naSetGKA_SentCmdByUDP(true);
            re = wifination.naInit(str);
        }
        if (JH_App.nICType == wifination.IC_SN) {
            wifination.naInit(str);
        }

        if (JH_App.nICType == wifination.IC_GP) {
            str = "http://192.168.25.1:8080/?action=stream";
            wifination.naInit(str);
        }

        if (JH_App.nICType == wifination.IC_GPRTSP) {
            str = "rtsp://192.168.26.1:8080/?action=stream";
            re = wifination.naInit(str);
            if (re == 0)
                wifination.naPlay();
        }

        if (JH_App.nICType == wifination.IC_GPH264) {
            str = "rtsp://192.168.27.1:8080/?action=stream";
            //str = "rtsp://192.168.16.1/stream0";
            wifination.naInit(str);
        }

        if (JH_App.nICType == wifination.IC_GPRTP) {
            str = "192.168.28.1:19200";
            wifination.naInit(str);
        }
        if (JH_App.nICType == wifination.IC_GPRTPB) {
            str = "192.168.29.1:19200";
            wifination.naInit(str);
        }
        if (JH_App.nICType == wifination.IC_GPH264A) {
            str = "192.168.30.1:8080";
            wifination.naSetGPFps(20);
            wifination.naInit(str);
        }
        if (JH_App.nICType == wifination.IC_GK_UDP) {
            str = "192.168.16.1";
            wifination.naSetGPFps(25);
            wifination.naInit(str);
        }
        //sendCmdHandle.removeCallbacksAndMessages(null);
        //sendCmdHandle.post(sendCmdRunnable);
    }


    public static Bitmap F_AdjBitmp(Bitmap bitmap) {
        // 获得图片的宽高
        int width = bitmap.getWidth();
        int height = bitmap.getHeight();
        // 设置想要的大小
        int newWidth = 720 / 8;
        int newHeight = 1280 / 8;
        // 计算缩放比例
        float scaleWidth = ((float) newWidth) / width;
        float scaleHeight = ((float) newHeight) / height;
        // 取得想要缩放的matrix参数
        Matrix matrix = new Matrix();
        matrix.postScale(scaleWidth, scaleHeight);
        // 得到新的图片
        Bitmap newbm = Bitmap.createBitmap(bitmap, 0, 0, width, height, matrix, true);
        return newbm;
    }

    public static Bitmap getVideoThumbnail(String videoPath) {
        Bitmap bitmap = null;
        MediaMetadataRetriever retriever = new MediaMetadataRetriever();
        try {
            retriever.setDataSource(videoPath);
           // bitmap = retriever.getFrameAtTime(0, MediaMetadataRetriever.OPTION_CLOSEST_SYNC);
            bitmap = retriever.getFrameAtTime();
        } catch (IllegalArgumentException e) {
            e.printStackTrace();
        } catch (RuntimeException e) {
            e.printStackTrace();
        } finally {
            try {
                retriever.release();
            } catch (RuntimeException e) {
                e.printStackTrace();
            }
        }
        if (bitmap != null) {
            return F_AdjBitmp(bitmap);
        }
        return bitmap;
    }

    public static int F_GetVideoCountTime(String path) {
        MediaMetadataRetriever mmr = new MediaMetadataRetriever();
        mmr.setDataSource(path);
        String duration = mmr.extractMetadata(MediaMetadataRetriever.METADATA_KEY_DURATION); // 播放时长单位为毫秒
        int du = -1;
        try {
            du = Integer.parseInt(duration);
        } catch (NumberFormatException e) {
            e.printStackTrace();
        }
        if (du != -1)
            du /= 1000;
        return du;
    }


    private static List<String> lstFile = new ArrayList<String>();  //结果 List


    public static  void GetFilesA(String Path, String Extension, boolean IsIterative)  //搜索目录，扩展名，是否进入子文件夹
    {
        if(lstFile==null)
        {
            lstFile = new ArrayList<String>();
        }
        File fa =  new File(Path);
        if(fa.isFile())
        {
            lstFile.clear();
            return;
        }
        File[] files = new File(Path).listFiles();

        File wfile = new File(sLocalVideo + "/s8.txt");
        if (wfile.exists())
        {
            wfile.delete();
        }

            try {
                wfile.createNewFile();
            }catch (IOException e)
            {

            }
        wfile.setWritable(true);

        BufferedWriter wr=null;
        try {
            wr = new BufferedWriter(new FileWriter(wfile));
        }
        catch (IOException e)
        {

        }
        for (int i = 0; i < files.length; i++)
        {
            File f = files[i];
            if (f.isFile())
            {
                if(Extension!=null) {
                    if (f.getPath().substring(f.getPath().length() - Extension.length()).equals(Extension))  //判断扩展名
                        lstFile.add(f.getName());
                }
                else
                {
                    StringBuffer sb = new StringBuffer();
                    try {
                        BufferedReader br = new BufferedReader(new FileReader(f));
                        String line = "";
                        while ((line = br.readLine()) != null) {
                            sb.append(line);
                        }
                        br.close();
                    }catch (FileNotFoundException e)
                    {

                    }
                    catch ( IOException e)
                    {

                    }
                    String str =  f.getName()+"  "+sb+"\n";

                    lstFile.add(str);
                    try {
                        if(wr!=null)
                            wr.write(str);
                    }catch (IOException e)
                    {

                    }
                }
            }
            else if (f.isDirectory() && f.getPath().indexOf("/.") == -1)  //忽略点文件（隐藏文件/文件夹）
            {
                if(IsIterative)
                    GetFilesA(f.getPath(), Extension, IsIterative);
            }
        }
        try {
            wr.close();
        }
        catch (IOException e)
        {

        }

    }


}
