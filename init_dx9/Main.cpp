#include <Windows.h>
#include <d3d9.h>
#pragma comment(lib, "d3d9.lib")

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR cmd, int cmdShow)
{
	auto window = CreateWindow(TEXT("edit"), 0, WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, 320, 240, 0, 0, 0, 0);
	auto deviceContext = GetDC(window);
	LPDIRECT3D9 g_pD3D;
	LPDIRECT3DDEVICE9 g_pD3DDev;
	if (!(g_pD3D = Direct3DCreate9(D3D_SDK_VERSION))) return 0;
	D3DPRESENT_PARAMETERS d3dpp = { 0,0,D3DFMT_UNKNOWN,0,D3DMULTISAMPLE_NONE,0,D3DSWAPEFFECT_DISCARD,NULL,TRUE,0,D3DFMT_UNKNOWN,0,0 };
	if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, window,D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &g_pD3DDev))){g_pD3D->Release(); return 0;}
	MSG msg = {};
	while (true)
	{
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_NCLBUTTONDOWN && msg.wParam == HTCLOSE){return 0;}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		g_pD3DDev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(116,169, 214), 1.0f, 0);
		g_pD3DDev->BeginScene();
		g_pD3DDev->EndScene();
		g_pD3DDev->Present(NULL, NULL, NULL, NULL);
	} 
	g_pD3DDev->Release();
	g_pD3D->Release();
}