#include "stdafx.h"
#include "string.h"
#include "cqp.h"
#include "base64.h"

#include "appmain.h"
#include "APIClient.h"
#include "APIServer.h"

#include <string>
#include <fstream>
extern APIClient *client;
extern int appAuthCode;


/********
 * Message Processor
 ********/
void prcsClientHello(const char *payload) {
	int port;
	sscanf_s(payload, "%d", &port);

	client->add(port);

	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char),
		"ServerHello %I32d %I32d %I32d %I32d",
		CLIENT_TIMEOUT, FRAME_PREFIX_SIZE, FRAME_PAYLOAD_SIZE, FRAME_SIZE);

	client->send(buffer, strlen(buffer));

	delete[] buffer;
}

void prcsSendPrivateMessage(const char *payload) {
	int64_t qq;
	char* text = new char[FRAME_PAYLOAD_SIZE];
	sscanf_s(payload, "%I64d %[^\n]", &qq, text, sizeof(char) * FRAME_PAYLOAD_SIZE);

	char* decodedText = new char[FRAME_PAYLOAD_SIZE];
	Base64decode(decodedText, text);

	CQ_sendPrivateMsg(appAuthCode, qq, decodedText);

	delete[] text;
	delete[] decodedText;
}

void prcsSendGroupMessage(const char *payload) {
	int64_t group;
	char* text = new char[FRAME_PAYLOAD_SIZE];
	sscanf_s(payload, "%I64d %[^\n]", &group, text, sizeof(char) * FRAME_PAYLOAD_SIZE);

	char* decodedText = new char[FRAME_PAYLOAD_SIZE];
	Base64decode(decodedText, text);

	CQ_sendGroupMsg(appAuthCode, group, decodedText);

	delete[] text;
	delete[] decodedText;
}

void prcsSendDiscussMessage(const char *payload) {
	int64_t discuss;
	char* text = new char[FRAME_PAYLOAD_SIZE];
	sscanf_s(payload, "%I64d %[^\n]", &discuss, text, sizeof(char) * FRAME_PAYLOAD_SIZE);

	char* decodedText = new char[FRAME_PAYLOAD_SIZE];
	Base64decode(decodedText, text);

	CQ_sendDiscussMsg(appAuthCode, discuss, decodedText);

	delete[] text;
	delete[] decodedText;
}

void prcsSendLike(const char *payload) {
	int64_t qq;
	int32_t times;
	sscanf_s(payload, "%I64d %I32d", &qq, &times);

	CQ_sendLikeV2(appAuthCode, qq, times);
}

void prcsSetGroupKick(const char *payload) {
	CQBOOL rejectaddrequest;
	int64_t group, qq;
	sscanf_s(payload, "%I64d %I64d %I32d", &group, &qq, &rejectaddrequest);

	CQ_setGroupKick(appAuthCode, group, qq, rejectaddrequest);
}

void prcsSetGroupBan(const char *payload) {
	int64_t group, qq, duration;
	sscanf_s(payload, "%I64d %I64d %I64d", &group, &qq, &duration);

	CQ_setGroupBan(appAuthCode, group, qq, duration);
}

void prcsSetGroupAdmin(const char *payload) {
	CQBOOL setadmin;
	int64_t group, qq;
	sscanf_s(payload, "%I64d %I64d %I32d", &group, &qq, &setadmin);

	CQ_setGroupAdmin(appAuthCode, group, qq, setadmin);
}

void prcsSetGroupWholeBan(const char *payload) {
	CQBOOL enableban;
	int64_t group;
	sscanf_s(payload, "%I64d %I32d", &group, &enableban);

	CQ_setGroupWholeBan(appAuthCode, group, enableban);
}

void prcsSetGroupAnonymousBan(const char *payload) {
	int64_t group, duration;
	char* anomymous = new char[FRAME_PAYLOAD_SIZE];
	sscanf_s(payload, "%I64d %[^\n] %I64d", &group, anomymous, sizeof(char) * FRAME_PAYLOAD_SIZE, &duration);

	char* decodedAnomymous = new char[FRAME_PAYLOAD_SIZE];
	Base64decode(decodedAnomymous, anomymous);

	CQ_setGroupAnonymousBan(appAuthCode, group, decodedAnomymous, duration);

	delete[] anomymous;
	delete[] decodedAnomymous;
}

