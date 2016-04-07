#pragma once

#include <time.h>
#include "winsock2.h"

struct APIClientInfo
{
	time_t hello;
	int port;
	struct sockaddr_in info;
};

class APIClient
{
public:
	APIClient(void);
	~APIClient(void);
	void add(const int port);
	void send(const char *buffer, const int len);
private:
	SOCKET sock;
	struct APIClientInfo clients[CLIENT_SIZE];
};
