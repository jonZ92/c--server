/**
1����������ƽ̨���У�
2��


*/
#include "EasyTcpServer.hpp"


int main()
{
	EasyTcpServer server;
	//��ʼ��
	server.InitSocket();
	//�󶨶˿���ip
	server.BindPort(nullptr,7000);
	//�����˿�
	server.Linsten_s(5);
	while(true){
		server.OnRun();
	}
	server.CloseService();
	getchar();
	return 0;
}
