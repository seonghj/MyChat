#pragma once
#include "stdafx.h"
#include "protocol.h"
#include "client.h"

class CCLIENT;

class CGUI {
public:
	CGUI() {};
	~CGUI() {};

	WNDCLASS wndclass;
	
	HWND hEdit1, hEdit2;
	HWND hDlg;
	HWND hList;

	MSG msg;

	int CurrDlgID;
	int SelectRoom = INVALIDID;

	char buf[BUFSIZE + 1];

	void DisplayText(const char* fmt, ...);
	void DisplayText(const wchar_t* fmt, ...);
	int StartDialog(HINSTANCE hInstance, UINT uMsg, CCLIENT* pClient);
	void OpenNewDialog(UINT Tdlg);

	void AddUserList(char* user);

	static INT_PTR DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

	HINSTANCE* GetHInstance() { return m_pHInstance; }

	void OnCommandDlg1(HWND hWnd, WPARAM wParam);
	void OnCommandDlg2(HWND hWnd, WPARAM wParam);
	void OnCommandDlg3(HWND hWnd, WPARAM wParam);

	void OnCommandDlg4(HWND hWnd, WPARAM wParam);

	void EndDlg();

private:
	CCLIENT* m_pClient = NULL;
	HINSTANCE* m_pHInstance;
};

//char test[10][10] = { "号1", "号2", "号3" , "号4" , "号5"
//, "号6", "号7", "号8", "号9", "号10" };