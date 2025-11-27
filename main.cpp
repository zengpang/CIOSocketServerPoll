#include <winsock2.h> // Windows Socket API 头文件
#include <ws2tcpip.h> // 提供对TCP/IP协议的支持
#include <stdio.h>    // 标准输入输出库
#include <stdlib.h>   // 标准库函数
#include <iostream>
#pragma comment(lib, "Ws2_32.lib") // 链接Winsock库
#define DEFAULT_PORT 8888
#define RECVBUF_SIZE 512
/**
 * socket 发送消息
 */
void socketSendMsg(SOCKET clientSocket, const char *sendMessage)
{
    if (!send(clientSocket, sendMessage, strlen(sendMessage), 0))
    {
        std::cerr << "send failed: " << WSAGetLastError() << std::endl; // 输出发送失败的错误信息
    }
}
int main(int, char **)
{
    WSADATA wsaData;                               // 用于存储socket初始化信息
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) // 初始化winsock
    {
        std::cout<<"WSAStartup failed"<<std::endl;
        return 1;
    }
    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // 创建TCP Socket
    if (listenSocket == INVALID_SOCKET)
    {
        std::cout<<"socket failed with error:"<<WSAGetLastError()<<std::endl;
        WSACleanup();
        return 1;
    }
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(DEFAULT_PORT);
    if (bind(listenSocket, (SOCKADDR *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        // 绑定Socket
        std::cout<<"bind failed with error:"<<WSAGetLastError()<<std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        std::cout<<"listen failed with error:"<<WSAGetLastError()<<std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    WSAPOLLFD fds[1];           // 定义pollfd 数组
    fds[0].fd = listenSocket;   // 设置监听的Socket
    fds[0].events = POLLRDNORM; // 监听读事件
    
    while (true)                // 主循环
    {
        int ret = WSAPoll(fds, 1, 1000); // 调用WSAPoll，超时时间为1秒
        if (ret == SOCKET_ERROR)         // 检查WSAPoll是否失败
        {
            std::cout<<"WSAPoll failed with error:"<<WSAGetLastError()<<std::endl;
            break;
        }
        
        if (ret > 0) // 如果有事件发生
        {
            if (fds[0].revents & POLLRDNORM) // 检查是否是读事件
            {
                SOCKET clientSocket = accept(listenSocket, NULL, NULL); // 接收客户端连接
                if (clientSocket == INVALID_SOCKET)                     // 检查是否接收成功
                {
                    std::cout<<"accept failed with error:"<<WSAGetLastError()<<std::endl;
                    continue;
                }
                std::cout<<"Client connected."<<WSAGetLastError()<<std::endl;

                // 处理客户请求
                char recvBuf[RECVBUF_SIZE];                                     // 定义接收缓冲区
                int recvBufLen = RECVBUF_SIZE;                                  // 缓冲区长度
                int bytesReceived = recv(clientSocket, recvBuf, recvBufLen, 0); // 接收数据
                const char *sendMsgStr = "服务端发送的消息";
                socketSendMsg(clientSocket, sendMsgStr);
                if (bytesReceived > 0) // 如果接收到数据
                {
                    std::cout<<"Received."<<std::string(recvBuf,0,bytesReceived)<<std::endl;
                }
                closesocket(clientSocket);
            }
        }
    }
    closesocket(listenSocket);
    WSACleanup();
    return 0;
}
