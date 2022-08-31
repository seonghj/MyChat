#include "GUI.h"
#include "resource.h"

// 에디트 컨트롤 출력 함수
void CGUI::DisplayText(const char* fmt, ...)
{
	va_list arg;
	va_start(arg, fmt);
	char cbuf[BUFSIZE * 2];
	vsprintf_s(cbuf, fmt, arg);
	va_end(arg);

	int nLength = GetWindowTextLength(hEdit2);
	SendMessage(hEdit2, EM_SETSEL, nLength, nLength);
	SendMessageA(hEdit2, EM_REPLACESEL, FALSE, (LPARAM)cbuf);
}

void CGUI::DisplayText(const wchar_t* fmt, ...)
{
	va_list arg;
	va_start(arg, fmt);
	wchar_t cbuf[BUFSIZE * 2];
	vswprintf_s(cbuf, fmt, arg);
	va_end(arg);

	int nLength = GetWindowTextLength(hEdit2);
	SendMessage(hEdit2, EM_SETSEL, nLength, nLength);
	SendMessageA(hEdit2, EM_REPLACESEL, FALSE, (LPARAM)cbuf);
}

int CGUI::StartDialog(HINSTANCE hInstance, UINT Tdlg, CNETWORK* pClient)
{
	m_pNetwork = pClient;
	m_pHInstance = &hInstance;
	CurrDlgID = Tdlg;
	return DialogBoxParamW(*m_pHInstance, MAKEINTRESOURCEW(Tdlg), hDlg, &DlgProc, (LPARAM)this);
}

void CGUI::OpenNewDialog(UINT Tdlg)
{
	//EndDialog(hDlg, IDCANCEL);
	CurrDlgID = Tdlg;
	DialogBoxParamW(*m_pHInstance, MAKEINTRESOURCEW(Tdlg), hDlg, &DlgProc, (LPARAM)this);
}

void CGUI::AddUserList(char* user)
{
	SendMessage(hUserList, LB_ADDSTRING, 0, (LPARAM)user);
}

void CGUI::DeleteUserList(char* user)
{
	char name[20];
	int cnt = SendMessage(hUserList, LB_GETCOUNT, 0, 0);
	int del;
	for (int i = 0; i <= cnt; i++) {
		SendMessage(hUserList, LB_GETTEXT, i, (LPARAM)name);
		if (strcmp(name, user) == 0) {
			SendMessage(hUserList, LB_DELETESTRING, i, 0);
			break;
		}
	}
}

// 대화상자 프로시저
INT_PTR CALLBACK CGUI::DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CGUI* self = (CGUI*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);

	switch (uMsg) {
	case WM_INITDIALOG: {
		::SetWindowLongPtr(hWnd, GWLP_USERDATA, lParam);
		self = (CGUI*)lParam;
		self->hDlg = hWnd;
		if (self->CurrDlgID == IDD_DIALOG1){
			self->hEdit1 = GetDlgItem(hWnd, IDC1_INPUT);
			self->hEdit2 = GetDlgItem(hWnd, IDC1_OUTPUT);
			SendMessage(self->hEdit1, EM_SETLIMITTEXT, BUFSIZE, 0);
			self->hUserList = GetDlgItem(hWnd, IDC1_LIST);
			self->m_pNetwork->SendGetUserListPacket();
		}
		else if (self->CurrDlgID == IDD_DIALOG4) {
			char Items[][15] = { "room1","room2","room3","room4","room5" };
			self->hRoomList = GetDlgItem(hWnd, IDC4_LIST);
			for (int i = 0; i < 5; ++i)
				SendMessage(self->hRoomList, LB_ADDSTRING, 0, (LPARAM)Items[i]);
		}
		return TRUE;
	}
	case WM_CTLCOLOREDIT:
	{
		
		return TRUE;
	}
	case WM_COMMAND: {
		if (self->CurrDlgID == IDD_DIALOG1)
			self->OnCommandDlg1(hWnd, wParam);
		else if (self->CurrDlgID == IDD_DIALOG2)
			self->OnCommandDlg2(hWnd, wParam);
		else if (self->CurrDlgID == IDD_DIALOG3)
			self->OnCommandDlg3(hWnd, wParam);
		else if (self->CurrDlgID == IDD_DIALOG4)
			self->OnCommandDlg4(hWnd, wParam);
		return FALSE;
	}
	}
	return FALSE;
}


