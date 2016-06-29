#include "PhoneDevice.h"
#include "PlatformDefine.h"
#include "Encode264.h"

#include "XDes.h"

#if TEST_SAVE_FILE
#include "XRecorder.h"
CXRecorder *m_recorder = NULL;
#endif
#if TEST_4_PLAT_LIVE
CPhoneDevice::CPhoneDevice(void) :m_plMsg(sizeof(MyStruct4PhoneDeviceSession))
#else
CPhoneDevice::CPhoneDevice(void)
#endif
{

	m_param = NULL;
	m_handle = NULL;
	m_pic_in = NULL;
	m_nal = NULL;

	//----------------------------------------------------------
	m_nal = (x264_nal_t *)malloc(sizeof(x264_nal_t));
	m_param = (x264_param_t *)malloc(sizeof(x264_param_t));
	m_pic_in = (x264_picture_t *)malloc(sizeof(x264_picture_t));
	m_pic_out = (x264_picture_t *)malloc(sizeof(x264_picture_t));
	//----------------------------------------------------------
	m_pBuffer = new char[2 * 1024 * 1024];


#if TEST_4_PLAT_LIVE
	

	m_szBuffer = new unsigned char[MAX_MSG_4_PHONE_DEVICE_SESSION];

	m_hWorkerThread = CROSS_THREAD_NULL;
	m_bLogin = FALSE;
	m_pTcpStream = NULL;
#endif



#if TEST_SAVE_FILE
	m_recorder = NULL;
#endif



}


