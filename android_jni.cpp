#include "stdio.h"
#include <jni.h>
#include <pthread.h>
//#include <map>
using namespace std;
#include "PlatformDefine.h"
#include "Encode264.h"
#include "PhoneDevice.h"






#ifdef __cplusplus
extern "C" {
#endif
	//
	JavaVM* g_JavaVM = NULL;
	void *pEnc264 = NULL;

	JNIEXPORT int JNICALL Java_com_warningenc_nativecaller_NativeCaller_InitEnc(JNIEnv *, jobject,jint width,jint height)
	{
#if TEST_4_PLAT_LIVE
		XNetStartStreamManager(3);
#endif
		pEnc264 = CreateEncode264Instance(width,height);
		return 0;
	}
	JNIEXPORT int JNICALL Java_com_warningenc_nativecaller_NativeCaller_DeiniEnc(JNIEnv *, jobject)
	{
		DestroyEncode264Instance(pEnc264);
#if TEST_4_PLAT_LIVE
		XNetStopStreamManager();
#endif
		return 0;
	}
	JNIEXPORT int JNICALL Java_com_warningenc_nativecaller_NativeCaller_InputData(JNIEnv * env, jobject, jint type, jbyteArray data, jint len)
	{
		//
		jsize lsize  = env->GetArrayLength(data);
		jbyte *audioBuf = (jbyte *)malloc(lsize * sizeof(jbyte));
		env->GetByteArrayRegion(data,0,lsize,audioBuf);
		//
		InputData(pEnc264,type,(char*)audioBuf, (int)len);
		//
		free(audioBuf);

		return 0;
	}




	jint JNI_OnLoad(JavaVM* vm, void* reserved)
	{
		//F_LOG;

		Log("JNI_OnLoad start");
		g_JavaVM = vm;


		JNIEnv* env = 0;
		jint result = -1;
		if (vm->GetEnv((void**)&env, JNI_VERSION_1_4) != JNI_OK)
		{
			Log("JNI_OnLoad error");
			goto bail;
		}

		result = JNI_VERSION_1_4;

	bail:
		Log("JNI_OnLoad end");
		return result;
	}

	void JNI_OnUnload(JavaVM *vm, void *reserved)
	{
		//F_LOG;
	}

#ifdef __cplusplus
}
#endif
