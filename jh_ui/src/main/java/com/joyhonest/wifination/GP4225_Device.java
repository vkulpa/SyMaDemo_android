package com.joyhonest.wifination;

import android.util.Log;

import org.simple.eventbus.EventBus;

import java.util.ArrayList;
import java.util.List;

public class GP4225_Device {


    public int nMode;
    public int nSD;
    public int VideosCount;
    public int LockedCount;
    public int PhotoCount;


    public List<MyFile> PhotoFileList;
    public List<MyFile> LockedFileList;
    public List<MyFile> VideoFileList;

    public GP4225_Device()
    {
        PhotoFileList = new ArrayList<>();
        LockedFileList = new ArrayList<>();
        VideoFileList = new ArrayList<>();
        VideosCount = 0;
        LockedCount = 0;
        PhotoCount = 0;
        nSD = 0;
        nMode = 0;
    }


    private  void F_AddFile(int i,MyFile file)
    {
        if(i==1)
        {
            VideoFileList.add(file);
        }
        if(i==2)
        {
            LockedFileList.add(file);
        }
        if(i==3)
        {
            PhotoFileList.add(file);
        }
    }


    public  void F_ClearAllList()
    {
        VideoFileList.clear();
        LockedFileList.clear();
        PhotoFileList.clear();
    }

    public boolean  GP4225_PressData(byte[] data)
    {
        int nStartInx, nEndInx,inx,nLen;
        if(data==null)
            return false;
        if(data.length<10)
            return false;

        if((data[0] & 0xFF) != 'F' ||
                (data[1] & 0xFF) != 'D' ||
                (data[2] & 0xFF) != 'W' ||
                (data[3] & 0xFF) != 'N')
        {
            return false;
        }


        String sFileName="";

        int m_cmd = data[4] & 0xFF + (data[5] & 0xFF) * 0x100;
        int s_cmd = data[6] & 0xFF + (data[7] & 0xFF) * 0x100;
        int n_len = data[8] & 0xFF + (data[9] & 0xFF) * 0x100;

        if (m_cmd == 0x0000 && s_cmd == 0x0001) {   //Device Status
            nMode = data[10] & 0xFF;
            nSD   = data[11] & 0xFF;
            VideosCount = ((data[12] & 0xFF) + (data[13] & 0xFF) * 0x100 + (data[14] & 0xFF) * 0x10000 + (data[15] & 0xFF) * 0x1000000);
            LockedCount = ((data[16] & 0xFF) + (data[17] & 0xFF) * 0x100 + (data[18] & 0xFF) * 0x10000 + (data[19] & 0xFF) * 0x1000000);
            PhotoCount  = ((data[20] & 0xFF) + (data[21] & 0xFF) * 0x100 + (data[22] & 0xFF) * 0x10000 + (data[23] & 0xFF) * 0x1000000);
            Log.e("TAG", "Rev Mode = " + nMode + " nSD = "+nSD + " nVideCount ="+ VideosCount + " LockedCount = "+ LockedCount + " PhotoCount =  "+ PhotoCount);
            F_ClearAllList();
        }
        if (m_cmd == 0x0002)  //GetFileList
        {
            if (s_cmd == 0x0001 || s_cmd == 0x0002 || s_cmd == 0x0003) {  //VideoList   LockFileList  //图片文件
                nStartInx = data[10] & 0xFF + (data[11] & 0xFF) * 0x100;
                nEndInx = data[12] & 0xFF + (data[13] & 0xFF) * 0x100;

                for (int ii = 0; ii <= nEndInx - nStartInx; ii++) {

                    inx = 14 + 32 + (ii * 68);
                    nLen = (data[inx]&0xFF)+(data[inx+1]&0xFF)*0x100+(data[inx+2]&0xFF)*0x10000+(data[inx+3]&0xFF)*0x1000000;
                    inx += 4;
                    int da = 0;
                    for (int xx = 0; xx < 32; xx++) {
                        if (data[inx + xx] != 0) {
                            da++;
                        }
                    }
                    if (da != 0) {
                        byte bytes[] = new byte[da];
                        System.arraycopy(data, inx, bytes, 0, da);
                        sFileName = new String(bytes);
                    }
                    MyFile file = new MyFile("", sFileName, nLen);
                    //F_AddFile(s_cmd,file);
                    EventBus.getDefault().post(file,"GP4225_RevFile");

                }
            }
            return true;
        }
        return  false;

    }

    public class MyFile {
        public String sPath;
        public int nLength;
        public String sFileName;
        //public String sPhoneFileName;

        public MyFile(String sPath_, String sFileName_, int nLength_)
        {
            sPath = sPath_;
            sFileName = sFileName_;
            nLength = nLength_;
            //sPhoneFileName = "";
        }

    }

}
