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
    // 환경 구성
    if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv)
        != SQL_SUCCESS)
        return false;
    // 버전 정보 설정
    if (SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, SQL_IS_INTEGER)
        != SQL_SUCCESS)
        return false;
    if (SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc)
        != SQL_SUCCESS)
        return false;
    // 접속
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

bool DB::Search_ID(char* id, char* pw)
{
    wchar_t query[512] = L"SELECT PassWord FROM mychat.userinfo WHERE ID = '";
    wchar_t wcID[20];
    wchar_t wcPW[20];

    char PW[20];
    SQLLEN len = 0;

    // PW 일치 검사
    wcscat_s(query, wcID);
    wcscat_s(query, L"'");
    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt)
        != SQL_SUCCESS)
        return false;

    if (SQLExecDirect(hStmt, (SQLWCHAR*)query, SQL_NTS)
        != SQL_SUCCESS) {
        printf("Search_ID Query invaild\n");
        return false;
    }
    SQLBindCol(hStmt, 1, SQL_C_CHAR, PW, sizeof(PW), &len);
    if (SQLFetch(hStmt) == SQL_NO_DATA) return false;
    if (hStmt) SQLCloseCursor(hStmt);

    if (strcmp(PW, pw) == 0) return true;
    return false;
}

bool DB::Insert_ID(char* id, char* pw)
{
    wchar_t query[512] = L"insert into mychat.userinfo VALUES ('";
    wchar_t wcID[20];
    wchar_t wcPW[20];

    MultiByteToWideChar(CP_ACP, 0, id, -1, wcID, sizeof(id));
    MultiByteToWideChar(CP_ACP, 0, pw, -1, wcPW, sizeof(pw));

    wcscat_s(query, wcID);
    wcscat_s(query, L"', '0', '");
    wcscat_s(query, wcPW);
    wcscat_s(query, L"')");

#ifdef Test_DB 
    wprintf(L"%s\n", query);
#endif

    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt)
        != SQL_SUCCESS)
        return false;

    if (SQLExecDirect(hStmt, (SQLWCHAR*)query, SQL_NTS)
        != SQL_SUCCESS) {
        printf("Insert_ID Query invaild\n");
        return false;
    }

    if (hStmt) SQLCloseCursor(hStmt);

    return true;
}

bool DB::Login(char* id)
{
    wchar_t query1[512] = L"SELECT isLogin FROM mychat.userinfo WHERE ID = '";
    wchar_t query2[512] = L"UPDATE mychat.userInfo SET isLogin = 1 WHERE ID = '";
    wchar_t wcID[20];
    SQLLEN len = 0;
    bool isLogin;

    MultiByteToWideChar(CP_ACP, 0, id, -1, wcID, sizeof(id));
    
    wcscat_s(query1, wcID);
    wcscat_s(query1, L"'");
    wcscat_s(query2, wcID);
    wcscat_s(query2, L"'");

#ifdef Test_DB 
    wprintf(L"%s\n", query);
#endif

    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt)
        != SQL_SUCCESS)
        return false;

    if (SQLExecDirect(hStmt, (SQLWCHAR*)query1, SQL_NTS)
        != SQL_SUCCESS) {
        printf("Login Query1 invaild\n");
        return false;
    }
    SQLBindCol(hStmt, 1, SQL_C_TINYINT, &isLogin, sizeof(isLogin), &len);
    if (SQLFetch(hStmt) == SQL_NO_DATA) return false;
    if (hStmt) SQLCloseCursor(hStmt);
    if (isLogin == true) return false;

    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt)
        != SQL_SUCCESS)
        return false;

    if (SQLExecDirect(hStmt, (SQLWCHAR*)query2, SQL_NTS)
        != SQL_SUCCESS) {
        printf("Login Query2 invaild\n");
        return false;
    }
    if (hStmt) SQLCloseCursor(hStmt);

    return true;
}

bool DB::Logout(char* id)
{
    wchar_t query[512] = L"UPDATE mychat.userInfo SET isLogin = 0 WHERE ID = '";
    wchar_t wcID[20];

    MultiByteToWideChar(CP_ACP, 0, id, -1, wcID, sizeof(id));
    wcscat_s(query, wcID);
    wcscat_s(query, L"'");

#ifdef Test_DB 
    wprintf(L"%s\n", query);
#endif

    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt)
        != SQL_SUCCESS)
        return false;

    if (SQLExecDirect(hStmt, (SQLWCHAR*)query, SQL_NTS)
        != SQL_SUCCESS) {
        printf("Logout Query invaild\n");
        return false;
    }

    if (hStmt) SQLCloseCursor(hStmt);

    return true;
}