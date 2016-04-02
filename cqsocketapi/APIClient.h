#pragma once

#include "winsock2.h"

class APIClient
{
public:
	APIClient(void);
	~APIClient(void);
	void send(const char *buffer, const int len);
private:
	SOCKET sock;
	struct sockaddr_in remoteInfo;
};
