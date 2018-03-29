package com.joyhonest.jh_fly;


import android.content.pm.ActivityInfo;
import android.os.Bundle;
import android.app.Fragment;
import android.os.Handler;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Surface;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.Switch;
import android.widget.TextView;

import com.joyhonest.jh_ui.JH_App;
import com.joyhonest.jh_ui.MyControl;
import com.joyhonest.jh_ui.R;
import com.joyhonest.jh_ui.Storage;
import com.joyhonest.wifination.fly_cmd;
import com.joyhonest.wifination.wifination;

import org.simple.eventbus.EventBus;

/**
 * A simple {@link Fragment} subclass.
 */
public class FlyPlayFragment extends Fragment implements View.OnClickListener {

    public static boolean bTestMode = false;

    private MyControl myControl;
    private Button Fly_Camera_Btn;
    private Button Photo_Record_Select_Btn;
    private MySwitch myswitch;
    private Button Photo_Record_Start_Btn;
    private Button Floder_Btn;
    private Button Return_Btn;
    private Button OneKeyReturn_Btn;

    private Button Speed_Btn;
    private Button Adj_Btn;
    private Button Gsensor_Btn;

    private Button Path_Btn;
    private Button VR_Btn;
    private Button HeadLess_Btn;


    private Button Finger_Btn;
    private Button StopFly_Btn;
    private Button UpDn_Btn;
    private Button More_Btn;


    private boolean bMore;


    private TextView Record_Time_TextCtrl;


    private boolean bControlUI = true;
    private boolean bPhoto = true;


    private TextView Location_TxtView;
    private LinearLayout Layout_LeftMenu;

    private LinearLayout Layout_Menu;

    private LinearLayout rect_layout;


    private TextView snapshot;


    private boolean bMenuHide;


