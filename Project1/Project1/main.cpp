

#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS 
#include<Windows.h>
#include<WinSock2.h>
#include<iostream>
#include<vector>

//链接windows 动态库
#pragma comment(lib,"ws2_32.lib")

using namespace std;


vector<SOCKET> g_client;




//设置端口值
const int post_id = 7000;
//设置ip值
const char  host_id[] = "localhost";
enum CMD {
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGINOUT_RESULT,
	CMD_NEW_USER_JOIN,
	CMD_LOGINOUT,
	CMD_ERROR
};

//定义数据包 
struct dataHead {
	//数据长度
	short dataLength;
	short cmd;
};

struct LoginResult :public dataHead {
	LoginResult()
	{
		dataLength = sizeof(LoginResult);
		cmd = CMD_LOGIN_RESULT;
		result = 0;
	}
	int result;

};
struct Loginout :public dataHead {
	Loginout()
	{
		dataLength = sizeof(Loginout);
		cmd = CMD_LOGINOUT;
	}
	char userName[32];
};

struct LoginOutResult :public  dataHead {
	LoginOutResult()
	{
		dataLength = sizeof(LoginOutResult);
		cmd = CMD_LOGINOUT_RESULT;
		result = 0;
	}
	int result;

};

//登录数据结构
struct Login :public dataHead {
	Login()
	{
		dataLength = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char userName[32];
	char password[32];
};
struct NEWUserLogin :public dataHead
{
	NEWUserLogin() {
		dataLength = sizeof(NEWUserLogin);
		cmd = CMD_NEW_USER_JOIN;
		socket_id = 0;
	}
	int socket_id;
};

//定义结构化数据
struct DataPackage
{
	int age;
	char name[32];
};





int processs(SOCKET _clientSock) {
	// 5 接收客户端数据请求
	//字节缓冲区
	char szRecv[4096] = {};
	int _cLen = recv(_clientSock, szRecv, sizeof(dataHead), 0);
	dataHead *header = (dataHead*)szRecv;
	if (_cLen <= 0)
	{
		printf("客户端已退出，连接接收。\n");
		return -1;
	}

	switch (header->cmd)
	{
	case CMD_LOGIN:
	{
		Login *login = (Login*)szRecv;
		recv(_clientSock, szRecv + sizeof(dataHead), header->dataLength - sizeof(dataHead), 0);
		printf("收到命令 数据长度 :%d ,userName=%s\n", login->dataLength, login->userName);

		//账号密码验证
		LoginResult ret;
		//send(_clientSock, (char*)&header, sizeof(header), 0);
		send(_clientSock, (char*)&ret, sizeof(ret), 0);

	}
	break;
	case CMD_LOGINOUT: {
		Loginout *lout = (Loginout*)szRecv;
		recv(_clientSock, szRecv + sizeof(dataHead), header->dataLength - sizeof(dataHead), 0);
		printf("收到命令 数据长度 :%d ,userName=%s\n", lout->dataLength, lout->userName);

		//账号密码验证
		LoginResult ret;
		send(_clientSock, (char*)&ret, sizeof(ret), 0);
	}
					   break;
	default: {
		dataHead hd_Len = { 0,CMD_ERROR };
		send(_clientSock, (char*)&hd_Len, sizeof(dataHead), 0);

	}
			 break;

	}


}



int main()
{
	//启动windows socket
	WORD word = MAKEWORD(2, 2);
	WSADATA data;
	WSAStartup(word, &data);
	// 1、建立一个socket
	// 第一 ipv几的协议  第二 使用字节流方式 第三  使用tcp方式
	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	// 2、  bind 绑定用与接收客户端链接的网络端口
	sockaddr_in _sin = {};
	//网络协议
	_sin.sin_family = AF_INET;
	//网络端口
	_sin.sin_port = htons(post_id);// 7000;
	//绑定ip地址  ip地址 INADDR_ANY  不限定
	_sin.sin_addr.S_un.S_addr = INADDR_ANY;

	if (SOCKET_ERROR == bind(sock, (sockaddr*)&_sin, sizeof(_sin)))
	{

		printf("绑定端口失败。 \n");
	}
	else {
		printf("绑定端口成功。 \n");
	}
	// 3、listen 监听网络端口
	//传入socket  链接数量为5

	if (SOCKET_ERROR == listen(sock, 5))
	{
		printf("监听网络端口失败。\n");
	}
	else {
		printf("监听网络端口成功。\n");
	}

	// 4、 accept 等待接受客户端连接
	
	//测试数据

	char _cMsgBuf[128] = {};
	while (true) {

		//
		fd_set fd_read;
		fd_set fd_write;
		fd_set fd_exp;
		// 清空 fd 集合 宏
		FD_ZERO(&fd_read);
		FD_ZERO(&fd_write);
		FD_ZERO(&fd_exp);
		//宏  set套接字  
		FD_SET(sock, &fd_read);
		FD_SET(sock, &fd_write);
		FD_SET(sock, &fd_exp);
		//timeval time_s = {0,0};
		for (size_t n = 0; n < g_client.size();n++) {			
			FD_SET(g_client[n], &fd_read);		
		}

		// select 函数最后参数   实现 select 函数是否为阻塞  NULL 阻塞模式 
		timeval time_s = {0,0};
		int ret=select(sock + 1, &fd_read, &fd_write, &fd_exp, &time_s);
		if (ret<0) {
			printf("select 任务结束。\n");
			break;
		}
		//判断集合中是否有本身
		if (FD_ISSET(sock, &fd_read)) {			
			FD_CLR(sock, &fd_read);
			//等待客户端连接
			sockaddr_in clientAddr = {};
			int nAddrLen = sizeof(sockaddr_in);
			SOCKET _clientSock = INVALID_SOCKET;
			_clientSock = accept(sock, (sockaddr*)&clientAddr, &nAddrLen);
			if (INVALID_SOCKET == _clientSock)
			{
				printf("接收到无效的客户端 socket.....\n");
			}

			for (size_t n = 0; n < g_client.size(); n++) {
				NEWUserLogin userLogin;
				send(g_client[n],(const char*)&userLogin,sizeof(NEWUserLogin),0);
			}
			g_client.push_back(_clientSock);		
			printf("新客户端加入。socket = %d,ip地址：=%s \n", (int)_clientSock, inet_ntoa(clientAddr.sin_addr));
		}
		//调用方法
		for (size_t n = 0; n < fd_read.fd_count ;  n++) {
			if (-1== processs(fd_read.fd_array[n])) {
				auto iter = find(g_client.begin(),g_client.end(), fd_read.fd_array[n]);
				if (iter!= g_client.end()) {
					g_client.erase(iter);
				}
			}		
		}
		printf("空闲时间处理其他业务\n");
	}
	// 在结束时，关闭客户端socket
	for (size_t n = 0; n < g_client.size(); n++) {
		closesocket(g_client[n]);

	}
	// 8、关闭socket
	closesocket(sock);
	printf("已退出 ，任务接收 \n");
	getchar();
	//windows socket 关闭
	WSACleanup();
	return 0;
}
