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
* ����Ӧ�õ�ApiVer��Appid������󽫲������
*/
CQEVENT(const char*, AppInfo, 0)() {
	return CQAPPINFO;
}


/*
* ����Ӧ��AuthCode����Q��ȡӦ����Ϣ��������ܸ�Ӧ�ã���������������������AuthCode��
* ��Ҫ�ڱ��������������κδ��룬���ⷢ���쳣���������ִ�г�ʼ����������Startup�¼���ִ�У�Type=1001����
*/
CQEVENT(int32_t, Initialize, 4)(int32_t AuthCode) {
	appAuthCode = AuthCode;

	return 0;
}


/*
* Type=1001 ��Q����
* ���۱�Ӧ���Ƿ����ã������������ڿ�Q������ִ��һ�Σ���������ִ��Ӧ�ó�ʼ�����롣
* ��Ǳ�Ҫ����������������ش��ڡ���������Ӳ˵������û��ֶ��򿪴��ڣ�
*/
CQEVENT(int32_t, __eventStartup, 0)() {

	return 0;
}


/*
* Type=1002 ��Q�˳�
* ���۱�Ӧ���Ƿ����ã������������ڿ�Q�˳�ǰִ��һ�Σ���������ִ�в���رմ��롣
* ������������Ϻ󣬿�Q���ܿ�رգ��벻Ҫ��ͨ���̵߳ȷ�ʽִ���������롣
*/
CQEVENT(int32_t, __eventExit, 0)() {

	return 0;
}

