#pragma once
#include "PCMDecoder.h"
#include "vorbis/vorbisfile.h"
#include <assert.h>
class OggDecoder : public PCMDecoder
{
public:
	OggDecoder(const char* pFileName) 
	{
		memset(&m_ovf, 0, sizeof(m_ovf));
		setSound(pFileName);
	}
	OggDecoder() { memset(&m_ovf, 0, sizeof(m_ovf)); }
	virtual ~OggDecoder(){ clear(); }
	virtual void clear()
	{
		if (!isReady())
		{
			ov_clear(&m_ovf);
		}
		memset(&m_ovf, 0, sizeof(m_ovf));
		PCMDecoder::clear();
	}
	virtual void setHead()
	{
		if (!isReady()) return;
		ov_time_seek(&m_ovf, 0.0);
	}
	virtual bool setSound(const char* filename)
	{
		clear();
		if (ov_fopen((char*)filename, &m_ovf) != 0)
			return false;
		SetFileName(filename);
		vorbis_info* info = ov_info(&m_ovf, -1);
		SetChannelNum(info->channels);
		SetBitRate(16);
		SetSamplingRate(info->rate);
		SetReady(true);
		return true;
	}
	virtual PCMDecoder* clone(){ return isReady() ? new OggDecoder(getFileName()) : 0;}
	virtual bool getSegment(char* buffer, unsigned int size, unsigned int * writeSize, bool * isEnd)
	{
		if (!isReady()) return false;
		if(buffer == 0)
		{
			if (isEnd) *isEnd = true;
			if (writeSize) * writeSize = 0;
			return false;
		}
		if (isEnd) *isEnd = false;
		memset(buffer, 0, size);
		const unsigned int requestSize_base = 4096 ;	// 読み込み単位
		unsigned int requestSize = requestSize_base;
		int bitstream = 0;
		int readSize = 0;
		unsigned int comSize = 0;
		bool isAdjust = false;
		if (size < requestSize)
		{
			requestSize = size;
			isAdjust = true;
		}
		while (1)
		{
			readSize = ov_read(&m_ovf, (char*)(buffer + comSize), requestSize, 0, 2, 1, &bitstream);
			if (readSize == 0)
			{
				if (isLoop() == true)
				{
					ov_time_seek(&m_ovf, 0.0);
				}
				else
				{
					if (isEnd) *isEnd = true;
					if(writeSize) *writeSize = comSize;
					return true;
				}
			}
			comSize += readSize;
			assert(comSize <= size);	// バッファオーバー
			if (comSize >= size)
			{
				if (writeSize) *writeSize = comSize;
				return true;
			}
			if (size - comSize < requestSize_base)
			{
				isAdjust = true;
				requestSize = size - comSize;
			}
		}
		if (writeSize) *writeSize = 0;
		return false;
	}
protected:
	OggVorbis_File m_ovf;
};
