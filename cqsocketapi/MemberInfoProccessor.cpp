#include "stdafx.h"
#include "MemberInfoProccessor.h"
#include "cqp.h"
#include "MemberInfoParser.h"
#include <math.h>

MemberInfoProccessor::MemberInfoProccessor()
{
}


MemberInfoProccessor::~MemberInfoProccessor()
{
}

bool MemberInfoProccessor::GetGroupMemberInfo(int auth, int64_t groupID, int64_t qqid, CQ_Type_GroupMember& groupMember)
{
	std::string encodeStr = CQ_getGroupMemberInfoV2(auth, groupID, qqid, false);
	if (0 == encodeStr.size())
	{
		return false;
	}
	std::string decodeStr = memberInfo_decode(encodeStr);
	if (decodeStr.size() < 40)
	{
		CQ_addLog(auth, CQLOG_ERROR, "MemberInfoProccessor::GetGroupMemberInfo", "decodeStr.size() < 40");
		return false;
	}
	return MemberInfoProccessor_TextToGroupMember(decodeStr, groupMember);
}

bool MemberInfoProccessor::MemberInfoProccessor_TextToGroupMember(std::string decodeStr, CQ_Type_GroupMember& groupMember)
{
	int i = 0;
	int count = 8;
	groupMember.GroupID = MemberInfoProccessor_GetNum(i, count, decodeStr);
	i += count;

	count = 8;
	groupMember.QQID = MemberInfoProccessor_GetNum(i, count, decodeStr);
	i += count;

	groupMember.username = MemberInfoProccessor_GetStr(i, decodeStr);
	i += 2 + groupMember.username.size(); //2 means string length bit

	groupMember.nick = MemberInfoProccessor_GetStr(i, decodeStr);
	i += 2 + groupMember.nick.size();

	count = 4;
	groupMember.sex = (int)MemberInfoProccessor_GetNum(i, 4, decodeStr);
	i += count;

	count = 4;
	groupMember.age = (int)MemberInfoProccessor_GetNum(i, 4, decodeStr);
	i += count;

	groupMember.area = MemberInfoProccessor_GetStr(i, decodeStr);
	i += 2 + groupMember.area.size();

	count = 4;
	groupMember.jointime = (int)MemberInfoProccessor_GetNum(i, 4, decodeStr);
	i += count;

	count = 4;
	groupMember.lastsent = (int)MemberInfoProccessor_GetNum(i, 4, decodeStr);
	i += count;

	groupMember.level_name = MemberInfoProccessor_GetStr(i, decodeStr);
	i += 2 + groupMember.level_name.size();

	count = 4;
	groupMember.permission = (int)MemberInfoProccessor_GetNum(i, 4, decodeStr);
	i += count;

	count = 4;
	groupMember.unfriendly = (MemberInfoProccessor_GetNum(i, 4, decodeStr) == 1);
	i += count;

	groupMember.title = MemberInfoProccessor_GetStr(i, decodeStr);
	i += 2 + groupMember.title.size();

	count = 4;
	groupMember.titleExpiretime = (int)MemberInfoProccessor_GetNum(i, 4, decodeStr);
	i += count;

	count = 4;
	groupMember.nickcanchange = (MemberInfoProccessor_GetNum(i, 4, decodeStr) == 1);
	i += count;

	return true;
}

int64_t MemberInfoProccessor::MemberInfoProccessor_GetNum(int index, long length, const std::string &decodeStr)
{
	std::string subString = decodeStr.substr(index, length);
	return From256to10(subString);
}

std::string MemberInfoProccessor::MemberInfoProccessor_GetStr(int index, const std::string &decodeStr)
{
	int length = (int)MemberInfoProccessor_GetNum(index, 2, decodeStr);
	if (length == 0)
	{
		return "";
	}

	return decodeStr.substr(index + 2, length);
}


int64_t MemberInfoProccessor::From256to10(std::string str256)
{
	int64_t sum = 0;
	for (int i = 0; i < (int)str256.size(); i++)
	{
		sum = sum * 256 + (unsigned char)str256[i];
	}
	return sum;
}

void MemberInfoProccessor::AddLog(int32_t AuthCode, int32_t priority, const char *category, const char *content)
{
	CQ_addLog(AuthCode, priority, category, content);
}



