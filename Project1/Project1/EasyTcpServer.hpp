
#ifndef  EasyTcpServer_hpp_
#define EasyTcpServer_hpp_
#ifdef _WIN32
		#define WIN32_LEAN_AND_MEAN
		#define _WINSOCK_DEPRECATED_NO_WARNINGS 
		#include<Windows.h>
		#include<WinSock2.h>
		//����windows ��̬��
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
	//��ʼ��socket
	SOCKET InitSocket()
	{
#ifdef _WIN32
		//����windows socket
		WORD word = MAKEWORD(2, 2);
		WSADATA data;
		WSAStartup(word, &data);
#endif

		if (INVALID_SOCKET != _socket) {
			printf("socket�ر����ӡ� \n");
			CloseService();
		}	
		_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _socket)
		{
			printf("����socket���� \n");
		}
		else {
			printf("����socket�ɹ��� \n");
		}
		return _socket;

	}
	//�󶨶˿�
	void BindPort(const char * ip,unsigned short port)
	{

		// ���ð󶨶˿�ʱ �ж��Ƿ��ʼ��
		if (INVALID_SOCKET == _socket) {
			printf("socket��ʼ���� \n");
			InitSocket();
		}
		sockaddr_in  _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);	
#ifdef _WIN32
		//��ip��ַ�ж��Ƿ�Ϊ��
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
			printf("���󣬰�����˿�ʧ��.....\n");
		}
		else {
			printf("������˿ڳɹ�.....\n");
		}
	}
	//�����˿�
	int Linsten_s(int n) {
		int ret = listen(_socket, n);
		if (SOCKET_ERROR== ret) {
			printf("���󣬼�������˿�ʧ��......\n");
		}
		else {
			printf("��������˿ڳɹ�......\n");
		}
		return ret;
	}
	//���տͻ�������
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
			printf("���յ���Ч�Ŀͻ��� socket.....\n");
		}
		else {
			NEWUserLogin userLogin;
			//���������ӿͻ�����Ϣ
			send(_clientSock, (char*)&userLogin, userLogin.dataLength, 0);
			//֪ͨ�����ͻ������³�Ա����
			SendDataAll(&userLogin);
			g_client.push_back(_clientSock);
			printf("�¿ͻ��˼��롣socket = %d,ip��ַ��=%s \n", (int)_clientSock, inet_ntoa(clientAddr.sin_addr));
		}
		return _clientSock;
	}
	//�ر�socket
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
	//����������Ϣ
	bool OnRun() {
		if (isRun()) {
			fd_set fd_read;
			fd_set fd_write;
			fd_set fd_exp;
			// ��� fd ���� ��
			FD_ZERO(&fd_read);
			FD_ZERO(&fd_write);
			FD_ZERO(&fd_exp);
			//��  set�׽���  
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
				printf("select ���������\n");
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

	//�Ƿ�����
	bool isRun() {
		return _socket != INVALID_SOCKET;
	}
	//�������� ����ճ�� ��ְ�

	int processs(SOCKET _clientSock) {
		// 5 ���տͻ�����������
		//�ֽڻ�����
		char szRecv[4096] = {};
		int _cLen = recv(_clientSock, szRecv, sizeof(dataHead), 0);
		dataHead* header = (dataHead*)szRecv;
		if (_cLen <= 0)
		{
			printf("�ͻ������˳������ӽ��ա�\n");
			return -1;
		}
		recv(_clientSock, szRecv + sizeof(dataHead), header->dataLength - sizeof(dataHead), 0);
		OnNetMsg(_clientSock, header);
		return 0;

	}

	//��������
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

	//��Ӧ������Ϣ
	void OnNetMsg(SOCKET _sock,dataHead* header) {
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			Login* login = (Login*)header;

			printf("�յ����� ���ݳ��� :%d ,userName=%s\n", login->dataLength, login->userName);

			//�˺�������֤
			LoginResult ret;
			send(_sock, (char*)&ret, sizeof(ret), 0);

		}
		break;
		case CMD_LOGINOUT: {
			Loginout* lout = (Loginout*)header;
			printf("�յ����� ���ݳ��� :%d ,userName=%s\n", lout->dataLength, lout->userName);
			//�˺�������֤
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

