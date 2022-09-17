#include "DB.h"

bool DB::Send_Query(char* query)
{
    int state = mysql_query(connection, query);
    if (state != 0)
    {
        printf("MySQL query error : %s\n", mysql_error(&conn));
        return 0;
    }
    else
        return 1;
}

bool DB::Connection_ODBC()
{
    if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv)
        != SQL_SUCCESS)
        return false;
    if (SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, SQL_IS_INTEGER)
        != SQL_SUCCESS)
        return false;
    if (SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc)
        != SQL_SUCCESS)
        return false;
    SQLSetConnectAttr(hDbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);
    if (SQLConnect(hDbc, (SQLWCHAR*)L"MyChat", SQL_NTS
        , (SQLWCHAR*)L"root", SQL_NTS
        , (SQLWCHAR*)L"123456789", SQL_NTS)
        != SQL_SUCCESS) {
        std::cout << "DB error!\n";
        return false;
    }
    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt)
        != SQL_SUCCESS) {
        std::cout << "DB error!\n";
        return false;
    }
    std::cout << "DB connected!\n";
    return true;
}

void DB::Disconnection_ODBC()
{
    if (hStmt)
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    if (hDbc)
        SQLDisconnect(hDbc);
    if (hDbc)
        SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
    if (hEnv)
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}


bool DB::Check_Login(char* id, char* pw, long long uid)
{
    wchar_t query[512] = L"SELECT PassWord, UID FROM mychat.userinfo WHERE ID = '";
    wchar_t wcID[40];
    wchar_t wcPW[40];

    char PW[45];
    SQLLEN len = 0;

    MultiByteToWideChar(CP_ACP, 0, id, -1, wcID, sizeof(id));

    wcscat_s(query, wcID);
    wcscat_s(query, L"'");

    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt)
        != SQL_SUCCESS)
        return false;

    wprintf(L"%s\n", query);

    if (SQLExecDirect(hStmt, (SQLWCHAR*)query, SQL_NTS)
        != SQL_SUCCESS) {
        printf("Check Login Query invaild\n");
        return false;
    }
    SQLBindCol(hStmt, 1, SQL_C_CHAR, PW, sizeof(PW), &len);
    SQLBindCol(hStmt, 2, SQL_C_SBIGINT, &uid, sizeof(SQLBIGINT), &len);
    if (SQLFetch(hStmt) == SQL_NO_DATA) {
        return false;
    }
    if (hStmt) SQLCloseCursor(hStmt);

    for (int i = 1; i < MAX_CLIENT; ++i) {
        if (!m_pServer->sessions[i].connected) continue;
        if (m_pServer->sessions[i].uid == uid) return false;
    }

    if (strcmp(PW, pw) == 0) return true;
    return false;
}

bool DB::Add_User(char* id, char* pw, int uid)
{
    wchar_t query[512] = L"insert into mychat.userinfo VALUES ('";
    wchar_t wcID[40];
    wchar_t wcPW[40];
    wchar_t wcUID[40];

    // UID »ý¼º
    std::time_t t = std::time(nullptr);
    std::tm now;
    localtime_s(&now, &t);

    char time[20];
    sprintf_s(time, "%03d%02d%02d%02d%02d%02d"
        , now.tm_year
        , now.tm_mon
        , now.tm_mday
        , now.tm_hour
        , now.tm_min
        , now.tm_sec);

    MultiByteToWideChar(CP_ACP, 0, id, -1, wcID, sizeof(id));
    MultiByteToWideChar(CP_ACP, 0, pw, -1, wcPW, sizeof(pw));
    MultiByteToWideChar(CP_ACP, 0, time, -1, wcUID, sizeof(time));

    wcscat_s(query, wcID);
    wcscat_s(query, L"', '");
    wcscat_s(query, wcPW);
    wcscat_s(query, L"', '");
    wcscat_s(query, wcUID);
    wcscat_s(query, L"')");
#ifdef Test_DB 
    wprintf(L"%s\n", query);
#endif

    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt)
        != SQL_SUCCESS)
        return false;

    if (SQLExecDirect(hStmt, (SQLWCHAR*)query, SQL_NTS)
        != SQL_SUCCESS) {
        printf("Add_User Query invaild\n");
        return false;
    }

    if (hStmt) SQLCloseCursor(hStmt);

    return true;
}

bool DB::SaveLog(char* id, char* buf, int room)
{
    wchar_t query[1024] = L"insert into mychat.chatlog VALUES ('";
    wchar_t wcID[40];
    wchar_t wcBuf[202];
    wchar_t wcRoom[10];
    std::string tmp = std::to_string(room);
    const char* roomnum = tmp.c_str();

    ZeroMemory(wcBuf, sizeof(wcBuf));
    MultiByteToWideChar(CP_ACP, 0, id, -1, wcID, sizeof(id));
    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, buf, strlen(buf), wcBuf, strlen(buf));
    MultiByteToWideChar(CP_ACP, 0, roomnum, -1, wcRoom, sizeof(roomnum));

    wcscat_s(query, wcRoom);
    wcscat_s(query, L"', '");
    wcscat_s(query, wcID);
    wcscat_s(query, L"', '");
    wcscat_s(query, wcBuf);
    wcscat_s(query, L"', CURRENT_TIMESTAMP())");
#ifdef Test_DB 
    wprintf(L"%s\n", query);
#endif

    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt)
        != SQL_SUCCESS)
        return false;

    if (SQLExecDirect(hStmt, (SQLWCHAR*)query, SQL_NTS)
        != SQL_SUCCESS) {
        printf("Query invaild\n");
        return false;
    }

    if (hStmt) SQLCloseCursor(hStmt);

    return true;
}