void prcsSetGroupAnonymous(const char *payload) {
	CQBOOL enableanomymous;
	int64_t group;
	sscanf_s(payload, "%I64d %I32d", &group, &enableanomymous);

	CQ_setGroupAnonymous(appAuthCode, group, enableanomymous);
}

void prcsSetGroupCard(const char *payload) {
	int64_t group, qq;
	char* newcard = new char[FRAME_PAYLOAD_SIZE];
	sscanf_s(payload, "%I64d %I64d %[^\n]", &group, &qq, newcard, sizeof(char) * FRAME_PAYLOAD_SIZE);

	char* decodedNewcard = new char[FRAME_PAYLOAD_SIZE];
	Base64decode(decodedNewcard, newcard);

	CQ_setGroupCard(appAuthCode, group, qq, decodedNewcard);

	delete[] newcard;
	delete[] decodedNewcard;
}

void prcsSetGroupLeave(const char *payload) {
	CQBOOL isdismiss;
	int64_t group;
	sscanf_s(payload, "%I64d %I32d", &group, &isdismiss);

	CQ_setGroupLeave(appAuthCode, group, isdismiss);
}

void prcsSetGroupSpecialTitle(const char *payload) {
	int64_t group, qq, duration;
	char* newspecialtitle = new char[FRAME_PAYLOAD_SIZE];
	sscanf_s(payload, "%I64d %I64d %[^\n] %I64d", &group, &qq, newspecialtitle, sizeof(char) * FRAME_PAYLOAD_SIZE, &duration);

	char* decodedNewspecialtitle = new char[FRAME_PAYLOAD_SIZE];
	Base64decode(decodedNewspecialtitle, newspecialtitle);

	CQ_setGroupSpecialTitle(appAuthCode, group, qq, decodedNewspecialtitle, duration);

	delete[] newspecialtitle;
	delete[] decodedNewspecialtitle;
}

void prcsSetDiscussLeave(const char *payload) {
	int64_t discuss;
	sscanf_s(payload, "%I64d", &discuss);

	CQ_setDiscussLeave(appAuthCode, discuss);
}

void prcsFriendAddRequest(const char *payload) {
	int32_t responseoperation;
	char* responseflag = new char[FRAME_PAYLOAD_SIZE / 2];
	char* remark = new char[FRAME_PAYLOAD_SIZE / 2];

	sscanf_s(payload, "%[^\n] %I32d %[^\n]",
		responseflag, sizeof(char) * FRAME_PAYLOAD_SIZE / 2,
		&responseoperation,
		remark, sizeof(char) * FRAME_PAYLOAD_SIZE / 2);

	char* decodedResponseflag = new char[FRAME_PAYLOAD_SIZE / 2];
	char* decodedRemark = new char[FRAME_PAYLOAD_SIZE / 2];
	Base64decode(decodedResponseflag, responseflag);
	Base64decode(decodedRemark, remark);

	CQ_setFriendAddRequest(appAuthCode, decodedResponseflag, responseoperation, decodedRemark);

	delete[] responseflag;
	delete[] remark;
	delete[] decodedResponseflag;
	delete[] decodedRemark;
}

void prcsGroupAddRequest(const char *payload) {
	int32_t requesttype;
	int32_t responseoperation;
	char* responseflag = new char[FRAME_PAYLOAD_SIZE / 2];
	char* reason = new char[FRAME_PAYLOAD_SIZE / 2];

	sscanf_s(payload, "%[^\n] %I32d %I32d %[^\n]",
		responseflag, sizeof(char) * FRAME_PAYLOAD_SIZE / 2,
		&requesttype,
		&responseoperation,
		reason, sizeof(char) * FRAME_PAYLOAD_SIZE / 2);

	char* decodedResponseflag = new char[FRAME_PAYLOAD_SIZE / 2];
	char* decodedReason = new char[FRAME_PAYLOAD_SIZE / 2];
	Base64decode(decodedResponseflag, responseflag);
	Base64decode(decodedReason, reason);

	CQ_setGroupAddRequestV2(appAuthCode, decodedResponseflag, requesttype, responseoperation, decodedReason);

	delete[] responseflag;
	delete[] reason;
	delete[] decodedResponseflag;
	delete[] decodedReason;
}

