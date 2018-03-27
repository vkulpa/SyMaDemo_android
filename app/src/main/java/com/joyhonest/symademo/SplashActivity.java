package com.joyhonest.symademo;


import android.annotation.SuppressLint;
import android.content.Context;
import android.content.Intent;

import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.Switch;
import android.widget.TextView;


import com.joyhonest.jh_fly.Fly_PlayActivity;
import com.joyhonest.jh_fly.MapTestActivity;
import com.joyhonest.jh_ui.JH_App;
import com.joyhonest.jh_ui.PlayActivity;

import java.io.IOException;
import java.nio.ByteBuffer;


public class SplashActivity extends AppCompatActivity
{
    Button   UI1_button;
    Button   UI2_button;
    Switch   SyMa_Switch;
    Switch   Type_Switch;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_splash);

        JH_App.init(getApplicationContext(),null,null,null,null);
     //   JH_App.bInitDispCtrol=false;
        JH_App.checkDeviceHasNavigationBar(this);
       // JH_App.GetFilesA("/proc/sys/net/ipv4",null,false);

        TextView ver_textView = (TextView)findViewById(R.id.ver_textView);
        ver_textView.setText(getAppVersionName(this));

        UI1_button = (Button)findViewById(R.id.but_Start);
        UI2_button = (Button)findViewById(R.id.button_sima);
        SyMa_Switch = (Switch)findViewById(R.id.switch1);
        Type_Switch = (Switch)findViewById(R.id.switch2);
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

                Intent mainIntent = new Intent(SplashActivity.this, PlayActivity.class);
                startActivity(mainIntent);
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
                Intent mainIntent = new Intent(SplashActivity.this, Fly_PlayActivity.class);
                //Intent mainIntent = new Intent(SplashActivity.this, MapTestActivity.class);

                startActivity(mainIntent);
            }
        });


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




}
