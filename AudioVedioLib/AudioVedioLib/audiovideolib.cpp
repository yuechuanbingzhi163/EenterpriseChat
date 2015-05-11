#define AUDIO_VEDIO_LIB  _declspec(dllexport)

#include <atlimage.h>
#include "audiovideolib.h"
using namespace avl;
using namespace cv;
using namespace std;

#pragma comment(lib,"winmm.lib")

int caudio::sm_bufcount=0;
char caudio::sm_buffer[BUFFER_SIZE]={0};
HANDLE caudio::m_event=CreateEvent(0,FALSE,FALSE,L"WAVEIN_EVENT");
int caudio::sm_waveoutcount=0;
bool caudio::sm_enable=true;
caudio* caudio::sm_instance=NULL;

cvideo* cvideo::sm_instance=NULL;

caudio::caudio(void)
{
	m_isWaveInWorking=false;
}

caudio::~caudio(void)
{
	if(m_event!=INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_event);
		m_event=INVALID_HANDLE_VALUE;
	}
	if(sm_instance!=NULL)
	{
		delete sm_instance;
		sm_instance=NULL;
	}
}

caudio* caudio::GetInstance()
{
	if(caudio::sm_instance==NULL)
	{
		sm_instance=new caudio();
	}
	return sm_instance;
}

void caudio::WaveInWork()
{
	/*WAVEINTHREADPARA threadparam;
	threadparam.m_instance=this;
	threadparam.m_proc=proc;
	threadparam.m_para=NULL;*/
	CloseHandle(CreateThread(NULL,0,ThreadWaveIn,this,0,NULL));
}

void caudio::WaveInStopWorking()
{
	if(m_isWaveInWorking)
	{
		SetEvent(m_event);
	}
}

void caudio::WaveOutWork()
{
	CloseHandle(CreateThread(NULL,0,ThreadWaveOut,this,0,NULL));
}

DWORD CALLBACK caudio::ThreadWaveIn(LPVOID lParam)
{

	/*WAVEINTHREADPARA *threadparam=(WAVEINTHREADPARA*)lParam;
	if(threadparam==NULL)
	{
		return -1;
	}*/
	caudio* instance=(caudio*)lParam;
	if(instance==NULL)
	{
		return -1;
	}
	if(instance->m_isWaveInWorking)
	{
		return -1;
	}
	instance->m_isWaveInWorking=true;

	memset(caudio::sm_buffer,0,BUFFER_SIZE);
	OutputDebugString(L"WaveInWork\r\n");
	//	CString info;
	// open   
	WAVEINCAPS wic;  
	waveInGetDevCaps((UINT_PTR)(instance->m_wavein), &wic, sizeof(WAVEINCAPS));  

	//info=wic.szPname;
	//::MessageBox(NULL,L"打开的输入设备："+info,NULL,0);  
	caudio::sm_enable=true;
	WAVEFORMATEX wavform;  
	wavform.wFormatTag = WAVE_FORMAT_PCM;  
	wavform.nChannels = 2;  
	wavform.nSamplesPerSec = 44100;  
	wavform.nAvgBytesPerSec = 44100*16*2/8;  
	wavform.nBlockAlign = 4;  
	wavform.wBitsPerSample = 16;  
	wavform.cbSize = 0;  
	if(MMSYSERR_NOERROR!=waveInOpen(&(instance->m_wavein), WAVE_MAPPER, &wavform, (DWORD_PTR)WaveInProc, 0, CALLBACK_FUNCTION))
	{
		OutputDebugString(L"打开录音设备失败！\r\n");
	}
	for (int i=0; i<FRAGMENT_NUM; i++)  
	{  
		instance->m_wh[i].lpData = new char[FRAGMENT_SIZE];  
		instance->m_wh[i].dwBufferLength = FRAGMENT_SIZE;  
		instance->m_wh[i].dwBytesRecorded = 0;  
		instance->m_wh[i].dwUser = NULL;  
		instance->m_wh[i].dwFlags = 0;  
		instance->m_wh[i].dwLoops = 1;  
		instance->m_wh[i].lpNext = NULL;  
		instance->m_wh[i].reserved = 0;  

		if(MMSYSERR_NOERROR!=waveInPrepareHeader(instance->m_wavein, &instance->m_wh[i], sizeof(WAVEHDR)))
		{
			OutputDebugString(L"准备buffer失败！！！\r\n");
		}
		if(MMSYSERR_NOERROR!=waveInAddBuffer(instance->m_wavein, &instance->m_wh[i], sizeof(WAVEHDR)))
		{
			OutputDebugString(L"加入buff失败！\r\n");
		}
	} 



	caudio::sm_bufcount = 0;

	if(MMSYSERR_NOERROR!=waveInStart(instance->m_wavein))
	{
		OutputDebugString(L"开始录音失败！！\r\n");
	}
	else
	{
		OutputDebugString(L"开始录音\r\n");
	}
	while (caudio::sm_bufcount < BUFFER_SIZE)  
	{ 
		if(WAIT_OBJECT_0==WaitForSingleObject(caudio::m_event,0))
		{
			sm_enable=false;
			break;
		}
		Sleep(1);

	}  
	OutputDebugString(L"录音完成\r\n");
	MessageBox(NULL,L"录音完成",NULL,0);

	Sleep(40);
	if(MMSYSERR_NOERROR==waveInStop(instance->m_wavein))
	{
		OutputDebugString(L"waveinstop\r\n");
	}

	if(MMSYSERR_NOERROR==waveInReset(instance->m_wavein))  
	{
		OutputDebugString(L"waveinreset\r\n");
	}
	else
	{
		OutputDebugString(L"waveinreset failed!!!\r\n");
	}
	for (int i=0; i<FRAGMENT_NUM; i++)  
	{  
		waveInUnprepareHeader(instance->m_wavein, &instance->m_wh[i], sizeof(WAVEHDR));  
		delete instance->m_wh[i].lpData;  
	}  
	/*waveInClose(instance->m_wavein); 
	if(threadparam->m_proc!=NULL)
	{
		threadparam->m_proc(threadparam->m_para);
	}*/
	instance->m_isWaveInWorking=false;
	OutputDebugString(L"waveInClose\r\n");
	return 0;
}