CPhoneDevice::~CPhoneDevice(void)
{


	Stop();

	//----------------------------------------------------------
	if (m_pic_in)
	{
		free(m_pic_in);
		m_pic_in = NULL;
	}
	if (m_pic_out)
	{
		free(m_pic_out);
		m_pic_out = NULL;
	}
	if (m_param)
	{
		free(m_param);
		m_param = NULL;
	}
	if (m_nal)
	{
		free(m_nal);
		m_nal = NULL;
	}
	//----------------------------------------------------------
	delete[] m_pBuffer;
#if TEST_4_PLAT_LIVE
	delete[] m_szBuffer;
#endif

}
void CPhoneDevice::Start(int width, int height)
{

	CROSS_TRACE("CPhoneDevice -- Start --11111\n");


#if TEST_SAVE_FILE
	if (m_recorder == NULL)
	{
		//CROSS_TRACE("CPhoneDevice -- Start -- 111111--------1\n");
		m_recorder = new CXRecorder();
		m_recorder->Start("/mnt/sdcard/", "avvv", 120, 64);
		//m_recorder->Start("/storage/emulated/0/", "avvv", 120, 64);
	}
#endif


	m_width = width;
	m_height = height;

	int m_frameRate = 25;
	int m_bitRate = 4 * 1024 * 1024;


	//移动设备上的H.264实时视频编码, 需要考虑到cpu占用与带宽这2个限制因素
	//veryfast superfast ultrafast 可编码效率越来越高，但是码流也增大
	CROSS_TRACE("CPhoneDevice -- Start -- 22222\n");
	//x264_param_default(m_param); //set default param
	//x264_param_default_preset(m_param, "veryfast", "zerolatency");
	//x264_param_default_preset(m_param, "superfast", "zerolatency");
	x264_param_default_preset(m_param, "ultrafast", "zerolatency");
	//
	m_param->i_threads = X264_SYNC_LOOKAHEAD_AUTO;/*cpuFlags 去空缓冲区继续使用不死锁保证*/
	m_param->i_log_level = X264_LOG_NONE;
	//m_param->i_nal_hrd = X264_NAL_HRD_NONE;
	//
	m_param->i_width = width; //set frame width
	m_param->i_height = height; //set frame height
	//
	m_param->i_frame_total = 0;//要编码的总帧数，不知道用0  
	m_param->i_bframe = 0;
	m_param->i_csp = X264_CSP_I420;
	//编码复杂度
	m_param->i_level_idc = 30;
	//图像质量控制 默认好像是23，没有仔细看
	//m_param->rc.f_rf_constant = 23; //23 实际质量，越大图像越花，越小越清晰。
	//m_param->rc.f_rf_constant_max = 45;//45 图像质量的最大值。
	//码率控制
	m_param->rc.i_rc_method = X264_RC_ABR;//CQP(恒定质量)，CRF(恒定码率)，ABR(平均码率)
	m_param->rc.i_vbv_max_bitrate = (int)((m_bitRate*1.2) / 1000); // 平均码率模式下，最大瞬时码率，默认0(与-B设置相同)
	m_param->rc.i_bitrate = (int)m_bitRate / 1000;
	//使用实时视频传输时，需要实时发送sps,pps数据
	m_param->b_repeat_headers = 1;  // 重复SPS/PPS 放到关键帧前面  
	//I帧间隔
	m_param->i_fps_num = (int)m_frameRate;
	m_param->i_fps_den = 1;
	m_param->i_keyint_max = m_frameRate*2;//2秒刷新一个I帧

	//baseline Main High 编码效率越来越低
	//x264_param_apply_profile(m_param, "High");
	//x264_param_apply_profile(m_param, "Main");
	x264_param_apply_profile(m_param, "baseline");
	
	//
	m_handle = x264_encoder_open(m_param);
	if (m_handle == 0) {
		//error
		CROSS_TRACE("CPhoneDevice -- x264_encoder_open -- error --\n");
		return;
	}
	//CROSS_TRACE("CPhoneDevice -- Start --33333\n");
	//
	i_pts = 0;
	x264_picture_init(m_pic_out);
	x264_picture_alloc(m_pic_in, X264_CSP_I420, m_param->i_width, m_param->i_height);/* Create a new pic */


#if TEST_4_PLAT_LIVE
	CROSS_TRACE("CPhoneDevice -- Start -- 33333\n");

	//////////////////////////////////////////////////////////////////////////

	if (m_hWorkerThread == CROSS_THREAD_NULL)
	{
		m_bStreamIsError = FALSE;
		m_nSendStep = NET_SEND_STEP_SLEEP;
		m_nSession = CrossGetTickCount64();
		m_bLogin = FALSE;
		m_hEvtWorkerStop = FALSE;
		m_hWorkerThread = CrossCreateThread(WorkerThread, this);
	}

	CROSS_TRACE("CPhoneDevice -- Start -- 444444\n");
#endif

}
void CPhoneDevice::Stop()
{
	CROSS_TRACE("CPhoneDevice -- Stop --\n");

#if TEST_SAVE_FILE
	if (m_recorder != NULL)
	{
		m_recorder->Stop();
		delete m_recorder;
		m_recorder = NULL;
	}
#endif

#if TEST_4_PLAT_LIVE
	if (m_hWorkerThread)
	{
		m_hEvtWorkerStop = TRUE;

		//关闭重连线程
		CrossWaitThread(m_hWorkerThread);
		CrossCloseThread(m_hWorkerThread);
		m_hWorkerThread = CROSS_THREAD_NULL;
		m_bLogin = FALSE;
	}

	LogoutPlat();
#endif
	if (m_handle)
	{
		//
		x264_picture_clean(m_pic_in);
		//
		x264_encoder_close(m_handle);
		m_handle = NULL;
	}

}
void CPhoneDevice::InputData(int enc264type, char * buffer, int len)
{
#if TEST_4_PLAT_LIVE
	CROSS_TRACE("CPhoneDevice -- InputData --\n");

	if (!m_bLogin)
	{
		return;
	}

	if (m_hEvtWorkerStop)
	{
		return;
	}
#endif
	if (!m_handle)
	{
		return;
	}

	//CROSS_TRACE("CPhoneDevice -- InputData --111 len = %d\n", len);

#if TEST_4_PLAT_LIVE
	m_csMsgYUV.Lock();
	MyStruct4PhoneDeviceSession *p = (MyStruct4PhoneDeviceSession *)m_plMsg.malloc();
	memcpy(p->buffer, buffer, len);
	p->nSendLen = len;
	p->nType = enc264type;
	m_msgListYUV.push_back(p);
	m_csMsgYUV.Unlock();


	//CROSS_TRACE("CPhoneDevice -- InputData --222 len = %d\n", len);
	return;
#endif
	


#if TEST_SAVE_FILE
	if (enc264type == ENCODE_TYPE_VIDEO_264)
	{

		CROSS_TRACE("CPhoneDevice -- InputData -- file -- 111111 \n");

		int i_data = 0;
		int nNal = -1;
		//int result = 0;
		int i = 0, j = 0;


		//CROSS_DWORD64 dwa = CrossGetTickCount64();


		int nPicSize = m_param->i_width*m_param->i_height;


		char * y = (char*)m_pic_in->img.plane[0];
		char * v = (char*)m_pic_in->img.plane[1];
		char * u = (char*)m_pic_in->img.plane[2];


		m_pic_in->img.i_plane = 3;
		m_pic_in->img.i_stride[0] = m_width;
		m_pic_in->img.i_stride[1] = m_width / 2;
		m_pic_in->img.i_stride[2] = m_width / 2;
		m_pic_in->img.i_csp = X264_CSP_I420;



		memcpy(y, buffer, nPicSize);
		for (i = 0; i < nPicSize / 4; i++)
		{
			*(u + i) = *(buffer + nPicSize + i * 2);
			*(v + i) = *(buffer + nPicSize + i * 2 + 1);
		}

		//CROSS_TRACE("CPhoneDevice -- InputData -- yuvyuv -- time = %d\n", CrossGetTickCount64() - dwa);


		m_pic_in->i_pts = i_pts++;

		if (x264_encoder_encode(m_handle, &(m_nal), &nNal, m_pic_in, m_pic_out) < 0)
		{
			//CROSS_TRACE("CPhoneDevice -- x264_encoder_encode -- error --\n");

			return;
		}

		//CROSS_TRACE("CPhoneDevice -- InputData -- yuvyuv 1 -- time = %d\n", CrossGetTickCount64() - dwa);

		m_len = 0;
		for (i = 0; i < nNal; i++)
		{
			memcpy(m_pBuffer + m_len, m_nal[i].p_payload, m_nal[i].i_payload);
			m_len += m_nal[i].i_payload;
		}

		//CROSS_TRACE("CPhoneDevice -- InputData -- yuvyuv 2 -- time = %d\n", CrossGetTickCount64() - dwa);
		CROSS_TRACE("CPhoneDevice -- InputData -- file -- 22222 \n");
		m_recorder->InputData(m_pBuffer, m_len, FREAM_TYPE_H264_IFRAME, 0, 25);

	}
#endif
}







