#pragma once

#include <string>

struct CQ_TYPE_QQ
{
	int64_t							QQID;	//QQºÅ
	std::string						nick;	//êÇ³Æ
	int								sex;	//ÐÔ±ð
	int								age;	//ÄêÁä
};

struct CQ_Type_GroupMember
{
	int64_t				GroupID;					// ÈººÅ
	int64_t			   	QQID;						// QQºÅ
	std::string			username;					// QQêÇ³Æ
	std::string			nick;						// ÈºÃûÆ¬
	int					sex;						// ÐÔ±ð 0/ÄÐ 1/Å®
	int					age;						// ÄêÁä
	std::string         area;						// µØÇø
	std::string			jointime;					// ÈëÈºÊ±¼ä
	int				lastsent;					// ÉÏ´Î·¢ÑÔÊ±¼ä
	std::string			level_name;					// Í·ÏÎÃû×Ö
	int				permission;					// È¨ÏÞµÈ¼¶ 1/³ÉÔ± 2/¹ÜÀíÔ± 3/ÈºÖ÷
	bool				unfriendly;					// ²»Á¼³ÉÔ±¼ÇÂ¼
	std::string			title;						// ×Ô¶¨ÒåÍ·ÏÎ
	int				titleExpiretime;			// Í·ÏÎ¹ýÆÚÊ±¼ä
	bool				nickcanchange;				// ¹ÜÀíÔ±ÊÇ·ñÄÜÐ­Öú¸ÄÃû
};
class MemberInfoProccessor
{
public:
	MemberInfoProccessor();
	~MemberInfoProccessor();

	bool GetGroupMemberInfo(int auth, int64_t groupID, int64_t qqid, CQ_Type_GroupMember& groupMember);
	void AddLog(int32_t AuthCode, int32_t priority, const char *category, const char *content);
private:
	bool MemberInfoProccessor_TextToGroupMember(std::string decodeStr, CQ_Type_GroupMember& groupMember);
	int64_t MemberInfoProccessor_GetNum(int index, long length, const std::string &decodeStr);
	std::string MemberInfoProccessor_GetStr(int index, const std::string &decodeStr);
	int64_t From256to10(std::string str256);

	int authCode;
};

