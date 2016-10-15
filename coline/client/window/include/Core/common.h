#ifndef ___CORE_COMMON_H______
#define ___CORE_COMMON_H______

#include <windows.h>
#include <string>
//PATH
#define PATH_LOCAL_CUSTOM_PICTURE   1   //自定义图片目录
#define PATH_LOCAL_PERSON           2   //个人目录
#define PATH_LOCAL_USER_HEAD        3   //个人头像目录
#define PATH_LOCAL_RECV_PATH        4   //个人接收文件目录
#define PATH_LOCAL_CACHE_PATH       5   //缓存目录
#define PATH_LOCAL_SKIN             6   //皮肤目录
#define PATH_CUSTOM_EMOTION         7   //自定义表情目录

//http Server const 
#define HTTP_SVR_URL_CUSTOM_PICTURE 1  //自定义上传URL
#define HTTP_SVR_URL_OFFLINE_FILE   2  //离线文件上传URL
#define HTTP_SVR_URL_HTTP           3  //文件服务器
#define HTTP_SVR_URL_USER_HEAD      4  //自定义头像上传URL
#define HTTP_SVR_URL_FAX            5  //传真地址
#define HTTP_SVR_MAIL_URL           6  //邮件服务器地址

#define PRINT_RUN_TIME_INTERVAL   //是否打印运行时间
//消息
#define WM_IM_BASE          WM_USER + 0x200
#define WM_DOWNORGCOMPLETE  (WM_IM_BASE + 0x01)  //组织成结构下载完成
#define WM_ERRORMSG         (WM_IM_BASE + 0x02)  //错误消息处理
#define WM_OPENCHATFRAME    (WM_IM_BASE + 0x03)  //打开聊天窗口
#define WM_SHOWCHATMESSAGE  (WM_IM_BASE + 0x04)  //显示聊天消息
#define WM_SHOWCHATTIPMSG   (WM_IM_BASE + 0x05)  //显示聊天窗口提示消息
#define WM_VIDEO_CONNECTED  (WM_IM_BASE + 0x06)  //视频连接
#define WM_SHOWTRAYTIPINFO  (WM_IM_BASE + 0x07)  //显示托盘消息
#define WM_OPENGROUPFRAME   (WM_IM_BASE + 0x08)  //打开讨论组窗口
#define WM_SHOWGROUPTIPMSG  (WM_IM_BASE + 0x09)  //在Group窗口上显示提示信息
#define WM_DRAWGROUPTOUI    (WM_IM_BASE + 0x0a)  //绘制讨论组到UI
#define WM_SHOWGROUPMSG     (WM_IM_BASE + 0x0b)  //显示讨论组消息
#define WM_ORGDL_PROGRESS   (WM_IM_BASE + 0x0c)  //上传下载进度
#define WM_CHAT_UPDL_PROGR  (WM_IM_BASE + 0x0d)  //聊天界面的上传下载进度
#define WM_GRP_UPDL_PROGR   (WM_IM_BASE + 0x0e)  //讨论组界面的上传下载进度
#define WM_CHAT_RMFILE_PRO  (WM_IM_BASE + 0x0f)  //移除文件传输进度条
#define WM_GRP_RM_FILE_PRO  (WM_IM_BASE + 0x10)  //移动讨论组内的文件传输进度条
#define WM_GRP_EXIT_GROUP   (WM_IM_BASE + 0x11)  //退出分组协议
#define WM_CHAT_APPEND_PRO  (WM_IM_BASE + 0x12)  // 在聊天窗口上加上文件传送进度条
#define WM_CHAT_REPLACE_PIC (WM_IM_BASE + 0x13) //替换自定义表情
#define WM_GRP_APPEND_PRO   (WM_IM_BASE + 0x14)  //在讨论组窗口上加上文件传送进度条
#define WM_BCRM_FILEPRO     (WM_IM_BASE + 0x15)  //在广播窗口中删除文件传输进度
#define WM_BCSW_FILEPRO     (WM_IM_BASE + 0x16)  //在广播窗口中显示文件传输进度
#define WM_OATIP_SHOWPANEL  (WM_IM_BASE + 0x17)  //OATip显示
#define WM_USER_DL_HEADER   (WM_IM_BASE + 0x18)  //用户头像下载完成
#define WM_CHAT_COMMAND     (WM_IM_BASE + 0x19)  //聊天窗口的同步命令处理
#define WM_APP_TERMINATE    (WM_IM_BASE + 0x1a)  //关闭整个应用程序
#define WM_CONTACTS_DL_COMP (WM_IM_BASE + 0x1b)  //联系人下载完毕
#define WM_CONTACTS_SVR_ACK (WM_IM_BASE + 0x1c)  //常用联系人服务器操作返回
#define WM_SHOW_FILE_LINK   (WM_IM_BASE + 0x1d)  //显示文件链接
#define WM_GRP_FILE_LINK    (WM_IM_BASE + 0x1e)  //显示讨论组的文件链接
#define WM_SHOW_DETAIL_INFO (WM_IM_BASE + 0x1f)  //显示详细资料
#define WM_PRESENCE_CHANGE  (WM_IM_BASE + 0x20)  //状态改变通知
#define WM_SHOWHOMEPAGE     (WM_IM_BASE + 0x21)  //弹出首页
#define WM_FREPRESENCECHG   (WM_IM_BASE + 0x22)  //常用联系人状态改变
#define WM_FRECNT_DETAIL    (WM_IM_BASE + 0x23)  //显示常用联系人
#define WM_GRP_ADD_USER     (WM_IM_BASE + 0x24)  //讨论组界面内增加一个用户
#define WM_GRP_DELETE       (WM_IM_BASE + 0x25)  //解散一个讨论组
#define WM_BANNER_DL_COMPL  (WM_IM_BASE + 0x26)  //广告条下载完毕 
#define WM_RM_FILEPROGRESS  (WM_IM_BASE + 0x27)  //去除文件传输进度条    
#define WM_SHOWTIPPANEL     (WM_IM_BASE + 0x28)  //显示简单的TIP
#define WM_RECEIVESMS       (WM_IM_BASE + 0x29)  //接收到新短信
#define WM_PRESENCECHANGE   (WM_IM_BASE + 0x2a)  //修改界面的状态
#define WM_PRESENCECHG_ORD  (WM_IM_BASE + 0x2b)  //修改状态并排序
#define WM_SIGNCHANGE       (WM_IM_BASE + 0x2c)  //签名改变

