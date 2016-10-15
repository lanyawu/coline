#ifndef ___CORE_COMMON_H______
#define ___CORE_COMMON_H______

#include <windows.h>
#include <string>
//PATH
#define PATH_LOCAL_CUSTOM_PICTURE   1   //�Զ���ͼƬĿ¼
#define PATH_LOCAL_PERSON           2   //����Ŀ¼
#define PATH_LOCAL_USER_HEAD        3   //����ͷ��Ŀ¼
#define PATH_LOCAL_RECV_PATH        4   //���˽����ļ�Ŀ¼
#define PATH_LOCAL_CACHE_PATH       5   //����Ŀ¼
#define PATH_LOCAL_SKIN             6   //Ƥ��Ŀ¼
#define PATH_CUSTOM_EMOTION         7   //�Զ������Ŀ¼

//http Server const 
#define HTTP_SVR_URL_CUSTOM_PICTURE 1  //�Զ����ϴ�URL
#define HTTP_SVR_URL_OFFLINE_FILE   2  //�����ļ��ϴ�URL
#define HTTP_SVR_URL_HTTP           3  //�ļ�������
#define HTTP_SVR_URL_USER_HEAD      4  //�Զ���ͷ���ϴ�URL
#define HTTP_SVR_URL_FAX            5  //�����ַ
#define HTTP_SVR_MAIL_URL           6  //�ʼ���������ַ

#define PRINT_RUN_TIME_INTERVAL   //�Ƿ��ӡ����ʱ��
//��Ϣ
#define WM_IM_BASE          WM_USER + 0x200
#define WM_DOWNORGCOMPLETE  (WM_IM_BASE + 0x01)  //��֯�ɽṹ�������
#define WM_ERRORMSG         (WM_IM_BASE + 0x02)  //������Ϣ����
#define WM_OPENCHATFRAME    (WM_IM_BASE + 0x03)  //�����촰��
#define WM_SHOWCHATMESSAGE  (WM_IM_BASE + 0x04)  //��ʾ������Ϣ
#define WM_SHOWCHATTIPMSG   (WM_IM_BASE + 0x05)  //��ʾ���촰����ʾ��Ϣ
#define WM_VIDEO_CONNECTED  (WM_IM_BASE + 0x06)  //��Ƶ����
#define WM_SHOWTRAYTIPINFO  (WM_IM_BASE + 0x07)  //��ʾ������Ϣ
#define WM_OPENGROUPFRAME   (WM_IM_BASE + 0x08)  //�������鴰��
#define WM_SHOWGROUPTIPMSG  (WM_IM_BASE + 0x09)  //��Group��������ʾ��ʾ��Ϣ
#define WM_DRAWGROUPTOUI    (WM_IM_BASE + 0x0a)  //���������鵽UI
#define WM_SHOWGROUPMSG     (WM_IM_BASE + 0x0b)  //��ʾ��������Ϣ
#define WM_ORGDL_PROGRESS   (WM_IM_BASE + 0x0c)  //�ϴ����ؽ���
#define WM_CHAT_UPDL_PROGR  (WM_IM_BASE + 0x0d)  //���������ϴ����ؽ���
#define WM_GRP_UPDL_PROGR   (WM_IM_BASE + 0x0e)  //�����������ϴ����ؽ���
#define WM_CHAT_RMFILE_PRO  (WM_IM_BASE + 0x0f)  //�Ƴ��ļ����������
#define WM_GRP_RM_FILE_PRO  (WM_IM_BASE + 0x10)  //�ƶ��������ڵ��ļ����������
#define WM_GRP_EXIT_GROUP   (WM_IM_BASE + 0x11)  //�˳�����Э��
#define WM_CHAT_APPEND_PRO  (WM_IM_BASE + 0x12)  // �����촰���ϼ����ļ����ͽ�����
#define WM_CHAT_REPLACE_PIC (WM_IM_BASE + 0x13) //�滻�Զ������
#define WM_GRP_APPEND_PRO   (WM_IM_BASE + 0x14)  //�������鴰���ϼ����ļ����ͽ�����
#define WM_BCRM_FILEPRO     (WM_IM_BASE + 0x15)  //�ڹ㲥������ɾ���ļ��������
#define WM_BCSW_FILEPRO     (WM_IM_BASE + 0x16)  //�ڹ㲥��������ʾ�ļ��������
#define WM_OATIP_SHOWPANEL  (WM_IM_BASE + 0x17)  //OATip��ʾ
#define WM_USER_DL_HEADER   (WM_IM_BASE + 0x18)  //�û�ͷ���������
#define WM_CHAT_COMMAND     (WM_IM_BASE + 0x19)  //���촰�ڵ�ͬ�������
#define WM_APP_TERMINATE    (WM_IM_BASE + 0x1a)  //�ر�����Ӧ�ó���
#define WM_CONTACTS_DL_COMP (WM_IM_BASE + 0x1b)  //��ϵ���������
#define WM_CONTACTS_SVR_ACK (WM_IM_BASE + 0x1c)  //������ϵ�˷�������������
#define WM_SHOW_FILE_LINK   (WM_IM_BASE + 0x1d)  //��ʾ�ļ�����
#define WM_GRP_FILE_LINK    (WM_IM_BASE + 0x1e)  //��ʾ��������ļ�����
#define WM_SHOW_DETAIL_INFO (WM_IM_BASE + 0x1f)  //��ʾ��ϸ����
#define WM_PRESENCE_CHANGE  (WM_IM_BASE + 0x20)  //״̬�ı�֪ͨ
#define WM_SHOWHOMEPAGE     (WM_IM_BASE + 0x21)  //������ҳ
#define WM_FREPRESENCECHG   (WM_IM_BASE + 0x22)  //������ϵ��״̬�ı�
#define WM_FRECNT_DETAIL    (WM_IM_BASE + 0x23)  //��ʾ������ϵ��
#define WM_GRP_ADD_USER     (WM_IM_BASE + 0x24)  //���������������һ���û�
#define WM_GRP_DELETE       (WM_IM_BASE + 0x25)  //��ɢһ��������
#define WM_BANNER_DL_COMPL  (WM_IM_BASE + 0x26)  //������������ 
#define WM_RM_FILEPROGRESS  (WM_IM_BASE + 0x27)  //ȥ���ļ����������    
#define WM_SHOWTIPPANEL     (WM_IM_BASE + 0x28)  //��ʾ�򵥵�TIP
#define WM_RECEIVESMS       (WM_IM_BASE + 0x29)  //���յ��¶���
#define WM_PRESENCECHANGE   (WM_IM_BASE + 0x2a)  //�޸Ľ����״̬
#define WM_PRESENCECHG_ORD  (WM_IM_BASE + 0x2b)  //�޸�״̬������
#define WM_SIGNCHANGE       (WM_IM_BASE + 0x2c)  //ǩ���ı�

