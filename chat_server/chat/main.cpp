#include "server.h"
#include "stdafx.h"
#include "DB.h"

CSERVER* g_pServer = new CSERVER;
DB* g_pDB = new DB;

int main()
{
	std::wcout.imbue(std::locale("korean"));

	g_pServer->m_pDB = g_pDB;
	g_pDB->Connection_ODBC();

	g_pServer->Init();

	g_pServer->Thread_join();
	std::cout << "½Ã¹ß\n" << std::endl;

	delete g_pServer;
	return 0;
}