#define _CRT_SECURE_NO_WARNINGS
#pragma comment ( lib, "libogg_static.lib" )
#pragma comment ( lib, "libvorbis_static.lib" )
#pragma comment ( lib, "libvorbisfile_static.lib" )
#include "OggDecoder.h"
#include "OggDecodeInMemory.h"
#include "PCMPlayerh.h"

IXAudio2*g_pXaudio = nullptr;
IXAudio2MasteringVoice* g_pMasteringVoice = nullptr;

bool CreateAudio()
{
	if (g_pXaudio != nullptr || g_pMasteringVoice != nullptr)		{ return false;}
	if (FAILED(CoInitializeEx(0, COINIT_MULTITHREADED)))			{ return false;}
	if (FAILED(XAudio2Create(&g_pXaudio)))							{ return false;}
	if (FAILED(g_pXaudio->CreateMasteringVoice(&g_pMasteringVoice))){ return false;}
	return true;
}

void DestroyAudio()
{
	if (g_pMasteringVoice){
		g_pMasteringVoice->DestroyVoice();
		g_pMasteringVoice = nullptr;
	}
	if (g_pXaudio){
		g_pXaudio->Release();
		g_pXaudio = nullptr;
	}
	CoUninitialize();
}

void StopProc()
{
	while (1){ Sleep(200); if (GetAsyncKeyState(VK_ESCAPE)) { break; }}
}

int main(int argc, char* argv[])
{
	CreateAudio();
	OggDecoderInMemory* pDecoder = new OggDecoderInMemory("Test.ogg");	//���������烍�[�h������
	//auto* pDecoder = new OggDecoder("Test.ogg");						//�t�@�C�����畁�ʂɃ��[�h�������Ƃ��͂�����
	PCMPlayer* player = new PCMPlayer(pDecoder, g_pXaudio);
	bool isLoop = true;
	player->setDevice(g_pXaudio);
	player->setDecoder(pDecoder);
	if (player->play(isLoop) == false)
	{
		printf("failed Test.ogg�t�@�C�����Ȃ����A�Đ��Ɏ��s���Ă��܂�\n");
	}
	StopProc();			//ESCAPE�L�[������܂ő҂�
	player->stop();
	printf("\n��~���܂���\n");
	DestroyAudio();
	delete pDecoder;
	delete player;
	return 0;
}
