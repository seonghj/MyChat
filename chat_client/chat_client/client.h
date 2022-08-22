#pragma once
#include "stdafx.h"
#include "protocol.h"
#include "GUI.h"
#include "resource.h"

class CGUI;

class CCLIENT{
public:
	CCLIENT() {};
	~CCLIENT() {};

	SOCKET sock;
	char Sendbuf[BUFSIZE];
	char Recvbuf[BUFSIZE];
	char ipaddr[50];
	DWORD recvbytes;
	DWORD sendbytes;
	WSABUF wsabuf;

	char userID[20];
	char userPW[20];

	// 1 = success 2 = fail
	int LoginState = 0;
	char serverIP[50];


	void display_error(const char* msg, int err_no);
	void err_quit(const char* msg);
	void ProcessPacket(char* buf);
	void RecvThreadFunc();
	void SendPacket(char* buf);
	void SendDisconnectPacket();
	void SendChatPacket(char* buf);
	void SendLoginPacket(char* id, char* pw);
	void SendJoinPacket(char* id, char* pw);
	void connect_server(CGUI* pGUI);
	void SetpGUI(CGUI* pGUI) { m_pGUI = pGUI; };

	std::thread RecvThread;
private:
	int My_Key = INVALIDID;
	CGUI* m_pGUI = NULL;
};