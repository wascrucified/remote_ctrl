#include "pch.h"
#include "ClientSocket.h"

CClientSocket* CClientSocket::m_instance = NULL;//这样m_instance就变成了全局的，显式的进行初始化

CClientSocket::CHelper CClientSocket::m_helper; // 达到了其他人想调用调不了的作用

CClientSocket* pclient = CClientSocket::getInstance();

std::string GetErrInfo(int wsaErrCode)
{
    std::string ret;
    LPVOID lpMsgBuf = NULL;
    FormatMessage(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        NULL,
        wsaErrCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    ret = (char*)lpMsgBuf;
    LocalFree(lpMsgBuf);
    return ret;
}