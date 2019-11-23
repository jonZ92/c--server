
#ifndef  EasyTcpServer_hpp_
#define EasyTcpServer_hpp_
#ifdef _WIN32
		#define WIN32_LEAN_AND_MEAN
		#define _WINSOCK_DEPRECATED_NO_WARNINGS 
		#include<Windows.h>
		#include<WinSock2.h>
		//链接windows 动态库
		#pragma comment(lib,"ws2_32.lib")
#else
		#include<unistd.h>
		#include<arpa/inet.h>
		#include<string.h>
		#define SOCKET int
		#define INVALID_SOCKET (SOCKET)(~0)
		#define SOCKET_ERROR                  (-1) 
#endif // _WIN32
#include<vector>
#include<iostream>
#include "MessageHeader.hpp"

class EasyTcpServer {
private:
	SOCKET _socket;
	std::vector<SOCKET> g_client;
public:
	
	EasyTcpServer() 
	{
		_socket = INVALID_SOCKET;
	}
	virtual ~EasyTcpServer()
	{
		CloseService();
	}
	//初始化socket
	SOCKET InitSocket()
	{
#ifdef _WIN32
		//启动windows socket
		WORD word = MAKEWORD(2, 2);
		WSADATA data;
		WSAStartup(word, &data);
#endif

		if (INVALID_SOCKET != _socket) {
			printf("socket关闭连接。 \n");
			CloseService();
		}	
		_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _socket)
		{
			printf("建立socket错误。 \n");
		}
		else {
			printf("建立socket成功。 \n");
		}
		return _socket;

	}
	//绑定端口
	void BindPort(const char * ip,unsigned short port)
	{

		// 调用绑定端口时 判断是否初始化
		if (INVALID_SOCKET == _socket) {
			printf("socket初始化。 \n");
			InitSocket();
		}
		sockaddr_in  _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);	
#ifdef _WIN32
		//对ip地址判断是否为空
		if (ip) {
			_sin.sin_addr.S_un.S_addr = inet_addr(ip);
		}
		else {
			_sin.sin_addr.S_un.S_addr = INADDR_ANY;
		}
#else 
		if (ip) {
			_sin.sin_addr.s_addr = inet_addr(ip);
		}
		else {
			_sin.sin_addr.s_addr = INADDR_ANY;
		}
