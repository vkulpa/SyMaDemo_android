package com.joyhonest.wifination;



import org.simple.eventbus.EventBus;


import java.util.ArrayList;
import java.util.List;



/**
 * Created by aiven on 2017/11/30.
 */

public class JH_Tools {

    //////////////

    public static List<Byte> wifiData = new ArrayList<Byte>(100);
    private static List<MyCmdData> array = new ArrayList<MyCmdData>(100);

    private static class MyCmdData {
        private byte[] data;
        private int udpInx;

        public MyCmdData() {
            data = null;
            udpInx = -1;
        }

        public MyCmdData(int idx, byte[] dat) {
            data = null;
            if (dat != null) {
                if (dat.length > 4) {
                    data = new byte[dat.length - 4];
                    for (int ix = 0; ix < dat.length - 4; ix++) {
                        data[ix] = dat[ix + 4];
                    }
                }
            }
            udpInx = idx;
        }
    }

    public static boolean AdjData(byte[] revdata) {
        int idx;
        if (revdata.length <= 4)
            return false;

        idx = revdata[0] + revdata[1] * 1000 + revdata[2] * 100000 + revdata[3] * 10000000;
        MyCmdData data = new MyCmdData(idx, revdata);
        MyCmdData data1;
        if (array.size() == 0) {
            array.add(0, data);
        } else {
            boolean bInsert = false;
            for (int i = 0; i < array.size(); i++) {
                data1 = array.get(i);
                if (idx < data1.udpInx) {
                    bInsert = true;
                    array.add(i, data);
                    break;
                } else if (idx == data1.udpInx) {
                    bInsert = true;
                    break;
                }
            }
            if (!bInsert) {
                array.add(data);
            }
        }
        return true;
    }

    private static boolean Process(byte[] data) {
        int ix;
        boolean bOK = false;
        if (data != null) {
            for (ix = 0; ix < data.length; ix++) {
                wifiData.add(data[ix]);
            }
            while (wifiData.size() >= 8) {

                if (wifiData.get(0).byteValue() == (byte) 0x66 && wifiData.get(7).byteValue() == (byte) 0x99) {
                    byte nChecksum = 0;
                    for (ix = 1; ix < 6; ix++) {
                        nChecksum = (byte) (nChecksum ^ wifiData.get(ix).byteValue());
                    }
                    if (nChecksum == wifiData.get(6).byteValue()) {
                        bOK = true;
                        byte[] revData = new byte[8];
                        for (ix = 0; ix < 8; ix++) {
                            revData[ix] = wifiData.get(ix).byteValue();
                        }
                        EventBus.getDefault().post(revData, "GetWifiSendData");
                        for (ix = 0; ix < 8; ix++) {
                            wifiData.remove(0);
                        }
                    } else {
                        wifiData.remove(0);
                    }
                } else {
                    wifiData.remove(0);
                }
            }

        }
        return bOK;
    }

    public static void FindCmd() {
        int nCount = 0;
        int nPre = 0;
        int idx = 0;
        int ix;
        int i;
        int nStart = 0;
        for (i = 0; i < array.size(); i++) {
            MyCmdData d1 = array.get(i);
            if (i == 0) {
                nCount = d1.data.length;
                nPre = d1.udpInx;
                nStart = i;
                if (nCount >= 8) {
                    byte[] dat = new byte[nCount];
                    for (int yy = 0; yy < d1.data.length; yy++) {
                        dat[yy] = d1.data[yy];
                    }
                    Process(dat);
                    {
                        for (ix = i; ix >= 0; ix--) {
                            array.remove(ix);
                        }
                    }
                    break;
                }
            } else {
                idx = d1.udpInx;
                if (idx - nPre == 1) {
                    nCount += d1.data.length;
                    if (nCount >= 8) {
                        int le = 0;
                        byte[] dat = new byte[nCount];
                        for (int xx = nStart; xx <= i; xx++) {
                            MyCmdData d2 = array.get(xx);
                            for (int yy = 0; yy < d2.data.length; yy++) {
                                dat[le] = d2.data[yy];
                                le++;
                            }
                        }
                        Process(dat);
                        {
                            for (ix = i; ix >= 0; ix--) {
                                array.remove(ix);
                            }
                        }
                        break;
                    }
                } else {
                    nCount = d1.data.length;
                    nStart = i;
                }
                nPre = idx;
            }
        }
    }

    public static void F_ClearData() {
        if (array.size() > 50) {
            array.clear();
            wifiData.clear();
        }
    }

}
