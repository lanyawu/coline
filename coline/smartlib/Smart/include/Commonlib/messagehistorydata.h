#ifndef __MESSAGEHISTORYDATA_H__
#define __MESSAGEHISTORYDATA_H__

#include <vector>
#include <Commonlib/SqliteDBOP.h>

//��ʷ��¼��ҳ��ÿҳ��¼��
#define MESSAGE_COUNT_PER_PAGE 10  //ÿҳ��������¼

#define USERS_GROUP_NAME    "Groups"  //�����
#define USERS_TABLE_NAME    "Users"   //�û���
#define HISTORY_TABLE_NAME  "History" //��ʷ��¼��

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
	int UserId;          //�û�Id
	int UserType;        //�û�����
	int GroupId;         //����ID
	char UserName[64];   //�û�����
	char NickName[64];   //�û��ǳ�
}LOCAL_USER_INFO_ITEM, *LPLOCAL_USER_INFO_ITEM;

typedef struct CHistoryMessageItem
{
	int MsgId;      //��ϢID ɾ���õ�Ψһ��ʶ��
	int FromUserId; //���ͷ��û�ID
	int ToUserId;   //���շ��û�ID
	int MsgType;    //��Ϣ���
	int MsgLen;     //��Ϣ���ݳ���
	char *szMsg;    //��Ϣ����
}HISTORY_MESSAGE_ITEM, *LPHISTORY_MESSAGE_ITEM;

using namespace std;
typedef vector<LPLOCAL_GROUP_INFO_ITEM> LOCAL_GROUP_INFO_LIST; //�����б�
typedef vector<LPLOCAL_USER_INFO_ITEM> LOCAL_USER_INFO_LIST; //�����û���Ϣ�б�
typedef vector<LPHISTORY_MESSAGE_ITEM> HISTORY_MESSAGE_ITEM_LIST; //��Ϣ�б�

class CMessageHistoryData
{
public:
	CMessageHistoryData(char *szUserFileName, char *szKey);
	~CMessageHistoryData(void);
private:
	void InitHistoryData();
public:
	//�ⲿ��������
	//������ز���
	void AddGroupInfo(int GroupId, int GroupType, int ParentId, char *szGroupName);
    BOOL GetGroupInfo(int GroupId, int &GroupType, int &ParentId, char *szGroupName);
	void DeleteGroupInfo(int GroupId);
	BOOL GetGroupList(LOCAL_GROUP_INFO_LIST &GroupList); //��ȡ�����б�
 
	//����һ��������Ϣ
    void AddFriendUser(int UserId, int UserType, char *szUserName, char *szNickName);
	//��ȡ������Ϣ
	BOOL GetFriendUserInfo(int UserId, int &UserType, char *szUserName, char *szNickName);
    //ɾ��һ��������Ϣ
	void DeleteFriendUser(int UserId);
	//��ȡ�����б�
	BOOL GetFriendUserList(int UserType, LOCAL_USER_INFO_LIST &UserList);

	//����һ�������¼
	void AddHistoryMsg(int iFromUserId, int iToUserId, int MsgType, char *szMsg);
    //ɾ��һ����ʷ�����¼
	void DeleteHistoryMsg(int MsgId);
	//��ȡ��ʷ��¼�б� 
	BOOL GetHistoryMsgList(int iCurrMsgId, int iFromUserId, int iToUserId,  
		     int &TotalCount, HISTORY_MESSAGE_ITEM_LIST &MsgList, bool bNextPage = true);
	//���������¼
	BOOL QueryHistoryMsgList(int iCurrMsgId, int &TotalCount, char *szKey, HISTORY_MESSAGE_ITEM_LIST &MsgList, bool bNextPage = true);

private:
	CSqliteDBOP m_SqliteDBOP;

};

#endif