DWORD CALLBACK caudio::ThreadWaveOut(LPVOID lParam)
{
	caudio *instance=(caudio*)lParam;
	if(instance==NULL)
	{
		return -1;
	}
	HWAVEOUT hWaveOut;  
	WAVEFORMATEX wavform;  
	wavform.wFormatTag = WAVE_FORMAT_PCM;  
	wavform.nChannels = 2;  
	wavform.nSamplesPerSec = 44100;  
	wavform.nAvgBytesPerSec = 44100*16*2/8;  
	wavform.nBlockAlign = 4;  
	wavform.wBitsPerSample = 16;  
	wavform.cbSize = 0;  

	waveOutOpen(&hWaveOut, WAVE_MAPPER,&wavform, (DWORD_PTR)WaveOutProc, 0, CALLBACK_FUNCTION);  

	WAVEOUTCAPS woc;  
	waveOutGetDevCaps((UINT_PTR)hWaveOut, &woc, sizeof(WAVEOUTCAPS));  

	// prepare buffer   
	WAVEHDR wavhdr;  
	wavhdr.lpData = (LPSTR)caudio::sm_buffer;  
	wavhdr.dwBufferLength =caudio::sm_bufcount;  
	wavhdr.dwFlags = 0;  
	wavhdr.dwLoops = 0;  

	waveOutPrepareHeader(hWaveOut, &wavhdr, sizeof(WAVEHDR));  

	// play   
	printf("Start to Play...\n");  

	caudio::sm_waveoutcount = 0;  
	waveOutWrite(hWaveOut, &wavhdr, sizeof(WAVEHDR));  
	while (caudio::sm_waveoutcount<caudio::sm_bufcount)  
	{  
		Sleep(1);  
	}  

	MessageBox(NULL,L"播放完毕！",NULL,0);
	// clean   
	waveOutReset(hWaveOut);  
	waveOutUnprepareHeader(hWaveOut, &wavhdr, sizeof(WAVEHDR));  
	waveOutClose(hWaveOut);  
	ResetEvent(m_event);
	OutputDebugString(L"waveoutclose\r\n");
	return 0;  
}

void CALLBACK caudio::WaveInProc(HWAVEIN hwi,UINT uMsg,DWORD_PTR dwInstance, DWORD_PTR dwParam1,DWORD_PTR dwParam2)
{
	LPWAVEHDR pwh = (LPWAVEHDR)dwParam1;  
	switch(uMsg)
	{
	case WIM_OPEN:
		//::MessageBox(NULL,L"打开录音设备",NULL,NULL);
		break;
	case WIM_DATA:
		//::MessageBox(NULL,L"录音一次",NULL,0);
		OutputDebugString(L"WIM_DATA");
		if(caudio::sm_enable)
		{
			if (caudio::sm_bufcount<BUFFER_SIZE)  
			{  
				OutputDebugString(L"复制数据");
				int temp = BUFFER_SIZE - sm_bufcount;  
				temp = (temp>(int)pwh->dwBytesRecorded) ? pwh->dwBytesRecorded : temp;  
				memcpy(sm_buffer+sm_bufcount, pwh->lpData, temp);  
				sm_bufcount += temp;  
				waveInAddBuffer(hwi, pwh, sizeof(WAVEHDR));  
			}
		}
		break;
	case WIM_CLOSE:
		break;
	default: break;
	}
}

void CALLBACK caudio::WaveOutProc(  HWAVEOUT hwo, UINT uMsg,DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)  
{  
	if (WOM_DONE == uMsg)  
	{  
		caudio::sm_waveoutcount = caudio::sm_bufcount;  
	}  
}  

 bool caudio::IsWaveInWorking()
{
	bool retbool=false;
	if( m_isWaveInWorking)
	{
		retbool = true;
	}
	else
	{
		retbool = false;
	}
	return  retbool;
}

