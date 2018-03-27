package com.joyhonest.wifination;

/**
 * Created by aiven on 2017/5/20.
 */

public class jh_dowload_callback {
    public String sFileName;
    public int nPercentage;
    public int nError;

    public jh_dowload_callback(int n, String sFilename_, int nErr) {
        sFileName = sFilename_;
        nPercentage = n;
        nError = nErr;
    }
}
