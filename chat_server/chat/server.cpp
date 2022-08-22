﻿#include "server.h"

void SESSION::init()
{
    //memset(this, 0x00, sizeof(SESSION));
    ZeroMemory(&over.overlapped, sizeof(over.overlapped));

    addrlen = 0;
    memset(packet_buf, 0, sizeof(packet_buf));
    packet_start = nullptr;
    recv_start = nullptr;

    connected = false;
    key = INVALIDID;

    over.dataBuffer.len = BUFSIZE;
    over.dataBuffer.buf = over.messageBuffer;

    memset(id, 0, sizeof(id));
    memset(pw, 0, sizeof(pw));
}

void CSERVER::display_error(const char* msg, int err_no)
{
    WCHAR* lpMsgBuf;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, err_no, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    std::cout << msg;
    std::wcout << L"���� " << lpMsgBuf << std::endl;
    LocalFree(lpMsgBuf);
}

void CSERVER::do_recv(int key)
{
    DWORD flags = 0;

    SOCKET client_s = sessions[key].sock;
    OVER_EX* over = &sessions[key].over;

    over->dataBuffer.len = BUFSIZE;
    over->dataBuffer.buf = over->messageBuffer;
    ZeroMemory(&over->overlapped, sizeof(over->overlapped));

    if (WSARecv(client_s, &over->dataBuffer, 1, (LPDWORD)sessions[key].prev_size,
        &flags, &(over->overlapped), NULL)) {
        int err_no = WSAGetLastError();
        if (err_no != WSA_IO_PENDING) {
            /*if (over->dataBuffer.buf[1] != 0) {
                printf("key: %d recv error: %d / packet: %d\n", key, err_no
                    , over->dataBuffer.buf[1]);
                Disconnect(key, sessions[key].over.roomID);
            }*/
        }
    }
}

void CSERVER::send_packet(int to, char* packet)
{
    SOCKET client_s = sessions[to].sock;

    OVER_EX* over = new OVER_EX;
    ZeroMemory(over, sizeof(OVER_EX));
    over->dataBuffer.buf = packet;
    over->dataBuffer.len = packet[0];

    memcpy(over->messageBuffer, packet, packet[0]);
    over->type = OE_send;
    ZeroMemory(&over->overlapped, sizeof(over->overlapped));

    if (WSASend(client_s, &over->dataBuffer, 1, NULL,
        0, &(over->overlapped), NULL)) {
        int err_no = WSAGetLastError();
        if (err_no != WSA_IO_PENDING) {
            printf("to: %d packet: %d send error: %d\n", to, packet[1], err_no);
        }
    }
}

void CSERVER::send_chat_to_all(int key, char* buf)
{
    SC_CHAT_PACKET p;
    p.size = sizeof(SC_CHAT_PACKET);
    p.type = PACKETTYPE::SC_CHAT;
    p.key = key;
    strcpy_s(p.buf, buf);

    char* s_buf = reinterpret_cast<char*>(&p);

    for (int i = 0; i < MAX_CLIENT; ++i) {
        if (!sessions[i].connected) continue;
        send_packet(i, s_buf);
    }
}

void CSERVER::send_loginOK_packet(int key)
{
    SC_LOGINOK_PACKET p;
    p.size = sizeof(SC_LOGINOK_PACKET);
    p.type = PACKETTYPE::SC_LOGINOK;
    p.key = key;

    send_packet(key, reinterpret_cast<char*>(&p));
}

void CSERVER::send_userlogin_packet(int key)
{
    SC_USERLOGIN_PACKET p;
    p.size = sizeof(SC_USERLOGIN_PACKET);
    p.type = PACKETTYPE::SC_USERLOGIN;
    p.key = key;
    strcpy_s(p.id, sizeof(sessions[key].id), sessions[key].id);

    char* s_buf = reinterpret_cast<char*>(&p);

    for (int i = 0; i < MAX_CLIENT; ++i) {
        if (!sessions[i].connected) continue;
        send_packet(i, s_buf);
    }
}

void CSERVER::send_loginFail_packet(int key)
{
    SC_LOGINFAIL_PACKET p;
    p.size = sizeof(SC_LOGINFAIL_PACKET);
    p.type = PACKETTYPE::SC_LOGINFAIL;
    p.key = key;

    send_packet(key, reinterpret_cast<char*>(&p));
}

void CSERVER::send_chat_packet(int key, char* buf)
{
    SC_CHAT_PACKET p;
    p.size = sizeof(SC_CHAT_PACKET);
    p.type = PACKETTYPE::SC_CHAT;
    p.key = key;
    memcpy(p.buf, buf, 100);

    send_packet(key, reinterpret_cast<char*>(&p));
}

void CSERVER::process_packet(int key, char* buf)
{
    switch (buf[1]) {
    case CS_LOGIN: {
        CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(buf);
        m_pDB->Search_ID(p->id, p->pw);
        bool b = m_pDB->Login(p->id);
        if (b) {
            send_loginOK_packet(key);
            strcpy_s(sessions[key].id, sizeof(p->id), p->id);
            strcpy_s(sessions[key].pw, sizeof(p->pw), p->pw);
            send_userlogin_packet(key);
        }
        else {
            send_loginFail_packet(key);
        }
        break;
    }
    case CS_CHAT: {
        CS_CHAT_PACKET* p = reinterpret_cast<CS_CHAT_PACKET*>(buf);
        send_chat_to_all(p->key, p->buf);
        break;
    }
    case CS_JOIN: {
        CS_JOIN_PACKET* p = reinterpret_cast<CS_JOIN_PACKET*>(buf);
        bool b = m_pDB->Insert_ID(p->id, p->pw);
        break;
    }
    case CS_DISCONNECT: {
        CS_DISCONNECT_PACKET* p = reinterpret_cast<CS_DISCONNECT_PACKET*>(buf);
        Disconnect(p->key);
        break;
    }
    default:
        break;
    }
}

