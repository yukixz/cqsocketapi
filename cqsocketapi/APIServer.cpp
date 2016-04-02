#include "stdafx.h"
#include "string.h"
#include "cqp.h"

#include "APIServer.h"
#include "appmain.h"
#include "base64.h"

extern int appAuthCode;


/********
 * Message Processer
 ********/
void prcsPrivateMessage(const char *payload) {
	int64_t qq;
	char text[FRAME_PAYLOAD_SIZE];
	sscanf_s(payload, "%d %[^\n]", &qq, text, sizeof(text));
	
	char decodedText[FRAME_PAYLOAD_SIZE];
	Base64decode(decodedText, text);

	CQ_sendPrivateMsg(appAuthCode, qq, decodedText);
}

void prcsGroupMessage(const char *payload) {
	int64_t group;
	char text[FRAME_PAYLOAD_SIZE];
	sscanf_s(payload, "%d %[^\n]", &group, text, sizeof(text));
	
	char decodedText[FRAME_PAYLOAD_SIZE];
	Base64decode(decodedText, text);

	CQ_sendGroupMsg(appAuthCode, group, decodedText);
}


/********
 * API Server
 ********/
APIServer::APIServer(void)
{
	WSADATA wsa;
	WSAStartup(MAKEWORD(1, 1), &wsa);
	
	localInfo.sin_family = AF_INET;
	localInfo.sin_addr.s_addr = INADDR_ANY;  // inet_addr("127.0.0.1")
	localInfo.sin_port = htons(API_SERVER_PORT);

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	bind(sock, (sockaddr *)&localInfo, sizeof(localInfo));
}


APIServer::~APIServer(void)
{
	closesocket(sock);
}


void APIServer::run()
{
	char buffer[FRAME_SIZE];
	char prefix[FRAME_PREFIX_SIZE];
	char payload[FRAME_PAYLOAD_SIZE];

	while (1) {
		memset(buffer, 0, sizeof(buffer));
		memset(prefix, 0, sizeof(prefix));
		memset(payload, 0, sizeof(payload));
		if (recv(sock, buffer, sizeof(buffer), 0) != SOCKET_ERROR) {
			sscanf_s(buffer, "%s %[^\n]", prefix, sizeof(prefix), payload, sizeof(payload));

			if (strcmp(prefix, "PrivateMessage") == 0) {
				prcsPrivateMessage(payload);
			}
			if (strcmp(prefix, "GroupMessage") == 0) {
				prcsGroupMessage(payload);
			}
		}
	}
}