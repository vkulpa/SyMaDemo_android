package com.joyhonest.symademo;



import android.Manifest;
import android.content.Context;
import android.content.Intent;

import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;

import android.os.Handler;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.Switch;
import android.widget.TextView;


import com.joyhonest.jh_fly.Fly_PlayActivity;
import com.joyhonest.jh_ui.JH_App;
import com.joyhonest.jh_ui.PlayActivity;
import com.joyhonest.wifination.wifination;

import org.simple.eventbus.EventBus;
import org.simple.eventbus.Subscriber;


public class SplashActivity extends AppCompatActivity
{
    Button   UI1_button;
    Button   UI2_button;
    Switch   SyMa_Switch;
    Switch   Type_Switch;


    //Handler   handler;
    Runnable  runnable;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_splash);

        JH_App.init(getApplicationContext(),null,null,null,null);
        JH_App.checkDeviceHasNavigationBar(this);


        TextView ver_textView = (TextView)findViewById(R.id.ver_textView);
        ver_textView.setText(getAppVersionName(this));

        UI1_button = (Button)findViewById(R.id.but_Start);
        UI2_button = (Button)findViewById(R.id.button_sima);
        SyMa_Switch = (Switch)findViewById(R.id.switch1);
        Type_Switch = (Switch)findViewById(R.id.switch2);

        /*
        handler = new Handler();
        runnable = new Runnable() {
            @Override
            public void run() {
                Log.v("TEST","0000000------------");
                handler.postDelayed(this,20);
            }
        };

        handler.post(runnable);
        */

        UI1_button.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v)
            {
                if(SyMa_Switch.isChecked())
                {
                    JH_App.bIsSyMa = false;
                }
                else
                {
                    JH_App.bIsSyMa = true;
                }
                if(Type_Switch.isChecked())
                {
                    JH_App.b720P = true;
                }
                else
                {
                    JH_App.b720P = false;
                }
                JH_App.bInitDispCtrol=true;

              int df =   ContextCompat.checkSelfPermission(getApplicationContext(),
                        Manifest.permission.WRITE_EXTERNAL_STORAGE);

                Intent mainIntent = new Intent(SplashActivity.this, PlayActivity.class);
                //Intent mainIntent = new Intent(SplashActivity.this, Fly_PlayActivity.class);
                startActivity(mainIntent);
              //  handler.removeCallbacksAndMessages(null);
            }
        });



        UI2_button.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(SyMa_Switch.isChecked())
                {
                    JH_App.bIsSyMa = false;
                }
                else
                {
                    JH_App.bIsSyMa = true;
                }
                if(Type_Switch.isChecked())
                {
                    JH_App.b720P = true;
                }
                else
                {
                    JH_App.b720P = true;
                }
                JH_App.bInitDispCtrol=true;


                JH_App.bRecordVoice = true;


                Intent mainIntent = new Intent(SplashActivity.this, Fly_PlayActivity.class);
                startActivity(mainIntent);
            }
        });


        EventBus.getDefault().register(this);



    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        EventBus.getDefault().unregister(this);
    }



    public  String getAppVersionName(Context context) {
        String versionName = "";
        int  versioncode=0;
        try {
            // ---get the package info---
            PackageManager pm = context.getPackageManager();
            PackageInfo pi = pm.getPackageInfo(context.getPackageName(), 0);
            versionName = pi.versionName;
            versioncode = pi.versionCode;
            if (versionName == null || versionName.length() <= 0)
            {
                return "";
            }
        } catch (Exception e) {
            Log.e("VersionInfo", "Exception", e);
        }
        return versionName+" build "+versioncode;
    }


    @Override
    protected void onResume() {
        super.onResume();
        new Handler().postDelayed(new Runnable() {
            @Override
            public void run() {
                wifination.naGetGP_RTSP_Status();
            }
        },50);

    }

    @Subscriber(tag = "GetWifiInfoData")
    private void GetWifiInfoData(byte[] cmd)
    {
        byte nType = cmd[40];
        byte nPassEdit =cmd[45];
        byte[] bb= new byte[8];
        for(int i=0;i<8;i++)
        {
            bb[i]=cmd[46+i];
        }
        String s = new String(bb);
        Log.e("INFO", "Info a=" + nPassEdit+"pass = "+s);
    }




}
