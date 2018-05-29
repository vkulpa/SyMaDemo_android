package com.joyhonest.wifination;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Matrix;
import android.opengl.GLSurfaceView;
import android.opengl.GLU;
import android.opengl.GLUtils;
import android.util.AttributeSet;

import com.joyhonest.jh_ui.R;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * Created by aiven on 2017/12/29.
 */

public class JH_GLSurfaceView extends GLSurfaceView {


    public  boolean  bDraw=true;

    public JH_GLSurfaceView(Context context_) {
        super(context_);
        init(context_);
    }

    /**
     * Standard View constructor. In order to render something, you
     * must call {@link #setRenderer} to register a renderer.
     */
    public JH_GLSurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init(context);

    }

    private void init(final Context context) {
        setEGLContextClientVersion(2);
        setRenderer(new GLSurfaceView.Renderer() {
            @Override
            public void onSurfaceCreated(GL10 gl, EGLConfig config) {
                wifination.init();
                //如果是 Tuk
                // naDecordInit();
            }

            @Override
            public void onSurfaceChanged(GL10 gl, int width, int height) {
                wifination.changeLayout(width, height);
            }

            @Override
            public void onDrawFrame(GL10 gl) {
                {
                    if(bDraw)
                        wifination.drawFrame();
                }
            }
        });
    }



    public static native int naDecordInit();

    public static native int naDecordRelease();

    public static native int naDecordFrame(byte[] data, int nLenA);


}