void CGUI::OnCommandDlg1(HWND hWnd, WPARAM wParam)
{
	switch (LOWORD(wParam)) {
	case ID1_SEND: {
		GetDlgItemTextA(hWnd, IDC1_INPUT, buf, BUFSIZE + 1);
		//g_GUI->DisplayText("%s\r\n", g_GUI->buf);
		m_pNetwork->SendChatPacket(buf);
		SendMessage(hEdit1, EM_SETSEL, 0, -1); // 텍스트 전체 선택
		ZeroMemory(buf, sizeof(buf));
		SetDlgItemTextA(hWnd, IDC1_INPUT, NULL);
		break;
	}
	case ID1_END: {
		m_pNetwork->SendDisconnectPacket();
		EndDialog(hWnd, IDCANCEL); // 대화상자 닫기
		if (m_pNetwork->LoginState == 1) m_pNetwork->RecvThread.join();
		break;
	}
	}
}

void CGUI::OnCommandDlg2(HWND hWnd, WPARAM wParam)
{
	switch (LOWORD(wParam)) {
	case IDC2_CONNECT: {
		GetDlgItemTextA(hWnd, IDC2_IPADDRESS, buf, BUFSIZE + 1);
		memcpy(m_pNetwork->serverIP, buf, 50);
		m_pNetwork->SetpGUI(this);
		m_pNetwork->connect_server(this);
		EndDlg();
		OpenNewDialog(IDD_DIALOG3);
		ZeroMemory(buf, sizeof(buf));
		break;
	}
	case IDC2_EXIT: {
		EndDialog(hWnd, IDCANCEL); // 대화상자 닫기
		break;
	}
	}
}

void CGUI::OnCommandDlg3(HWND hWnd, WPARAM wParam)
{
	switch (LOWORD(wParam)) {
	case IDC3_LOGIN: {
		GetDlgItemTextA(hWnd, IDC3_ID, m_pNetwork->userID, 20);
		GetDlgItemTextA(hWnd, IDC3_PW, m_pNetwork->userPW, 20);
		m_pNetwork->SetpGUI(this);
		m_pNetwork->SendLoginPacket(m_pNetwork->userID, m_pNetwork->userPW);
		while (1) {
			if (m_pNetwork->LoginState == 1) {
				EndDlg();
				OpenNewDialog(IDD_DIALOG4);
				break;
			}
			else if (m_pNetwork->LoginState == 2) break;
			Sleep(2000);
		}
		break;
	}
	case IDC3_JOIN: {
		GetDlgItemTextA(hWnd, IDC3_ID, m_pNetwork->userID, 20);
		GetDlgItemTextA(hWnd, IDC3_PW, m_pNetwork->userPW, 20);
		m_pNetwork->SendJoinAccountPacket(m_pNetwork->userID, m_pNetwork->userPW);
		break;
	}
	case IDC3_END: {
		m_pNetwork->SendDisconnectPacket();
		EndDialog(hWnd, IDCANCEL); // 대화상자 닫기
		if (m_pNetwork->LoginState != 0) m_pNetwork->RecvThread.join();
		break;
	}
	}
}

void CGUI::OnCommandDlg4(HWND hWnd, WPARAM wParam)
{
	switch (LOWORD(wParam)) {
	case IDC4_LIST: {
		if (HIWORD(wParam) == LBN_SELCHANGE){
			SelectRoom = SendMessage(hRoomList, LB_GETCURSEL, 0, 0);
			char text[15];
			SendMessage(hRoomList, LB_GETTEXT, SelectRoom, (LPARAM)text);
		}
		break;
	}
	case IDC4_JOIN: {
		if (SelectRoom == INVALIDID) break;
		m_pNetwork->SendJoinRoomPacket(SelectRoom);
		while (1) {
			if (m_pNetwork->JoinRoom == 1) {
				EndDlg();
				OpenNewDialog(IDD_DIALOG1);
				break;
			}
			else if (m_pNetwork->JoinRoom == 2) break;
			Sleep(2000);
		}
		break;
	}
	case IDC4_END: {
		m_pNetwork->SendDisconnectPacket();
		EndDialog(hWnd, IDCANCEL); // 대화상자 닫기
		if (m_pNetwork->LoginState != 0) m_pNetwork->RecvThread.join();
		break;
	}
	}
}

void CGUI::EndDlg()
{
	EndDialog(hDlg, IDCANCEL);
}