#if TEST_4_PLAT_LIVE
void CPhoneDevice::DeleteStreamData()
{
	m_pTcpStream = NULL;

	//////////////////////////////////////////////////////////////////////////
	m_csMsg.Lock();
	while (m_msgList.size() > 0)
	{
		MyStruct4PhoneDeviceSession * p = m_msgList.front();
		m_msgList.pop_front();
		m_plMsg.free(p);
	}
	m_csMsg.Unlock();

	//////////////////////////////////////////////////////////////////////////
	m_csMsgYUV.Lock();
	while (m_msgList.size() > 0)
	{
		MyStruct4PhoneDeviceSession * p = m_msgListYUV.front();
		m_msgListYUV.pop_front();
		m_plMsg.free(p);
	}
	m_csMsgYUV.Unlock();

}
void CPhoneDevice::OnPacketCompleteNetStreamData(int32_t dwCompleteBytes, int32_t bRecieve)
{
	if (NULL == m_pTcpStream)
	{
		return;
	}
	if (m_bStreamIsError)
	{
		return;
	}
	////网络出错，用户断开连接
	if (dwCompleteBytes < 0)
	{
		m_bStreamIsError = TRUE;
		return;
	}

	if (bRecieve)
	{
		//CROSS_TRACE("CPhoneDevice -- OnPacketCompleteNetStreamData -- recv -- \n");



		if (0 == dwCompleteBytes)
		{
			m_nRcvStep = NET_RECIEVE_STEP_HEAD;
			m_pTcpStream->AsyncRead(&m_head, sizeof(m_head));
			return;
		}
		else
		{
			if (m_nRcvStep == NET_RECIEVE_STEP_HEAD)
			{
				m_dataLen = m_head.datalen;
				if (m_dataLen > MAX_MSG_4_PHONE_DEVICE_SESSION)
				{
					m_bStreamIsError = TRUE;
					return;
				}
				if (m_dataLen == 0)
				{
					DoMsg();
					m_nRcvStep = NET_RECIEVE_STEP_HEAD;
					m_pTcpStream->AsyncRead(&m_head, sizeof(m_head));
				}
				else
				{
					m_nRcvStep = NET_RECIEVE_STEP_BODY;
					m_pTcpStream->AsyncRead(m_szBuffer, m_dataLen);
				}
				return;
			}
			if (m_nRcvStep == NET_RECIEVE_STEP_BODY)
			{
				DoMsg();
				m_nRcvStep = NET_RECIEVE_STEP_HEAD;
				m_pTcpStream->AsyncRead(&m_head, sizeof(m_head));
				return;
			}
		}
	}
	else
	{

		//CROSS_TRACE("CPhoneDevice -- OnPacketCompleteNetStreamData -- send -- \n");
		////////////////////////////////////////////////////////////////////////
		if (NET_SEND_STEP_SLEEP == m_nSendStep)
		{
			if (m_msgList.size() > 0)
			{
				m_nSendStep = NET_SEND_STEP_SEND;
				m_pTcpStream->AsyncWrite(m_msgList.front(), ((MyStruct4PhoneDeviceSession *)m_msgList.front())->nSendLen);
				return;
			}
			else
			{
				m_pTcpStream->PostDelayWriteStatus();//强制调用一次发送回调
				return;
			}
		}
		//////////////////////////////////////////////////////////////////////////
		if (NET_SEND_STEP_SEND == m_nSendStep)
		{
			m_nSendStep = NET_SEND_STEP_SLEEP;
			m_csMsg.Lock();
			MyStruct4PhoneDeviceSession * p = m_msgList.front();
			m_msgList.pop_front();
			m_plMsg.free(p);
			m_csMsg.Unlock();

			CROSS_TRACE("CPhoneDevice -- send Data -- -- \n");

			if (m_msgList.size() > 0)
			{
				m_nSendStep = NET_SEND_STEP_SEND;
				m_pTcpStream->AsyncWrite(m_msgList.front(), ((MyStruct4PhoneDeviceSession *)m_msgList.front())->nSendLen);
				return;
			}
			else
			{
				m_pTcpStream->PostDelayWriteStatus();//强制调用一次发送回调
				return;
			}
		}
	}
}

