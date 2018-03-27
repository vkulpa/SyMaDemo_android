package com.joyhonest.jh_fly;

import android.animation.ObjectAnimator;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.support.annotation.Nullable;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnTouchListener;
import android.widget.ImageView;
import android.widget.RelativeLayout;

import com.joyhonest.jh_ui.R;
import com.joyhonest.jh_ui.Storage;

import org.simple.eventbus.EventBus;

/**
 * Created by aiven on 2017/11/18.
 */

public class MySwitch extends RelativeLayout {

    private Bitmap bmpLeft;
    private Bitmap bmpRight;
    private ImageView img;

    int firstX = 0;
    int firstY = 0;
    boolean bLeft = true;

    public MySwitch(Context context) {
        super(context);
        F_Init();
    }

    public MySwitch(Context context, AttributeSet attrs) {
        super(context, attrs);
        F_Init();
    }

    public MySwitch(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        F_Init();
    }

    private int nWidth = 0;
    private int nHeght = 0;

    private void F_Init() {
        bLeft = true;
        //bmpRight = BitmapFactory.decodeResource(this.getContext().getResources(), R.mipmap.record_icon_fly_jh);
        //bmpLeft = BitmapFactory.decodeResource(this.getContext().getResources(), R.mipmap.photo_icon_a_fly_jh);
        ImageView imgb = new ImageView(getContext());
        RelativeLayout.LayoutParams params = new RelativeLayout.LayoutParams(-1, -1);
        imgb.setBackgroundResource(R.mipmap.switch_back_1_fly_jh);
        this.addView(imgb);
        img = new ImageView(getContext());
        params = new RelativeLayout.LayoutParams(-1, -1);
        img.setBackgroundResource(R.mipmap.photo_icon_a_fly_jh);
        this.addView(img);
    }

    @Override
    protected void onSizeChanged(int w, int h, int oldw, int oldh) {
        super.onSizeChanged(w, h, oldw, oldh);
        nWidth = w;
        nHeght = h;

    }

    int nOp = 0;

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        int action = event.getAction();
        if (action == MotionEvent.ACTION_DOWN) {
            firstX = (int) event.getX();
        }
        nOp = 0;
        if (action == MotionEvent.ACTION_UP) {
            int da = Storage.dip2px(getContext(), 8);
            int x = (int) event.getX();
            if (x - firstX < -da) {

                if (!bLeft) {
                    bLeft = true;
                    nOp = 1;
                    F_DispIcon();
                }
            } else if (x - firstX > da) {
                if (bLeft) {
                    bLeft = false;
                    nOp = 2;
                    F_DispIcon();
                }
            } else {
                bLeft = !bLeft;
                if (bLeft) {
                    nOp = 1;
                } else {
                    nOp = 2;
                }
                F_DispIcon();

            }


            EventBus.getDefault().post(bLeft, "SwitchChanged");

        }

        //    if (MotionEvent.ACTION_UP == action) {

        return true;
        //return super.onTouchEvent(event);
    }


    private void F_DispIcon() {
        if (nOp == 1) {
            img.setBackgroundResource(R.mipmap.photo_icon_a_fly_jh);
            ObjectAnimator.ofFloat(img, "X", nWidth - (nHeght), 0).setDuration(200).start();
        }
        if (nOp == 2) {
            img.setBackgroundResource(R.mipmap.record_icon_fly_jh);
            ObjectAnimator.ofFloat(img, "X", 0, nWidth - (nHeght)).setDuration(200).start();
        }

        /*
        if(bLeft)
        {
            img.setBackgroundResource(R.mipmap.photo_icon_a_fly_jh);
        }
        else
        {
            img.setBackgroundResource(R.mipmap.record_icon_fly_jh);
        }
        */
    }

    public void F_SetPhoto(boolean b) {
        bLeft = b;
        F_DispIcon();

    }


}
