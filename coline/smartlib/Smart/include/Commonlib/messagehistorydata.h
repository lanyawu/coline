#ifndef __MESSAGEHISTORYDATA_H__
#define __MESSAGEHISTORYDATA_H__

#include <vector>
#include <Commonlib/SqliteDBOP.h>

//历史记录分页，每页记录数
#define MESSAGE_COUNT_PER_PAGE 10  //每页多少条记录

#define USERS_GROUP_NAME    "Groups"  //分组表
#define USERS_TABLE_NAME    "Users"   //用户表
#define HISTORY_TABLE_NAME  "History" //历史记录表

#define CREATE_GROUP_TABLE_SQL  "create table Groups(ID INTEGER PRIMARY KEY, GroupId INTEGER, GroupType INTEGER, ParentId INTEGER, GroupName VARCHAR(64));"
#define CREATE_USER_TABLE_SQL    "create table Users(ID INTEGER PRIMARY KEY, UserId INTEGER, GroupId INTEGER, UserType INTEGER, UserName VARCHAR(64), NickName VARCHAR(64));"
#define CREATE_HISTORY_TABLE_SQL "create table History(ID INTEGER PRIMARY KEY, FromUserId INTEGER, ToUserId INTEGER, MsgType INTEGER, Msg VARCHAR(2048));"

typedef struct CLocalGroupInfoItem
{
	int GroupId;
	int GroupType;
	int ParentId;
	char szGroupName[64];
}LOCAL_GROUP_INFO_ITEM, *LPLOCAL_GROUP_INFO_ITEM;

typedef struct CLocalUserInfoItem
{
	int UserId;          //用户Id
	int UserType;        //用户类型
	int GroupId;         //分组ID
	char UserName[64];   //用户名称
	char NickName[64];   //用户昵称
}LOCAL_USER_INFO_ITEM, *LPLOCAL_USER_INFO_ITEM;

typedef struct CHistoryMessageItem
{
	int MsgId;      //消息ID 删除用的唯一标识码
	int FromUserId; //发送方用户ID
	int ToUserId;   //接收方用户ID
	int MsgType;    //消息类别
	int MsgLen;     //消息内容长度
	char *szMsg;    //消息内容
}HISTORY_MESSAGE_ITEM, *LPHISTORY_MESSAGE_ITEM;

using namespace std;
typedef vector<LPLOCAL_GROUP_INFO_ITEM> LOCAL_GROUP_INFO_LIST; //分组列表
typedef vector<LPLOCAL_USER_INFO_ITEM> LOCAL_USER_INFO_LIST; //本地用户信息列表
typedef vector<LPHISTORY_MESSAGE_ITEM> HISTORY_MESSAGE_ITEM_LIST; //消息列表

class CMessageHistoryData
{
public:
	CMessageHistoryData(char *szUserFileName, char *szKey);
	~CMessageHistoryData(void);
private:
	void InitHistoryData();
public:
	//外部方法调用
	//分组相关操作
	void AddGroupInfo(int GroupId, int GroupType, int ParentId, char *szGroupName);
    BOOL GetGroupInfo(int GroupId, int &GroupType, int &ParentId, char *szGroupName);
	void DeleteGroupInfo(int GroupId);
	BOOL GetGroupList(LOCAL_GROUP_INFO_LIST &GroupList); //获取分组列表
 
	//增加一个好友信息
    void AddFriendUser(int UserId, int UserType, char *szUserName, char *szNickName);
	//获取好友信息
	BOOL GetFriendUserInfo(int UserId, int &UserType, char *szUserName, char *szNickName);
    //删除一个好友信息
	void DeleteFriendUser(int UserId);
	//获取好友列表
	BOOL GetFriendUserList(int UserType, LOCAL_USER_INFO_LIST &UserList);

	//增加一个聊天记录
	void AddHistoryMsg(int iFromUserId, int iToUserId, int MsgType, char *szMsg);
    //删除一个历史聊天记录
	void DeleteHistoryMsg(int MsgId);
	//获取历史记录列表 
	BOOL GetHistoryMsgList(int iCurrMsgId, int iFromUserId, int iToUserId,  
		     int &TotalCount, HISTORY_MESSAGE_ITEM_LIST &MsgList, bool bNextPage = true);
	//检索聊天记录
	BOOL QueryHistoryMsgList(int iCurrMsgId, int &TotalCount, char *szKey, HISTORY_MESSAGE_ITEM_LIST &MsgList, bool bNextPage = true);

private:
	CSqliteDBOP m_SqliteDBOP;

};

#endif