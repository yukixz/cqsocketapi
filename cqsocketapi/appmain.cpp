#include "stdafx.h"
#include "string.h"
#include "process.h"
#include "cqp.h"

#include "appmain.h"
#include "base64.h"
#include "winsock2.h"
#include "APIClient.h"
#include "APIServer.h"
#include "sstream"

using namespace std;

APIClient *client = nullptr;
APIServer *server = nullptr;
int appAuthCode = -1;

int SERVER_PORT = 11235;
int CLIENT_SIZE = 32;
int CLIENT_TIMEOUT = 300;
int FRAME_PREFIX_SIZE = 256;
int FRAME_PAYLOAD_SIZE = 32768;
int FRAME_SIZE = 33025;

unsigned __stdcall startServer(void *args) {
	server = new APIServer();
	server->run();
	return 0;
}

CQEVENT(const char*, AppInfo, 0)() {
	return CQAPPINFO;
}

CQEVENT(int32_t, Initialize, 4)(int32_t AuthCode) {
	appAuthCode = AuthCode;
	return 0;
}

CQEVENT(int32_t, __eventStartup, 0)() {
	return 0;
}

CQEVENT(int32_t, __eventExit, 0)() {
	delete client;
	delete server;
	return 0;
}

CQEVENT(int32_t, __eventEnable, 0)() {

	string configFolder = ".\\app\\" CQAPPID;
	string configFile = configFolder + "\\config.ini";

	if (GetFileAttributes(configFile.data()) == -1) {
		if (GetFileAttributes(configFolder.data()) == -1) {
			CreateDirectory(configFolder.data(), NULL);
		}
		CloseHandle(CreateFile(configFile.data(), GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL));
		CQ_addLog(appAuthCode, CQLOG_INFO, "提示信息", "配置文件不存在，将以默认值自动生成");
	}

	stringstream ss;  string value; int valueInt = -1;

#define tryGetInt(id)\
	valueInt = GetPrivateProfileInt("Server", #id, -1, configFile.data());\
	if (valueInt != -1) {id = valueInt;} else {\
		ss << id; ss >> value;\
		WritePrivateProfileString("Server", #id, value.data(), configFile.data());\
	}\
	ss.clear(); value.clear();valueInt = -1;\

	tryGetInt(SERVER_PORT);
	tryGetInt(CLIENT_SIZE);
	tryGetInt(CLIENT_TIMEOUT);
	tryGetInt(FRAME_PREFIX_SIZE);
	tryGetInt(FRAME_PAYLOAD_SIZE);

	FRAME_SIZE = (FRAME_PREFIX_SIZE + FRAME_PAYLOAD_SIZE + 1);

	client = new APIClient();

	unsigned tid;
	HANDLE thd;
	thd = (HANDLE)_beginthreadex(NULL, 0, startServer, NULL, 0, &tid);

	return 0;
}

CQEVENT(int32_t, __eventDisable, 0)() {

	return 0;
}

/*
* Type=21 私聊消息
*/
CQEVENT(int32_t, __eventPrivateMsg, 24)(int32_t subType, int32_t sendTime, int64_t fromQQ, const char *msg, int32_t font) {

	char* encoded_msg = new char[FRAME_PAYLOAD_SIZE];
	Base64encode(encoded_msg, msg, strlen(msg));

	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "PrivateMessage %I32d %I64d %s", subType, fromQQ, encoded_msg);
	client->send(buffer, strlen(buffer));

	delete[] encoded_msg;
	delete[] buffer;

	return EVENT_IGNORE;
}

/*
* Type=2 群消息
*/
CQEVENT(int32_t, __eventGroupMsg, 36)(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t fromQQ, const char *fromAnonymous, const char *msg, int32_t font) {

	char* encoded_msg = new char[FRAME_PAYLOAD_SIZE];
	Base64encode(encoded_msg, msg, strlen(msg));

	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "GroupMessage %I32d %I64d %I64d %s", subType, fromGroup, fromQQ, encoded_msg);
	client->send(buffer, strlen(buffer));

	delete[] encoded_msg;
	delete[] buffer;

	return EVENT_IGNORE;
}