//������Ͷ���
#define PLUGIN_TYPE_UIMANAGER  1  //���ڹ�����
#define PLUGIN_TYPE_LOGINFRAME 2  //

#define PLUGIN_TYPE_MAINFRAME  3  //��������
#define PLUGIN_TYPE_CONTACTS   4  //��ϵ�˲��
#define PLUGIN_TYPE_MSGMGR     5  //��Ϣ������
#define PLUGIN_TYPE_CHATFRAME  6  //������
#define PLUGIN_TYPE_GROUPFRAME 7  //��������
#define PLUGIN_TYPE_CONFIGURE  8  //���ò��
#define PLUGIN_TYPE_TRAYMSG    9  //��������Ϣ֪ͨ���
#define PLUGIN_TYPE_PROTOCOL   10 //��Ϣ������ 
#define PLUGIN_TYPE_EXTERNAL   11 //������չ���

 
#define MAX_PLUGIN_TYPE_ID  PLUGIN_TYPE_EXTERNAL

static char PLUGIN_TYPE_NAMES[12][16] = {"unknown", "uimanager", "loginframe", "mainframe", "contacts", 
                                        "msgmgr", "chatframe", "groupframe", "configure", "traymsg", 
										"protocol", "external"};
static char PLUGIN_REGISTER_DIR[] = "SOFTWARE\\GoCom\\Plugins";

 
//����ؼ��ڵ����ƶ���
#define  UI_MAIN_WINDOW_NAME          "MainWindow"
#define  GLOBAL_CONFIG_FILE_NAME       "config.xml"

