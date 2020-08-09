#pragma once
#include "PCMDecoder.h"
#include <process.h>
#include <xaudio2.h>
class PCMPlayer
{
public:
	enum STATE
	{
		STATE_NONE,
		STATE_PLAY,
		STATE_PAUSE,
		STATE_STOP
	};
	PCMPlayer() { clear(); }
	PCMPlayer( PCMDecoder* pDecoder, IXAudio2* pXaudio = nullptr):m_pPCMDecoder(pDecoder),m_pXaudio(pXaudio)
	{
		clear();
		setDecoder(pDecoder);
	}
	~PCMPlayer() { TerminateThread();}
	void TerminateThread() 
	{
		m_isTerminate = true;
		bool end = false;
		while (!end && m_threadHandle != 0)
		{
			DWORD flag = WaitForSingleObject((HANDLE)(__int64)m_threadHandle, 100);
			switch(flag)
			{
				case WAIT_FAILED:		end = true; break;
				case WAIT_OBJECT_0:		end = true; break;
				case WAIT_TIMEOUT:		break;
				default: break;
			}
		}
		m_isTerminate = false;
		m_threadHandle = 0;
	}
	void clear()
	{
		TerminateThread();
		memset(&m_waveFormat,	0, sizeof(m_waveFormat));
		m_isReady = false;
		m_isLoop = true;
		m_state = STATE_NONE;
		if (m_pSourceVoice)
		{
			m_pSourceVoice->Stop();
			m_pSourceVoice->DestroyVoice();
		}
		if (m_SoundBuffer[0] != nullptr)
		{
			delete (m_SoundBuffer[0]);
			m_SoundBuffer[0] = nullptr;
		}
		if (m_SoundBuffer[1] != nullptr)
		{
			delete (m_SoundBuffer[1]);
			m_SoundBuffer[1] = nullptr;
		}
	}
	bool initializeBuffer()
	{
		if (m_pPCMDecoder == 0 || m_pPCMDecoder == nullptr) return false;
		m_pPCMDecoder->setHead();
		return true;
	}
	static void __cdecl streamThread(void* playerPtr)
	{
		PCMPlayer* player = (PCMPlayer*)playerPtr;
		while (player->m_isTerminate == false)
		{
			switch (player->getState())
			{
				case STATE_PLAY:
					XAUDIO2_VOICE_STATE state;
					player->m_pSourceVoice->GetState(&state);
					if (state.BuffersQueued <= 1)//再生キューに常に２つのバッファを溜めておく
					{
						if (player->m_is_end && player->m_isLoop)
						{
							player->m_pPCMDecoder->setHead();
						}
						player->add_next_buffer();
					}
					break;
				case STATE_STOP:break;
				case STATE_PAUSE:break;
				default:break;
			}
			Sleep(100);
		}
	}
	HWND GetConsoleHwnd(void)
	{
		TCHAR pszWindowTitle[1024];
		GetConsoleTitle(pszWindowTitle, 1024);
		return FindWindow(NULL, pszWindowTitle);
	}
	bool setDecoder(PCMDecoder* pCMDecoder)
	{
		if ( m_pPCMDecoder == 0 || m_pPCMDecoder == nullptr)
		{
			m_isReady = false;
			return false;
		}
		m_state = STATE_STOP;
		if (!pCMDecoder->getWaveFormatEx(m_waveFormat))
		{
			return false;
		}
		m_pPCMDecoder = pCMDecoder;
		if (FAILED(m_pXaudio->CreateSourceVoice(&m_pSourceVoice, &m_waveFormat)))
		{
			return false;
		}
		int dataSize = m_waveFormat.nAvgBytesPerSec * m_playTime;
		m_SoundBuffer[0] = (char*)malloc(dataSize);
		m_SoundBuffer[1] = (char*)malloc(dataSize);
		PCMPlayer* player = (PCMPlayer*)this;
		player->m_pPCMDecoder->getSegment(m_SoundBuffer[m_cursor], dataSize , &m_writeSize, &m_is_end);
		XAUDIO2_BUFFER buf{};
		buf.pAudioData = (BYTE*)m_SoundBuffer[0];
		buf.Flags = XAUDIO2_END_OF_STREAM;
		buf.AudioBytes = dataSize;
		m_cursor = m_cursor == 0 ? 1 : 0;
		if (!initializeBuffer()) return false;
		if (m_threadHandle == 0)
		{
			m_threadHandle = (unsigned int)_beginthread(PCMPlayer::streamThread, 0, (void*)this);
		}
		m_isReady = true;
		return true;
	}
	void setDevice(IXAudio2* pDevice) { m_pXaudio = pDevice; }
	STATE getState() { return m_state; }
	void add_next_buffer(void)
	{
		int dataSize = m_waveFormat.nAvgBytesPerSec * m_playTime;
		m_pPCMDecoder->getSegment(m_SoundBuffer[m_cursor], dataSize, &m_writeSize, &m_is_end);
		XAUDIO2_BUFFER buf{};
		buf.pAudioData = (BYTE*)m_SoundBuffer[m_cursor];
		buf.Flags = XAUDIO2_END_OF_STREAM;
		buf.AudioBytes = m_writeSize;
		m_pSourceVoice->SubmitSourceBuffer(&buf);
		m_cursor = m_cursor == 0 ? 1 : 0;
	}
	bool isReady()	 { return m_isReady; }
	bool isPlaying() { return m_state == STATE_PLAY; }
	bool play(bool isLoop)
	{
		if (!m_isReady) return false;
		m_isLoop = isLoop;
		m_pPCMDecoder->SetLoop(isLoop);
		m_pSourceVoice->Start();
		m_state = STATE_PLAY;
		return true;
	}
	void pause()
	{
		if (m_state != STATE_PLAY)
		{
			play(m_isLoop);
			return;
		}
		m_pSourceVoice->Stop();
		m_state = STATE_PAUSE;
	}
	void stop()
	{
		if (!isReady()) return;
		if (m_state == STATE_STOP) return;
		m_state = STATE_STOP;
		clear();
		setDecoder(m_pPCMDecoder);
	}
	void setVolume(float volume)
	{
		if (!isReady()) return;
		m_pSourceVoice->SetVolume(volume);
	}

private:
	PCMDecoder*						m_pPCMDecoder;			//デコーダ情報(ogg,waveファイル等を想定)
	WAVEFORMATEX					m_waveFormat;			//WAVEFORMATEX構造体
	unsigned int					m_threadHandle = 0;		//ストリーム再生スレッドハンドル
	bool							m_isTerminate = false;	//スレッド停止
	bool							m_isReady;				//準備できた？
	bool							m_isLoop;				//ループする？
	STATE							m_state;				//再生状態
	unsigned int					m_writeSize = 0;		//実際にバッファに書き込まれた量
	unsigned int					m_cursor = 0;			//サウンドダブルバッファのどちらを見てるか
	bool							m_is_end = false;		//ストリームの終了タイミング
	const unsigned int				m_playTime = 1;			//1バッファ毎に何秒分データ保持するか
	char* m_SoundBuffer[2];									//サウンドダブルバッファ用
	IXAudio2* m_pXaudio;									//XAudio2のポインタ
	IXAudio2SourceVoice* m_pSourceVoice = nullptr;			//音源
};