/*
* Type=4 讨论组消息
*/
CQEVENT(int32_t, __eventDiscussMsg, 32)(int32_t subType, int32_t sendTime, int64_t fromDiscuss, int64_t fromQQ, const char *msg, int32_t font) {

	char* encoded_msg = new char[FRAME_PAYLOAD_SIZE];
	Base64encode(encoded_msg, msg, strlen(msg));

	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "DiscussMessage %I32d %I64d %I64d %s", subType, fromDiscuss, fromQQ, encoded_msg);
	client->send(buffer, strlen(buffer));

	delete[] encoded_msg;
	delete[] buffer;

	return EVENT_IGNORE;
}

/*
* Type=101 群事件-管理员变动
*/
CQEVENT(int32_t, __eventSystem_GroupAdmin, 24)(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t beingOperateQQ) {

	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "GroupAdmin %I32d %I64d %I64d", subType, fromGroup, beingOperateQQ);
	client->send(buffer, strlen(buffer));

	delete[] buffer;

	return EVENT_IGNORE;
}

/*
* Type=102 群事件-群成员减少
*/
CQEVENT(int32_t, __eventSystem_GroupMemberDecrease, 32)(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t fromQQ, int64_t beingOperateQQ) {

	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "GroupMemberDecrease %I32d %I64d %I64d %I64d", subType, fromGroup, fromQQ, beingOperateQQ);
	client->send(buffer, strlen(buffer));

	delete[] buffer;

	return EVENT_IGNORE;
}

/*
* Type=103 群事件-群成员增加
*/
CQEVENT(int32_t, __eventSystem_GroupMemberIncrease, 32)(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t fromQQ, int64_t beingOperateQQ) {

	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "GroupMemberIncrease %I32d %I64d %I64d %I64d", subType, fromGroup, fromQQ, beingOperateQQ);
	client->send(buffer, strlen(buffer));

	delete[] buffer;

	return EVENT_IGNORE;
}


/*
* Type=201 好友事件-好友已添加
*/
CQEVENT(int32_t, __eventFriend_Add, 16)(int32_t subType, int32_t sendTime, int64_t fromQQ) {

	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "FriendAdded %I32d %I64d", subType, fromQQ);
	client->send(buffer, strlen(buffer));

	delete[] buffer;

	return EVENT_IGNORE;
}

/*
* Type=301 请求-好友添加
*/
CQEVENT(int32_t, __eventRequest_AddFriend, 24)(int32_t subType, int32_t sendTime, int64_t fromQQ, const char *msg, const char *responseFlag) {

	char* encoded_msg = new char[FRAME_PAYLOAD_SIZE / 2];
	Base64encode(encoded_msg, msg, strlen(msg));

	char* encoded_flag = new char[FRAME_PAYLOAD_SIZE / 2];
	Base64encode(encoded_flag, responseFlag, strlen(responseFlag));

	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "AddFriend %I32d %I64d %s %s", subType, fromQQ, encoded_msg, encoded_flag);
	client->send(buffer, strlen(buffer));

	delete[] encoded_msg;
	delete[] encoded_flag;
	delete[] buffer;

	return EVENT_IGNORE;
}

/*
* Type=302 请求-群添加
*/
CQEVENT(int32_t, __eventRequest_AddGroup, 32)(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t fromQQ, const char *msg, const char *responseFlag) {
	
	char* encoded_msg = new char[FRAME_PAYLOAD_SIZE / 2];
	Base64encode(encoded_msg, msg, strlen(msg));

	char* encoded_flag = new char[FRAME_PAYLOAD_SIZE / 2];
	Base64encode(encoded_flag, responseFlag, strlen(responseFlag));

	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "AddGroup %I32d %I64d %I64d %s %s", subType, fromGroup, fromQQ, encoded_msg, encoded_flag);
	client->send(buffer, strlen(buffer));

	delete[] encoded_msg;
	delete[] encoded_flag;
	delete[] buffer;

	return EVENT_IGNORE;
}

CQEVENT(int32_t, _eventGroupUpload, 28)(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t fromQQ, const char *file) {

	char* encoded_file = new char[FRAME_PAYLOAD_SIZE];
	Base64encode(encoded_file, file, strlen(file));

	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "GroupUpload %I32d %I64d %I64d %s", subType, fromGroup, fromQQ, encoded_file);
	client->send(buffer, strlen(buffer));

	delete[] encoded_file;
	delete[] buffer;

	return EVENT_IGNORE;
}