#endif // _WIN32
		int ret = bind(_socket, (sockaddr*)&_sin, sizeof(_sin));
		if (SOCKET_ERROR== ret)
		{
			printf("错误，绑定网络端口失败.....\n");
		}
		else {
			printf("绑定网络端口成功.....\n");
		}
	}
	//监听端口
	int Linsten_s(int n) {
		int ret = listen(_socket, n);
		if (SOCKET_ERROR== ret) {
			printf("错误，监听网络端口失败......\n");
		}
		else {
			printf("监听网络端口成功......\n");
		}
		return ret;
	}
	//接收客户端连接
	SOCKET receive_s() {
		sockaddr_in clientAddr = {};
		int nAddrLen = sizeof(sockaddr_in);
		SOCKET _clientSock = INVALID_SOCKET;
#ifdef WIN32
		_clientSock = accept(_socket, (sockaddr*)&clientAddr, &nAddrLen);
#else
		_clientSock = accept(_socket, (sockaddr*)&clientAddr,(socklen_t*)&nAddrLen);
#endif // WIN32

		
		if (INVALID_SOCKET == _clientSock)
		{
			printf("接收到无效的客户端 socket.....\n");
		}
		else {
			NEWUserLogin userLogin;
			//发给新连接客户端信息
			send(_clientSock, (char*)&userLogin, userLogin.dataLength, 0);
			//通知其他客户端有新成员加入
			SendDataAll(&userLogin);
			g_client.push_back(_clientSock);
			printf("新客户端加入。socket = %d,ip地址：=%s \n", (int)_clientSock, inet_ntoa(clientAddr.sin_addr));
		}
		return _clientSock;
	}
	//关闭socket
	void CloseService() {
		if (_socket!=INVALID_SOCKET) {
#ifdef _WIN32
			for (int n =(int) g_client.size() - 1; n >= 0;n--) {
				closesocket(g_client[n]);
			}
			closesocket(_socket);
			WSACleanup();

#else
			for (int n = (int)g_client.size() - 1; n >= 0; n--) {
				close(g_client[n]);
			}
			close(_socket);
#endif // 

		}
	
	}
	//处理网络消息
	bool OnRun() {
		if (isRun()) {
			fd_set fd_read;
			fd_set fd_write;
			fd_set fd_exp;
			// 清空 fd 集合 宏
			FD_ZERO(&fd_read);
			FD_ZERO(&fd_write);
			FD_ZERO(&fd_exp);
			//宏  set套接字  
			FD_SET(_socket, &fd_read);
			FD_SET(_socket, &fd_write);
			FD_SET(_socket, &fd_exp);
			SOCKET max_sock = _socket;
			for (size_t n = 0; n < g_client.size(); n++) {
				FD_SET(g_client[n], &fd_read);
				if (max_sock < g_client[n]) {
					max_sock =g_client[n];
				}
			}
			timeval t_s = {1,0};
			int ret = select(max_sock + 1, &fd_read, &fd_write, &fd_exp, &t_s);
			if (ret < 0) {
				printf("select 任务结束。\n");
				CloseService();
				return false;
			}
			if (FD_ISSET(_socket, &fd_read)) {
				FD_CLR(_socket, &fd_read);
				receive_s();
			}
				for (int n = (int)g_client.size() - 1; n >= 0; n--) {
					if (FD_ISSET(g_client[n],&fd_read)) {
						if (-1== processs(g_client[n])) {
							auto iter = g_client.begin() + n;
							if (iter!= g_client.end()) 
							{
								g_client.erase(iter);
							}
						}
					}
				
				}
		
			return true;
		}
		return false;
	}

	//是否工作中
	bool isRun() {
		return _socket != INVALID_SOCKET;
	}
	//接收数据 处理粘包 拆分包

	int processs(SOCKET _clientSock) {
		// 5 接收客户端数据请求
		//字节缓冲区
		char szRecv[4096] = {};
		int _cLen = recv(_clientSock, szRecv, sizeof(dataHead), 0);
		dataHead* header = (dataHead*)szRecv;
		if (_cLen <= 0)
		{
			printf("客户端已退出，连接接收。\n");
			return -1;
		}
		recv(_clientSock, szRecv + sizeof(dataHead), header->dataLength - sizeof(dataHead), 0);
		OnNetMsg(_clientSock, header);
		return 0;

	}

	//发送数据
	int SendData(SOCKET _sock,dataHead* header) {

		if (isRun() && header) {
			send(_sock, (char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}

	void SendDataAll( dataHead* header) {
		if (isRun() && header) {
			for (int n = 0; n < g_client.size(); n++) {
				SendData(g_client[n], header);
			}
		}	
	}

	//响应网络消息
	void OnNetMsg(SOCKET _sock,dataHead* header) {
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			Login* login = (Login*)header;

			printf("收到命令 数据长度 :%d ,userName=%s\n", login->dataLength, login->userName);

			//账号密码验证
			LoginResult ret;
			send(_sock, (char*)&ret, sizeof(ret), 0);

		}
		break;
		case CMD_LOGINOUT: {
			Loginout* lout = (Loginout*)header;
			printf("收到命令 数据长度 :%d ,userName=%s\n", lout->dataLength, lout->userName);
			//账号密码验证
			LoginResult ret;
			send(_sock, (char*)&ret, sizeof(ret), 0);
		}
						 break;
		default: {
			dataHead hd_Len = { 0,CMD_ERROR };
			send(_sock, (char*)&hd_Len, sizeof(dataHead), 0);
		}
			   break;
		}
	}
};

	
#endif // !EasyTcpServer_hpp_

