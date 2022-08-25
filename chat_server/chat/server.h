#pragma once
#include "stdafx.h"
#include "protocol.h"
#include "DB.h"

class SESSION{
public:

    SESSION() {}
    ~SESSION() {}

    OVER_EX                  over;
    SOCKET                   sock;
    SOCKADDR_IN              clientaddr;
    int                      addrlen;
    char                     packet_buf[BUFSIZE];
    int                      prev_size = 0;
    char*                    packet_start;
    char*                    recv_start;

    bool                     connected = false;
    int                      key       = INVALIDID;
    int                      room      = INVALIDID;

    char                     id[20];
    char                     pw[20];

    void init();

private:
    std::mutex               s_lock;
};

class ROOM {
public:
    std::vector<int> USERS;

    bool             working = false;
    std::mutex       room_lock;

    void init();
};


class CSERVER {
public: 
    void display_error(const char* msg, int err_no);
    void do_recv(int key);
    void send_packet(int to, char* packet);
    void SendChat(int key, int room, char* buf);
    void SendLoginOK(int key);
    void SendUserLogin(int key);
    void SendLoginFail(int key);
    void SendChat(int key, char* buf);
    void SendJoinRoom(int key, int room);
    void process_packet(int key, char* buf);
    void Disconnect(int key);
    void WorkerFunc();
    bool Init();
    void Thread_join();
    int SetKey();
    int SetRoomKey();

    SESSION sessions[MAX_CLIENT];
    ROOM    rooms[MAX_ROOM];

    DB* m_pDB;

private:
    SOCKET                         listenSocket;
    HANDLE                         hcp;

    std::vector <std::thread>      working_threads;
};