void CSERVER::Disconnect(int key)
{
    closesocket(sessions[key].sock);
    bool b = m_pDB->Logout(sessions[key].id);
    if (b)
        std::cout << "logout: " << key << std::endl;
    sessions[key].init();
}

void CSERVER::WorkerFunc()
{
    DWORD Transferred;
    ULONG key;
    WSAOVERLAPPED* over;

    while (1) {
        BOOL retval = GetQueuedCompletionStatus(hcp, &Transferred,
            (PULONG_PTR)&key, &over, INFINITE);

        OVER_EX* over_ex = reinterpret_cast<OVER_EX*>(over);

        if (false == retval) {
            display_error("GQCS: ", WSAGetLastError());
            Disconnect(key);
        }
        if (SERVERID != key && Transferred == 0) {
            Disconnect(key);
        }

        switch (over_ex->type) {
        case OE_accept: {
            int client_key = SetKey();
            if (client_key != -1) {
                sessions[client_key].sock = over_ex->csocket;
                sessions[client_key].over.type = OE_recv;
                sessions[client_key].prev_size = 0;
                sessions[client_key].init();
                sessions[client_key].connected = TRUE;
                sessions[client_key].key = client_key;
                CreateIoCompletionPort((HANDLE)sessions[client_key].sock, hcp, client_key, 0);
                
                do_recv(client_key);
            }
            else {
                closesocket(over_ex->csocket);
                break;
            }

            ZeroMemory(&over_ex->overlapped, sizeof(over_ex->overlapped));
            SOCKET c_socket = WSASocketW(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
            over_ex->csocket = c_socket;
            BOOL ret = AcceptEx(listenSocket, c_socket
                , over_ex->messageBuffer
                , 0, 32, 32, NULL, &over_ex->overlapped);
            if (false == ret) {
                int err_num = WSAGetLastError();
                if (err_num != WSA_IO_PENDING)
                {
                    display_error("AcceptEx_error ", err_num);
                    printf("%d\n", err_num);
                }
            }
            /*getpeername(sessions[client_key].sock, (SOCKADDR*)&sessions[client_key].clientaddr
                , &sessions[client_key].addrlen);*/
            std::cout << "login: " << client_key << std::endl;
            SC_CONNECT_PACKET p;
            p.size = sizeof(p);
            p.type = PACKETTYPE::SC_CONNECT;
            p.key = client_key;
            send_packet(client_key, reinterpret_cast<char*>(&p));
            break;
        }
        case OE_recv: {
            char* packet_ptr = over_ex->messageBuffer;
            int required_data = Transferred + sessions[key].prev_size;
            int packet_size = packet_ptr[0];
            while (required_data >= packet_size) {
                if (required_data >= BUFSIZE) break;
                if (packet_size <= 0) break;
                //printf("num_data: %d, packet_size: %d prev_size: %d\n", num_data, packet_size, sessions[key].prev_size);
                process_packet(key, packet_ptr);
                required_data -= packet_size;
                packet_ptr += packet_size;
                packet_size = packet_ptr[0];
            }
            packet_size = 0;
            sessions[key].prev_size = 0;
            if (0 != required_data)
                memcpy(over_ex->messageBuffer, packet_ptr, required_data);
            do_recv(key);
            break;
        }
        case OE_send: {
            delete over_ex;
            break;
        }
        }

    }
}

bool CSERVER::Init()
{
    for (auto& s : sessions) {
        s.init();
    }

    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    hcp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
    if (hcp == NULL) return 0;

    listenSocket = WSASocketW(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    CreateIoCompletionPort(reinterpret_cast<HANDLE>(listenSocket), hcp, 0, 0);
    SOCKADDR_IN serverAddr;
    memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVERPORT);
    serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    ::bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(SOCKADDR_IN));
    listen(listenSocket, SOMAXCONN);

    OVER_EX accept_over;
    accept_over.type = OE_accept;
    memset(&accept_over.overlapped, 0, sizeof(accept_over.overlapped));
    SOCKET c_socket = WSASocketW(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    accept_over.csocket = c_socket;
    BOOL ret = AcceptEx(listenSocket, c_socket, accept_over.messageBuffer
        , SERVERID, 32, 32, NULL, &accept_over.overlapped);
    if (false == ret) {
        int err_num = WSAGetLastError();
        if (err_num != WSA_IO_PENDING)
            display_error("AcceptEx_error", err_num);
    }
    printf("ready\n");

    SYSTEM_INFO si;
    GetSystemInfo(&si);

    for (int i = 0; i < (int)si.dwNumberOfProcessors; i++)
        working_threads.emplace_back(std::thread(&CSERVER::WorkerFunc, this));

    return 1;
}

void CSERVER::Thread_join()
{
    for (auto& t : working_threads)
        t.join();

    CloseHandle(hcp);
}

int CSERVER::SetKey()
{
    int cnt = 1;
    while (true) {
        if (cnt == MAX_CLIENT)
            return -1;
        if (sessions[cnt].connected == false)
            return cnt;
        ++cnt;
    }
}
