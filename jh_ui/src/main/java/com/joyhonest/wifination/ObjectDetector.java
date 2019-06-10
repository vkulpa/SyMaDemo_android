package com.joyhonest.wifination;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Matrix;
import android.graphics.RectF;
import android.os.Handler;
import android.os.HandlerThread;
import android.util.Log;

//import com.joyhonest.jh_ui.JH_App;

import org.simple.eventbus.EventBus;

import java.io.IOException;
import java.util.List;

public class ObjectDetector
{


    private static Context  AppContext=null;
    //private   static  int   cropSize = 300;
    public    static  int  nWidth=300;
    public    static  int  nHeight=300;

    private  boolean  bBusy = false;
    private  boolean   bStar=false;
    private Classifier detector;

    public static  float MINIMUM_CONFIDENCE_TF_OD_API = 0.80f;
    //private static final int TF_OD_API_INPUT_SIZE = 300;

    private static final String TF_OD_API_MODEL_FILE ="file:///android_asset/frozen_inference_graph.pb";
    private static final String TF_OD_API_LABELS_FILE = "file:///android_asset/mydata.txt";



    private Matrix frameToCropTransform;
    private Matrix cropToFrameTransform;
    private Bitmap croppedBitmap = null;
    private Canvas canvas;
    private Handler handler;
    private HandlerThread handlerThread;



    public void  F_SetWidth_Height(int nW,int nH)
    {
        nWidth = nW;
        nHeight = nH;
        if(croppedBitmap!=null)
            croppedBitmap.recycle();
        croppedBitmap = Bitmap.createBitmap(nWidth, nHeight, Bitmap.Config.ARGB_8888);
        canvas = new Canvas(croppedBitmap);
    }

    private   ObjectDetector()
    {
        croppedBitmap = Bitmap.createBitmap(nWidth, nHeight, Bitmap.Config.ARGB_8888);
        canvas = new Canvas(croppedBitmap);
    }

    public  void SetAppCentext(Context context)
    {

        if(AppContext==null) {
            AppContext = context;
        }
        if(AppContext!=null)
        {
            if(detector==null)
            {
                try {
//                    detector = JH_ObjectDetectionAPIModel.create(
//                            AppContext.getAssets(), TF_OD_API_MODEL_FILE, TF_OD_API_LABELS_FILE, TF_OD_API_INPUT_SIZE);
                    //cropSize = TF_OD_API_INPUT_SIZE;
                    detector = JH_ObjectDetectionAPIModel.create(AppContext.getAssets(), TF_OD_API_MODEL_FILE, TF_OD_API_LABELS_FILE, nWidth,nHeight);
                } catch (final IOException e) {
                }
            }
        }

    }

    public static ObjectDetector getInstance() {
        return SingleTonHoulder.singleTonInstance;
    }

    //静态内部类
    public static class SingleTonHoulder {
        private static final ObjectDetector singleTonInstance = new ObjectDetector();
    }

    public void F_Start(boolean _bStart)
    {
        if(bStar && !_bStart)
        {

            if(handlerThread !=null)
            {
                handlerThread.quit();
            }
            if(handler!=null)
            {
                handler.removeCallbacksAndMessages(null);
            }
            bStar = _bStart;
            return;
        }
        if(!bStar && _bStart)
        {
            handlerThread = new HandlerThread("_Obj__jhabc_");
            handlerThread.start();
            handler = new Handler(handlerThread.getLooper());
            bStar = _bStart;
        }
    }

    public  int GetNumber(Bitmap bmp)
    {
        if(bmp==null) {
            return -1;
        }
        if(!bStar)
            return -2;

        if(bBusy)
        {
            return -3;
        }

        bBusy = true;
        //final  Bitmap bmp = bmpA;
        if(frameToCropTransform ==null)
        {
            int width = bmp.getWidth();
            int height =bmp.getHeight();
            frameToCropTransform =
                    ImageUtils.getTransformationMatrix(
                            width, height,
                            nWidth, nHeight,
                            0, false);

            cropToFrameTransform = new Matrix();
            frameToCropTransform.invert(cropToFrameTransform);
        }
        canvas.drawBitmap(bmp, frameToCropTransform, null);

        runInBackground(new Runnable() {
            @Override
            public void run() {
                progressImage();
            }
        });
        return 0;

    }


    private  void progressImage()
    {

        //ImageUtils.saveBitmap(croppedBitmap);
        final List<Classifier.Recognition> results = detector.recognizeImage(croppedBitmap);
        float minimumConfidence = MINIMUM_CONFIDENCE_TF_OD_API;

        boolean bFind =false;
        String id="";
        boolean bfirset=true;
        int nMax=0;
        int nre=0;
        int SetMax = (int) (minimumConfidence * 100);
        for (final Classifier.Recognition result : results)
        {

            final RectF location = result.getLocation();
            if(location!=null) {
                if (bfirset)// && result.getConfidence() >= minimumConfidence)
                {
                    nre = (int) (result.getConfidence() * 100);
                    id = result.getTitle();
                    nMax = nre;
                } else {
                    int tm = (int) (result.getConfidence() * 100);
                    if (tm >= nre) {
                        nre = tm;
                        nMax = nre;
                        id = result.getTitle();
                    }
                }
                bfirset = false;
            }

//            if (location != null && result.getConfidence() >= minimumConfidence)
//            {
//                //Log.i("MyTAG",result.getTitle());
//                String id = result.getTitle();
//                bFind = true;
//                EventBus.getDefault().post(id,"GetGueset");
//                break;
//            }
        }

        if(!id.isEmpty())
        {
            if(nMax>=SetMax)
            {
                bFind=true;
                EventBus.getDefault().post(id,"GetGueset");
            }
        }

        if(!bFind)
        {
            //Log.i("MyTAG","Not Found!!");
            EventBus.getDefault().post("","GetGueset");
        }
        bBusy = false;
    }



    private synchronized void runInBackground(final Runnable r) {
        if (handler != null) {
            handler.post(r);
        }
    }
}
