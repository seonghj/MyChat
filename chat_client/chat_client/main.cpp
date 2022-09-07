#include "GUI.h"
#include "CNETWORK.h"
#include "resource.h"
#include <windows.h>

#ifdef _DEBUG
	#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
#endif


CGUI* g_GUI = new CGUI;
CNETWORK* g_cnetwork = new CNETWORK;

HINSTANCE* g_hInstance;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	g_hInstance = &hInstance;
	g_GUI->StartDialog(hInstance, IDD_DIALOG2, g_cnetwork);

	g_cnetwork->RecvThread.join();
	return 0;
}