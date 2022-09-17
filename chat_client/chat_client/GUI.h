#pragma once
#include "stdafx.h"
#include "protocol.h"
#include "CNETWORK.h"

class CNETWORK;

class CGUI {
public:
	CGUI() {};
	~CGUI() {};

	WNDCLASS wndclass;
	
	HWND hEdit1, hEdit2;
	HWND hDlg;
	HWND hUserList, hRoomList;
	HWND hMessageBox;

	MSG msg;

	int CurrDlgID;
	int SelectRoom = INVALIDID;

	char buf[BUFSIZE + 1];

	void DisplayText(const char* fmt, ...);
	void DisplayText(const wchar_t* fmt, ...);
	int StartDialog(HINSTANCE hInstance, UINT uMsg, CNETWORK* pClient);
	void OpenNewDialog(UINT Tdlg);

	void AddUserList(char* user);

	void DeleteUserList(char* user);

	static INT_PTR DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

	HINSTANCE* GetHInstance() { return m_pHInstance; }

	void OnCommandDlg1(HWND hWnd, WPARAM wParam);
	void OnCommandDlg2(HWND hWnd, WPARAM wParam);
	void OnCommandDlg3(HWND hWnd, WPARAM wParam);

	void OnCommandDlg4(HWND hWnd, WPARAM wParam);

	void OnCommandDlg5(HWND hWnd, WPARAM wParam);

	void EndDlg();

private:
	CNETWORK* m_pNetwork = NULL;
	HINSTANCE* m_pHInstance;
};