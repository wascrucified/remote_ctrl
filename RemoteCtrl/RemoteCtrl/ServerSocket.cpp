#include "pch.h"
#include "ServerSocket.h"

//CServerSocket server;

CServerSocket* CServerSocket::m_instance = NULL;//这样m_instance就变成了全局的，显式的进行初始化

CServerSocket::CHelper CServerSocket::m_helper; // 达到了其他人想调用调不了的作用

CServerSocket* pserver = CServerSocket::getInstance();