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
	char *ipv4, *port;
	char* splitter = ":";
	char *input = new char[strlen(payload) + 1];
	strcpy(input, payload);
	ipv4 = strtok(input, splitter);
	port = strtok(NULL, ":");
	int temp = atoi(port);
	//CQ_addLog(appAuthCode, CQLOG_INFO,"APIClient", ipv4);
	//CQ_addLog(appAuthCode, CQLOG_INFO, "APIClient", port);
	char log[1024];
	//sprintf_s(log, "Client added: %d.", temp);
	//CQ_addLog(appAuthCode, CQLOG_INFO, "APIClient", log);
	client->add(temp, ipv4);
	char* buffer = "ServerHello";
	client->send(buffer, strlen(buffer));
}

void prcsPrivateMessage(const char *payload) {
	int64_t id;
	char* text = new char[FRAME_PAYLOAD_SIZE];
	sscanf_s(payload, "%I64d %[^\n]", &id, text, sizeof(char) * FRAME_PAYLOAD_SIZE);

	char* decodedText = new char[FRAME_PAYLOAD_SIZE];
	Base64decode(decodedText, text);

	CQ_sendPrivateMsg(appAuthCode, id, decodedText);
}

void prcsGroupMessage(const char *payload) {
	int64_t id;
	char* text = new char[FRAME_PAYLOAD_SIZE];
	sscanf_s(payload, "%I64d %[^\n]", &id, text, sizeof(char) * FRAME_PAYLOAD_SIZE);

	char* decodedText = new char[FRAME_PAYLOAD_SIZE];
	Base64decode(decodedText, text);

	CQ_sendGroupMsg(appAuthCode, id, decodedText);
}

void prcsDiscussMessage(const char *payload) {
	int64_t id;
	char* text = new char[FRAME_PAYLOAD_SIZE];
	sscanf_s(payload, "%I64d %[^\n]", &id, text, sizeof(char) * FRAME_PAYLOAD_SIZE);

	char* decodedText = new char[FRAME_PAYLOAD_SIZE];
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
	localInfo.sin_addr.s_addr = inet_addr("0.0.0.0");
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
	char* buffer = new char[FRAME_SIZE];
	char* prefix = new char[FRAME_PREFIX_SIZE];
	char* payload = new char[FRAME_PAYLOAD_SIZE];

	while (1) {
		memset(buffer, 0, sizeof(char) * FRAME_SIZE);
		memset(prefix, 0, sizeof(char) * FRAME_PREFIX_SIZE);
		memset(payload, 0, sizeof(char) * FRAME_PAYLOAD_SIZE);
		if (recv(sock, buffer, sizeof(char) * FRAME_SIZE, 0) != SOCKET_ERROR) {
			sscanf_s(buffer, "%s %[^\n]", prefix, sizeof(char) * FRAME_PREFIX_SIZE, payload, sizeof(char) * FRAME_PAYLOAD_SIZE);

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
				prcsDiscussMessage(payload);
				continue;
			}
			// Unknown prefix
			prcsUnknownFramePrefix(buffer);
		}
	}
}