cvideo::cvideo()
{
	m_stopEvent=CreateEvent(NULL,FALSE,FALSE,STOPEVENT);
	m_delay=50;
	m_camera=NULL;
}

cvideo::~cvideo()
{
	if(cvideo::sm_instance!=NULL)
	{
		delete cvideo::sm_instance;
		cvideo::sm_instance=NULL;
	}
}

cvideo* cvideo::GetInstance()
{
	if(cvideo::sm_instance==NULL)
	{
		cvideo::sm_instance=new cvideo();
	}
	return cvideo::sm_instance;
}

void  cvideo::DrawToHDC(Mat mat, HDC hDC, RECT rect)
{
	//Mat mat;
	//cvtColor(dst,mat,CV_RGB2GRAY);
	CImage img; //ATL::CImage
	int w = mat.cols;  //宽
	int h = mat.rows;  //高
	int chinnels = mat.channels();
	img.Create(w, h, chinnels << 3);

	if(chinnels==1)
	{
		RGBQUAD* pColorTable = NULL;
		int nMaxColors = img.GetMaxColorTableEntries();
		pColorTable = new RGBQUAD[nMaxColors];
		img.GetColorTable(0, nMaxColors, pColorTable);
		for (int i = 0; i < nMaxColors; i++)
		{
			pColorTable->rgbBlue = (BYTE)i;
			pColorTable->rgbGreen = (BYTE)i;
			pColorTable->rgbRed = (BYTE)i;
		}
		img.SetColorTable(0, nMaxColors, pColorTable);
		delete[] pColorTable;
	}

	int i, j, k;
	BYTE *pSource = NULL;
	BYTE *pImgData = (BYTE *)img.GetBits();
	int step = img.GetPitch();

	if (chinnels == 1)
	{
		for (i = 0; i < h; ++i)
		{
			pSource = mat.ptr<BYTE>(i);
			for (j = 0; j < w; ++j)
			{
				*(pImgData + i*step + j) = pSource[j];
			}
		}
	}
	else if (chinnels == 3)
	{
		for (i = 0; i < h; ++i)
		{
			pSource = mat.ptr<BYTE>(i);
			for (j = 0; j < w; ++j)
			{
				for (k = 0; k < 3; ++k)
				{
					*(pImgData + i*step + j * 3 + k) = pSource[j * 3 + k];
				}
			}
		}
	}
	else
	{
		OutputDebugString(TEXT("仅支持灰度图/3通道彩色图"));
		return;
	}

	SetStretchBltMode(hDC, COLORONCOLOR);
	img.StretchBlt(hDC, rect, SRCCOPY);
	img.Destroy();
}

bool cvideo::OpenVideoCapture()
{
	m_camera =new VideoCapture(0);
	return m_camera->isOpened();
	//CloseHandle(CreateThread(NULL,0,ThreadOpenVideoCapture,this,0,NULL));
	/*m_camera =new VideoCapture(0);
	if(m_camera==NULL||!m_camera->isOpened())
	{
	return false;
	}
	return true;*/
}

bool cvideo::OpenVideoCapture(const char* filename)
{
	m_camera =new VideoCapture(filename); //将视频加载到设备中
	return m_camera->isOpened();
}

void cvideo::CloseVideoCapture()
{
	if(m_camera->isOpened())
	{
		m_camera->release();
	}
}

DWORD CALLBACK cvideo::ThreadOpenVideoCapture(LPVOID lParam)
{
	cvideo* instance = (cvideo*)lParam;
	if(instance==NULL)
	{
		return  -1;
	}
	instance->m_camera =new VideoCapture(0);
	if(false == (instance->m_camera)->isOpened())
	{
		return -1;
	}

	//Mat frame;

	//while(true)
	//{
	//	if(WAIT_OBJECT_0!=WaitForSingleObject(instance->m_stopEvent,0))
	//	{
	//		return 0;
	//	}
	//	/*cap>>frame;*/
	//}
	return 0;
}

void cvideo::SetDelay(int delay)
{
	m_delay=delay;
}

Mat cvideo::GetFrame()
{

	Mat frame;
	if(m_camera->isOpened())
	{
		m_camera->read (frame);
	}
	return frame;
}

void cvideo::PlayVideo(HDC hDC, RECT rect)
{
	int framecount=0;

	long totalFrameNumber=m_camera->get(CAP_PROP_FRAME_COUNT); //获取视频文件总帧数

	m_camera->set(CAP_PROP_POS_FRAMES,0);  //设置为视频当前帧

	
	double fps = m_camera->get(CAP_PROP_FPS);
	
	int delay=1000/fps;
	
	//int kernel_size = 3;  
   // Mat kernel = Mat::ones(kernel_size,kernel_size,CV_32F)/(float)(kernel_size*kernel_size);  

	while(framecount<totalFrameNumber)
	{
		Mat frame;
		if(!m_camera->read(frame))
		{
			break;
		}
		DrawToHDC(frame,hDC,rect);
		Sleep(delay);
	}
	m_camera->release();
}

