package com.example.core;

import android.Manifest;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.widget.ImageView;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.android.Utils;
import org.opencv.core.Mat;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.net.URL;
import java.net.URLConnection;

import static android.os.Environment.getExternalStorageState;
import static java.lang.Boolean.TRUE;
import static java.lang.Math.PI;
import static java.lang.Math.acos;
import static java.lang.Math.asin;
import static java.lang.Math.atan;
import static java.lang.Math.cos;
import static java.lang.Math.sin;
import static java.lang.StrictMath.abs;


public class MainActivity extends AppCompatActivity {
    int chunk2display=1;
    volatile int yes2DL=0;
    volatile int yes2PL=0;
    volatile int dlFinished=0;
    volatile int totalDlChunk=0;
    volatile int totalPlChunk=0;
    long   chunk2loadFile=1L;
    volatile int pan=0;
    volatile  int dlChunkPan=0;
    volatile  int dlChunkPan1=0;
    volatile int lastChunkReqPan=0;
    volatile int totalPan=0;

    private static final String TAG = "OCVSample::Activity";
    private BaseLoaderCallback _baseLoaderCallback = new BaseLoaderCallback(this) {
        @Override
        public void onManagerConnected(int status) {
            switch (status) {
                case LoaderCallbackInterface.SUCCESS: {
                    Log.i(TAG, "OpenCV loaded successfully");
                    // Load ndk built module, as specified in moduleName in build.gradle               // after opencv initialization
                    System.loadLibrary("native-lib");
                }
                break;
                default: {
                    super.onManagerConnected(status);
                }
            }
        }
    };


    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        ActivityCompat.requestPermissions(MainActivity.this,
                new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE},
                1);

    }




    @Override
    public void onResume() {
        super.onResume();
        if (!OpenCVLoader.initDebug()) {
            Log.d(TAG, "Internal OpenCV library not found. Using OpenCV Manager for initialization");
            OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_3_0_0, this, _baseLoaderCallback);
        } else {
            Log.d(TAG, "OpenCV library found inside package. Using it!");
            _baseLoaderCallback.onManagerConnected(LoaderCallbackInterface.SUCCESS);
        }

    }


    @Override
    public void onPause() {
        super.onPause();

    }

    private class MyTask extends AsyncTask<Long, Void, Void> {

        // Runs in UI before background thread is called
        @Override
        protected void onPreExecute() {
            super.onPreExecute();
            // Do something like display a progress bar
        }

        // This is run in a background thread
        @Override
        protected Void doInBackground(Long... param) {
            Long addr= param[0];
            long x=param[1];
            long p=param[2];
           // int pan=(int)p;
            int chunk2load=(int)x;
            while(true)
            {
                if(yes2DL==1)
                {
                    String videoPath=downloadFileHttp(chunk2load, totalPan);
                    int xx=pan;
                    dlFinished=loadVideoFromDevice(addr, videoPath, chunk2load);
                    System.out.println("dl finished.total DL:..................................................................."+chunk2load);
                    totalDlChunk=totalDlChunk+1;
                    yes2DL=0;
                    yes2PL=1;
                    chunk2load=chunk2load+1;
                   dlChunkPan1=dlChunkPan;
                }
            }

        }
    }


    @Override
    public void onRequestPermissionsResult(int requestCode, String permissions[], int[] grantResults) {
        switch (requestCode) {
            case 1: {

                if (grantResults.length > 0
                        && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    initCoREparameters();
                    System.out.println("CoRE param updated..........................................>>>>");

                    yes2DL=1;
                    dlThread();
                    while(totalPlChunk>=totalDlChunk)//totalDlChunk<=totalPlChunk
                    {  }
                    playThread();


                } else {
                    // permission denied, boo! Disable the functionality that depends on this permission.
                    Toast.makeText(MainActivity.this, "Permission denied to read your External storage", Toast.LENGTH_SHORT).show();
                }
                return;
            }

        }
    }

    public void dlThread()
    {

                Mat m = new Mat();
                long cameraPan = totalPan;
                new MyTask().execute(m.getNativeObjAddr(), chunk2loadFile, cameraPan);//calling load video from device using videocapture in background

    }

    public void playThread()
    {

        System.out.println("here..");
        final Handler handler = new Handler();
        handler.postDelayed(new Runnable(){
            Long start = System.currentTimeMillis();
            int ia=0;
            Mat m1=new Mat();
            Long FirstStart = System.currentTimeMillis();
            int dlChunkPan1xx=0;
            public void run()
                    {
                        int lastPan=0;

                        ImageView iv = (ImageView) findViewById(R.id.imageView);
                        iv.invalidate();
                        Bitmap bm;
                        iv.setOnTouchListener(new View.OnTouchListener(){
                            @Override
                            public boolean onTouch(View v, MotionEvent event) {
                                final float x = event.getX();
                                final float y = event.getY();
                                float lastXAxis = x;
                                float lastYAxis = y;
                                int add=5;
                                System.out.println("touch.............x:>: "+ x+" y:"+ y);
                                if (x>1200)
                                {
                                    totalPan=(totalPan+add);
                                }
                                else
                                {
                                    totalPan=(totalPan-add);
                                }

                                Toast.makeText(MainActivity.this, "touch..................", Toast.LENGTH_SHORT).show();
                                return true;
                            }
                        });
                        Long current = System.currentTimeMillis();
                        long playTime=current-start;
                        long i=0;
                        while(playTime<20)
                        {   current = System.currentTimeMillis();
                            playTime=current-start;

                        }
                        start=current;
                        long frameTime=current-FirstStart;

                        if(ia==0)
                            {
                                dlChunkPan1xx=lastChunkReqPan;
                            }
                        int cameraPan=totalPan-dlChunkPan1xx;


                        CoREoperationPerFrame(m1.getNativeObjAddr(), ia, chunk2display, cameraPan,dlChunkPan1xx ); // ia increaseas and one after another frame comses out
                        bm = Bitmap.createBitmap(m1.cols(), m1.rows(), Bitmap.Config.ARGB_8888);
                        Utils.matToBitmap(m1, bm);

                        iv.setImageBitmap(bm);
                        handler.postDelayed(this, 1);
                        if (ia==119)
                        {
                            chunk2display=chunk2display+1;
                            totalPlChunk=totalPlChunk+1;
                            while(yes2PL==0)
                            {
                            }
                            System.out.println("ended...............................................................................................................");
                            ia=-1;


                        }

                        if (ia==60)
                        {
                            yes2DL=1;
                            yes2PL=0;
                        }
                        ia++;
                    }
                }, 2);

    }


    public String downloadFileHttp(int chunkN, int pan)
    {
        String fPath="";
        try
        {
           // String sourceBaseAddr="http://10.0.2.2:80/3vid2crf3trace/android/tilt0/";
            String sourceBaseAddr="http://192.168.43.179:80/3vid2crf3trace/android/tilt0/";
            String result="30_rhino.webm4_"+getFileName2Req(sourceBaseAddr,chunkN, totalPan);
           // String result="30_rhino.AVI6_"+getFileName2Req(sourceBaseAddr,chunkN, pan);
            //URL url = new URL("http://128.10.120.226:80/video1.mp4");
            String name=sourceBaseAddr+result;
            //String name1=sourceBaseAddr+"diving_1_10_0.mp4";
            URL url = new URL(name);
            String fname=result;
           // Toast.makeText(MainActivity.this, result, Toast.LENGTH_SHORT).show();
            System.out.println("requested file name................................>>>"+ name+ " "+chunkN);
            //System.out.println("path "+Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS));

            URLConnection ucon = url.openConnection();
            ucon.setReadTimeout(50000);
            ucon.setConnectTimeout(100000);
            InputStream is = ucon.getInputStream();
            BufferedInputStream inStream = new BufferedInputStream(is, 1024 * 500);
            File file = new File("/storage/emulated/0/Download/" + fname);
            fPath=file.getPath();
          // fPath=Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS)+"/"+ fname;
            //fPath="/storage/emulated/0/Download/diving_1_10_40.mp4";

            if (!file.exists())
            {

                file.createNewFile();
                FileOutputStream outStream = new FileOutputStream(file);
                byte[] buff = new byte[500 * 1024];

                int len;
                while ((len = inStream.read(buff)) != -1) {
                    outStream.write(buff, 0, len);
                }

                outStream.flush();
                outStream.close();
                inStream.close();
            }

            System.out.println("File saved.......>>>"+fPath);
        }
        catch (Exception e)
        {
            e.printStackTrace();
            System.out.println("Cant save file.......>>>"+fPath);
            System.exit(1);
        }

        return fPath;
    }

    public String getFileName2Req(String srcBaseAddr, int chunkN, int pan)
    {

       // lastChunkReqPan=pan;
        int panTemp=-185;
        int panAngle=5;
        while ((int) pan > panTemp) {
            panTemp = panTemp + panAngle;
        }
        int reqpann = panTemp;
        lastChunkReqPan=panTemp;
        reqpann=reqpann%360;

        if (reqpann>180)
        {
            reqpann=reqpann-360;
        }
        if (reqpann<-180)
        {
            reqpann=360+reqpann;
        }

        int reqtilt=0;
        String result= chunkN + "_" + reqtilt + "_" + reqpann + ".avi.mp4";

        return result;
    }

    public native void initCoREparameters();
    public native int loadVideoFromDevice(long addr, String videoPath, int chunkN);
    public native void CoREoperationPerFrame(long addr, int fi, int chunkN, int cameraPan, int baseAngle);
}
