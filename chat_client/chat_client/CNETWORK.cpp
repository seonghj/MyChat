#include "CNETWORK.h"
#pragma warning(disable : 4996)

void CNETWORK::display_error(const char* msg, int err_no)
{
    WCHAR* lpMsgBuf;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, err_no, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    std::cout << msg;
    std::wcout << L"���� " << lpMsgBuf << std::endl;
    LocalFree(lpMsgBuf);
}

void CNETWORK::err_quit(const char* msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    MessageBox(NULL, (LPCTSTR)lpMsgBuf, (LPCSTR)msg, MB_ICONERROR);
    LocalFree(lpMsgBuf);
    exit(1);
}

void CNETWORK::ProcessPacket(char* buf)
{
    Packet* p = reinterpret_cast<Packet*>(buf);
    switch (p->type){
    case PACKETTYPE::SC_CONNECT: {
        SC_CONNECT_PACKET* p = reinterpret_cast<SC_CONNECT_PACKET*>(buf);
        My_Key = p->key;
        //m_pGUI->OpenNewDialog(IDD_DIALOG3);
        break;
    }
    case PACKETTYPE::SC_LOGINOK: {
        SC_LOGINOK_PACKET* p = reinterpret_cast<SC_LOGINOK_PACKET*>(buf);
        LoginState = 1;
        break;
    }
    case PACKETTYPE::SC_USERLOGIN: {
        SC_USERLOGIN_PACKET* p = reinterpret_cast<SC_USERLOGIN_PACKET*>(buf);
        m_pGUI->AddUserList(p->id);
        break;
    }
    case PACKETTYPE::SC_LOGINFAIL: {
        SC_LOGINFAIL_PACKET* p = reinterpret_cast<SC_LOGINFAIL_PACKET*>(buf);
        LoginState = 2;
        MessageBox(m_pGUI->hMessageBox, "Login fail", "Login Fail", MB_OK);
        break;
    }
    case PACKETTYPE::SC_CHAT: {
        SC_CHAT_PACKET* p = reinterpret_cast<SC_CHAT_PACKET*>(buf);
        m_pGUI->DisplayText("%s: ", p->id);
        m_pGUI->DisplayText("%s\r\n", p->buf);
        break;
    }
    case PACKETTYPE::SC_JOINROOM: {
        SC_JOINROOM_PACKET* p = reinterpret_cast<SC_JOINROOM_PACKET*>(buf);
        JoinRoom = 1;
        break;
    }
    case PACKETTYPE::SC_USERLIST: {
        SC_USERLIST_PACKET* p = reinterpret_cast<SC_USERLIST_PACKET*>(buf);
        for (int i = 0; i < 10; i++) {
            if (!strcmp(p->id[i], "end")) break;
            m_pGUI->AddUserList(p->id[i]);
        }
        break;
    }
    case PACKETTYPE::SC_USERLOGOUT: {
        SC_USERLOGOUT_PACKET* p = reinterpret_cast<SC_USERLOGOUT_PACKET*>(buf);
        m_pGUI->DeleteUserList(p->id);
        break;
    }
    default:
        break;
    }
}

