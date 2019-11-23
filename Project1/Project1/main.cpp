/**
1、服务器多平台运行，
2、


*/
#include "EasyTcpServer.hpp"


int main()
{
	EasyTcpServer server;
	//初始化
	server.InitSocket();
	//绑定端口与ip
	server.BindPort(nullptr,7000);
	//监听端口
	server.Linsten_s(5);
	while(true){
		server.OnRun();
	}
	server.CloseService();
	getchar();
	return 0;
}
