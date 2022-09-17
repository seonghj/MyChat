#pragma once
#include "stdafx.h"
#include "sql.h"
#include "sqlext.h"
#include <mysql.h>
#include "server.h"
#pragma comment(lib, "libmySQL.lib")

#define DB_HOST "127.0.0.1"
#define DB_USER "root"
#define DB_PW "tjdwo@1034"
#define DB_NAME "GS_termproject"

class CSERVER;

class DB
{
public:
	MYSQL* connection = NULL;
	MYSQL conn;
	MYSQL_RES* sql_result;
	MYSQL_ROW sql_row;
	char query[255];
	bool isRun = false;

	bool Send_Query(char* query);

	SQLHENV hEnv;
	SQLHDBC hDbc;
	SQLHSTMT hStmt = 0;

	bool Connection_ODBC();
	void Disconnection_ODBC();
	bool Check_Login(char* id, char* pw, long long uid);
	bool Add_User(char* id, char* pw, int uid);
	bool SaveLog(char* id, char* buf, int room);

	void Set_pServer(CSERVER* s) { m_pServer = s; }

	CSERVER* m_pServer = NULL;
};