#define  DEFAULT_CONFIGURE_FILE_NAME  "coline.cfg"
#define  APPLICATION_PATH_NAME        "coline\\"

#define CFE_BOLD          0x0001
#define CFE_ITALIC        0x0002
#define CFE_UNDERLINE     0x0004
#define CFE_STRIKEOUT     0x0008
#define CFE_PROTECTED     0x0010
#define CFE_LINK          0x0020
#define CFE_AUTOCOLOR     0x40000000 

#define UI_NICK_NAME_COLOR      0xFF0000 //#FF00FF
#define UI_NICK_NAME_COLOR_PEER 0x0000FF

#define CHAT_FRAME_OFFSET_X  20
#define CHAT_FRAME_OFFSET_Y  20

//
//16λ ���0xFFFF
#define CUSTOM_LINK_FLAG_OFFSET     16    //���ӱ�־λ��

#define CUSTOM_LINK_FLAG_RECV       0x01  //�ͻ�������Flag ����
#define CUSTOM_LINK_FLAG_SAVEAS     0x02  //�ͻ�������Flag ���Ϊ
#define CUSTOM_LINK_FLAG_CANCEL     0x04  //�ͻ�������Flag ȡ��
#define CUSTOM_LINK_FLAG_REFUSE     0x08  //�ͻ�������Flag �ܾ�
#define CUSTOM_LINK_FLAG_OFFLINE    0x10  //�ͻ�������Flag �����ļ�
#define CUSTOM_LINK_FLAG_RMC_ACCEPT 0x20  //Զ�̿��ƽ���
#define CUSTOM_LINK_FLAG_RMC_REFUSE 0x40  //Զ�̿��ƾܾ�
#define CUSTOM_LINK_FLAG_RMC_CTRL   0x80  //�ܿ�
#define CUSTOM_LINK_FLAG_V_ACCEPT   0x100  //��Ƶ����-����
#define CUSTOM_LINK_FLAG_V_REFUSE   0x200 //��Ƶ����-�ܾ�

//���ô�����Ϣ��
#define CORE_ERROR_SOCKET_CLOSED  -1 //���类�ر�
#define CORE_ERROR_KICKOUT        -2 //�û���������
#define CORE_ERROR_LOGOUT         -3 //����ע��
//
#define FREQUENCY_CONTACT_TYPE_ID  1

//����Ȩ�޶���
#define USER_ROLE_SEND_MESSAGE    1  //������Ϣ
#define USER_ROLE_SEND_FILE       2  //�����ļ�
#define USER_ROLE_SEND_SMS        4  //���Ͷ���
#define USER_ROLE_GROUP           8  //������
#define GROUP_ROLE_HIDE           16 //�����Ƿ�����

#pragma warning(disable:4996)

__inline int GetSubIdxByPresence(const char *szPresence)
{
	if (szPresence)
	{
		if (stricmp(szPresence, "online") == 0)
			return 0;
		else if (stricmp(szPresence, "away") == 0)
			return 1;
		else if (stricmp(szPresence, "busy") == 0)
			return 2;
		else if (stricmp(szPresence, "appearoffline") == 0)
			return 3; 
		else if (stricmp(szPresence, "offline") == 0)
			return 4;
	}
	return 0;
}

__inline int GetMenuIdByPresence(const char *szPresence)
{
	if (szPresence)
	{
		if (stricmp(szPresence, "online") == 0)
			return 1;
		else if (stricmp(szPresence, "away") == 0)
			return 2;
		else if (stricmp(szPresence, "busy") == 0)
			return 3;
		else if (stricmp(szPresence, "appearoffline") == 0)
			return 4;
		else if (stricmp(szPresence, "offline") == 0)
			return 5;
	}
    return 0;
}

bool __inline SepNameDomainByUserName(const char *szUserName, std::string &strName, std::string &strDomain)
{
	if (szUserName)
	{
		strName = szUserName;
		int nPos = strName.find('@');
		if (nPos != std::string::npos)
		{
			strDomain = strName.substr(nPos + 1);
			strName = strName.substr(0, nPos);
			return true;
		} 
	}
	return false;
}

#endif

#pragma warning(default:4996)
