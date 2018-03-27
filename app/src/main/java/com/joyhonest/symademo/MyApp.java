package com.joyhonest.symademo;

import android.app.Application;

import com.squareup.leakcanary.LeakCanary;

/**
 * Created by aiven on 2017/9/5.
 */

public class MyApp extends Application {
    @Override
    public void onCreate() {
        super.onCreate();
        //LeakCanary.install(this);
    }
}
