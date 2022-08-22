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

int CGUI::StartDialog(HINSTANCE hInstance, UINT Tdlg, CCLIENT* pClient)
{
	m_pClient = pClient;
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

// 대화상자 프로시저
INT_PTR CALLBACK CGUI::DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CGUI* self = (CGUI*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);

	switch (uMsg) {
	case WM_INITDIALOG: {
		::SetWindowLongPtr(hWnd, GWLP_USERDATA, lParam);
		self = (CGUI*)lParam;
		self->hDlg = hWnd;
		if (self->CurrDlgID == IDD_DIALOG1) {
			char Items[][15] = { "Apple","Orange","Melon","Graph","Strawberry" };
			self->hEdit1 = GetDlgItem(hWnd, IDC1_INPUT);
			self->hEdit2 = GetDlgItem(hWnd, IDC1_OUTPUT);
			SendMessage(self->hEdit1, EM_SETLIMITTEXT, BUFSIZE, 0);
			self->hUserList = GetDlgItem(hWnd, IDC1_LIST);
		}
		return TRUE;
	}
	case WM_COMMAND: {
		if (self->CurrDlgID == IDD_DIALOG1)
			self->OnCommandDlg1(hWnd, wParam);
		else if (self->CurrDlgID == IDD_DIALOG2)
			self->OnCommandDlg2(hWnd, wParam);
		else if (self->CurrDlgID == IDD_DIALOG3)
			self->OnCommandDlg3(hWnd, wParam);
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
		m_pClient->SendChatPacket(buf);
		SendMessage(hEdit1, EM_SETSEL, 0, -1); // 텍스트 전체 선택
		ZeroMemory(buf, sizeof(buf));
		SetDlgItemTextA(hWnd, IDC1_INPUT, NULL);
		break;
	}
	case ID1_END: {
		m_pClient->SendDisconnectPacket();
		EndDialog(hWnd, IDCANCEL); // 대화상자 닫기
		if (m_pClient->LoginState == 1) m_pClient->RecvThread.join();
		break;
	}
	}
}

void CGUI::OnCommandDlg2(HWND hWnd, WPARAM wParam)
{
	switch (LOWORD(wParam)) {
	case IDC2_CONNECT: {
		GetDlgItemTextA(hWnd, IDC2_IPADDRESS, buf, BUFSIZE + 1);
		memcpy(m_pClient->serverIP, buf, 50);
		m_pClient->SetpGUI(this);
		m_pClient->connect_server(this);
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
		GetDlgItemTextA(hWnd, IDC3_ID, m_pClient->userID, 20);
		GetDlgItemTextA(hWnd, IDC3_PW, m_pClient->userPW, 20);
		m_pClient->SetpGUI(this);
		m_pClient->SendLoginPacket(m_pClient->userID, m_pClient->userPW);
		while (1) {
			if (m_pClient->LoginState == 1) {
				EndDlg();
				OpenNewDialog(IDD_DIALOG1);
				break;
			}
			else if (m_pClient->LoginState == 2) break;
			Sleep(2000);
		}
		break;
	}
	case IDC3_JOIN: {
		GetDlgItemTextA(hWnd, IDC3_ID, m_pClient->userID, 20);
		GetDlgItemTextA(hWnd, IDC3_PW, m_pClient->userPW, 20);
		m_pClient->SendJoinPacket(m_pClient->userID, m_pClient->userPW);
		break;
	}
	case IDC3_END: {
		m_pClient->SendDisconnectPacket();
		EndDialog(hWnd, IDCANCEL); // 대화상자 닫기
		if (m_pClient->LoginState != 0) m_pClient->RecvThread.join();
		break;
	}
	}
}

void CGUI::EndDlg()
{
	EndDialog(hDlg, IDCANCEL);
}

