#pragma once
#ifdef AUDIO_VEDIO_LIB
#else
#define AUDIO_VEDIO_LIB  _declspec(dllimport)
#endif

#include "stdafx.h"
#include <mmsystem.h>
#include <opencv2/highgui/highgui.hpp>  
#include <opencv2/imgproc/imgproc.hpp>  
#include <opencv2/core/core.hpp>  



#define BUFFER_SIZE (44100*16*2/8*10)    // 录制声音长度  
#define FRAGMENT_SIZE 1024              // 缓存区大小  
#define FRAGMENT_NUM 4     

const TCHAR  STOPEVENT[]=L"STOP_EVENT";

namespace avl
{
	class AUDIO_VEDIO_LIB caudio
	{
	public:
		static caudio* GetInstance();
		~caudio(void);
		void WaveInWork();
		void WaveInStopWorking();
		void WaveOutWork();
		const bool IsWaveInWorking();
	private:
		caudio(void);
		static DWORD CALLBACK ThreadWaveIn(LPVOID lParam);
		static DWORD CALLBACK ThreadWaveOut(LPVOID lParam);
		static void CALLBACK WaveInProc(HWAVEIN hwi,UINT uMsg,DWORD_PTR dwInstance, DWORD_PTR dwParam1,DWORD_PTR dwParam2);
		static void CALLBACK WaveOutProc(HWAVEOUT hwo,UINT uMsg,DWORD_PTR dwInstance, DWORD_PTR dwParam1,DWORD_PTR dwParam2);
	private:
		HWAVEIN m_wavein;
		HWAVEIN m_waveout;
		WAVEHDR m_wh[FRAGMENT_NUM];
		bool m_isWaveInWorking;     //是否正在录音
	private:
		static caudio* sm_instance; //但实例对象
		static int sm_bufcount;     //当前录音容器使用量，字节
		static char sm_buffer[BUFFER_SIZE]; //录音波形容器
		static HANDLE m_event;      //停止录音事件对象
		static bool sm_enable;      //是否可以添加WAVEHDR
		static int sm_waveoutcount;  //播放录音量，字节
	};

	class AUDIO_VEDIO_LIB cvideo
	{
	public:
		static cvideo* GetInstance();
		bool OpenVideoCapture();
		void CloseVideoCapture();
		cv::Mat GetFrame();
		void DrawToHDC(cv::Mat mat, HDC hDC, RECT rect);
		void SetDelay(int delay);
		~cvideo();
	private:
		cvideo();
		static DWORD CALLBACK ThreadOpenVideoCapture(LPVOID lParam);
	private:
		static cvideo* sm_instance; //单实例
		HANDLE m_stopEvent;         //停止摄像头事件对象
		int  m_delay;				//获取摄像头帧间隔
		cv::VideoCapture* m_camera; //video设备对象
	};
}

