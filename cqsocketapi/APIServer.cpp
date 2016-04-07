#include "stdafx.h"
#include "string.h"
#include "cqp.h"
#include "base64.h"

#include "appmain.h"
#include "APIClient.h"
#include "APIServer.h"

extern APIClient *client;
extern int appAuthCode;


/********
 * Message Processer
 ********/
void prcsClientHello(const char *payload) {
	int port;
	sscanf_s(payload, "%d", &port);

	client->add(port);
}

void prcsPrivateMessage(const char *payload) {
	int64_t id;
	char text[FRAME_PAYLOAD_SIZE];
	sscanf_s(payload, "%d %[^\n]", &id, text, sizeof(text));
	
	char decodedText[FRAME_PAYLOAD_SIZE];
	Base64decode(decodedText, text);

	CQ_sendPrivateMsg(appAuthCode, id, decodedText);
}

void prcsGroupMessage(const char *payload) {
	int64_t id;
	char text[FRAME_PAYLOAD_SIZE];
	sscanf_s(payload, "%d %[^\n]", &id, text, sizeof(text));
	
	char decodedText[FRAME_PAYLOAD_SIZE];
	Base64decode(decodedText, text);

	CQ_sendGroupMsg(appAuthCode, id, decodedText);
}

void prcsDiscussMessage(const char *payload) {
	int64_t id;
	char text[FRAME_PAYLOAD_SIZE];
	sscanf_s(payload, "%d %[^\n]", &id, text, sizeof(text));
	
	char decodedText[FRAME_PAYLOAD_SIZE];
	Base64decode(decodedText, text);

	CQ_sendDiscussMsg(appAuthCode, id, decodedText);
}

void prcsUnknownFramePrefix(const char *buffer) {
	char category[] = "UnknownFramePrefix";
	CQ_addLog(appAuthCode, CQLOG_WARNING, category, buffer);
}


/********
 * API Server
 ********/
APIServer::APIServer(void)
{
	WSADATA wsa;
	WSAStartup(MAKEWORD(1, 1), &wsa);
	
	localInfo.sin_family = AF_INET;
	localInfo.sin_addr.s_addr = inet_addr("127.0.0.1");
	localInfo.sin_port = htons(SERVER_PORT);

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
			
			if (strcmp(prefix, "ClientHello") == 0) {
				prcsClientHello(payload);
				continue;
			}
			if (strcmp(prefix, "PrivateMessage") == 0) {
				prcsPrivateMessage(payload);
				continue;
			}
			if (strcmp(prefix, "GroupMessage") == 0) {
				prcsGroupMessage(payload);
				continue;
			}
			if (strcmp(prefix, "DiscussMessage") == 0) {
				prcsGroupMessage(payload);
				continue;
			}
			// Unknown prefix
			prcsUnknownFramePrefix(buffer);
		}
	}
}