    public FlyPlayFragment() {
        // Required empty public constructor
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {


        View view = inflater.inflate(R.layout.fragment_fly_play_jh, container, false);
        view.findViewById(R.id.rooglayout).setBackgroundColor(0x00010000);

        rect_layout = (LinearLayout) view.findViewById(R.id.rect_layout);

        snapshot = (TextView) view.findViewById(R.id.snapshot);
        snapshot.setVisibility(View.INVISIBLE);

        Layout_LeftMenu = (LinearLayout) view.findViewById(R.id.Layout_LeftMenu);
        Layout_Menu = (LinearLayout) view.findViewById(R.id.Menu_Layout);

        Location_TxtView = (TextView) view.findViewById(R.id.Location_TxtView);
        Location_TxtView.setText("");
        myControl = (MyControl) view.findViewById(R.id.myControl);
        myControl.F_SetImage(R.mipmap.cir_back_fly_jh_b, R.mipmap.cir_fly_jh);
        myControl.F_SetDispText(false);

        Record_Time_TextCtrl = (TextView) view.findViewById(R.id.Record_textView);


        Fly_Camera_Btn = (Button) view.findViewById(R.id.button01);
        //Photo_Record_Select_Btn = (Button)view.findViewById(R.id.button02);
        myswitch = (MySwitch) view.findViewById(R.id.button02);
        Photo_Record_Start_Btn = (Button) view.findViewById(R.id.button03);
        Floder_Btn = (Button) view.findViewById(R.id.button04);
        Return_Btn = (Button) view.findViewById(R.id.return_btn);

        //OneKeyReturn_Btn = (Button)view.findViewById(R.id.button05);

        Speed_Btn = (Button) view.findViewById(R.id.button05);
        Adj_Btn = (Button) view.findViewById(R.id.button06);
        Gsensor_Btn = (Button) view.findViewById(R.id.button07);

        Path_Btn = (Button) view.findViewById(R.id.button08);
        VR_Btn = (Button) view.findViewById(R.id.button09);
        HeadLess_Btn = (Button) view.findViewById(R.id.button0A);

        //Finger_Btn = (Button)view.findViewById(R.id.button08);

        StopFly_Btn = (Button) view.findViewById(R.id.button10);
        UpDn_Btn = (Button) view.findViewById(R.id.button11);

        More_Btn = (Button) view.findViewById(R.id.button_more);


        More_Btn.setOnClickListener(this);
        Fly_Camera_Btn.setOnClickListener(this);
        //  Photo_Record_Select_Btn.setOnClickListener(this);
        Photo_Record_Start_Btn.setOnClickListener(this);
        Floder_Btn.setOnClickListener(this);
        Return_Btn.setOnClickListener(this);

        StopFly_Btn.setOnClickListener(this);

        UpDn_Btn.setOnClickListener(this);
        bPhoto = true;

        Speed_Btn.setOnClickListener(this);
        Adj_Btn.setOnClickListener(this);
        Gsensor_Btn.setOnClickListener(this);

        Path_Btn.setOnClickListener(this);
        VR_Btn.setOnClickListener(this);
        HeadLess_Btn.setOnClickListener(this);
        Layout_Menu.setOnClickListener(this);
        bMore = false;


        F_DispUI();
        F_SetLocaiotn("");
        Location_TxtView.setTextColor(0xFFFF0000);
        F_DispMore();

        bMenuHide = true;
        Layout_LeftMenu.setVisibility(View.INVISIBLE);

        F_DispAllMenu(false);

        new Handler().postDelayed(new Runnable() {
            @Override
            public void run() {
                F_Adj_rect();
            }
        }, 10);
        return view;
    }

    private void F_SetRectIcon(Button button, int rect) {
        LinearLayout.LayoutParams params = (LinearLayout.LayoutParams) button.getLayoutParams();
        params.width = rect;
        params.height = rect;
        button.setLayoutParams(params);
    }

    private void F_Adj_rect() {
        int rect = Math.min(rect_layout.getWidth(), rect_layout.getHeight());
        int pitch = Storage.dip2px(getActivity(), 6);
        int rr = Storage.dip2px(getActivity(), 40);
        if (rect > rr) {
            rect = rr;
            rect -= pitch;
        }

        F_SetRectIcon(Speed_Btn, rect);
        F_SetRectIcon(Adj_Btn, rect);
        F_SetRectIcon(Gsensor_Btn, rect);
        F_SetRectIcon(Path_Btn, rect);
        F_SetRectIcon(VR_Btn, rect);
        F_SetRectIcon(HeadLess_Btn, rect);


    }

    public void F_DispMessage(String str) {
        snapshot.setText(str);
        snapshot.setVisibility(View.VISIBLE);
        snapshot.bringToFront();
        new Handler().postDelayed(new Runnable() {
            @Override
            public void run() {
                snapshot.setVisibility(View.INVISIBLE);
            }
        }, 500);
    }

    private void F_DispMore() {
        if (bMore) {
            Path_Btn.setVisibility(View.VISIBLE);
            VR_Btn.setVisibility(View.VISIBLE);
            HeadLess_Btn.setVisibility(View.VISIBLE);

        } else {
            Path_Btn.setVisibility(View.INVISIBLE);
            VR_Btn.setVisibility(View.INVISIBLE);
            HeadLess_Btn.setVisibility(View.INVISIBLE);
        }
    }


    public void F_SetLocaiotn(String str) {
        Location_TxtView.setVisibility(View.INVISIBLE);
        Location_TxtView.setText(str);
    }


    public void F_DispSpeedIcon() {

        if (Speed_Btn != null) {
            if (JH_App.bHiSpeed) {
                Speed_Btn.setBackgroundResource(R.mipmap.speed_high_fly_jh);
            } else {
                Speed_Btn.setBackgroundResource(R.mipmap.speed_low_fly_jh);
            }
        }

    }

    public void F_DispGSensorIcon() {

        if (Gsensor_Btn != null) {
            if (JH_App.bSensor) {
                myControl.F_SetMode(1);
                Gsensor_Btn.setBackgroundResource(R.mipmap.gsensor_icon_sel_fly_jh);
                int org = getActivity().getWindowManager().getDefaultDisplay().getRotation();

                if (Surface.ROTATION_270 == org) {
                    getActivity().setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE);
                } else {
                    getActivity().setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
                }
            } else {
                myControl.F_SetMode(0);
                Gsensor_Btn.setBackgroundResource(R.mipmap.gsensor_icon_fly_jh);
                getActivity().setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_SENSOR_LANDSCAPE);
            }
        }
    }

    private void F_DispHeadLessIcon() {

        if (HeadLess_Btn != null) {
            if (JH_App.bHeadLess) {
                HeadLess_Btn.setBackgroundResource(R.mipmap.headless_sel_fly_jh);
            } else {
                HeadLess_Btn.setBackgroundResource(R.mipmap.headless_fly_jh);
            }
        }

    }


    public void F_InitDisp() {
        F_Disp3DUI();
    }

    public void F_Disp3DUI() {
        if (JH_App.bVR)
        {
            boolean b = bControlUI;
            bControlUI = false;
            F_DispUI();
            bControlUI = b;
            sentHander.removeCallbacksAndMessages(null);
            sentHander.post(sentRunnable);
            Fly_Camera_Btn.setVisibility(View.INVISIBLE);
            myswitch.setVisibility(View.INVISIBLE);
            Photo_Record_Start_Btn.setVisibility(View.INVISIBLE);
            Record_Time_TextCtrl.setVisibility(View.INVISIBLE);
            Floder_Btn.setVisibility(View.INVISIBLE);

        }
        else {
            F_DispUI();
            Fly_Camera_Btn.setVisibility(View.VISIBLE);
            myswitch.setVisibility(View.VISIBLE);
            Photo_Record_Start_Btn.setVisibility(View.VISIBLE);
            Record_Time_TextCtrl.setVisibility(View.VISIBLE);
            Floder_Btn.setVisibility(View.VISIBLE);
        }
        wifination.naSet3D(JH_App.bVR);
        wifination.F_AdjBackGround(getActivity(), R.mipmap.loginbackground_fly_jh);//R.mipmap.loginbackground_jh)


    }

    private void F_DispAllMenu(boolean bDisp) {
        if (bDisp) {
            Layout_LeftMenu.setVisibility(View.VISIBLE);
            F_DispMore();
        } else {
            Layout_LeftMenu.setVisibility(View.INVISIBLE);
        }

    }

    int ixxxx = 0;
    int ch = 0x55;

    String strRecordFilename = "";

    @Override
    public void onClick(View v) {
        if (Layout_Menu == v) {
            if (JH_App.bVR)
                return;
            bMenuHide = false;
            F_DispAllMenu(true);
        }
        if (v == More_Btn) {
            bMore = !bMore;
            F_DispMore();
        }
        if (v == StopFly_Btn) {
            JH_App.bStop = true;
            StopFly_Btn.setBackgroundResource(R.mipmap.stop_sel_fly_jh);
            new Handler().postDelayed(new Runnable() {
                @Override
                public void run() {
                    JH_App.bStop = false;
                    if((JH_App.nSdStatus & JH_App.LocalRecording) !=0)
                    {
                        StopFly_Btn.setBackgroundResource(R.mipmap.stop_nor_fly_jh);
                    }
                    else
                    {
                        StopFly_Btn.setBackgroundResource(R.mipmap.stop_nor_fly_jh_b);
                    }

                }
            }, 500);
        }

        if (v == UpDn_Btn) {
            JH_App.bUp = !JH_App.bUp;
            UpDn_Btn.setBackgroundResource(R.mipmap.keyup_dn_sel_fly_jh);
            new Handler().postDelayed(new Runnable() {
                @Override
                public void run() {
                    if((JH_App.nSdStatus & JH_App.LocalRecording)!=0)
                    {
                        UpDn_Btn.setBackgroundResource(R.mipmap.keyup_dn_fly_jh);
                    }
                    else
                    {
                        UpDn_Btn.setBackgroundResource(R.mipmap.keyup_dn_fly_jh_b);
                    }

                }
            }, 300);
        }
        if (v == Path_Btn) {
            EventBus.getDefault().post("abc", "GotoPath");
        }
        if (v == VR_Btn) {
            JH_App.bVR = !JH_App.bVR;
            F_Disp3DUI();

            ((Fly_PlayActivity) getActivity()).F_RefSurface();
        }

        if (v == Speed_Btn) {
            JH_App.bHiSpeed = !JH_App.bHiSpeed;
            F_DispSpeedIcon();
        }
        if (v == Gsensor_Btn) {
            JH_App.bSensor = !JH_App.bSensor;
            F_DispGSensorIcon();

        }
        if (v == HeadLess_Btn) {
            JH_App.bHeadLess = !JH_App.bHeadLess;
            F_DispHeadLessIcon();

        }

        if (v == Adj_Btn) {
            JH_App.bAdj = true;
            Adj_Btn.setBackgroundResource(R.mipmap.adj_sel_fly_jh);
            new Handler().postDelayed(new Runnable() {
                @Override
                public void run() {
                    JH_App.bAdj = false;
                    Adj_Btn.setBackgroundResource(R.mipmap.adj_fly_jh);

                }
            }, 500);
        }

        if (v == Fly_Camera_Btn) {
            bControlUI = !bControlUI;
            F_DispUI();
            if (bControlUI) {
                bMenuHide = true;
                bMore = false;
                F_DispAllMenu(false);
            }

                /*
                ixxxx=2;
                cmd[0]=(byte)ixxxx;
                cmd[1]=(byte)(ixxxx>>8);
                cmd[2]=(byte)(ixxxx>>16);
                cmd[3]=(byte)(ixxxx>>24);
                cmd[4] = (byte)(cmd[5]^cmd[6]^cmd[7]^cmd[8]^cmd[9]);
                cmd[5] = (byte)0x99;
                wifination.naSentCmd(cmd,6);

                ixxxx = 1;
                cmd[0]=(byte)ixxxx;
                cmd[1]=(byte)(ixxxx>>8);
                cmd[2]=(byte)(ixxxx>>16);
                cmd[3]=(byte)(ixxxx>>24);
                cmd[4] = 0x66;
                cmd[5] = 0x00;
                cmd[10] = (byte)(cmd[5]^cmd[6]^cmd[7]^cmd[8]^cmd[9]);
                wifination.naSentCmd(cmd,10);


                ixxxx = 0;
                cmd[0]=(byte)ixxxx;
                cmd[1]=(byte)(ixxxx>>8);
                cmd[2]=(byte)(ixxxx>>16);
                cmd[3]=(byte)(ixxxx>>24);
                cmd[10] = (byte)(cmd[5]^cmd[6]^cmd[7]^cmd[8]^cmd[9]);
                wifination.naSentCmd(cmd,6);

                ixxxx=3;
                cmd[0]=(byte)ixxxx;
                cmd[1]=(byte)(ixxxx>>8);
                cmd[2]=(byte)(ixxxx>>16);
                cmd[3]=(byte)(ixxxx>>24);
                cmd[4] = 0x66;
                cmd[5] = 0x00;
                cmd[6] = (byte)0xAA;
                cmd[7] = 0x55;
                cmd[8] = (byte)0xAA;
                cmd[9] = (byte)0x99;


                wifination.naSentCmd(cmd,10);

                ixxxx = 4;
                cmd[0]=(byte)ixxxx;
                cmd[1]=(byte)(ixxxx>>8);
                cmd[2]=(byte)(ixxxx>>16);
                cmd[3]=(byte)(ixxxx>>24);
                cmd[4] = (byte)(cmd[5]^cmd[6]^cmd[7]^cmd[8]^cmd[9]);
                cmd[5] = (byte)0x99;
                wifination.naSentCmd(cmd,6);
*/


        }
        if (v == Return_Btn) {
            EventBus.getDefault().post("exit", "Exit");
        }

        if (v == Floder_Btn) {
            // wifination.naSetVideoSurface(null);
            Integer nFragment = JH_Fly_Setting.Brow_Select_Fragment;
            EventBus.getDefault().post(nFragment, "gotoFragment");

        }
        if (v == Photo_Record_Start_Btn) {
            if ((JH_App.nSdStatus & JH_App.Status_Connected) == 0)
                return;
            if (bPhoto) {

                if ((JH_App.nSdStatus & JH_App.SD_SNAP) != 0)     //SD 拍照还没有完成。就不进行此次拍照
                    return;

                if (JH_App.bPhone_SNAP)
                    return;
                JH_App.bPhone_SNAP = true;
                final String str = JH_App.F_GetSaveName(true);
                wifination.naSnapPhoto(str, wifination.TYPE_BOTH_PHONE_SD);
                Handler handler = new Handler();
                Runnable runnable = new Runnable() {
                    @Override
                    public void run() {
                        JH_App.bPhone_SNAP = false;
                        //  JH_App.F_Save2ToGallery(getActivity(), str,true);
                    }
                };
                handler.postDelayed(runnable, 500);

            } else {
                if ((JH_App.nSdStatus & JH_App.Status_Connected) == 0) {
                    Log.e("Error:", "Not DispVideo!");
                    return;
                }

                if ((JH_App.nSdStatus & JH_App.LocalRecording) != 0) {
                    wifination.naStopRecord_All();
                    Photo_Record_Start_Btn.setBackgroundResource(R.mipmap.photo_record_icon_fly_jh);
                    // JH_App.F_Save2ToGallery(getActivity(), strRecordFilename,false);
                } else {
                    strRecordFilename = JH_App.F_GetSaveName(false);
                    wifination.naStartRecord(strRecordFilename, wifination.TYPE_BOTH_PHONE_SD);
                    Record_Time_TextCtrl.setText("00:00");
                    Photo_Record_Start_Btn.setBackgroundResource(R.mipmap.photo_recording_icon_fly_jh);
                }
            }
        }

    }

    public void F_StartAdjRecord(boolean b) {

        if (b) {
            RecHander.removeCallbacksAndMessages(null);
            RecHander.post(RecRunnable);
        } else {
            RecHander.removeCallbacksAndMessages(null);
        }
    }

    @Override
    public void onResume() {
        super.onResume();
        F_StartAdjRecord(true);
    }


    public void F_DispRecorTime() {
        if ((JH_App.nSdStatus & JH_App.LocalRecording) != 0) {
            int sec =wifination.naGetRecordTime()/1000;
            int nMin = (sec / 60);
            if (nMin > 99)
                nMin = 0;
            int nSec = (int) (sec % 60);
            String str = String.format("%02d:%02d", nMin, nSec);
            Record_Time_TextCtrl.setText(str);
            Record_Time_TextCtrl.setVisibility(View.VISIBLE);
        } else {
            Record_Time_TextCtrl.setVisibility(View.INVISIBLE);
        }
    }


    Handler RecHander = new Handler();
    Runnable RecRunnable = new Runnable() {
        @Override
        public void run() {
            F_DispRecorTime();
            if ((JH_App.nSdStatus & JH_App.Status_Connected) != 0) {
                if (JH_App.bNeedStartsasyRecord) {
                    String str = JH_App.F_GetSaveName(false);
                    wifination.naStartRecord(str, wifination.TYPE_ONLY_PHONE);
                   // JH_App.nRecTime = System.currentTimeMillis() / 1000;
                    Record_Time_TextCtrl.setText("00:00");
                    JH_App.bNeedStartsasyRecord = false;
                }
            }
            RecHander.postDelayed(this, 500);
        }
    };

    public void F_DispRcordIcon(Boolean bRecordding) {
        if (bRecordding) {
            Record_Time_TextCtrl.setVisibility(View.VISIBLE);

        } else {
            Record_Time_TextCtrl.setVisibility(View.INVISIBLE);
        }
    }

    public void F_SetPhoto(boolean b) {
        bPhoto = b;
        F_DispPhoto_Record();
    }



    public void F_DispPhoto_Record() {

        if (bPhoto) {
            Photo_Record_Start_Btn.setBackgroundResource(R.mipmap.photo_icon_fly_jh);
            Record_Time_TextCtrl.setVisibility(View.INVISIBLE);
        } else {

            if ((JH_App.nSdStatus & JH_App.LocalRecording) != 0) {
                Record_Time_TextCtrl.setVisibility(View.VISIBLE);
                Photo_Record_Start_Btn.setBackgroundResource(R.mipmap.photo_recording_icon_fly_jh);
                myControl.F_SetImage(R.mipmap.cir_back_fly_jh, R.mipmap.cir_fly_jh);
                myControl.F_SetFlyRecord(true);
                if(bControlUI) {
                    Fly_Camera_Btn.setBackgroundResource(R.mipmap.remote_control_fly_jh);
                }
                else
                {
                    Fly_Camera_Btn.setBackgroundResource(R.mipmap.no_remote_fly_jh);
                }


                Floder_Btn.setBackgroundResource(R.mipmap.folder_fly_jh);
                StopFly_Btn.setBackgroundResource(R.mipmap.stop_nor_fly_jh);
                UpDn_Btn.setBackgroundResource(R.mipmap.keyup_dn_fly_jh);
            }
            else
            {
                Photo_Record_Start_Btn.setBackgroundResource(R.mipmap.photo_record_icon_fly_jh);
                myControl.F_SetImage(R.mipmap.cir_back_fly_jh_b, R.mipmap.cir_fly_jh);
                myControl.F_SetFlyRecord(false);

                if(bControlUI) {
                    Fly_Camera_Btn.setBackgroundResource(R.mipmap.remote_control_fly_jh_b);
                }
                else
                {
                    Fly_Camera_Btn.setBackgroundResource(R.mipmap.no_remote_fly_jh_b);
                }

                Floder_Btn.setBackgroundResource(R.mipmap.folder_fly_jh_b);
                StopFly_Btn.setBackgroundResource(R.mipmap.stop_nor_fly_jh_b);
                UpDn_Btn.setBackgroundResource(R.mipmap.keyup_dn_fly_jh_b);
            }
        }
    }

    public void F_StartStopSentCmd(boolean bStart) {
        if (bStart) {
            sentHander.removeCallbacksAndMessages(null);
            sentHander.post(sentRunnable);
        } else {
            sentHander.removeCallbacksAndMessages(null);
        }
    }

    public void F_StopSentCmd() {
        F_StartStopSentCmd(false);
    }

    public void F_DispUI() {
        F_DispHeadLessIcon();
        F_DispSpeedIcon();
        F_DispGSensorIcon();
        if (bControlUI) {
            Fly_Camera_Btn.setBackgroundResource(R.mipmap.remote_control_fly_jh_b);


            UpDn_Btn.setVisibility(View.VISIBLE);
            myControl.setVisibility(View.VISIBLE);
            StopFly_Btn.setVisibility(View.VISIBLE);


            Speed_Btn.setVisibility(View.VISIBLE);
            Adj_Btn.setVisibility(View.VISIBLE);
            Gsensor_Btn.setVisibility(View.VISIBLE);
            More_Btn.setVisibility(View.VISIBLE);
            F_DispMore();


            F_StartStopSentCmd(true);

        } else {

            Fly_Camera_Btn.setBackgroundResource(R.mipmap.no_remote_fly_jh);
            UpDn_Btn.setVisibility(View.INVISIBLE);
            myControl.setVisibility(View.INVISIBLE);
            StopFly_Btn.setVisibility(View.INVISIBLE);
            More_Btn.setVisibility(View.INVISIBLE);

            Speed_Btn.setVisibility(View.INVISIBLE);
            Adj_Btn.setVisibility(View.INVISIBLE);
            Gsensor_Btn.setVisibility(View.INVISIBLE);

            Path_Btn.setVisibility(View.INVISIBLE);
            VR_Btn.setVisibility(View.INVISIBLE);
            HeadLess_Btn.setVisibility(View.INVISIBLE);

            F_StartStopSentCmd(false);
        }
        F_DispPhoto_Record();

    }


    private Handler sentHander = new Handler();
    private Runnable sentRunnable = new Runnable() {
        @Override
        public void run() {
            F_SentCmd();
            sentHander.postDelayed(this, 35);
        }
    };

    byte[] cmd = new byte[20];

    private void F_SentCmd() {
        if (!bControlUI) {
            return;
        }
        //if(myControl.getVisibility()!=View.VISIBLE)
        //    return;
        if (JH_App.bisPathMode)
            return;

        int i = 0;
        int X1, Y1, X2, Y2, X_ADJ2, Y_ADJ2, X_ADJ1;
        int X2_bak;
        int Y2_bak;

        X1 = myControl.F_GetRotate();
        Y1 = myControl.F_GetThrottle();
        X2 = myControl.F_GetLeftRight();
        Y2 = myControl.F_GetForwardBack();
        X_ADJ1 = myControl.F_GetRotateAdj();
        X_ADJ2 = myControl.F_GetLeftRightAdj();
        Y_ADJ2 = myControl.F_GetForwardBackAdj();



        if (X2 > 0x70 && X2 < 0x90) {
            X2 = 0x80;
        }

        if (Y2 > 0x70 && Y2 < 0x90) {
            Y2 = 0x80;
        }

        if (X1 > (0x80 - 0x25) && X1 < (0x80 + 0x25)) {
            X1 = 0x80;
        }
        i = 0;


        if (Y2 > 0x80) {
            Y2 -= 0x80;
        } else if (Y2 < 0x80) {
            Y2 = 0x80 - Y2;
            Y2 += 0x80;
            if (Y2 > 0xFF) {
                Y2 = 0xFF;
            }
        }

        if (X1 > 0x80) {

        } else if (X1 < 0x80) {
            X1 = 0x80 - X1;
            if (X1 > 0x7F) {
                X1 = 0x7F;
            }
        }

        if (X2 > 0x80) {
        } else if (X2 < 0x80) {
            X2 = 0x80 - X2;
            if (X2 > 0x7F) {
                X2 = 0x7F;
            }
        }


        cmd[0] = (byte) Y1;   //油门
        cmd[1] = (byte) Y2;
        cmd[2] = (byte) X1;
        cmd[3] = (byte) X2;

        cmd[4] = 0x20;          //油门微调  这里没有。

        int da = Y_ADJ2 - 0x80;
        if (da < 0)               // 后调
        {
            da = 0 - da;
            da += 0x20;
            if (da > 0x3F) {
                da = 0x3F;
            }
        } else if (da > 0) {
            if (da > 0x1F)
                da = 0x1F;
        } else {
            da = 0x00;
        }


        cmd[5] = (byte) da;       //前后微调
        if (JH_App.bHiSpeed)
            cmd[5] |= 0x80;          //高速模式

        cmd[5] |= 0x40;


        da = X_ADJ1 - 0x80;          //旋转微调
        if (da < 0) {
            da = 0 - da;
            if (da > 0x1F) {
                da = 0x1F;
            }
        } else if (da > 0) {
            da += 0x20;
            if (da > 0x3F)
                da = 0x3F;
        } else {
            da = 0x20;
        }


        cmd[6] = (byte) da;


        da = X_ADJ2 - 0x80;
        if (da < 0) {

            da = 0 - da;
            if (da > 0x1F) {
                da = 0x1F;
            }
        } else if (da > 0) {
            da += 0x20;
            if (da > 0x3F)
                da = 0x3F;
        } else {
            da = 0x20;
        }

        cmd[7] = (byte) da;        //平移

        if (JH_App.bUp) {
            cmd[7] |= 0x40;
        }
        if (JH_App.bHeadLess) {
            cmd[7] |= 0x80;
        }

        cmd[8] = 0;

        if (JH_App.bStop) {
            cmd[8] |= 0x10;
        }
        if (JH_App.bAdj) {
            cmd[8] |= 0x20;
        }

        if (JH_App.bDn) {
            cmd[8] |= 0x08;
        }
        cmd[9] = (byte) (((cmd[0] ^ cmd[1] ^ cmd[2] ^ cmd[3] ^ cmd[4] ^ cmd[5] ^ cmd[6] ^ cmd[7] ^ cmd[8]) & 0xFF) + 0x55);
        wifination.naSentCmd(cmd, 10);
        //Log.e("Cmd:  ","Sent NromalComd  X1=" +X1+" Y1="+Y1+" X2="+X2+" Y2="+Y2);


        /*

        int da = X_ADJ1 - 0x80;          //旋转微调
        da += 32;
        if (da < 0)
            da = 0;
        if (da > 63) {
            da = 63;
        }
        X_ADJ1 = da;

        da = X_ADJ2 - 0x80;          //
        da += 32;
        if (da < 0)
            da = 0;
        if (da > 63) {
            da = 63;
        }
        X_ADJ2 = da;

        da = Y_ADJ2 - 0x80;          //
        da += 32;
        if (da < 0)
            da = 0;
        if (da > 63) {
            da = 63;
        }
        Y_ADJ2 = da;


        if (X1 > (0x80 - 0x25) && X1 < (0x80 + 0x25)) {
            X1 = 0x80;
        }


        if (Y2 > 0x80) {
            Y2 -= 0x80;

        } else if (Y2 < 0x80) {
            Y2 = 0x80 - Y2;
            Y2 += 0x80;
            if (Y2 > 0xFF) {
                Y2 = 0xFF;
            }
        }

        if (X1 > 0x80) {
        } else if (X1 < 0x80) {
            X1 = 0x80 - X1;
            if (X1 > 0x7F) {
                X1 = 0x7F;
            }
        }

        if (X2 > 0x80) {
        } else if (X2 < 0x80) {
            X2 = 0x80 - X2;
            if (X2 > 0x7F) {
                X2 = 0x7F;
            }
        }


        fly_cmd.Thro = Y1;
        fly_cmd.Yaw = X1;

        fly_cmd.Pitch = Y2;
        fly_cmd.Roll = X2;

        fly_cmd.TrimYaw = X_ADJ1;

        fly_cmd.TrimPitch = Y_ADJ2;
        fly_cmd.TrimRoll = X_ADJ2;


        fly_cmd.CFMode = 0;
        fly_cmd.FastMode = 0;
        fly_cmd.GpsMode = 0;
        fly_cmd.LevelCor = 0;

        fly_cmd.AutoTakeoff = 0;
        fly_cmd.AutoLand = 0;
        fly_cmd.Stop = 0;

        if (JH_App.bHiSpeed) {
            fly_cmd.FastMode = 02;
        }

        if (JH_App.bGPSMode) {
            fly_cmd.GpsMode = 1;
        }
        if (JH_App.bAdj) {
            fly_cmd.LevelCor = 1;
        }
        if (JH_App.bDn) {
            fly_cmd.AutoLand = 1;
        }

        if (JH_App.bUp) {
            fly_cmd.AutoTakeoff = 1;
        }
        if (fly_cmd.AutoLand != 0) {
            fly_cmd.AutoTakeoff = 0;
        }
        if (JH_App.bStop) {
            fly_cmd.Stop = 1;
        }
        wifination.naFillFlyCmdByC(1);
        // wifination.naSentCmd(fly_cmd.cmd, 15);
        //  Log.e("SentCmd","Sent OK!");
        */
    }
}
