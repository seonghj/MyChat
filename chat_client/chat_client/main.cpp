#include "GUI.h"
#include "client.h"
#include "resource.h"
#include <windows.h>

#ifdef _DEBUG
	#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
#endif


CGUI* g_GUI = new CGUI;
CCLIENT* g_client = new CCLIENT;

HINSTANCE* g_hInstance;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	g_hInstance = &hInstance;
	g_GUI->StartDialog(hInstance, IDD_DIALOG2, g_client);
	return 0;
}