//插件类型定义
#define PLUGIN_TYPE_UIMANAGER  1  //窗口管理插件
#define PLUGIN_TYPE_LOGINFRAME 2  //

#define PLUGIN_TYPE_MAINFRAME  3  //主界面插件
#define PLUGIN_TYPE_CONTACTS   4  //联系人插件
#define PLUGIN_TYPE_MSGMGR     5  //消息管理插件
#define PLUGIN_TYPE_CHATFRAME  6  //聊天插件
#define PLUGIN_TYPE_GROUPFRAME 7  //讨论组插件
#define PLUGIN_TYPE_CONFIGURE  8  //配置插件
#define PLUGIN_TYPE_TRAYMSG    9  //任务栏消息通知插件
#define PLUGIN_TYPE_PROTOCOL   10 //消息处理插件 
#define PLUGIN_TYPE_EXTERNAL   11 //其它扩展插件

 
#define MAX_PLUGIN_TYPE_ID  PLUGIN_TYPE_EXTERNAL

static char PLUGIN_TYPE_NAMES[12][16] = {"unknown", "uimanager", "loginframe", "mainframe", "contacts", 
                                        "msgmgr", "chatframe", "groupframe", "configure", "traymsg", 
										"protocol", "external"};
static char PLUGIN_REGISTER_DIR[] = "SOFTWARE\\GoCom\\Plugins";

 
//界面关键节点名称定义
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
//16位 最大0xFFFF
#define CUSTOM_LINK_FLAG_OFFSET     16    //链接标志位数

#define CUSTOM_LINK_FLAG_RECV       0x01  //客户区链接Flag 接收
#define CUSTOM_LINK_FLAG_SAVEAS     0x02  //客户区链接Flag 另存为
#define CUSTOM_LINK_FLAG_CANCEL     0x04  //客户区链接Flag 取消
#define CUSTOM_LINK_FLAG_REFUSE     0x08  //客户区链接Flag 拒绝
#define CUSTOM_LINK_FLAG_OFFLINE    0x10  //客户区链接Flag 离线文件
#define CUSTOM_LINK_FLAG_RMC_ACCEPT 0x20  //远程控制接受
#define CUSTOM_LINK_FLAG_RMC_REFUSE 0x40  //远程控制拒绝
#define CUSTOM_LINK_FLAG_RMC_CTRL   0x80  //受控
#define CUSTOM_LINK_FLAG_V_ACCEPT   0x100  //视频请求-接受
#define CUSTOM_LINK_FLAG_V_REFUSE   0x200 //视频请求-拒绝

//公用错误消息号
#define CORE_ERROR_SOCKET_CLOSED  -1 //网络被关闭
#define CORE_ERROR_KICKOUT        -2 //用户被踢下线
#define CORE_ERROR_LOGOUT         -3 //主动注销
//
#define FREQUENCY_CONTACT_TYPE_ID  1

//常用权限定义
#define USER_ROLE_SEND_MESSAGE    1  //发送消息
#define USER_ROLE_SEND_FILE       2  //发送文件
#define USER_ROLE_SEND_SMS        4  //发送短信
#define USER_ROLE_GROUP           8  //讨论组
#define GROUP_ROLE_HIDE           16 //部门是否隐藏

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
