#pragma once

#include "winsock2.h"

class APIServer
{
public:
	APIServer(void);
	~APIServer(void);
	void run();
private:
	SOCKET sock;
	struct sockaddr_in localInfo;
};

