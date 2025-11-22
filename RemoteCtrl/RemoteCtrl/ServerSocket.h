#pragma once
#include "pch.h"
#include "framework.h"

// 服务器套接字类
class CServerSocket
{
public:
    static CServerSocket* getInstance() {
        //static静态函数没用this指针，没办法直接访问成员变量m_instance
        //若成员变量也为static静态的，就可以了
        if (m_instance == NULL) {
            m_instance = new CServerSocket;
        }
        return m_instance;
    }
    //套接字初始化
    bool InitSocket() {

        if (m_sock == -1) return false;

        // 配置服务器地址信息
        sockaddr_in serv_adr, client_adr;
        memset(&serv_adr, 0, sizeof(serv_adr));
        serv_adr.sin_family = AF_INET;
        serv_adr.sin_addr.s_addr = INADDR_ANY;  // 监听所有网络接口
        serv_adr.sin_port = htons(9527);        // 设置端口号为9527

        // 2、绑定套接字到指定地址和端口
        if (bind(m_sock, (sockaddr*)&serv_adr, sizeof(serv_adr)) == -1) {
            return false;
        }

        // 3、开始监听连接
          //  backlog=1，允许一个连接在队列中等待
        if (listen(m_sock, 1) == -1) {
            return false;
        }
        return true;
    }

    //accept操作，获取客户端
    bool AcceptClient() {
        sockaddr_in client_adr;
        int cli_sz = sizeof(client_adr);
        m_client = accept(m_sock, (sockaddr*)&client_adr, &cli_sz);
        if (m_client == -1) return false;
        return true;
        // 5、接收发送数据
        //recv(client, buffer, sizeof(buffer), 0);
        //send(client, buffer, sizeof(buffer), 0);

    }
    
    int DealCommand() {
        char buffer[1024] = "";
        while (true) {
            int len = recv(m_client, buffer, sizeof(buffer), 0);
            if (len <= 0) {
                return -1;
            }
            //TODO:处理命令
        }
    }

    bool Send(const char* pData, int nSize) {
        if (m_client == -1) return false;  // 检查客户端套接字是否有效
        return send(m_client, pData, nSize, 0) > 0;
    }
private:
    SOCKET m_client;
    SOCKET m_sock;
    // 赋值运算符重载
    CServerSocket& operator=(const CServerSocket& ss) {}
    // 拷贝构造函数
    CServerSocket(const CServerSocket& ss) {
        m_sock = ss.m_sock;
        m_client = ss.m_client;
    }

    // 构造函数 - 初始化套接字环境
    CServerSocket() {
        m_client = INVALID_SOCKET;//INVALID_SOCKET是一个宏，用于初始化socket，值为-1
        if (InitSockEnv() == FALSE) {
            MessageBox(NULL, _T("无法初始化套接字环境,请检查网络设置！"), _T("初始化错误！"), MB_OK | MB_ICONERROR);
            exit(0);
        }
        // 1、创建服务器套接字（放到构造函数里，让它成为成员变量）
        m_sock = socket(PF_INET, SOCK_STREAM, 0);
    }

    // 析构函数 - 清理套接字环境
    ~CServerSocket() {
        // 6、关闭套接字和清理Winsock
        closesocket(m_sock);
        WSACleanup();
    }

    // 初始化套接字环境
    BOOL InitSockEnv() {
        WSADATA data; 
        // 初始化Winsock库，请求版本1.1
        if (WSAStartup(MAKEWORD(1, 1), &data) != 0) {  // 修正：WSAStartup 和 MAKEWORD
            return FALSE;
        }
        return TRUE;
    }

    static void releaseInstance() {//静态方法，用于安全释放单例实例
        if (m_instance != NULL) {
            CServerSocket* tmp = m_instance;
            m_instance = NULL;
            delete tmp;
        }
    }
    static CServerSocket* m_instance; // 静态单例实例指针
    class CHelper { // RAII助手类，在构造时获取单例，析构时自动释放单例资源
    public:
        CHelper() {
            CServerSocket::getInstance();
        }
        ~CHelper() {
            CServerSocket::releaseInstance();
        }
    };
    static CHelper m_helper;
};

extern CServerSocket server;