void CPhoneDevice::DoMsg()
{
	m_dTickHearbeat = CrossGetTickCount64();
}



int CPhoneDevice::LoginPlat()
{





	CROSS_TRACE("CPhoneDevice -- Login \n");
	MyStruct4PhoneDeviceSession * pSendmsg = new MyStruct4PhoneDeviceSession;

	MyStruct4PhoneDeviceSession *pRecvmsg = new MyStruct4PhoneDeviceSession;
	do
	{
		CROSS_TRACE("CPhoneDevice -- Login 1\n");
		m_pTcpStream = XNetCreateStream4Connect("192.168.2.120", 8002, 10);//23.99.10
		if (0 != XNetConnectStream(m_pTcpStream))
		{
			CROSS_TRACE("CPhoneDevice -- connect error\n");
			break;
		}
		CROSS_TRACE("CPhoneDevice -- Login 2\n");
		//
		pSendmsg->head.magicnum = MAGIC_NUM;
		pSendmsg->head.result = _X_CMD_RESULT_OK;
		pSendmsg->head.cmd = _X_CMD_LOGIN;
		pSendmsg->head.datalen = sizeof(_login_t);

		//
		_login_t login;
		memset(&login, 0, sizeof(login));

		char dest[256] = { 0 };
		XDESEncode("19544", 19544, dest);
		memcpy(login.pwd, dest, strlen(dest));
		//
		memcpy(pSendmsg->buffer, &login, sizeof(_login_t));
		//
		pSendmsg->nSendLen = sizeof(_head_t) + sizeof(_login_t);

		//AAAAAAAA:发送Serial给平台，平台将返回请求注册信息，要求KEY//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
		if (0 != m_pTcpStream->SyncWriteAndRead(pSendmsg, pSendmsg->nSendLen, pRecvmsg, sizeof(_head_t)))
		{
			CROSS_TRACE("CPhoneDevice -- connect error 1\n");
			break;
		}
		if (_X_CMD_LOGIN != pRecvmsg->head.cmd)
		{
			CROSS_TRACE("CPhoneDevice -- connect error 2\n");
			break;
		}
		CROSS_TRACE("CPhoneDevice -- Login 3\n");
		//BBBBBBBB：发给平台Session KEY，并接收验证平台的注册//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
		pSendmsg->head.magicnum = MAGIC_NUM;
		pSendmsg->head.result = _X_CMD_RESULT_OK;
		pSendmsg->head.cmd = _X_CMD_LOGIN_KEY;
		pSendmsg->head.session = m_nSession;
		pSendmsg->head.datalen = 0;
		if (0 != m_pTcpStream->SyncWriteAndRead(pSendmsg, sizeof(_head_t), pRecvmsg, sizeof(_head_t) + sizeof(_login_t)))
		{
			CROSS_TRACE("CPhoneDevice -- connect error 3\n");
			break;
		}
		if (_X_CMD_LOGIN != pRecvmsg->head.cmd)
		{
			CROSS_TRACE("CPhoneDevice -- connect error 4\n");
			break;
		}
		CROSS_TRACE("CPhoneDevice -- Login 4\n");
		//
		//验证KEY，暂时略过
		//
		//CCCCCCCC：验证平台的注册,返回成功//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
		pSendmsg->head.magicnum = MAGIC_NUM;
		pSendmsg->head.result = _X_CMD_RESULT_OK;
		pSendmsg->head.cmd = _X_CMD_LOGIN;
		pSendmsg->head.session = m_nSession;
		pSendmsg->head.datalen = 0;
		if (0 != m_pTcpStream->SyncWriteAndRead(pSendmsg, sizeof(_head_t), 0, 0))
		{
			CROSS_TRACE("CPhoneDevice -- connect error 5\n");
			break;
		}

		CROSS_TRACE("CPhoneDevice -- connect okokokokokok\n");
		//////////////////////////////////////////////////////////////////////////
		m_dTickHearbeat = CrossGetTickCount64();
		m_pTcpStream->SetStreamData(this);
		delete pSendmsg;
		delete pRecvmsg;
		return 0;
		//////////////////////////////////////////////////////////////////////////


	} while (0);

	if (m_pTcpStream)
	{
		m_pTcpStream->Release();
		m_pTcpStream = NULL;//这里没有SetStreamData，所以不能do...while
	}

	delete pSendmsg;
	delete pRecvmsg;
	return -1;
}
int CPhoneDevice::LogoutPlat()
{
	if (m_pTcpStream)
	{
		m_pTcpStream->Release();
		do
		{
			CrossSleep(1);
		} while (m_pTcpStream);
	}
}

