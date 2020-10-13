/*
 * Copyright 2020 Xu Ruijun
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
// copy from OpenCV Sample tutorial1 camera preview
package com.example.sh3dscaner;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.CameraActivity;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewFrame;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.core.CvType;
import org.opencv.core.Mat;
import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewListener2;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Rect;
import android.os.Bundle;
import android.os.Looper;
import android.util.Log;
import android.view.MenuItem;
import android.view.SurfaceView;
import android.view.TextureView;
import android.view.WindowManager;
import android.widget.TextView;
import android.widget.Toast;

import java.util.Collections;
import java.util.List;
import java.util.concurrent.Semaphore;
import org.opencv.android.Utils;

import static com.example.sh3dscaner.ImageProcess.*;

public class OpenCVTest extends CameraActivity implements CvCameraViewListener2 {
    private static final String TAG = "OCVSample::Activity";

    private CameraBridgeViewBase mOpenCvCameraView;
    private TextView ProcessTime_TextView;
    private boolean              mIsJavaCamera = true;
    private MenuItem             mItemSwitchCamera = null;
    private Mat img_rgb;
    private Mat out_mat;
    private Bitmap bmp;
    private String pt_str;
    private Status imp_status = new Status();
    private int N_frames=0;
    private Semaphore mSemp = new Semaphore(1);
    private TextureView fft_view;
    private Rect mRect;
    private Paint mPaint;
    private boolean draw_thread_is_running=true;
    private Thread thread_draw;

    private BaseLoaderCallback mLoaderCallback = new BaseLoaderCallback(this) {
        @Override
        public void onManagerConnected(int status) {
            if (status == LoaderCallbackInterface.SUCCESS) {
                Log.i(TAG, "OpenCV loaded successfully");
                mOpenCvCameraView.enableView();
            } else {
                super.onManagerConnected(status);
            }
        }
    };

    Runnable draw_bmp  = new Runnable() {
        @Override
        public void run() {
            while(draw_thread_is_running) {
                try {
                    mSemp.acquire();
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                Canvas mCanvas = fft_view.lockCanvas();
                if(mCanvas != null) {
                    mCanvas.drawBitmap(bmp, null, mRect, mPaint);
                    fft_view.unlockCanvasAndPost(mCanvas);
                }
            }
        }
    };

    public OpenCVTest() {
        Log.i(TAG, "Instantiated new " + this.getClass());
    }

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        Log.i(TAG, "called onCreate");
        super.onCreate(savedInstanceState);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        setContentView(R.layout.opencv_test);
        mOpenCvCameraView = findViewById(R.id.tutorial1_activity_java_surface_view);
        fft_view = findViewById(R.id.fft_view);
        mOpenCvCameraView.setVisibility(SurfaceView.VISIBLE);
        mOpenCvCameraView.setCvCameraViewListener(this);
        ProcessTime_TextView = findViewById(R.id.process_time);
    }

    @Override
    public void onPause()
    {
        super.onPause();
        if (mOpenCvCameraView != null)
            mOpenCvCameraView.disableView();
    }

    @Override
    public void onResume()
    {
        super.onResume();
        if (!OpenCVLoader.initDebug()) {
            Log.d(TAG, "Internal OpenCV library not found. Using OpenCV Manager for initialization");
            OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_3_0_0, this, mLoaderCallback);
        } else {
            Log.d(TAG, "OpenCV library found inside package. Using it!");
            mLoaderCallback.onManagerConnected(LoaderCallbackInterface.SUCCESS);
        }
    }

    @Override
    protected List<? extends CameraBridgeViewBase> getCameraViewList() {
        return Collections.singletonList(mOpenCvCameraView);
    }

    public void onDestroy() {
        super.onDestroy();
        if (mOpenCvCameraView != null)
            mOpenCvCameraView.disableView();
    }

    public void onCameraViewStarted(int width, int height) {
        //OptFlow_init(height, width);
        img_rgb = new Mat(height, width, CvType.CV_8UC4);
        out_mat = new Mat(64,64, CvType.CV_8UC1);
        bmp = Bitmap.createBitmap(64, 64, Bitmap.Config.ARGB_8888); //创建与输出尺寸相同的Bitmap
        optflow_FFT_init(64);
        mRect = new Rect(0,0, bmp.getWidth(), bmp.getHeight());
        mPaint = new Paint();
        thread_draw = new Thread(draw_bmp);
        thread_draw.start();
    }

    public void onCameraViewStopped() {
        img_rgb.release();
    }

    public Mat onCameraFrame(CvCameraViewFrame inputFrame) {
        img_rgb = inputFrame.rgba();

        //OptFlow_LK(img_rgb.getNativeObjAddr(), imp_status);
        optflow_FFT_update(img_rgb.getNativeObjAddr(), out_mat.getNativeObjAddr(), imp_status);
        Utils.matToBitmap(out_mat, bmp);
        if (mSemp.availablePermits() == 0) {
            mSemp.release();
        }else{
            Log.e(TAG, "mSemp.availablePermits() != 0");
        }

        if(N_frames%30 == 1) {
            double pt = imp_status.process_time / 1e9;
            pt_str = String.format("proc% 4.1fms, use% 4.1f%%",
                    pt * 1000, pt * 30.0 * 100);
            Log.i(TAG, pt_str);
            runOnUiThread(showPT_runnable);
        }
        N_frames++;
        return img_rgb;
    }

    Runnable showPT_runnable = new Runnable() {
        @Override
        public void run() {
            ProcessTime_TextView.setText(pt_str);
        }
    };
}