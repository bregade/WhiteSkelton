#pragma once
#include <Windows.h>

class PCMDecoder
{
public :
	PCMDecoder() {clear();}
	virtual ~PCMDecoder(){}
	virtual bool			getSegment(char* buffer, unsigned int size, unsigned int* writeSize, bool * isEnd) = 0;
	virtual void			setHead()						= 0;
	virtual bool			setSound(const char* filename)	= 0;
	virtual PCMDecoder*     clone() = 0;
	bool isLoop()			{ return m_isLoop; }
	bool isReady()			{ return m_isReady; }
	void SetFileName		(const char* pFileName)		{ strcpy(m_filePath, pFileName);}
	void SetLoop			(bool bLoop)				{ m_isLoop			= bLoop; }
	void SetReady			(bool isReady)				{ m_isReady			= isReady;}
	void SetChannelNum		(unsigned int channelNum)	{ m_channelNum		= channelNum;}
	void SetSamplingRate	(unsigned int samplingRate) { m_SamplingRate	= samplingRate;}
	void SetBitRate			(unsigned int bitrate)		{ m_bitRate			= bitrate;}
	const char* getFileName(){ return m_filePath; }
	virtual void clear()
	{
		m_isLoop		= false;
		m_isReady		= false;
		m_channelNum	= 0;
		m_SamplingRate	= 0;
		m_bitRate		= 0;
		memset(m_filePath, 0, 256);
	}
	bool getWaveFormatEx(WAVEFORMATEX& waveFormatEx)
	{
		if (!isReady()){ return false;}
		waveFormatEx.wFormatTag			= WAVE_FORMAT_PCM;
		waveFormatEx.nChannels			= m_channelNum;
		waveFormatEx.nSamplesPerSec		= m_SamplingRate;
		waveFormatEx.wBitsPerSample		= m_bitRate;
		waveFormatEx.nBlockAlign		= m_channelNum * m_bitRate / 8;
		waveFormatEx.nAvgBytesPerSec	= waveFormatEx.nSamplesPerSec * waveFormatEx.nBlockAlign;
		waveFormatEx.cbSize = 0;
		return true;
	}
private:
	bool			m_isLoop;
	bool			m_isReady;
	char			m_filePath[256];
	unsigned int	m_channelNum;
	unsigned int	m_SamplingRate;
	unsigned int	m_bitRate;
};