int CPhoneDevice::WorkerThread(void* param)
{
	CPhoneDevice *pService = (CPhoneDevice*)param;
	pService->Worker();
	return 0;
}
//---------------------------------------------------------------------------------------
void CPhoneDevice::Worker()
{
	//CROSS_TRACE("CPhoneDevice -- Worker 1\n");
	m_bLogin = FALSE;
	while (!m_hEvtWorkerStop)// (::WaitForSingleObject(m_hEvtWorkerStop, 3 * 1000) != WAIT_OBJECT_0)
	{

		//CROSS_TRACE("CPhoneDevice -- Worker 2\n");
		//////////////////////////////////////////////////////////////////////////
		if (!m_bLogin)
		{
			//CROSS_TRACE("CPhoneDevice -- Worker 3\n");
			if (0 != LoginPlat())
			{
				//CROSS_TRACE("CPhoneDevice -- Worker 4\n");
				m_bLogin = FALSE;
				if (!m_hEvtWorkerStop)
				{
					//TRACE("Login error!\n");
					for (int i = 0; i < 30; i++)
					{
						CrossSleep(100);
						if (m_hEvtWorkerStop)
							break;
					}
					continue;
				}

			}
			else
			{
				m_bLogin = TRUE;
			}
			//CROSS_TRACE("CPhoneDevice -- Worker 5\n");
		}
		//CROSS_TRACE("CPhoneDevice -- Worker 6\n");

		//////////////////////////////////////////////////////////////////////////
		if ((CrossGetTickCount64() - m_dTickHearbeat) > 20 * 1000)
		{
			m_bLogin = FALSE;
			LogoutPlat();
		}




		//////////////////////////////////////////////////////////////////////////
		{
			//CROSS_TRACE("CPhoneDevice -- Worker -- 1111\n");


			MyStruct4PhoneDeviceSession * pYUV = NULL;
			//CROSS_TRACE("CPhoneDevice -- Worker -- 222\n");
			m_csMsgYUV.Lock();
			if (m_msgListYUV.size() <= 0)
			{
				//CROSS_TRACE("CPhoneDevice -- Worker -- 333\n");
				m_csMsgYUV.Unlock();
				CrossSleep(1);
				//CROSS_TRACE("CPhoneDevice -- Worker -- 444\n");
				continue;
			}
			//CROSS_TRACE("CPhoneDevice -- Worker -- 555\n");
			pYUV = m_msgListYUV.front();
			//CROSS_TRACE("CPhoneDevice -- Worker -- 666\n");
			m_msgListYUV.pop_front();
			//CROSS_TRACE("CPhoneDevice -- Worker -- 777\n");
			//m_plMsg.free(pYUV);
			CROSS_TRACE("CPhoneDevice -- InputData -- yuvyuv -- m_msgListYUV.size()=%d\n", m_msgListYUV.size());
			m_csMsgYUV.Unlock();

			//continue;




			if (pYUV->nType == ENCODE_TYPE_VIDEO_264)
			{


				int i_data = 0;
				int nNal = -1;
				//int result = 0;
				int i = 0, j = 0;


				CROSS_DWORD64 dwa = CrossGetTickCount64();


				int nPicSize = m_param->i_width*m_param->i_height;


				char * y = (char*)m_pic_in->img.plane[0];
				char * v = (char*)m_pic_in->img.plane[1];
				char * u = (char*)m_pic_in->img.plane[2];


				m_pic_in->img.i_plane = 3;
				m_pic_in->img.i_stride[0] = m_width;
				m_pic_in->img.i_stride[1] = m_width / 2;
				m_pic_in->img.i_stride[2] = m_width / 2;
				m_pic_in->img.i_csp = X264_CSP_I420;



				memcpy(y, pYUV->buffer, nPicSize);
				for (i = 0; i < nPicSize / 4; i++)
				{
					*(u + i) = *(pYUV->buffer + nPicSize + i * 2);
					*(v + i) = *(pYUV->buffer + nPicSize + i * 2 + 1);
				}

				//CROSS_TRACE("CPhoneDevice -- InputData -- yuvyuv -- time = %d\n", CrossGetTickCount64() - dwa);


				m_pic_in->i_pts = i_pts++;

				if (x264_encoder_encode(m_handle, &(m_nal), &nNal, m_pic_in, m_pic_out) < 0)
				{
					CROSS_TRACE("CPhoneDevice -- x264_encoder_encode -- error --\n");

					return;
				}

				CROSS_TRACE("CPhoneDevice -- InputData -- yuvyuv 1 -- time = %d\n", CrossGetTickCount64() - dwa);

				m_len = 0;

				for (i = 0; i < nNal; i++)
				{
					//请注意i_payload好像没有startcode 00 00 00 01，需要手动写入
					//但是即使没有，ffmpeg也能正常解出
					memcpy(m_pBuffer + m_len, m_nal[i].p_payload, m_nal[i].i_payload);
					m_len += m_nal[i].i_payload;
					//result += m_nal[i].i_payload;
				}

				//CROSS_TRACE("CPhoneDevice -- InputData -- yuvyuv 2 -- time = %d\n", CrossGetTickCount64() - dwa);


				if (m_bLogin)
				{
					MyStruct4PhoneDeviceSession *p = (MyStruct4PhoneDeviceSession *)m_plMsg.malloc();
					p->head.magicnum = MAGIC_NUM;
					p->head.cmd = _X_CMD_STREAM;
					p->head.result = _X_CMD_RESULT_OK;
					p->head.session = m_nSession;
					p->head.datalen = m_len + sizeof(_stream_t);

					int k = sizeof(rqst_head) + m_len + sizeof(_stream_t);
					p->nSendLen = sizeof(rqst_head) + m_len + sizeof(_stream_t);


					_stream_t m_stream;
					m_stream.avType = FREAM_TYPE_H264_IFRAME;
					m_stream.datalen = m_len;
					m_stream.tick = CrossGetTickCount64();

					memcpy(p->buffer, &m_stream, sizeof(_stream_t));

					memcpy(p->buffer + sizeof(_stream_t), m_pBuffer, m_len);

					//CROSS_TRACE("CPhoneDevice -- InputData -- -- m_len=%d  p->nSendLen=%d \n", m_len, p->nSendLen);

					m_csMsg.Lock();

					// 					if (m_msgList.size()>25)
					// 					{
					// 						while (m_msgList.size() > 0)
					// 						{
					// 							MyStruct4PhoneDeviceSession * p = m_msgList.front();
					// 							m_msgList.pop_front();
					// 							m_plMsg.free(p);
					// 						}
					// 					}

					m_msgList.push_back(p);

					CROSS_TRACE("CPhoneDevice -- InputData -- yuvyuv -- m_msgList.size()=%d\n", m_msgList.size());
					m_csMsg.Unlock();
				}

				//CROSS_TRACE("CPhoneDevice -- InputData -- yuvyuv 3 -- time = %d\n", CrossGetTickCount64() - dwa);

				//CROSS_TRACE("CPhoneDevice -- InputData -- BBBBBBBBBBBBBBBBBBBB -- len = %d\n", len);
#if TEST_SAVE_FILE
				m_recorder->InputData(m_pBuffer, m_len, FREAM_TYPE_H264_IFRAME, 0, 30);
#endif
			}

			m_plMsg.free(pYUV);
		}

		//////////////////////////////////////////////////////////////////////////
		//CrossSleep(10);

	}


	// #ifdef _PLATFORM_WINDOW
	// 	TRACE("##@@@@@@  CYYJPlayInterface::Worker() Exit!\n");
	// #endif

}
#endif