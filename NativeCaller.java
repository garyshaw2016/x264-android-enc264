package com.warningenc.nativecaller;

import android.content.Context;

public class NativeCaller {
	/** Called when the activity is first created. */
	static {
		System.loadLibrary("crypto");
		System.loadLibrary("ssl");

		System.loadLibrary("enc264");
	}

	private static final String LOG_TAG = "NativeCaller";

//	enum emEnc264Type
//	{
//		//video
//		ENCODE_TYPE_VIDEO_264 = 0,
//		//audio
//		ENCODE_TYPE_AUDIO_PCM
//	};
	
	// talk
	public native static int InitEnc(int width,int height);
	public native static int DeiniEnc();
	public native static int InputData(int type,byte[] data, int len);


}