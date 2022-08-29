#pragma once
#include <WinSock2.h>

constexpr int SERVERPORT = 3500;
constexpr int BUFSIZE = 128;
constexpr int MAX_CLIENT = 1000;
constexpr int MAX_USER_INROOM = 100;
constexpr int MAX_ROOM = 100;
constexpr int SERVERID = 0;
constexpr int INVALIDID = 987654321;

#define SERVERIP   "127.0.0.1"
//#define SERVERIP   "39.120.192.92"

enum OVER_EX_Type {
	OE_accept,
	OE_send,
	OE_recv,
};

struct OVER_EX
{
	WSAOVERLAPPED	overlapped;
	WSABUF			dataBuffer;
	char			messageBuffer[BUFSIZE];
	SOCKET			csocket;
	OVER_EX_Type    type;
};

enum PACKETTYPE {
	SC_CONNECT,
	SC_LOGINOK,
	SC_LOGINFAIL,
	SC_USERLOGIN,
	SC_USERLOGOUT,
	SC_JOINROOM,
	SC_CHAT,
	SC_USERLIST,

	CS_DISCONNECT,
	CS_LOGIN,
	CS_CHAT,
	CS_JOINACCOUNT,
	CS_JOINROOM,
	CS_GETUSERLIST
};

struct Packet {
	unsigned int size;
	char type;
	unsigned int key;
};

struct SC_CONNECT_PACKET :public Packet {
};

struct SC_LOGINOK_PACKET :public Packet {
};

struct SC_LOGINFAIL_PACKET :public Packet {
};

struct SC_USERLOGIN_PACKET :public Packet {
	char id[20];
};
struct SC_USERLOGOUT_PACKET :public Packet {
	char id[20];
};

struct SC_CHAT_PACKET :public Packet {
	char id[20];
	char buf[100];
	int time;
};

struct SC_JOINROOM_PACKET :public Packet {
	unsigned int room;
};

struct SC_USERLIST_PACKET :public Packet {
	char id[11][20];
};

struct CS_DISCONNECT_PACKET :public Packet {
};

struct CS_LOGIN_PACKET :public Packet {
	char id[20];
	char pw[20];
};

struct CS_JOINACCOUNT_PACKET :public Packet {
	char id[20];
	char pw[20];
};
struct CS_CHAT_PACKET :public Packet {
	char buf[100];
};

struct CS_JOINROOM_PACKET :public Packet {
	unsigned int room;
};

struct CS_GETUSERLIST_PACKET :public Packet {
	unsigned int room;
};