void prcsGetGroupMemberInfo(const char *payload) {
	CQBOOL nocache;
	int64_t group, qq;
	sscanf_s(payload, "%I64d %I64d %I32d", &group, &qq, &nocache);

	//char* encoded_info = new char[FRAME_PAYLOAD_SIZE];

	auto info = CQ_getGroupMemberInfoV2(appAuthCode, group, qq, nocache);
	//Base64encode(encoded_info, info, strlen(info));

	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "SrvGroupMemberInfo %s", info);
	client->send(buffer, strlen(buffer));

	//delete[] encoded_info;
	delete[] buffer;
}

void prcsGetGroupMemberList(const char *payload) {
	int64_t group;
	sscanf_s(payload, "%I64d", &group);
	char* encoded_path = new char[FRAME_PAYLOAD_SIZE];
	std::string appPath(CQ_getAppDirectory(appAuthCode));
	std::string cachePath = appPath + "GroupListCache\\";

	std::string filename = std::string(cachePath) + std::to_string(group) + ".g";
	CreateDirectory(cachePath.c_str(), nullptr);
	std::ofstream fout(filename.c_str(), std::ofstream::out);
	auto list = CQ_getGroupMemberList(appAuthCode, group);
	if (fout.is_open()) {
		fout << std::string(list);
		fout.close();
	}
	Base64encode(encoded_path, filename.c_str(), strlen(filename.c_str()));
	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "SrvGroupMemberList %s", encoded_path);
	client->send(buffer, strlen(buffer));

	delete[] encoded_path;
	delete[] buffer;
}

void prcsGetStrangerInfo(const char *payload) {
	CQBOOL nocache;
	int64_t qq;
	sscanf_s(payload, "%I64d %I32d", &qq, &nocache);

	char* encoded_info = new char[FRAME_PAYLOAD_SIZE];

	auto info = CQ_getStrangerInfo(appAuthCode, qq, nocache);
	Base64encode(encoded_info, info, strlen(info));

	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "SrvStrangerInfo %s", encoded_info);
	client->send(buffer, strlen(buffer));

	delete[] encoded_info;
	delete[] buffer;
}

void prcsGetCookies() {

	char* encoded_cookies = new char[FRAME_PAYLOAD_SIZE];
	auto cookies = CQ_getCookies(appAuthCode);
	Base64encode(encoded_cookies, cookies, strlen(cookies));

	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "SrvCookies %s", encoded_cookies);
	client->send(buffer, strlen(buffer));

	delete[] encoded_cookies;
	delete[] buffer;
}

void prcsGetCsrfToken() {

	int32_t token = CQ_getCsrfToken(appAuthCode);

	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "SrvCsrfToken %I32d", token);
	client->send(buffer, strlen(buffer));

	delete[] buffer;
}

void prcsGetLoginQQ() {

	int64_t qq = CQ_getLoginQQ(appAuthCode);

	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "SrvLoginQQ %I64d", qq);
	client->send(buffer, strlen(buffer));

	delete[] buffer;
}

void prcsGetLoginNickname() {

	char* encoded_nickname = new char[FRAME_PAYLOAD_SIZE];
	auto nickname = CQ_getLoginNick(appAuthCode);
	Base64encode(encoded_nickname, nickname, strlen(nickname));

	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "SrvLoginNickname %s", encoded_nickname);
	client->send(buffer, strlen(buffer));

	delete[] encoded_nickname;
	delete[] buffer;
}

void prcsGetAppDirectory() {

	char* encoded_appDir = new char[FRAME_PAYLOAD_SIZE];
	auto appDir = CQ_getAppDirectory(appAuthCode);
	Base64encode(encoded_appDir, appDir, strlen(appDir));

	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "SrvAppDirectory %s", encoded_appDir);
	client->send(buffer, strlen(buffer));

	delete[] encoded_appDir;
	delete[] buffer;
}

