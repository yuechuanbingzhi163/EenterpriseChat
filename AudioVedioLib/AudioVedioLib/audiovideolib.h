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



#define BUFFER_SIZE (44100*16*2/8*10)    // ¼����������  
#define FRAGMENT_SIZE 1024              // ��������С  
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
		bool m_isWaveInWorking;     //�Ƿ�����¼��
	private:
		static caudio* sm_instance; //��ʵ������
		static int sm_bufcount;     //��ǰ¼������ʹ�������ֽ�
		static char sm_buffer[BUFFER_SIZE]; //¼����������
		static HANDLE m_event;      //ֹͣ¼���¼�����
		static bool sm_enable;      //�Ƿ�������WAVEHDR
		static int sm_waveoutcount;  //����¼�������ֽ�
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
		static cvideo* sm_instance; //��ʵ��
		HANDLE m_stopEvent;         //ֹͣ����ͷ�¼�����
		int  m_delay;				//��ȡ����ͷ֡���
		cv::VideoCapture* m_camera; //video�豸����
	};
}

