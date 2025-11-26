#pragma once
#include "pch.h"
#include "framework.h"
#include <string>
#include <vector>

#pragma pack(push)
#pragma pack(1)

class CPacket
{
public:
    // 默认构造函数
    CPacket() : sHead(0), nLength(0), sCmd(0), sSum(0) {}
    // 构造函数：根据命令和数据创建包
    CPacket(WORD nCmd, const BYTE* pData, size_t nSize) {
        sHead = 0xFEFF;                    // 设置固定包头
        nLength = nSize + 4;               // 包长度 = 数据长度 + 命令(2) + 校验和(2)
        sCmd = nCmd;                       // 设置控制命令
        if (nSize > 0) {
            // 拷贝数据到包中
            strData.resize(nSize);
            memcpy((void*)strData.c_str(), pData, nSize);
        }
        else {
            strData.clear();
        }

        // 计算校验和
        sSum = 0;
        for (size_t j = 0; j < strData.size(); j++) {
            sSum += (BYTE)strData[j] & 0xFF;
        }
    }
    // 拷贝构造函数
    CPacket(const CPacket& pack) {
        sHead = pack.sHead;
        nLength = pack.nLength;
        sCmd = pack.sCmd;
        strData = pack.strData;
        sSum = pack.sSum;
    }
    // 从字节数据构造包
    CPacket(const BYTE* pData, size_t& nSize) {
        size_t i = 0;
        // 查找包头标识 0xFEFF
        for (; i < nSize; i++) {
            if (*(WORD*)(pData + i) == 0xFEFF) {
                sHead = *(WORD*)(pData + i);
                i += 2;//让i越过包头
                break;
            }
        }

        if (i + 4 + 2 + 2 > nSize) {  // 包数据可能不全，或者包头未能全部接收到
            nSize = 0;
            return;
        }

        nLength = *(DWORD*)(pData + i);
        i += 4;

        if (nLength + i > nSize) {  // 包未完全接收到，就返回，解析失败
            nSize = 0;
            return;
        }

        sCmd = *(WORD*)(pData + i);
        i += 2;

        if (nLength > 4) {
            // 解析数据部分（包长度减去命令字段和校验和字段）
            strData.resize(nLength - 2 - 2);
            memcpy((void*)strData.c_str(), pData + i, nLength - 4);
            i += nLength - 4;
        }

        sSum = *(WORD*)(pData + i);
        i += 2;
        // 计算数据的校验和
        WORD sum = 0;
        for (size_t j = 0; j < strData.size(); j++) {
            sum += (BYTE)strData[j] & 0xFF;
        }

        // 验证校验和
        if (sum == sSum) {
            nSize = i;  // 总包大小
            return;
        }

        nSize = 0;  // 校验失败，返回0表示解析失败,大于0成功
    }
    ~CPacket() {}
    //运算符重载
    // 赋值运算符重载
    CPacket& operator=(const CPacket& pack) {
        if (this != &pack) {
            sHead = pack.sHead;
            nLength = pack.nLength;
            sCmd = pack.sCmd;
            strData = pack.strData;
            sSum = pack.sSum;
        }
        return *this;//这样可以连等了
    }
    int Size() {
        // 返回整个包数据的大小：包头(2) + 长度字段(4) + 包内容(nLength)
        return nLength + 6;
    }

    const char* Data() {
        strOut.resize(nLength + 6);           // 调整缓冲区大小以容纳整个包
        BYTE* pData = (BYTE*)strOut.c_str();  // 获取缓冲区指针用于数据组装

        *(WORD*)pData = sHead;
        pData += 2;
        *(DWORD*)pData = nLength;
        pData += 4;
        *(WORD*)pData = sCmd;
        pData += 2;
        memcpy(pData, strData.c_str(), strData.size());
        pData += strData.size();
        *(WORD*)pData = sSum;

        return strOut.c_str();                // 返回组装好的包数据
    }

public:
    WORD sHead;        // 固定包头标识 0xFEFF
    DWORD nLength;     // 包长度（从控制命令开始，到和校验结束）
    WORD sCmd;         // 控制命令
    std::string strData; // 包数据
    WORD sSum;         // 和校验
    std::string strOut; // 整个包的数据
};
#pragma pack(pop)

typedef struct MouseEvent {
    MouseEvent() {
        nAction = 0;
        nButton = -1;
        ptXY.x = 0;
        ptXY.y = 0;
    }

    WORD nAction;  // 点击、移动、双击
    WORD nButton;  // 左键、右键、中键
    POINT ptXY;    // 坐标
} MOUSEEV, * PMOUSEEV;

typedef struct file_info {
    file_info() {
        IsInvalid = FALSE;
        IsDirectory = -1;
        HasNext = TRUE;
        memset(szFileName, 0, sizeof(szFileName));
    }

    BOOL IsInvalid; // 是否有效
    BOOL IsDirectory; // 是否为目录 0 否 1 是
    BOOL HasNext; // 是否还有后续 0 没有 1 有
    char szFileName[256]; // 文件名
} FILEINFO, * PFILEINFO;

