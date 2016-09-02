#include "stdafx.h"
#include "cqp.h"

#include "appmain.h"
#include "APIClient.h"

extern int appAuthCode;


APIClient::APIClient(void)
{
	WSADATA wsa;
	WSAStartup(MAKEWORD(1, 1), &wsa);
	
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	clients = new APIClientInfo[CLIENT_SIZE];
	for (int i = 0; i < CLIENT_SIZE; i++) {
		clients[i].hello = -1;
	}
}


APIClient::~APIClient(void)
{
	closesocket(sock);
}


void APIClient::add(const int port)
{
	int free_i = -1;
	int port_i = -1;
	char log[1024];

	for (int i = 0; i < CLIENT_SIZE; i++) {
		if (free_i < 0 && clients[i].hello < 0) {
			free_i = i;
		}
		if (port_i < 0 && clients[i].port == port) {
			port_i = i;
		}
	}
	// Update hello timestamp if client port is registered.
	if (port_i >= 0) {
		clients[port_i].hello = time(0);
		sprintf_s(log, "Client updated: %d.", port);
		CQ_addLog(appAuthCode, CQLOG_INFO, "APIClient", log);
	}
	// Add new client port.
	else if (free_i >= 0) {
		clients[free_i].hello = time(0);
		clients[free_i].port = port;
		clients[free_i].info.sin_family = AF_INET;
		clients[free_i].info.sin_addr.s_addr = inet_addr("127.0.0.1");
		clients[free_i].info.sin_port = htons(port);
		sprintf_s(log, "Client added: %d.", port);
		CQ_addLog(appAuthCode, CQLOG_INFO, "APIClient", log);
	}
	// Send a warning when client cap if full.
	else {
		sprintf_s(log, "Client cap is full! Cannot add port: %d.", port);
		CQ_addLog(appAuthCode, CQLOG_WARNING, "APIClient", log);
	}
}


void APIClient::send(const char *buffer, const int len)
{
	time_t timeout = time(0) - CLIENT_TIMEOUT;
	char log[1024];

	for (int i = 0; i < CLIENT_SIZE; i++) {
		if (clients[i].hello < 0) {
			continue;
		}
		if (clients[i].hello < timeout) {
			clients[i].hello = -1;
			sprintf_s(log, "Client removed: %d.", clients[i].port);
			CQ_addLog(appAuthCode, CQLOG_INFO, "APIClient", log);
			continue;
		}
		sendto(sock, buffer, len, 0, (sockaddr *)&clients[i].info, sizeof(clients[i].info));
	}
}

void APIClient::send(const char *buffer, const int len, int port)
{
	for (int i = 0; i < CLIENT_SIZE; i++) {
		if (clients[i].port == port) {
			sendto(sock, buffer, len, 0, (sockaddr *)&clients[i].info, sizeof(clients[i].info));
		}
	}
}