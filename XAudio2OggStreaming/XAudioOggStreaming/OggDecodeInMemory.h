#pragma once
#include "OggDecoder.h"

class OggDecoderInMemory : public OggDecoder
{
public:
	OggDecoderInMemory():m_size(0), m_curPos(0){}
	OggDecoderInMemory(const char* filepath) {setSound(filepath);}
	virtual ~OggDecoderInMemory() { clear(); }
	virtual void clear()
	{
		if (m_buffer)
		{
			delete[] m_buffer;
			m_buffer = 0;
			m_size = 0;
			m_curPos = 0;
		}
		OggDecoder::clear();
	}
	static OggDecoderInMemory* getMyPtr(void* stream)
	{
		OggDecoderInMemory* p = 0;
		memcpy(&p, stream, sizeof(OggDecoderInMemory*));
		return p;
	}
	static size_t read(void* buffer, size_t size, size_t maxCount, void* stream)
	{
		if (buffer == 0) return 0;
		OggDecoderInMemory* p = (OggDecoderInMemory*)stream;
		int resSize = p->m_size - p->m_curPos;
		size_t count = resSize / size;
		if (count > maxCount)
		{
			count = maxCount;
		}
		memcpy(buffer, p->m_buffer + p->m_curPos, size * count);
		p->m_curPos += size * count;
		return count;
	}
	static int seek(void* buffer, ogg_int64_t offset, int flag)
	{
		OggDecoderInMemory* p = (OggDecoderInMemory*)buffer;
		switch (flag)
		{
			case SEEK_CUR: p->m_curPos += offset;				break;
			case SEEK_END: p->m_curPos = p->m_size + offset;	break;
			case SEEK_SET: p->m_curPos = offset;				break;
			default: return -1;
		}
		if (p->m_curPos > p->m_size)
		{
			p->m_curPos = p->m_size;
			return -1;
		}
		else if(p->m_curPos < 0)
		{
			p->m_curPos = 0;
			return -1;
		}
		return 0;
	}
	static int close(void* buffer) { return 0; }
	static long tell(void* buffer) { return  ((OggDecoderInMemory*)buffer)->m_curPos; }
	virtual bool setSound(const char* fileName)
	{
		clear();
		FILE* pFile = fopen(fileName, "rb");
		if (pFile == 0)
		{
			return false;
		}
		fseek(pFile, 0, SEEK_END);
		m_size = ftell(pFile);
		fseek(pFile, 0, SEEK_SET);
		m_buffer = new char[m_size + sizeof(OggDecoderInMemory*)];
		OggDecoderInMemory* p = this;
		memcpy(m_buffer, &p, sizeof(OggDecoderInMemory*));
		size_t readSize = fread(m_buffer + sizeof(OggDecoderInMemory*), m_size, 1, pFile);
		if (readSize != 1)
		{
			clear();
			return false;
		}
		ov_callbacks callbacks =
		{
			&OggDecoderInMemory::read,
			&OggDecoderInMemory::seek,
			&OggDecoderInMemory::close,
			&OggDecoderInMemory::tell
		};
		if (ov_open_callbacks(this, &m_ovf, 0, 0, callbacks) != 0)
		{
			clear();
			return false;
		}
		SetFileName(fileName);
		vorbis_info *info = ov_info(&m_ovf, -1);
		SetChannelNum(info->channels);
		SetBitRate(16);
		SetSamplingRate(info->rate);
		SetReady(true);
		return true;
	}
	PCMDecoder* clone()
	{
		if (!isReady()) return 0;
		OggDecoderInMemory* cloneobj = new OggDecoderInMemory();
		*cloneobj = *this;
		cloneobj->m_curPos = 0;
		ov_callbacks callbacks =
		{
			&OggDecoderInMemory::read,
			&OggDecoderInMemory::seek,
			&OggDecoderInMemory::close,
			&OggDecoderInMemory::tell
		};
		if (ov_open_callbacks(cloneobj, &cloneobj->m_ovf, 0, 0, callbacks) != 0)
		{
			cloneobj->clear();
			return false;
		}
		cloneobj->SetFileName(getFileName());
		cloneobj->SetReady(true);
		return cloneobj;
	}
private:
	char* m_buffer;
	int m_size;
	long m_curPos;
};