/*
* Type=1003 Ӧ���ѱ�����
* ��Ӧ�ñ����ú󣬽��յ����¼���
* �����Q����ʱӦ���ѱ����ã�����_eventStartup(Type=1001,��Q����)�����ú󣬱�����Ҳ��������һ�Ρ�
* ��Ǳ�Ҫ����������������ش��ڡ���������Ӳ˵������û��ֶ��򿪴��ڣ�
*/
CQEVENT(int32_t, __eventEnable, 0)() {

	string configFolder = ".\\app\\" CQAPPID;
	string configFile = configFolder + "\\config.ini";

	if (GetFileAttributes(configFile.data()) == -1) {
		if (GetFileAttributes(configFolder.data()) == -1) {
			CreateDirectory(configFolder.data(), NULL);
		}
		CloseHandle(CreateFile(configFile.data(), GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL));
		CQ_addLog(appAuthCode, CQLOG_INFO, "��ʾ��Ϣ", "�����ļ������ڣ�����Ĭ��ֵ�Զ�����");
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


/*
* Type=1004 Ӧ�ý���ͣ��
* ��Ӧ�ñ�ͣ��ǰ�����յ����¼���
* �����Q����ʱӦ���ѱ�ͣ�ã��򱾺���*����*�����á�
* ���۱�Ӧ���Ƿ����ã���Q�ر�ǰ��������*����*�����á�
*/
CQEVENT(int32_t, __eventDisable, 0)() {

	return 0;
}


/*
* Type=21 ˽����Ϣ
* subType �����ͣ�11/���Ժ��� 1/��������״̬ 2/����Ⱥ 3/����������
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
* Type=2 Ⱥ��Ϣ
*/
CQEVENT(int32_t, __eventGroupMsg, 36)(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t fromQQ, const char *fromAnonymous, const char *msg, int32_t font) {

	struct CQ_Type_GroupMember memberinfo;
	MemberInfoProccessor proc;
	bool gotInfo = proc.GetGroupMemberInfo(appAuthCode, fromGroup, fromQQ, memberinfo);
	std::string nickname_result;
	if (gotInfo) {
		nickname_result = memberinfo.nick;
		if (nickname_result.length() == 0) {
			nickname_result = "δ����Ⱥ��Ƭ";
		}
	}
	else {
		nickname_result = "δ����Ⱥ��Ƭ";
	}
	const char *nickname = nickname_result.c_str();
	char* encoded_nickname = new char[1024];
	char* encoded_msg = new char[FRAME_PAYLOAD_SIZE];
	Base64encode(encoded_msg, msg, strlen(msg));
	Base64encode(encoded_nickname, nickname, strlen(nickname));
	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "GroupMessage %I64d %I64d %s %s", fromGroup, fromQQ, encoded_nickname, encoded_msg);
	client->send(buffer, strlen(buffer));

	return EVENT_IGNORE;
}


/*
* Type=4 ��������Ϣ
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
* Type=101 Ⱥ�¼�-����Ա�䶯
* subType �����ͣ�1/��ȡ������Ա 2/�����ù���Ա
*/
CQEVENT(int32_t, __eventSystem_GroupAdmin, 24)(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t beingOperateQQ) {
	/*
	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "GroupAdmin %I32d %I64d %I64d", fromGroup, subType, beingOperateQQ);
	client->send(buffer, strlen(buffer));
	*/
	return EVENT_IGNORE;
}


/*
* Type=102 Ⱥ�¼�-Ⱥ��Ա����
* subType �����ͣ�1/ȺԱ�뿪 2/ȺԱ���� 3/�Լ�(����¼��)����
* fromQQ ������QQ(��subTypeΪ2��3ʱ����)
* beingOperateQQ ������QQ
*/
CQEVENT(int32_t, __eventSystem_GroupMemberDecrease, 32)(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t fromQQ, int64_t beingOperateQQ) {
	/*
	char* buffer = new char[FRAME_SIZE];
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "GroupMemberDecrease %I64d %I64d %I64d", fromGroup, fromQQ, beingOperateQQ);
	client->send(buffer, strlen(buffer));
	*/
	return EVENT_IGNORE;
}


/*
* Type=103 Ⱥ�¼�-Ⱥ��Ա����
* subType �����ͣ�1/����Ա��ͬ�� 2/����Ա����
* fromQQ ������QQ(������ԱQQ)
* beingOperateQQ ������QQ(����Ⱥ��QQ)
*/
CQEVENT(int32_t, __eventSystem_GroupMemberIncrease, 32)(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t fromQQ, int64_t beingOperateQQ) {
	struct CQ_Type_GroupMember memberinfo;
	MemberInfoProccessor proc;
	bool gotInfo = proc.GetGroupMemberInfo(appAuthCode, fromGroup, fromQQ, memberinfo);
	std::string username_result;
	if (gotInfo) {
		username_result = memberinfo.username;
	}
	else {
		username_result = "";
	}

	const char *username = username_result.c_str();
	const char* mssg_pre = "��ӭ�����:";
	const char * mssg_post = "��Ⱥ ������/qq ��Ϣ��qqȺ����Ϣ��";
	char* mssg = new char[FRAME_PAYLOAD_SIZE];
	sprintf_s(mssg, FRAME_PAYLOAD_SIZE * sizeof(char), "%s %s %s", mssg_pre, username, mssg_post);
	const char *nickname = "ϵͳ��Ϣ";
	char* encoded_nickname = new char[1024];
	char* encoded_msg = new char[FRAME_PAYLOAD_SIZE];
	Base64encode(encoded_nickname, nickname, strlen(nickname));
	Base64encode(encoded_msg, mssg, strlen(mssg));
	char* buffer = new char[FRAME_SIZE];
	int64_t QQid = 12345;
	sprintf_s(buffer, FRAME_SIZE * sizeof(char), "GroupMessage %I64d %I64d %s %s", fromGroup, QQid, encoded_nickname, encoded_msg);
	client->send(buffer, strlen(buffer));
	return EVENT_IGNORE;
}


/*
* Type=201 �����¼�-���������
*/
CQEVENT(int32_t, __eventFriend_Add, 16)(int32_t subType, int32_t sendTime, int64_t fromQQ) {

	return EVENT_IGNORE;
}


/*
* Type=301 ����-�������
* msg ����
* responseFlag ������ʶ(����������)
*/
CQEVENT(int32_t, __eventRequest_AddFriend, 24)(int32_t subType, int32_t sendTime, int64_t fromQQ, const char *msg, const char *responseFlag) {

	return EVENT_IGNORE;
}


/*
* Type=302 ����-Ⱥ���
* subType �����ͣ�1/����������Ⱥ 2/�Լ�(����¼��)������Ⱥ
* msg ����
* responseFlag ������ʶ(����������)
*/
CQEVENT(int32_t, __eventRequest_AddGroup, 32)(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t fromQQ, const char *msg, const char *responseFlag) {

	return EVENT_IGNORE;
}

/*
* �˵������� .json �ļ������ò˵���Ŀ��������
* �����ʹ�ò˵������� .json ���˴�ɾ�����ò˵�
*/
/*
CQEVENT(int32_t, __menuA, 0)() {
MessageBoxA(NULL, "����menuA�����������봰�ڣ����߽�������������", "" ,0);
return 0;
}

CQEVENT(int32_t, __menuB, 0)() {
MessageBoxA(NULL, "����menuB�����������봰�ڣ����߽�������������", "" ,0);
return 0;
}
*/
