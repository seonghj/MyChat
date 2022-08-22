#pragma once
#include "stdafx.h"
#include "protocol.h"
#include "DB.h"

class SESSION
{
public:

    SESSION() {}
    SESSION(const SESSION& session) {}
    SESSION(BOOL b) : connected(b) {}
    ~SESSION() {}

    OVER_EX                  over;
    SOCKET                   sock;
    SOCKADDR_IN              clientaddr;
    int                      addrlen;
    char                     packet_buf[BUFSIZE];
    int                      prev_size = 0;
    char*                    packet_start;
    char*                    recv_start;

    std::atomic<bool>        connected = false;
    std::atomic<int>         key = INVALIDID;

    char                     id[20];
    char                     pw[20];

    void init();

private:
    std::mutex               s_lock;
};


class CSERVER 
{
public: 
    void display_error(const char* msg, int err_no);
    void do_recv(int key);
    void send_packet(int to, char* packet);
    void send_chat_to_all(int key, char* buf);
    void send_loginOK_packet(int key);
    void send_userlogin_packet(int key);
    void send_loginFail_packet(int key);
    void send_chat_packet(int key, char* buf);
    void process_packet(int key, char* buf);
    void Disconnect(int key);
    void WorkerFunc();
    bool Init();
    void Thread_join();
    int SetKey();

    SESSION sessions[MAX_CLIENT];

    DB* m_pDB;

private:
    SOCKET                         listenSocket;
    HANDLE                         hcp;

    std::vector <std::thread>      working_threads;
};