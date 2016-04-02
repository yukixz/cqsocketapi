#include "stdafx.h"

#include "APIClient.h"
#include "appmain.h"


APIClient::APIClient(void)
{
	WSADATA wsa;
	WSAStartup(MAKEWORD(1, 1), &wsa);
	
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	remoteInfo.sin_family = AF_INET;
	remoteInfo.sin_addr.s_addr = inet_addr("127.0.0.1");
	remoteInfo.sin_port = htons(API_CLIENT_PORT);
}


APIClient::~APIClient(void)
{
	closesocket(sock);
}


void APIClient::send(const char *buffer, const int len)
{
	sendto(sock, buffer, len, 0, (sockaddr *)&remoteInfo, sizeof(remoteInfo));
}