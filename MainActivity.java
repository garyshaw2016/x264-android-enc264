package com.warningsys.enc264;

import java.io.File;
import java.util.List;

import android.app.Activity;
import android.content.res.Configuration;
import android.graphics.PixelFormat;
import android.hardware.Camera;
import android.hardware.Camera.Parameters;
import android.hardware.Camera.Size;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.Menu;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.widget.Toast;

import com.warningenc.nativecaller.NativeCaller;
import com.warningsys.live.R;

public class MainActivity extends Activity implements android.view.SurfaceHolder.Callback {
 
	
	private static final String TAG = "ssss";
	
	// 定义对象
	 private SurfaceView mSurfaceview = null;  // SurfaceView对象：(视图组件)视频显示 
	 private SurfaceHolder mSurfaceHolder = null;  // SurfaceHolder对象：(抽象接口)SurfaceView支持类 
	 private Camera mCamera =null;     // Camera对象，相机预览
	 
	 boolean bIfPreview = false;
	 int mPreviewWidth = 1280 , mPreviewHeight= 720;
	 
	 int Framerate4me = 25;
	 
	 
	 private Camera.AutoFocusCallback mAutoFocusCallBack;
	 private Camera.PreviewCallback previewCallback;
	 
	 private Camera.PreviewCallback mJpegPreviewCallback;
	 private Camera.AutoFocusCallback mAutoFocusCallback;
	 
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		NativeCaller.InitEnc(mPreviewWidth, mPreviewHeight);
		initSurfaceView();
		File sdDir = null;
		boolean sdCardExist = Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED); // 判断sd卡是否存在
		if (sdCardExist) {
			sdDir = Environment.getExternalStorageDirectory();// 获取跟目录
		}
		
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

	
	
	
	
	
	@Override
	protected void onDestroy() {
		// TODO Auto-generated method stub
		
		NativeCaller.DeiniEnc();
		super.onDestroy();
	}

	@Override
	public void finish() {
		// TODO Auto-generated method stub
		super.finish();
	}
 
	// InitSurfaceView
	 private void initSurfaceView()
	 {
	  mSurfaceview = (SurfaceView) this.findViewById(R.id.surfaceView1);
	  mSurfaceHolder = mSurfaceview.getHolder(); // 绑定SurfaceView，取得SurfaceHolder对象
	  mSurfaceHolder.addCallback(MainActivity.this); // SurfaceHolder加入回调接口
	  // mSurfaceHolder.setFixedSize(176, 144); // 预览大小設置
	  mSurfaceHolder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);// 設置顯示器類型，setType必须设置
	  
	  
	  
	  mAutoFocusCallBack = new Camera.AutoFocusCallback() {
          @Override
          public void onAutoFocus(boolean success, Camera camera) {
              if (success) {
                  // isAutoFocus = true;
                  camera.setOneShotPreviewCallback(previewCallback);
                  Log.d(TAG, "onAutoFocus success");
              }
          }
      };

