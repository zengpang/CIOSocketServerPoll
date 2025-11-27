#include <winsock2.h> // Windows Socket API 头文件
#include <ws2tcpip.h> // 提供对TCP/IP协议的支持
#include <stdio.h>    // 标准输入输出库
#include <stdlib.h>   // 标准库函数
#include <iostream>
#pragma comment(lib, "Ws2_32.lib") // 链接Winsock库
#define DEFAULT_PORT 8080
#define RECVBUF_SIZE 512
int main(int, char **)
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartup failed.\n");
        return 1;
    }
    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET)
    {
        printf("socket failed with error:%d\n", WSAGetLastError());
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
        printf("bind failed with error:%d\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        printf("listen failed with error:%d\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    WSAPOLLFD fds[1];           // 定义pollfd 数组
    fds[0].fd = listenSocket;   // 设置监听的Socket
    fds[0].events = POLLRDNORM; // 监听读事件
    while (true)
    {
        int ret = WSAPoll(fds, 1, 1000);
        if (ret == SOCKET_ERROR)
        {
            printf("WSAPoll failed with error:%d\n", WSAGetLastError());
            break;
        }
        if (ret > 0)
        {
            if (fds[0].revents & POLLRDBAND)
            {
                SOCKET clientSocket = accept(listenSocket, NULL, NULL);
                if (clientSocket == INVALID_SOCKET)
                {
                    printf("accept failed with error:%d\n", WSAGetLastError());
                    continue;
                }
                printf("Client connected.\n");
                // 处理客户请求
                char recvBuf[RECVBUF_SIZE];
                int recvBufLen = RECVBUF_SIZE;
                int bytesReceived = recv(clientSocket, recvBuf, recvBufLen, 0); // 接收数据
                if (bytesReceived > 0)
                {
                    printf("Received:%s\n", recvBuf);
                }
                closesocket(clientSocket);
            }
        }
    }
    closesocket(listenSocket);
    WSACleanup();
    return 0;
}
