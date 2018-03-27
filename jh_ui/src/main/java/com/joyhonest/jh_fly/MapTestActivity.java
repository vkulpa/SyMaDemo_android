package com.joyhonest.jh_fly;

import android.location.Location;
import android.os.Handler;
import android.os.RemoteException;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;


import com.amap.api.maps.AMap;
import com.amap.api.maps.AMapException;
import com.amap.api.maps.CameraUpdate;
import com.amap.api.maps.CameraUpdateFactory;
import com.amap.api.maps.CoordinateConverter;
import com.amap.api.maps.MapView;
import com.amap.api.maps.MapsInitializer;
import com.amap.api.maps.model.BitmapDescriptorFactory;
import com.amap.api.maps.model.CameraPosition;
import com.amap.api.maps.model.LatLng;
import com.amap.api.maps.model.MarkerOptions;
import com.amap.api.maps.model.MyLocationStyle;
import com.amap.api.maps.offlinemap.OfflineMapManager;
import com.joyhonest.jh_ui.JH_App;
import com.joyhonest.jh_ui.R;

import org.simple.eventbus.EventBus;
import org.simple.eventbus.Subscriber;

public class MapTestActivity extends AppCompatActivity implements OfflineMapManager.OfflineMapDownloadListener {

    OfflineMapManager amapManager;
    MapView mMapView = null;
    MyLocationStyle myLocationStyle;
    AMap aMap = null;

    private final static LatLng SYDNEY = new LatLng(-33.86759, 151.2088);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_map_test);
        JH_App.nType = JH_App.nStyle_fly;
        EventBus.getDefault().register(this);

        try {
            MapsInitializer.initialize(this.getApplicationContext());
        } catch (RemoteException e) {

        }

        MapsInitializer.loadWorldGridMap(true);

        mMapView = (MapView) findViewById(R.id.map);
        mMapView.onCreate(savedInstanceState);
        InitMap();
        JH_App.openGPSSettings();

    }

    private void InitMap() {
        if (aMap == null) {
            aMap = mMapView.getMap();
            if (aMap == null)
                return;
        }

        //构造OfflineMapManager对象
        amapManager = new OfflineMapManager(this, this);
//按照citycode下载
        //amapManager.downloadByCityCode(String citycode)；
//按照cityname下载
        try {
            amapManager.downloadByCityName("shenzhen");
            amapManager.downloadByCityName("shantou");
        } catch (AMapException e) {

        }

        /*
        MapsInitializer.loadWorldGridMap(true);
        aMap.moveCamera(CameraUpdateFactory.newLatLngZoom(SYDNEY, 14));
        //CameraUpdate cameraUpdate = CameraUpdateFactory.zoomTo(14);
        //aMap.moveCamera(cameraUpdate);
        //aMap.setMapType(AMap.MAP_TYPE_NORMAL);
        //aMap.setMapLanguage(AMap.CHINESE);

        myLocationStyle = new MyLocationStyle();//初始化定位蓝点样式类
        myLocationStyle.myLocationType(MyLocationStyle.LOCATION_TYPE_LOCATE);//连续定位、且将视角移动到地图中心点，定位点依照设备方向旋转，并且会跟随设备移动。（1秒1次定位）如果不设置myLocationType，默认也会执行此种模式。
        myLocationStyle.interval(1000); //设置连续定位模式下的定位间隔，只在连续定位模式下生效，单次定位模式下不会生效。单位为毫秒。
        aMap.setMyLocationStyle(myLocationStyle);//设置定位蓝点的Style
        myLocationStyle.showMyLocation(true);

        aMap.getUiSettings().setMyLocationButtonEnabled(true);//设置默认定位按钮是否显示，非必需设置。
        aMap.setMyLocationEnabled(true);// 设置为true表示启动显示定位蓝点，false表示隐藏定位蓝点并不进行定位，默认是false。
        aMap.setOnMyLocationChangeListener(this);
        */

    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        //在activity执行onDestroy时执行mMapView.onDestroy()，销毁地图
        amapManager.stop();
        aMap.setMyLocationEnabled(false);
        mMapView.onDestroy();
        EventBus.getDefault().unregister(this);
    }

    @Override
    protected void onResume() {
        super.onResume();
        amapManager.restart();
        //在activity执行onResume时执行mMapView.onResume ()，重新绘制加载地图
        mMapView.onResume();
    }

    @Override
    protected void onPause() {
        super.onPause();
        //在activity执行onPause时执行mMapView.onPause ()，暂停地图的绘制
        amapManager.pause();
        mMapView.onPause();
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        //在activity执行onSaveInstanceState时执行mMapView.onSaveInstanceState (outState)，保存地图当前的状态
        mMapView.onSaveInstanceState(outState);
    }


    private void changeCamera(CameraUpdate update) {
        aMap.moveCamera(update);
    }


    @Override
    public void onDownload(int i, int i1, String s) {
        Log.e("offline:", "Donload OK==" + s);
    }

    @Override
    public void onCheckUpdate(boolean b, String s) {

    }

    @Override
    public void onRemove(boolean b, String s, String s1) {
    }

    @Subscriber(tag = "GPS_LocationChanged")
    private void GPS_LocationChanged(Location location) {
        if (location.getLatitude() < -300 || location.getLongitude() < -300)
            return;
        LatLng latlng = new LatLng(location.getLatitude(), location.getLongitude());
        CoordinateConverter converter = new CoordinateConverter(this);
        // CoordType.GPS 待转换坐标类型
        CoordinateConverter.CoordType mcoordtype = CoordinateConverter.CoordType.valueOf("GPS");
        converter.from(mcoordtype);
        // sourceLatLng待转换坐标点
        converter.coord(latlng);
        // 执行转换操作
        LatLng desLatLng = converter.convert();
        changeCamera(CameraUpdateFactory.newCameraPosition(new CameraPosition(
                desLatLng, 18, 0, 0)));
        aMap.clear();
        aMap.addMarker(new MarkerOptions().position(desLatLng)
                .icon(BitmapDescriptorFactory
                        .defaultMarker(BitmapDescriptorFactory.HUE_BLUE)));
    }
}
