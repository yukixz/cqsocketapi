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
#include "MemberInfoProccessor.h"

using namespace std;

APIClient *client = NULL;
APIServer *server = NULL;
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

/* 
* 返回应用的ApiVer、Appid，打包后将不会调用
*/
CQEVENT(const char*, AppInfo, 0)() {
	return CQAPPINFO;
}


/* 
* 接收应用AuthCode，酷Q读取应用信息后，如果接受该应用，将会调用这个函数并传递AuthCode。
* 不要在本函数处理其他任何代码，以免发生异常情况。如需执行初始化代码请在Startup事件中执行（Type=1001）。
*/
CQEVENT(int32_t, Initialize, 4)(int32_t AuthCode) {
	appAuthCode = AuthCode;

	return 0;
}


/*
* Type=1001 酷Q启动
* 无论本应用是否被启用，本函数都会在酷Q启动后执行一次，请在这里执行应用初始化代码。
* 如非必要，不建议在这里加载窗口。（可以添加菜单，让用户手动打开窗口）
*/
CQEVENT(int32_t, __eventStartup, 0)() {

	return 0;
}


/*
* Type=1002 酷Q退出
* 无论本应用是否被启用，本函数都会在酷Q退出前执行一次，请在这里执行插件关闭代码。
* 本函数调用完毕后，酷Q将很快关闭，请不要再通过线程等方式执行其他代码。
*/
CQEVENT(int32_t, __eventExit, 0)() {

	return 0;
}

/*
* Type=1003 应用已被启用
* 当应用被启用后，将收到此事件。
* 如果酷Q载入时应用已被启用，则在_eventStartup(Type=1001,酷Q启动)被调用后，本函数也将被调用一次。
* 如非必要，不建议在这里加载窗口。（可以添加菜单，让用户手动打开窗口）
*/
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
	thd = (HANDLE) _beginthreadex(NULL, 0, startServer, NULL, 0, &tid);

	return 0;
}


/*
* Type=1004 应用将被停用
* 当应用被停用前，将收到此事件。
* 如果酷Q载入时应用已被停用，则本函数*不会*被调用。
* 无论本应用是否被启用，酷Q关闭前本函数都*不会*被调用。
*/
CQEVENT(int32_t, __eventDisable, 0)() {

	return 0;
}


/*
* Type=21 私聊消息
* subType 子类型，11/来自好友 1/来自在线状态 2/来自群 3/来自讨论组
*/
CQEVENT(int32_t, __eventPrivateMsg, 24)(int32_t subType, int32_t sendTime, int64_t fromQQ, const char *msg, int32_t font) {

	char* encoded_msg = new char[FRAME_PAYLOAD_SIZE];
	Base64encode(encoded_msg, msg, strlen(msg));

	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "PrivateMessage %I64d %s", fromQQ, encoded_msg);
	client->send(buffer, strlen(buffer));

	return EVENT_IGNORE;
}


/*
* Type=2 群消息
*/
CQEVENT(int32_t, __eventGroupMsg, 36)(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t fromQQ, const char *fromAnonymous, const char *msg, int32_t font) {
	


	char* encoded_msg = new char[FRAME_PAYLOAD_SIZE];
	Base64encode(encoded_msg, msg, strlen(msg));

	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "GroupMessage %I64d %I64d %s", fromGroup, fromQQ, encoded_msg);
	client->send(buffer, strlen(buffer));

	return EVENT_IGNORE;
}


/*
* Type=4 讨论组消息
*/
CQEVENT(int32_t, __eventDiscussMsg, 32)(int32_t subType, int32_t sendTime, int64_t fromDiscuss, int64_t fromQQ, const char *msg, int32_t font) {
	
	char* encoded_msg = new char[FRAME_PAYLOAD_SIZE];
	Base64encode(encoded_msg, msg, strlen(msg));

	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "DiscussMessage %I64d %I64d %s", fromDiscuss, fromQQ, encoded_msg);
	client->send(buffer, strlen(buffer));

	return EVENT_IGNORE;
}


/*
* Type=101 群事件-管理员变动
* subType 子类型，1/被取消管理员 2/被设置管理员
*/
CQEVENT(int32_t, __eventSystem_GroupAdmin, 24)(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t beingOperateQQ) {

	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "GroupAdmin %I32d %I64d %I64d", fromGroup, subType, beingOperateQQ);
	client->send(buffer, strlen(buffer));

	return EVENT_IGNORE;
}


/*
* Type=102 群事件-群成员减少
* subType 子类型，1/群员离开 2/群员被踢 3/自己(即登录号)被踢
* fromQQ 操作者QQ(仅subType为2、3时存在)
* beingOperateQQ 被操作QQ
*/
CQEVENT(int32_t, __eventSystem_GroupMemberDecrease, 32)(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t fromQQ, int64_t beingOperateQQ) {

	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "GroupMemberDecrease %I64d %I64d %I64d", fromGroup, fromQQ, beingOperateQQ);
	client->send(buffer, strlen(buffer));

	return EVENT_IGNORE;
}


/*
* Type=103 群事件-群成员增加
* subType 子类型，1/管理员已同意 2/管理员邀请
* fromQQ 操作者QQ(即管理员QQ)
* beingOperateQQ 被操作QQ(即加群的QQ)
*/
CQEVENT(int32_t, __eventSystem_GroupMemberIncrease, 32)(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t fromQQ, int64_t beingOperateQQ) {

	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "GroupMemberIncrease %I64d %I64d %I64d", fromGroup, fromQQ, beingOperateQQ);
	client->send(buffer, strlen(buffer));

	return EVENT_IGNORE;
}


/*
* Type=201 好友事件-好友已添加
*/
CQEVENT(int32_t, __eventFriend_Add, 16)(int32_t subType, int32_t sendTime, int64_t fromQQ) {

	return EVENT_IGNORE;
}


/*
* Type=301 请求-好友添加
* msg 附言
* responseFlag 反馈标识(处理请求用)
*/
CQEVENT(int32_t, __eventRequest_AddFriend, 24)(int32_t subType, int32_t sendTime, int64_t fromQQ, const char *msg, const char *responseFlag) {

	return EVENT_IGNORE;
}


/*
* Type=302 请求-群添加
* subType 子类型，1/他人申请入群 2/自己(即登录号)受邀入群
* msg 附言
* responseFlag 反馈标识(处理请求用)
*/
CQEVENT(int32_t, __eventRequest_AddGroup, 32)(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t fromQQ, const char *msg, const char *responseFlag) {

	return EVENT_IGNORE;
}

/*
* 菜单，可在 .json 文件中设置菜单数目、函数名
* 如果不使用菜单，请在 .json 及此处删除无用菜单
*/
/*
CQEVENT(int32_t, __menuA, 0)() {
	MessageBoxA(NULL, "这是menuA，在这里载入窗口，或者进行其他工作。", "" ,0);
	return 0;
}

CQEVENT(int32_t, __menuB, 0)() {
	MessageBoxA(NULL, "这是menuB，在这里载入窗口，或者进行其他工作。", "" ,0);
	return 0;
}
*/