void CNETWORK::RecvThreadFunc() 
{
    DWORD flags = 0;
    int retval = 0;
    int saved_packet_size = 0;

    bool isRun = true;

    char recvbuf[BUFSIZE];
    char buffer[BUFSIZE];

    int rest_size = retval;
    DWORD packet_size = 0;


    WSABUF r_wsabuf;
    r_wsabuf.buf = recvbuf;
    r_wsabuf.len = BUFSIZE;

    while (isRun) {
        recvbytes = 0;
        retval = WSARecv(sock, &r_wsabuf, 1, &recvbytes, &flags, NULL, NULL);
        if (retval == SOCKET_ERROR) {
            int err_no = WSAGetLastError();
            if (err_no != WSA_IO_PENDING) {
                printf("recv error %d packet: %d error\n", err_no, r_wsabuf.buf[1]);
                printf("server disconnect\n");
                closesocket(sock);
                break;
            }
        }
        int rest_size = recvbytes;
        unsigned char* buf_ptr = reinterpret_cast<unsigned char*>(recvbuf);
        while (rest_size > 0)
        {
            if (0 == packet_size)
                packet_size = recvbytes;
            if (rest_size + saved_packet_size >= packet_size) {
                std::memcpy(buffer + saved_packet_size, buf_ptr, packet_size - saved_packet_size);
                ProcessPacket(buffer);
                buf_ptr += packet_size - saved_packet_size;
                rest_size -= packet_size - saved_packet_size;
                packet_size = 0;
                saved_packet_size = 0;
            }
            else {
                std::memcpy(buffer + saved_packet_size, buf_ptr, rest_size);
                saved_packet_size += rest_size;
                rest_size = 0;
            }
        }
    }
}

void CNETWORK::SendPacket(char* buf)
{
    Packet* p = reinterpret_cast<Packet*>(buf);
    int retval = 0;
    wsabuf.len = p->size;
    wsabuf.buf = buf;
    retval = WSASend(sock, &wsabuf, 1, &sendbytes, 0, NULL, NULL);
    if (retval == SOCKET_ERROR) {
        printf("%d: ", WSAGetLastError());
        display_error("send()", retval);
    }
}

void CNETWORK::SendDisconnectPacket()
{
    CS_DISCONNECT_PACKET p;
    p.size = sizeof(p);
    p.type = PACKETTYPE::CS_DISCONNECT;
    p.key = My_Key;
    SendPacket(reinterpret_cast<char*>(&p));
}

void CNETWORK::SendChatPacket(char* buf)
{
    CS_CHAT_PACKET p;
    p.size = sizeof(p);
    p.type = PACKETTYPE::CS_CHAT;
    p.key = My_Key;
    strcpy_s(p.buf, buf);
    SendPacket(reinterpret_cast<char*>(&p));
}

void CNETWORK::SendLoginPacket(char* id, char* pw)
{
    CS_LOGIN_PACKET p;
    p.size = sizeof(p);
    p.type = PACKETTYPE::CS_LOGIN;
    p.key = My_Key;
    strcpy_s(p.id, id);
    strcpy_s(p.pw, pw);
    SendPacket(reinterpret_cast<char*>(&p));
}

void CNETWORK::SendJoinAccountPacket(char* id, char* pw)
{
    CS_JOINACCOUNT_PACKET p;
    p.size = sizeof(p);
    p.type = PACKETTYPE::CS_JOINACCOUNT;
    p.key = My_Key;
    strcpy_s(p.id, id);
    strcpy_s(p.pw, pw);
    SendPacket(reinterpret_cast<char*>(&p));
}

void CNETWORK::SendJoinRoomPacket(int room)
{
    CS_JOINROOM_PACKET p;
    p.size = sizeof(p);
    p.type = PACKETTYPE::CS_JOINROOM;
    p.key = My_Key;
    p.room = room;
   
    SendPacket(reinterpret_cast<char*>(&p));
}

void CNETWORK::SendGetUserListPacket()
{
    CS_GETUSERLIST_PACKET p;
    p.size = sizeof(p);
    p.type = PACKETTYPE::CS_GETUSERLIST;
    p.key = My_Key;
    p.room = RoomNum;

    SendPacket(reinterpret_cast<char*>(&p));
}

void CNETWORK::connect_server(CGUI* pGUI)
{
    m_pGUI = pGUI;
    int retval = 0;

    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    // socket()
    sock = WSASocketW(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (sock == INVALID_SOCKET)err_quit("socket()");

    SOCKADDR_IN serveraddr;
    ZeroMemory(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(serverIP);
    serveraddr.sin_port = htons(SERVERPORT);
    retval = connect(sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) err_quit("connect()");
    ServerConnected = TRUE;
    RecvThread = std::thread(&CNETWORK::RecvThreadFunc, this);
}