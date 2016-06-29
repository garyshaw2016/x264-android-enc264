#ifndef ENCODE_264_X_456W34324
#define ENCODE_264_X_456W34324


#define TEST_SAVE_FILE 1 //save to file


enum emEnc264Type
{
	//video
	ENCODE_TYPE_VIDEO_264 = 0,
	//audio
	ENCODE_TYPE_AUDIO_PCM
};


void * CreateEncode264Instance(int width ,int height);
void InputData(void * ins, int enc264type/*emEnc264Type*/, char * buffer ,int len);
void DestroyEncode264Instance(void * ins);







#endif //ENCODE_264_X_456W34324