std::string GetErrInfo(int wsaErrCode);


// 服务器套接字类
class CClientSocket
{
public:
    static CClientSocket* getInstance() {
        //static静态函数没用this指针，没办法直接访问成员变量m_instance
        //若成员变量也为static静态的，就可以了
        if (m_instance == NULL) {
            m_instance = new CClientSocket;
        }
        return m_instance;
    }
    //套接字初始化
    bool InitSocket(int nIP, int nPort) {
        if (m_sock != INVALID_SOCKET) {
            CloseSocket();
        }
        // 1、创建服务器套接字（放到构造函数里，让它成为成员变量）
        m_sock = socket(PF_INET, SOCK_STREAM, 0);
        if (m_sock == -1) return false;

        // 配置服务器地址信息
        sockaddr_in serv_adr, client_adr;
        memset(&serv_adr, 0, sizeof(serv_adr));
        serv_adr.sin_family = AF_INET;
        serv_adr.sin_addr.s_addr = htonl(nIP);  
        serv_adr.sin_port = htons(nPort);

        if (serv_adr.sin_addr.s_addr == INADDR_NONE) {
            AfxMessageBox("指定的IP地址不存在!");
            return false;
        }

        int ret = connect(m_sock, (sockaddr*)&serv_adr, sizeof(serv_adr));

        if (ret == -1) {
            AfxMessageBox("连接失败!");
            TRACE("连接失败 %d %s\r\n", WSAGetLastError(), GetErrInfo(WSAGetLastError()).c_str());
            return false;
        }

        return true;
    }


#define BUFFERSIZE 4096
    int DealCommand() {
        if (m_sock == -1) return -1;
        char* buffer = m_buffer.data();
        memset(buffer, 0, BUFFERSIZE);
        size_t index = 0;
        while (true) {
            //得到的长度
            size_t len = recv(m_sock, buffer + index, BUFFERSIZE - index, 0);
            if (len <= 0) {
                
                return -1;
            }
            index += len;
            //使用的长度
            len = index;
            m_packet = CPacket((BYTE*)buffer, len);
            if (len > 0) {
                //解析成功
                memmove(buffer, buffer + len, BUFFERSIZE - len);
                index -= len;

                return m_packet.sCmd;
            }
        }

        return -1;
    }

    bool Send(const char* pData, int nSize) {
        if (m_sock == -1) return false;  // 检查客户端套接字是否有效
        return send(m_sock, pData, nSize, 0) > 0;
    }
    bool Send(CPacket& pack) {
        TRACE("m_sock = %d\r\n", m_sock);
        if (m_sock == -1) return false;  // 检查客户端套接字是否有效
        return send(m_sock, pack.Data(), pack.Size(), 0) > 0;
    }

    bool GetFilePath(std::string& strPath) {
        if ((m_packet.sCmd >= 2) && (m_packet.sCmd <= 4)) {//2 代表选择获取文件列表功能的标识
            strPath = m_packet.strData;
            return true;
        }
        return false;
    }

    bool GetMouseEvent(MOUSEEV& mouse) {
        if (m_packet.sCmd == 5) {
            memcpy(&mouse, m_packet.strData.c_str(), sizeof(MOUSEEV));
            return true;
        }
        return false;
    }

    CPacket& GetPacket() {
        return m_packet;
    }
    void CloseSocket() {
        closesocket(m_sock);
        m_sock = INVALID_SOCKET;
    }

private:
    std::vector<char> m_buffer;
    SOCKET m_client;
    SOCKET m_sock;
    CPacket m_packet;
    // 赋值运算符重载
    CClientSocket& operator=(const CClientSocket& ss) {}
    // 拷贝构造函数
    CClientSocket(const CClientSocket& ss) {
        m_sock = ss.m_sock;
    }

    // 构造函数 - 初始化套接字环境
    CClientSocket() {
        if (InitSockEnv() == FALSE) {
            MessageBox(NULL, _T("无法初始化套接字环境,请检查网络设置！"), _T("初始化错误！"), MB_OK | MB_ICONERROR);
            exit(0);
        }
        m_buffer.resize(BUFFERSIZE);
    }

    // 析构函数 - 清理套接字环境
    ~CClientSocket() {
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
            CClientSocket* tmp = m_instance;
            m_instance = NULL;
            delete tmp;
        }
    }
    static CClientSocket* m_instance; // 静态单例实例指针
    class CHelper { // RAII助手类，在构造时获取单例，析构时自动释放单例资源
    public:
        CHelper() {
            CClientSocket::getInstance();
        }
        ~CHelper() {
            CClientSocket::releaseInstance();
        }
    };
    static CHelper m_helper;
};

extern CClientSocket server;