//      previewCallback = new Camera.PreviewCallback() {
//          @Override
//          public void onPreviewFrame(byte[] data, Camera arg1) {
//              if (data != null)
//              {
//                  Camera.Parameters parameters = mCamera.getParameters();
//                  int imageFormat = parameters.getPreviewFormat();
//                  Log.i("map", "Image Format: " + imageFormat);
//
//                  Log.i("CameraPreviewCallback", "data length:" + data.length);
//                  if (imageFormat == ImageFormat.NV21)
//                  {
//                      // get full picture
//                      Bitmap image = null;
//                      int w = parameters.getPreviewSize().width;
//                      int h = parameters.getPreviewSize().height;
//                        
//                      Rect rect = new Rect(0, 0, w, h); 
//                      YuvImage img = new YuvImage(data, ImageFormat.NV21, w, h, null);
//                      ByteArrayOutputStream baos = new ByteArrayOutputStream();
//                      if (img.compressToJpeg(rect, 100, baos)) 
//                      { 
//                          image =  BitmapFactory.decodeByteArray(baos.toByteArray(), 0, baos.size());
//                          imageView.setImageBitmap(image);
//                      }
//              
//                  }
//              }
//          }
//      };


    	   
    	   
      mJpegPreviewCallback = new Camera.PreviewCallback()
      {
       @Override
       public void onPreviewFrame(byte[] data, Camera camera) 
       {
        //传递进来的data,默认是YUV420SP的
    // TODO Auto-generated method stub    
        try
        {
         Log.i(TAG, "PreviewCallback  data.length ="+data.length);
         Log.i(TAG, "SurfaceHolder.PreviewCallback  data.length = "+data.length);
         NativeCaller.InputData(0, data, data.length);

         
        } catch (Exception e)
        {
         Log.v("System.out", e.toString());
        }// endtry    
       }// endonPriview  
      };
      
      
      
      
	 }
	 
	 
	 //======================================================================================================
	 /*【SurfaceHolder.Callback 回调函数】*/
	// SurfaceView启动时/初次实例化，预览界面被创建时，该方法被调用。
	 public void surfaceCreated(SurfaceHolder holder)
	 {
	  // TODO Auto-generated method stub  
	  mCamera = Camera.open();// 开启摄像头（2.3版本后支持多摄像头,需传入参数） 
	  try
	  {   
	   Log.i(TAG, "SurfaceHolder.Callback：surface Created");
	   mCamera.setPreviewDisplay(mSurfaceHolder);//set the surface to be used for live preview

	  } catch (Exception ex)
	  {
	   if(null != mCamera)
	   {
	    mCamera.release();
	    mCamera = null;     
	   }
	   Log.i(TAG+"initCamera", ex.getMessage());
	  }
	 }
	 
	// 当SurfaceView/预览界面的格式和大小发生改变时，该方法被调用
	 public void surfaceChanged(SurfaceHolder holder, int format, int width, int height)
	 {
	  // TODO Auto-generated method stub  
	  Log.i(TAG, "SurfaceHolder.Callback：Surface Changed");
	  //mPreviewHeight = height;
	  //mPreviewWidth = width;
	  initCamera();  
	 }
	 public void surfaceDestroyed(SurfaceHolder holder) 
	 // SurfaceView销毁时，该方法被调用
	 {
	  // TODO Auto-generated method stub
	  Log.i(TAG, "SurfaceHolder.Callback：Surface Destroyed");
	  if(null != mCamera)
	  {
	   mCamera.setPreviewCallback(null); //！！这个必须在前，不然退出出错
	   mCamera.stopPreview(); 
	   bIfPreview = false; 
	   mCamera.release();
	   mCamera = null;     
	  }
	 }
	//======================================================================================================
	 
	 
	 
	 /*【2】【相机预览】*/
	 private void initCamera()//surfaceChanged中调用
	 {
	  Log.i(TAG, "going into initCamera");
	  if (bIfPreview)
	  {
	   mCamera.stopPreview();//stopCamera();
	  }
	  if(null != mCamera)
	  {
	   try
	   {
	    /* Camera Service settings*/    
	    Camera.Parameters parameters = mCamera.getParameters();
	    // parameters.setFlashMode("off"); // 无闪光灯
	    parameters.setPictureFormat(PixelFormat.JPEG); //Sets the image format for picture 设定相片格式为JPEG，默认为NV21    
	    parameters.setPreviewFormat(PixelFormat.YCbCr_420_SP); //Sets the image format for preview picture，默认为NV21
	    /*【ImageFormat】JPEG/NV16(YCrCb format，used for Video)/NV21(YCrCb format，used for Image)/RGB_565/YUY2/YU12*/
	    
	    // 【调试】获取caera支持的PictrueSize，看看能否设置？？
	    List<Size> pictureSizes = mCamera.getParameters().getSupportedPictureSizes();
	    List<Size> previewSizes = mCamera.getParameters().getSupportedPreviewSizes();
	    List<Integer> previewFormats = mCamera.getParameters().getSupportedPreviewFormats();
	    List<Integer> previewFrameRates = mCamera.getParameters().getSupportedPreviewFrameRates();
	    Log.i(TAG+"initCamera", "cyy support parameters is ");
	    Size psize = null;
	    for (int i = 0; i < pictureSizes.size(); i++)
	    {
	     psize = pictureSizes.get(i);
	     Log.i(TAG+"initCamera", "PictrueSize,width: " + psize.width + " height" + psize.height);
	    }
	    for (int i = 0; i < previewSizes.size(); i++)
	    {
	     psize = previewSizes.get(i);
	     Log.i(TAG+"initCamera", "PreviewSize,width: " + psize.width + " height" + psize.height);
	    }
	    Integer pf = null;
	    for (int i = 0; i < previewFormats.size(); i++)
	    {
	     pf = previewFormats.get(i);
	     Log.i(TAG+"initCamera", "previewformates:" + pf);
	    }
	    
	    
	 List<int[]> range=parameters.getSupportedPreviewFpsRange(); 
	 Log.d(TAG, "range:"+range.size()); 
	 for(int j=0;j<range.size();j++) { 
		 int[] r=range.get(j); 
		 for(int k=0;k<r.length;k++) 
		 { 
			 Log.d(TAG, "SupportedPreviewFpsRange" + r[k]); 
		} 
		 } 
	 //parameters.setPreviewFpsRange (35*1000,35*1000);
	 parameters.setPreviewFrameRate(Framerate4me);
	    // 设置拍照和预览图片大小
	    parameters.setPictureSize(mPreviewWidth, mPreviewHeight); //指定拍照图片的大小
	    parameters.setPreviewSize(mPreviewWidth, mPreviewHeight); // 指定preview的大小 
	//这两个属性 如果这两个属性设置的和真实手机的不一样时，就会报错
	    
	     

	    
	// 横竖屏镜头自动调整
	    if (this.getResources().getConfiguration().orientation != Configuration.ORIENTATION_LANDSCAPE) 
	    {
	     parameters.set("orientation", "portrait"); //
	     parameters.set("rotation", 90); // 镜头角度转90度（默认摄像头是横拍） 
	     mCamera.setDisplayOrientation(90); // 在2.2以上可以使用
	    } else// 如果是横屏
	    {
	     parameters.set("orientation", "landscape"); //
	     mCamera.setDisplayOrientation(0); // 在2.2以上可以使用
	    } 
	    
	    /* 视频流编码处理 */ 
	    //添加对视频流处理函数
	    
	    
	// 设定配置参数并开启预览
	    //mCamera.setParameters(parameters); // 将Camera.Parameters设定予Camera    
	    //mCamera.startPreview(); // 打开预览画面
	    
	    //mCamera.autoFocus(mAutoFocusCallback);//自动聚焦
	    
	    
	    mCamera.cancelAutoFocus();
        if (parameters.getFocusMode() != Camera.Parameters.FOCUS_MODE_AUTO) {
            parameters.setFocusMode(Parameters.FOCUS_MODE_AUTO);
        }
//        if (parameters.getMaxNumFocusAreas() > 0) {
//            List<Area> mylist = new ArrayList<Area>();
//            mylist.add(new Camera.Area(focusRect, 1000));
//            parameters.setFocusAreas(mylist);
//        }
        
        try {
        	mCamera.cancelAutoFocus();
        	mCamera.setParameters(parameters);
        	mCamera.startPreview();
        	//mCamera.setPreviewCallbackWithBuffer(mJpegPreviewCallback);
        	
        	mCamera.setPreviewCallback(mJpegPreviewCallback);
        	mCamera.autoFocus(new Camera.AutoFocusCallback() {

				@Override
				public void onAutoFocus(boolean arg0, Camera arg1) {
					// TODO Auto-generated method stub
					if (mCamera.getParameters().getFocusMode() != Camera.Parameters.FOCUS_MODE_CONTINUOUS_PICTURE) {
                        Parameters parameters = mCamera.getParameters();
                        parameters.setFocusMode(Camera.Parameters.FOCUS_MODE_CONTINUOUS_PICTURE);
                        if (parameters.getMaxNumFocusAreas() > 0) {
                            parameters.setFocusAreas(null);
                        }
                        mCamera.setParameters(parameters);
                        mCamera.startPreview();
                    }

		    	     if(arg0){
		       	     // mCamera.setOneShotPreviewCallback(null);
		       	         //Toast.makeText(MainActivity.this,
		       	           //"自动聚焦成功" , Toast.LENGTH_SHORT).show();
		       	     }
		    	     
				}

            });
        } catch (Exception e) {
            e.printStackTrace();
        }

        
//        if (parameters.getMaxNumFocusAreas() > 0) {
//            List<Area> mylist = new ArrayList<Area>();
//            mylist.add(new Camera.Area(focusRect, 1000));
//            parameters.setFocusAreas(mylist);
//        }
        
        
	    
//	    parameters.setFocusMode(Camera.Parameters.FOCUS_MODE_CONTINUOUS_PICTURE); 
//	    
//	    if (parameters.getFocusMode() != Camera.Parameters.FOCUS_MODE_AUTO) {
//            parameters.setFocusMode(Camera.Parameters.FOCUS_MODE_AUTO);
//        }
	    
	    
	    
	    bIfPreview = true;
	    
	    // 【调试】设置后的图片大小和预览大小以及帧率
	    Camera.Size csize = mCamera.getParameters().getPreviewSize();
	    mPreviewHeight = csize.height; //
	    mPreviewWidth = csize.width;
	    Log.i(TAG+"initCamera", "after setting, previewSize:width: " + csize.width + " height: " + csize.height);
	    csize = mCamera.getParameters().getPictureSize();
	    Log.i(TAG+"initCamera", "after setting, pictruesize:width: " + csize.width + " height: " + csize.height);
	    Log.i(TAG+"initCamera", "after setting, previewformate is " + mCamera.getParameters().getPreviewFormat());
	    Log.i(TAG+"initCamera", "after setting, previewframetate is " + mCamera.getParameters().getPreviewFrameRate());
	   } catch (Exception e)
	   { 
	    e.printStackTrace();
	   }
	  }
	 }
	 
	 
//	// 【获取视频预览帧的接口】
//	  mJpegPreviewCallback = new Camera.PreviewCallback()
//	  {
//	   @Override
//	   public void onPreviewFrame(byte[] data, Camera camera) 
//	   {
//	    //传递进来的data,默认是YUV420SP的
//	// TODO Auto-generated method stub    
//	    try
//	    {
//	     Log.i(TAG, "going into onPreviewFrame");
//	     //mYUV420sp = data;   // 获取原生的YUV420SP数据
//	     YUVIMGLEN = data.length;
//	     
//	     // 拷贝原生yuv420sp数据
//	     mYuvBufferlock.acquire();
//	     System.arraycopy(data, 0, mYUV420SPSendBuffer, 0, data.length);
//	     //System.arraycopy(data, 0, mWrtieBuffer, 0, data.length);
//	     mYuvBufferlock.release();
//	     
//	     // 开启编码线程，如开启PEG编码方式线程
//	     mSendThread1.start();
//	     
//	    } catch (Exception e)
//	    {
//	     Log.v("System.out", e.toString());
//	    }// endtry    
//	   }// endonPriview  
//	  };
}