//////////////////////////////////////////////////////////////////////////
//	Not Implemented
//////////////////////////////////////////////////////////////////////////
//	CQ_setFatal
//	CQ_addLog

void prcsUnknownFramePrefix(const char *buffer) {
	char category[] = "UnknownFramePrefix";
	CQ_addLog(appAuthCode, CQLOG_WARNING, category, buffer);
}


/********
 * API Server
 ********/
APIServer::APIServer(void) {
	WSADATA wsa;
	WSAStartup(MAKEWORD(1, 1), &wsa);

	localInfo.sin_family = AF_INET;
	localInfo.sin_addr.s_addr = inet_addr("127.0.0.1");
	localInfo.sin_port = htons(SERVER_PORT);

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	bind(sock, (sockaddr *)&localInfo, sizeof(localInfo));
}


APIServer::~APIServer(void) {
	closesocket(sock);
}


void APIServer::run() {
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
				prcsSendPrivateMessage(payload);
				continue;
			}
			if (strcmp(prefix, "GroupMessage") == 0) {
				prcsSendGroupMessage(payload);
				continue;
			}
			if (strcmp(prefix, "DiscussMessage") == 0) {
				prcsSendDiscussMessage(payload);
				continue;
			}

			if (strcmp(prefix, "Like") == 0) {
				prcsSendLike(payload);
				continue;
			}

			if (strcmp(prefix, "GroupKick") == 0) {
				prcsSetGroupKick(payload);
				continue;
			}

			if (strcmp(prefix, "GroupBan") == 0) {
				prcsSetGroupBan(payload);
				continue;
			}

			if (strcmp(prefix, "GroupAdmin") == 0) {
				prcsSetGroupAdmin(payload);
				continue;
			}

			if (strcmp(prefix, "GroupWholeBan") == 0) {
				prcsSetGroupWholeBan(payload);
				continue;
			}

			if (strcmp(prefix, "GroupAnonymousBan") == 0) {
				prcsSetGroupAnonymousBan(payload);
				continue;
			}

			if (strcmp(prefix, "GroupAnonymous") == 0) {
				prcsSetGroupAnonymous(payload);
				continue;
			}

			if (strcmp(prefix, "GroupCard") == 0) {
				prcsSetGroupCard(payload);
				continue;
			}

			if (strcmp(prefix, "GroupLeave") == 0) {
				prcsSetGroupLeave(payload);
				continue;
			}

			if (strcmp(prefix, "GroupSpecialTitle") == 0) {
				prcsSetGroupSpecialTitle(payload);
				continue;
			}

			if (strcmp(prefix, "DiscussLeave") == 0) {
				prcsSetDiscussLeave(payload);
				continue;
			}

			if (strcmp(prefix, "FriendAddRequest") == 0) {
				prcsFriendAddRequest(payload);
				continue;
			}

			if (strcmp(prefix, "GroupAddRequest") == 0) {
				prcsGroupAddRequest(payload);
				continue;
			}

			if (strcmp(prefix, "GroupMemberInfo") == 0) {
				prcsGetGroupMemberInfo(payload);
				continue;
			}

			if (strcmp(prefix, "GroupMemberList") == 0) {
				prcsGetGroupMemberList(payload);
				continue;
			}

			if (strcmp(prefix, "StrangerInfo") == 0) {
				prcsGetStrangerInfo(payload);
				continue;
			}

			if (strcmp(prefix, "Cookies") == 0) {
				prcsGetCookies();
				continue;
			}

			if (strcmp(prefix, "CsrfToken") == 0) {
				prcsGetCsrfToken();
				continue;
			}

			if (strcmp(prefix, "LoginQQ") == 0) {
				prcsGetLoginQQ();
				continue;
			}

			if (strcmp(prefix, "LoginNick") == 0) {
				prcsGetLoginNickname();
				continue;
			}

			if (strcmp(prefix, "AppDirectory") == 0) {
				prcsGetAppDirectory();
				continue;
			}

			// Unknown prefix
			prcsUnknownFramePrefix(buffer);
		}
	}

}