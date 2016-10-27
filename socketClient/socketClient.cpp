#include <iostream>
using namespace std;
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")


int main(void)
{
	WSAData data = { 0 };
	WSAStartup(MAKEWORD(2, 2), &data);
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == sock)
	{
		cout << "socket error" << endl;
	}

	HOSTENT *localhost = gethostbyname("");
	char *ip = inet_ntoa(*(struct in_addr*)*localhost->h_addr_list);
	cout << ip << endl;
	sockaddr_in server_addr = { 0 };
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(8866);
	server_addr.sin_addr.s_addr = inet_addr(ip);

	if (SOCKET_ERROR == connect(sock, (sockaddr*)&server_addr, sizeof(server_addr)))
	{
		cout << "connect" << endl;
	}
	char buf[100];
	while (cin.getline(buf, 100-1))
	{
		buf[cin.gcount()] = 0;
		send(sock, buf, cin.gcount(),0);
	}

	getchar();
	return 0;
}