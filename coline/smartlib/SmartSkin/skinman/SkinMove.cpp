#include "StdAfx.h"
#include "SkinMove.h"

#include <fstream>
#include <map>
#include <commonlib/graphicplus.h>
#include <commonlib/systemutils.h>
#include <commonlib/stringutils.h>
#include <crypto/crypto.h>

#pragma warning(disable:4996)
//客户端程序所有的图片id
typedef enum ImageID
{
	IMGID_FIRST_ = 0,
	IMGID_UPLEFT_LOGO = 1,//所有界面左上角logo
	IMGID_LOGON_HEADBGROUND,//登陆界面头像背景 
	IMGID_MINBTN,	//最小化
	IMGID_MAXBTN,  //最大化
	IMGID_CLOSEBTN,//关闭按钮
	IMGID_LOGONBTN,//登陆按钮
	IMGID_REGISTER,//注册
	IMGID_CLEARLOGONUSER,//消除登陆名
	IMGID_OPTIONBOX,//复选框的选中图标
	IMGID_DROPDOWNBOX_BUTTON,//下拉框的下拉按钮-正常状态
	IMGID_LOGONDLG_BACKGROUND,//所有界面的背景图-可变大小
	IMGID_RESTOREBTN,//恢复按钮
	IMGID_MAIN_ONLINESTATUS_BTN,//主界面更该在线状态按钮
	IMGID_MAIN_HEADBGOUND,//主界面头像背景
	IMGID_MAIN_SYSMENU,//主界面系统菜单
	IMGID_MAIN_SEARCHBTN,//主界面查询按钮
	IMGID_MAIN_ADDFRIEND,//主界面添加好友或群图片
	IMGID_MAIN_TABIT,//主界面tab页-好友等联系人
	IMGID_MAIN_TABHOME,//主界面tab页-主页调转按钮
	IMGID_MAIN_TABMANAGE,//主界面tab页-管理，包括系统配置、圈子等
	IMGID_MAIN_TABMSN,//主界面tab页-MSN相关，具体作用不明
	IMGID_SCROLLUP,//滚动条向上按钮
	IMGID_SCROLLBTN,//滚动条拖拽按钮
	IMGID_SCROLLDOWN,//滚动条向下按钮
	IMGID_TREE_EXPAND,//树视图-子树展开提示图
	IMGID_TREE_REDUCE,//树视图-子树未展开提示图
	IMGID_SLIDEBUTTON,//滑页按钮
	IMGID_MAIN_MIDTABBTN,//主界面，中间联系人等的tab按钮
	IMGID_COMPANY_HOMEURL,//公司主页url
	IMGID_CUSTOM_EXPERIENCE,//客户体验图标
	IMGID_MAIN_SEARCHBGROUND,//主界面搜索栏背景
	IMGID_STATUS_ONLINE,//在线状态图标：在线
	IMGID_STATUS_HIDDEN,//隐身
	IMGID_STATUS_AWAY,//离开
	IMGID_STATUS_LOGOUT,//注销
	IMGID_FILE_TRANSFER,//传送文件
	IMGID_CHAT_INVITATION,//邀请讨论
	IMGID_SENDING_MAIL,//发送邮件
	IMGID_CHAT_HISTORY,//聊天记录
	IMGID_MESSAGE_SETTING,//消息设置
	IMGID_GROUP_MANAGEMENT,//群管理
	IMGID_EXIT_CHAT,//退出讨论
	IMGID_DISMISS_CHAT,//解散讨论
	IMGID_SHIELD_MESSAGE,//屏蔽消息
	IMGID_MSGDISPLAY_BGROUND,//消息显示区域背景
	IMGID_MSGINPUT_BGROUND,//消息输入区域背景
	IMGID_SIGNATURE_BGROUND,//个性签名背景
	IMGID_SIGNATURE_UPARROW,//个性签名的向上滚动箭头
	IMGID_SIGNATURE_DOWNARROW,//个性签名的向下滚动箭头
	IMGID_PERSONALINFO_COPHAT,//消息显示区：个人信息部分的帽子图标
	IMGID_PERSONALINFO_HEAD,//消息显示区：个人头像默认图标
	IMGID_CHAT_CUSTOMTEST_BGROUND,//聊天：客户体验背景
	IMGID_CHAT_CUSTOMTEXT_CLOSE,//聊天：客户体验close图
	IMGID_CHAT_TOOLBTN_FACE,//聊天：表情
	IMGID_CHAT_TOOLBTN_FONT,//聊天：字体
	IMGID_CHAT_TOOLBTN_CUTSCREEN,//聊天：截屏
	IMGID_CHAT_TOOLBTN_QUICKANS,//聊天：快捷回复
	IMGID_CHAT_TOOLBTN_CALC,//聊天：计算器
	IMGID_CHAT_TOOLBTN_NOTEBOOK,//聊天：记事本
	IMGID_CHAT_TOOLBTN_DOWNARROW,//聊天：右侧下拉箭头
	IMGID_CHAT_SENDINGBTN,//聊天：发送按钮
	IMGID_CHAT_TABBGROUND,//聊天：右侧tab页背景
	IMGID_CHAT_TABBUTTON,//聊天：右侧tab页按钮
	IMGID_CHAT_PERSONALINFO,//聊天:右侧tab页内电话等
	IMGID_GROUP_RIGHTBGROUND,//群界面,右侧背景
	IMGID_GROUP_MANAGER,//群主图标
	IMGID_TEMPCHAT_RIGHTBGROUND,//临时讨论右侧背景
	IMGID_MAIN_MIDTABBKGND,//主界面中间tab页背景
	IMGID_MAIN_BOTTAB_LEFTBTN,//主界面底部tab左边tab按钮
	IMGID_MAIN_BOTTAB_RIGHTBTN,//主界面底部tab右边tab按钮
	IMGID_MAIN_BOTTAB_MIDBTN,//主界面底部tab中间tab按钮
	IMGID_MAIN_BOTTAB_BKGND,//主界面底部tab背景
	IMGID_HISTORY_EXPORT,//导出
	IMGID_HISTORY_CLEARALL,//清除
	IMGID_HISTORY_REFRESH,//更新
	IMGID_HISTORY_DELETE,//删除
	IMGID_HISTORY_SEARCH,//搜索按钮
	IMGID_HISTORY_CLOCK,//树上方label图标
	IMGID_HISTORY_CHATNODE,//树-聊天组节点
	IMGID_HISTORY_DISCUSSNODE,//树-讨论组节点
	IMGID_HISTORY_LEAFNODE,//树-叶子节点
	IMGID_HISTORY_BROADCAST,//树-系统消息图标
	IMGID_HISTORY_TABPLANE,//tab平板模式
	IMGID_HISTORY_TABTRADITION,//tab传统模式
	IMGID_HISTORY_GOTO,//转到按钮
	IMGID_HISTORY_FRISTPAGE,//首页
	IMGID_HISTORY_PREPAGE,//上一页
	IMGID_HISTORY_NEXTPAGE,//下一页
	IMGID_HISTORY_LASTPAGE,//末页
	IMGID_MSGBOX_BKGND,//msgbox背景
	IMGID_MSGBOX_SUCCESS,//操作完成图标（对号）
	IMGID_MSGBOX_INFORMATION,//提示图标（叹号）
	IMGID_MSGBOX_QUESTION,//问题图标（问号）
	IMGID_MSGBOX_ERROR,//错误图标(X号)
	IMGID_MSGBOX_OK,
	IMGID_MSGBOX_CANCEL,
	IMGID_SYSCFG_TABPERSONAL,//tab-个人设置
	IMGID_SYSCFG_TABNORMAL,//tab-常规设置
	IMGID_SYSCFG_TABAUTHEN,//tab-身份认证
	IMGID_SYSCFG_TABREPLY,//tab-回复设置
	IMGID_SYSCFG_TABSOUND,//tab-声音设置
	IMGID_SYSCFG_TABFILETF,//tab-文件传输
	IMGID_SYSCFG_TABHOTKEY,//tab-热键设置
	IMGID_SYSCFG_TABPWD,//tab-密码设置
	IMGID_SYSCFG_HEADBKGND,//个人配置-头像背景
	IMGID_SYSCFG_BTNCHANGEHEAD,//个人配置-更改头像按钮
	IMGID_SYSCFG_BTNAPPLY,//应用按钮
	IMGID_SYSCFG_BTNRADIO,//系统配置-radiobutton
	IMGID_SYSCFG_BTNCHECK,//系统配置-checkbutton
	IMGID_SYSCFG_BTNMODIFY,//系统配置-修改按钮
	IMGID_SYSCFG_BTNADD,//系统配置-添加按钮
	IMGID_SYSCFG_BTNDELETE,//系统配置-删除按钮
	IMGID_SYSCFG_BTNPLAYMUSIC,//系统配置-播放生音
	IMGID_SYSCFG_BTNBROWSE,//系统配置-浏览
	IMGID_SYSCFG_BTNDEIRECTORY,//系统配置-管理目录
	IMGID_SYSCFG_BTNOK,//选择头像-确定
	IMGID_SYSCFG_BTNCANCEL,//选择头像-取消
	IMGID_SYSCFG_BTNBROWSEBIG,//带省略号的浏览按钮
	IMGID_SYSCFG_IMGSEPARATORLINE,//系统配置-分割线
	IMGID_SCROLL_BTNLEFT,//滚动条-左移
	IMGID_SCROLL_BTNDRAG,//滚动条-中间
	IMGID_SCROLL_BTNRIGHT,//滚动条-右移
	IMGID_SEARCH_ADIMAGE,//搜索-广告图片
	IMGID_SEARCH_TABFRIEND,//搜索-搜索好友
	IMGID_SEARCH_TABGROUP,//搜索-搜索群
	IMGID_SEARCH_BTNRADIO,//搜索-radio
	IMGID_SEARCH_BTNPRIOR,//搜索-上一步
	IMGID_SEARCH_BTNNEXT,//搜索-下一步
	IMGID_SEARCH_BTNADD,//搜索-添加
	IMGID_SEARCH_BTNSEARCH,//搜索-查找
	IMGID_SEARCH_BTNEXIT,//搜索-退出
	IMGID_SEARCH_LOGO,//搜索-放大镜
	IMGID_SEARCH_LOOKUP,//搜索-查看资料
	IMGID_SEARCH_ARROWLEFT,//搜索-左翻页
	IMGID_SEARCH_ARROWRIGHT,//搜索-右翻页
	IMGID_ABOUTDLG_BKGND,//aboutdlg-背景
	IMGID_BROADCAST_BTNXCLOSE,//广播-x关闭按钮
	IMGID_BROADCAST_BTNSEND,//广播-发送按钮
	IMGID_BROADCAST_BTNCLOSE,//广播-关闭
	IMGID_BROADCAST_BTNADD,//广播-添加接收人
	IMGID_BROADCAST_BTNNEXTPAGE,//广播-后一页
	IMGID_BROADCAST_BTNPRIORPAGE,//广播-前一页
	IMGID_BROADCAST_BTNTALK,//广播-与发送人交谈按钮
	IMGID_GENERATECODES_TABBTNHTML,//产生网页-tabhtml
	IMGID_GENERATECODES_TABBTNDIZ,//产生网页-tabdiz
	IMGID_GENERATECODES_BTNPLAYSOUND,//产生网页-播放声音
	IMGID_GENERATECODES_BTNCOPYCODE,//产生网页-复制代码
	IMGID_SELUSERS_BTNDELETE,//选择讨论用户-删除按钮
	IMGID_SELUSERS_BTNADD,//选择讨论用户-添加按钮
	IMGID_BKGND_UNSIZEABLE,//窗口背景-不可变大小
	IMGID_SCREENTRUNCATION_BKGND,//截屏窗口背景
	IMGID_MAIN_HIDDENSUBIMG,//主界面-隐身子图标
	IMGID_MAIN_AWAYSUBIMG,//主界面-离开子图标
	IMGID_CHAT_SEND_DROPDOWNBTN,//聊天：发送按钮旁的下拉按钮
	IMGID_MENU_CHECK,//菜单：选中状态
	IMGID_NETTEST_TEST,//网络测试：测试按钮
	IMGID_UPDATE_SEARCHAVAIL,//更新-检查可用更新
	IMGID_UPDATE_FILEDETAILED,//更新-文件明细
	IMGID_UPDATE_DOWNLOADINSTALL,//更新-下载安装
	IMGID_UPDATE_INSTALL,//更新-安装
	IMGID_UPDATE_SUCCESS,//更新-成功
	IMGID_UPDATE_FAILED,//更新-失败
	IMGID_UPDATE_FILE_SUCCESS,//更新-文件-成功
	IMGID_UPDATE_FILE_FAILED,//更新-文件-失败
	IMGID_UPDATE_FILE_ARROW,//更新-文件-箭头
	IMGID_UPDATE_FILE_WARNING,//更新-文件-警告
	IMGID_UPDATE_FILE_ERROR,//更新-文件-错误
	IMGID_UPDATE_FILE_EXCLAMATION,//更新-文件-叹号
	IMGID_SELUSERS_GROUP,//选择用户-组节点
	IMGID_SELUSERS_UPLEVEL,//选择用户-上一级
	IMGID_HISTORY_IMPORT,//聊天记录-导入
	IMGID_IMPORT_IMPORT,//导入对话框-导入
	IMGID_LAST_,
}IMAGEID;

//图片列表格式
typedef enum ImageListFmt
{
	ILF_FIRST_ = 0,
	ILF_VERTICAL = 1,
	ILF_HORIZONTAL,
	ILF_LAST_,
	ILF_INVALID_
}IMAGELISTFMT;

//图片列表透色
typedef enum
{
	TRANSCOLOR__FIRST = 0,
	TRANSCOLOR_PURPLE,//紫红
	TRANSCOLOR_BLUE,//蓝色
	TRANSCOLOR_GREY,
	TRANSCOLOR__LAST,
	TRANSCOLOR__INVALID,
}TRASPARENT_COLOR;

//
#define   XML_FILENAME   "config.xml"
#define   IMAGE_PATH     "images"

//皮肤数据库中的表名称
const char SKIN_TABLE_NAME[] = "skin";	  //皮肤配置表
const char IMAGE_TABLE_NAME[] = "images";   //皮肤中用到的图片列表
//skin表的列名称
const char SKIN_COL_ID[] = "ID";
const char SKIN_COL_NAME[] = "name";
const char SKIN_COL_FORMAT[] = "format";
const char SKIN_COL_DATA[] = "data";
const char SKIN_COL_DATALEN[] = "datalen";//压缩之前的大小
//images表的列名称
const char IMAGES_COL_ID[] = "ID";
const char IMAGES_COL_IMAGEID[] = "imageid";
const char IMAGES_COL_TYPE[] = "imagetype";
const char IMAGES_COL_DESCRIPTION[] = "imagedescription";
const char IMAGES_COL_SIZE[] = "imagesize";
const char IMAGES_COL_DATA[] = "imagedata";
const char IMAGES_COL_ILFMT[] = "imagelistfmt";
const char IMAGES_COL_TRANSPCOLOR[] = "transpcolor";
const char IMAGES_COL_SUBIMGCOUNT[] = "subimgcount";
//创建表的sql语句
const char CREATE_SKIN_TABLE_SQL[] = 
{
	"CREATE TABLE skin(ID INTEGER PRIMARY KEY,"
	"name VARCHAR(32), format INTEGER, data BLOB,"
	"datalen INTEGER);"
};
const char CREATE_IMAGE_TABLE_SQL[] = 
{
	"CREATE TABLE images(ID INTEGER PRIMARY KEY AUTOINCREMENT,"
	"imageid INTEGER UNIQUE, imagetype INTEGER," 
	"imagedescription VARCHAR(255), imagesize INTEGER, "
	"imagedata BLOB, imagelistfmt INTEGER,"
	"transpcolor INTEGER, subimgcount INTEGER);"
};

//图片扩展名
const char IMAGE_FILE_EXT[][GRAHPIC_TYPE_UNKNOWN_] = {"","bmp", "gif", "jpg", "png", "ico",
	                                                  "tif", "tga", "pcx", "wbmp", "wmf",
	                                                  "jp2", "jpc", "pgx", "pnm", "ras", 
	                                                  "jbg", "mng", "ska", "raw", "", ""};
//
typedef struct CSkinImageParam
{
	int nImageType;
	int nTransColor;
	int nSubCount;
    int nVert;
	int nSrcLen;
}SKIN_IMAGE_PARAM;
//图片信息
typedef struct CSkinImageInfo {
	char szFileFlag[32];
	IMAGELISTFMT  nListFormat;
	int  nCount;
}SKIN_IMAGE_INFO;

const SKIN_IMAGE_INFO SKIN_IMAGE_LIST[IMGID_LAST_] = {
{"0", ILF_FIRST_, 0}, {"corp_ico", ILF_VERTICAL, 1}, {"login_headerbkg", ILF_VERTICAL, 1}, {"minbtn", ILF_VERTICAL, 4},
{"maxbtn", ILF_VERTICAL, 4}, {"closebtn", ILF_VERTICAL, 4}, {"login_logonbtn", ILF_VERTICAL, 4}, {"login_register", ILF_VERTICAL, 4},
{"login_clearlogonuser", ILF_VERTICAL, 4}, {"login_optionbox", ILF_VERTICAL, 4}, {"login_dropdownboxbtn", ILF_VERTICAL, 4}, {"login_logondlgbkg", ILF_FIRST_, 1},
{"login_restorebtn", ILF_VERTICAL, 4}, {"login_onlinestatusbtn", ILF_VERTICAL, 4}, {"main_headerbkg", ILF_VERTICAL, 4}, {"systemmenu", ILF_VERTICAL, 4},
{"main_searchbtn", ILF_VERTICAL, 4}, {"main_addfriendbtn", ILF_VERTICAL, 4}, {"main_tabit", ILF_VERTICAL, 4}, {"main_tabhome", ILF_VERTICAL, 4}, {"main_tabmanager", ILF_VERTICAL, 4},
{"main_tabmsn", ILF_VERTICAL, 4}, {"scrollup", ILF_VERTICAL, 4}, {"scrollbtn", ILF_VERTICAL, 4}, {"scrolldown", ILF_VERTICAL, 4}, {"main_treeexpand", ILF_VERTICAL, 2},
{"main_treereduce", ILF_VERTICAL, 2}, {"main_slidebtn", ILF_VERTICAL, 4}, {"main_midtabbtn", ILF_VERTICAL, 4}, {"main_homeurl", ILF_VERTICAL, 4}, {"main_customexp", ILF_VERTICAL, 1},
{"main_searchbkg", ILF_VERTICAL, 1}, {"main_statusonline", ILF_VERTICAL, 1}, {"main_statushidden", ILF_VERTICAL, 1}, {"main_statusaway", ILF_VERTICAL, 1}, {"main_statusLogout", ILF_VERTICAL, 1},
{"chat_transfer", ILF_VERTICAL, 4}, {"chat_invitation", ILF_VERTICAL, 4}, {"chat_email", ILF_VERTICAL, 4}, {"chat_history", ILF_VERTICAL, 4}, 
{"chat_setting", ILF_VERTICAL, 4}, {"group_manager", ILF_VERTICAL, 4}, {"group_exit", ILF_VERTICAL, 4}, {"group_dismiss", ILF_VERTICAL, 4},
{"chat_shield_msg", ILF_VERTICAL, 4}, {"chat_disp_bkg", ILF_VERTICAL, 1}, {"chat_input_bkg", ILF_VERTICAL, 1}, {"main_signature_bkg", ILF_VERTICAL, 1},
{"signature_up_btn", ILF_VERTICAL, 4}, {"signature_down_btn", ILF_VERTICAL, 4}, {"chat_cophat", ILF_VERTICAL, 4}, {"chat_person_header", ILF_VERTICAL, 4},
{"chat_custom_test_bkg", ILF_VERTICAL, 1}, {"chat_custom_test_close", ILF_VERTICAL, 4}, {"chat_toolbar_face", ILF_VERTICAL, 4}, {"chat_toolbar_font", ILF_VERTICAL, 4},
{"chat_toolbar_cutscreen", ILF_VERTICAL, 4}, {"chat_toolbar_quick", ILF_VERTICAL, 4}, {"chat_toolbar_calc", ILF_VERTICAL, 4}, {"chat_toolbar_notebook", ILF_VERTICAL, 4},
{"chat_toolbar_down", ILF_VERTICAL, 4}, {"chat_send_btn", ILF_VERTICAL, 4}, {"chat_tab_bkg", ILF_VERTICAL, 1}, {"chat_tab_btn", ILF_VERTICAL, 4},
{"chat_person_info", ILF_VERTICAL, 5},  {"group_right_bkg", ILF_VERTICAL, 1}, {"group_owner_ico", ILF_VERTICAL, 1}, {"group_temp_bkg", ILF_VERTICAL, 1},
{"main_midtab_bkg", ILF_VERTICAL, 1}, {"main_bottomtab_left", ILF_VERTICAL, 4}, {"main_bottomtab_right", ILF_VERTICAL, 4}, {"main_bottomtab_mid", ILF_VERTICAL, 4},
{"main_bottomtab_bkg", ILF_VERTICAL, 1}, {"history_export", ILF_VERTICAL, 4}, {"history_clearall", ILF_VERTICAL, 4}, {"history_refresh", ILF_VERTICAL, 4},
{"history_delete", ILF_VERTICAL, 4}, {"history_search", ILF_VERTICAL, 4}, {"history_label", ILF_VERTICAL, 1}, {"history_chatNode", ILF_VERTICAL, 1},
{"history_discuss", ILF_VERTICAL, 1}, {"history_leaf", ILF_VERTICAL, 1}, {"history_broadcast", ILF_VERTICAL, 4}, {"history_tabplane", ILF_VERTICAL, 4},
{"history_addtion", ILF_VERTICAL, 4}, {"history_goto", ILF_VERTICAL, 4}, {"history_firstpage", ILF_VERTICAL, 4}, {"history_prepage", ILF_VERTICAL, 4},
{"history_nextpage", ILF_VERTICAL, 4}, {"history_lastpage", ILF_VERTICAL, 4}, {"msgbox_bkg", ILF_VERTICAL, 1}, {"msgbox_succ", ILF_VERTICAL, 1},
{"msgbox_infomation", ILF_VERTICAL, 1}, {"msgbox_question", ILF_VERTICAL, 1}, {"msgbox_error", ILF_VERTICAL, 1}, {"msgbox_ok", ILF_VERTICAL, 4},
{"msgbox_cancel", ILF_VERTICAL, 4}, {"syscfg_tab_person", ILF_VERTICAL, 4}, {"syscfg_tab_normal", ILF_VERTICAL, 4}, {"syscfg_tab_authen", ILF_VERTICAL, 4},
{"syscfg_tab_reply", ILF_VERTICAL, 4}, {"syscfg_tab_sound", ILF_VERTICAL, 4}, {"syscfg_tab_filetrans", ILF_VERTICAL, 4}, {"syscfg_tab_hotkey", ILF_VERTICAL, 4},
{"syscfg_tab_pwd", ILF_VERTICAL, 4}, {"syscfg_headerbkg", ILF_VERTICAL, 1}, {"syscfg_changeheader", ILF_VERTICAL, 4}, {"syscfg_apply", ILF_VERTICAL, 4},
{"syscfg_radio", ILF_VERTICAL, 4}, {"syscfg_check", ILF_VERTICAL, 4}, {"syscfg_modify", ILF_VERTICAL, 4}, {"syscfg_add", ILF_VERTICAL, 4}, {"syscfg_delete", ILF_VERTICAL, 4},
{"syscfg_play", ILF_VERTICAL, 4}, {"syscfg_browser", ILF_VERTICAL, 4}, {"syscfg_directory", ILF_VERTICAL, 4}, {"syscfg_ok", ILF_VERTICAL, 4}, {"syscfg_cancel", ILF_VERTICAL, 4},
{"syscfg_more", ILF_VERTICAL, 4}, {"syscfg_separator", ILF_VERTICAL, 1}, {"scroll_left", ILF_VERTICAL, 4}, {"scroll_drag", ILF_VERTICAL, 4}, {"scroll_right", ILF_VERTICAL, 4},
{"search_ad", ILF_VERTICAL, 1}, {"search_tab_friend", ILF_VERTICAL, 4}, {"search_tab_group", ILF_VERTICAL, 4}, {"search_tab_radio", ILF_VERTICAL, 4},
{"search_pre", ILF_VERTICAL, 4}, {"search_next", ILF_VERTICAL, 4}, {"search_add", ILF_VERTICAL, 4}, {"search_search", ILF_VERTICAL, 4}, {"search_exit", ILF_VERTICAL, 4},
{"search_logo", ILF_VERTICAL, 1}, {"search_detail", ILF_VERTICAL, 4}, {"search_arrowleft", ILF_VERTICAL, 4}, {"search_arrowright", ILF_VERTICAL, 4},
{"aboutdlg_bkg", ILF_VERTICAL, 1}, {"broadcast_xclose", ILF_VERTICAL, 4}, {"broadcast_send", ILF_VERTICAL, 4}, {"broadcast_close", ILF_VERTICAL, 4},
{"broadcast_add", ILF_VERTICAL, 4}, {"broadcast_nextpage", ILF_VERTICAL, 4}, {"broadcast_prepage", ILF_VERTICAL, 4}, {"broadcast_talk", ILF_VERTICAL, 4},
{"gen_tab_html", ILF_VERTICAL, 4}, {"gen_tab_diz", ILF_VERTICAL, 4}, {"gen_play", ILF_VERTICAL, 4}, {"gen_copy", ILF_VERTICAL, 4}, {"seluser_del", ILF_VERTICAL, 4},
{"seluser_add", ILF_VERTICAL, 4}, {"dlg_bkg", ILF_VERTICAL, 1}, {"cutscreen_bkg", ILF_VERTICAL, 1}, {"main_hidden", ILF_VERTICAL, 1}, {"main_away", ILF_VERTICAL, 1},
{"chat_send_dropdown", ILF_VERTICAL, 4}, {"menu_check", ILF_VERTICAL, 2}, {"nettest_test", ILF_VERTICAL, 4}, {"update_search", ILF_VERTICAL, 4}, 
{"update_detail", ILF_VERTICAL, 4}, {"update_down", ILF_VERTICAL, 4}, {"update_install", ILF_VERTICAL, 4}, {"update_succ", ILF_VERTICAL, 4},
{"update_failed", ILF_VERTICAL, 4}, {"update_file_succ", ILF_VERTICAL, 4}, {"update_file_failed", ILF_VERTICAL, 4}, {"update_file_arrow", ILF_VERTICAL, 4},
{"update_file_warning", ILF_VERTICAL, 4}, {"update_file_error", ILF_VERTICAL, 4}, {"update_file_excl", ILF_VERTICAL, 4}, {"seluser_group", ILF_VERTICAL, 4},
{"selusers_uplevel", ILF_VERTICAL, 4}, {"history_import", ILF_VERTICAL, 4}, {"importdlg_import", ILF_VERTICAL, 4}
};
 
 
int GetImageInfoByName(const char *szName, SKIN_IMAGE_INFO &Info)
{
	for (int i = 0; i < IMGID_LAST_; i ++)
	{
		if (strcmp(szName, SKIN_IMAGE_LIST[i].szFileFlag) == 0)
		{
			Info = SKIN_IMAGE_LIST[i];
			return i;
		}
	}
	return IMGID_FIRST_;
}

int GetImageTypeByExt(const char *szExt)
{
	for (int i = 0; i < GRAHPIC_TYPE_UNKNOWN_; i ++)
	{
		if (strcmp(szExt, IMAGE_FILE_EXT[i]) == 0)
		{
			return i;
		}
	}
	return 0;
}

CSkinMove::CSkinMove(const char *szSkinFileName, const char *szKey):
           m_DBOP(szSkinFileName, szKey)
{
	if( !m_DBOP.TableIsExists( SKIN_TABLE_NAME))
		m_DBOP.Execute(CREATE_SKIN_TABLE_SQL );
	if (!m_DBOP.TableIsExists(IMAGE_TABLE_NAME))
		m_DBOP.Execute(CREATE_IMAGE_TABLE_SQL);
}

CSkinMove::~CSkinMove(void)
{
	//
}

BOOL CSkinMove::ImportFromPath(const char *szPath)
{
	if (!CSystemUtils::DirectoryIsExists(szPath))
		return FALSE;
	//xml
	char szFileName[MAX_PATH] = {0};
	char szIncludeDeliPath[MAX_PATH] = {0};
	CSystemUtils::IncludePathDelimiter(szPath, szIncludeDeliPath, MAX_PATH);
	sprintf(szFileName, "%s%s", szIncludeDeliPath,XML_FILENAME);
	char *szStream = NULL;
	DWORD dwSize = 0;
	char szSql[512] = {0};
	BOOL bResult = TRUE;
	if (LoadFromFile(szFileName, &szStream, dwSize))
	{
		BYTE *pDest = new BYTE[dwSize];
		memset(pDest, 0, dwSize);
		UINT32 dwDestLen = dwSize;
		if (zlib_compress(&pDest, &dwDestLen, (BYTE *)szStream, dwSize))
		{
			//加载的时候，如果已经有皮肤数据，删除之
			sprintf( szSql, "DELETE FROM %s", SKIN_TABLE_NAME );
			m_DBOP.Execute( szSql );

			sprintf( szSql, "INSERT INTO %s(%s,%s,%s,%s) VALUES('%s',%d,?,%d);",
				SKIN_TABLE_NAME, 
				SKIN_COL_NAME, SKIN_COL_FORMAT, SKIN_COL_DATA, SKIN_COL_DATALEN,
				"config", 1/*UTF8*/,  dwSize);

			bResult = m_DBOP.InsertBlob(szSql, (const char*)pDest, dwDestLen);
		}
		delete []pDest;
	}
	if (szStream)
		delete []szStream;
	dwSize = 0;
	szStream = NULL;
	if (!bResult)
		return FALSE;
	//images
	WIN32_FIND_DATA fd = {0};
	TCHAR szwFileName[MAX_PATH] = {0};
	char szImagePath[MAX_PATH] = {0};
	sprintf(szImagePath, "%s%s\\", szIncludeDeliPath, IMAGE_PATH);
	sprintf(szFileName, "%s*.*",  szImagePath);
	CStringConversion::StringToWideChar(szFileName, szwFileName, MAX_PATH);
	HANDLE hFile = ::FindFirstFile(szwFileName, &fd);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		char szFlag[MAX_PATH] = {0};
		char szExt[MAX_PATH] = {0};
		char cFileName[MAX_PATH] = {0};
		int nImageId;
		int nImageType;
		SKIN_IMAGE_INFO Info = {0};
		do
		{
			memset(cFileName, 0, MAX_PATH);
			CStringConversion::WideCharToString(fd.cFileName, cFileName, MAX_PATH);
			if((::strcmp(cFileName, "." ) != 0) && (strcmp(cFileName, "..") != 0))
			{
				if(( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) == 0 ) //非目录
				{
					memset(szFlag, 0, MAX_PATH);
					memset(szExt, 0, MAX_PATH);
				
					CSystemUtils::ExtractFileExtName(cFileName, szExt, MAX_PATH);
					strncpy(szFlag, cFileName, ::strlen(cFileName) - ::strlen(szExt) - 1);

					printf(".");
					nImageId = GetImageInfoByName(szFlag, Info);
					if (nImageId > IMGID_FIRST_)
					{
						sprintf(szFileName, "%s%s", szImagePath, cFileName);
						if (LoadFromFile(szFileName, &szStream, dwSize))
						{
							nImageType = GetImageTypeByExt(szExt);
							//插入数据库
							sprintf(szSql ,"INSERT INTO %s(%s,%s,%s,%s,%s,%s,%s,%s) VALUES(%d,%d,'%s',%d,?,%d,%d,%d)", 
								IMAGE_TABLE_NAME, IMAGES_COL_IMAGEID, IMAGES_COL_TYPE, IMAGES_COL_DESCRIPTION, IMAGES_COL_SIZE, 
								IMAGES_COL_DATA, IMAGES_COL_ILFMT, IMAGES_COL_TRANSPCOLOR, IMAGES_COL_SUBIMGCOUNT,
								nImageId, nImageType, "", dwSize,
								Info.nListFormat, 1, Info.nCount);
							
							if (nImageType == GRAPHIC_TYPE_BMP)
							{
								BYTE *pZlib = new BYTE[dwSize];
								UINT32 uZlibSize = dwSize;
								if (zlib_compress(&pZlib, &uZlibSize, (BYTE *)szStream, dwSize))
								{
									m_DBOP.InsertBlob(szSql, (const char *)pZlib, uZlibSize);
								}
								delete []pZlib;
							} else
							{
								m_DBOP.InsertBlob(szSql, szStream, dwSize);
							} //end if (nImageType...
						} //end if (LoadFromFile...
						//初始化变量
						if (szStream)
							delete []szStream;
						szStream = NULL;
						dwSize = 0;
					} // end if (nImageId..
				}// end if (fd....
			} // end if (strcmp...
		} while (::FindNextFile(hFile, &fd));
		::FindClose(hFile);
	}
	return TRUE;
}

BOOL CSkinMove::LoadFromFile(const char *szFileName, char **szStream, DWORD &dwSize)
{
	if (!CSystemUtils::FileIsExists(szFileName))
		return FALSE;
	TCHAR szwFileName[MAX_PATH] = {0};
	CStringConversion::StringToWideChar(szFileName, szwFileName, MAX_PATH);
	std::ifstream ifs(szwFileName, std::ios::in | std::ios::binary);
	if (ifs.is_open())
	{
		ifs.seekg(0, std::ios::end);
		dwSize = ifs.tellg();
        *szStream = new char[dwSize + 1];
		ifs.seekg(0, std::ios::beg);
		ifs.read(*szStream, dwSize);
		ifs.close();
		return TRUE;
	}
	return FALSE;
}

BOOL CSkinMove::StreamToFile(const char *szFileName, const char *szStream, const DWORD dwSize)
{
	TCHAR szwFileName[MAX_PATH] = {0};
	CStringConversion::StringToWideChar(szFileName, szwFileName, MAX_PATH);
	std::ofstream ofs(szwFileName, std::ios::out | std::ios::binary);
	if (ofs.is_open())
	{
		ofs.write((char *)szStream, dwSize);
		ofs.close();
		return TRUE;
	}
	return FALSE;
}

BOOL CSkinMove::ExportToPath(const char *szPath)
{
	char szIncludeDeliPath[MAX_PATH] = {0};
	CSystemUtils::IncludePathDelimiter(szPath, szIncludeDeliPath, MAX_PATH);
	if (!CSystemUtils::ForceDirectories(szIncludeDeliPath))
		return FALSE;

	//xml
	char** result = 0;
	int row = 0;
	int col = 0;
	char sql[512] = { 0 };
	sprintf( sql, "SELECT %s,%s,%s,%s FROM %s",
		SKIN_COL_ID, SKIN_COL_NAME, SKIN_COL_FORMAT, SKIN_COL_DATALEN,
		SKIN_TABLE_NAME );
	BOOL bRet = m_DBOP.Open( sql, &result, row, col );
	if( !bRet )
		return FALSE;
	if( row <= 0 )
	{
		m_DBOP.Free_Result( result );
		return FALSE;
	}

	UINT32 uDestLen = atoi(result[7]);
	int iSkinId = atoi(result[4]);
	m_DBOP.Free_Result( result );

	//get stream data
	memset( sql, 0, sizeof(sql) );
	sprintf( sql, "SELECT %s FROM %s WHERE %s=%d", 
		SKIN_COL_DATA, SKIN_TABLE_NAME,
		SKIN_COL_ID, iSkinId );

	char *pSrc = NULL;
	UINT32 pSrcLen = 0;
	char szFileName[512] = {0};
	bRet = m_DBOP.GetBlob(sql, &pSrc, (DWORD &)pSrcLen, 0 );
	if( !bRet || pSrcLen<= 0 )
		return FALSE;

	BYTE *pDest = new BYTE[uDestLen + 1];
	if (zlib_uncompress(pDest, &uDestLen, (BYTE *)pSrc, pSrcLen))
	{
		pDest[uDestLen] = 0;
	    sprintf(szFileName, "%s%s", szIncludeDeliPath, XML_FILENAME);
		StreamToFile(szFileName, (const char *)pDest, uDestLen);
	}

	 
	delete [] pDest;
	m_DBOP.Free_Blob( &pSrc );

	//images
	char szImagePath[MAX_PATH] = {0};
	char szImagesFileName[MAX_PATH] = {0};
	sprintf(szImagesFileName, "%simage.xml", szIncludeDeliPath);
	sprintf(szImagePath, "%s%s\\", szIncludeDeliPath, IMAGE_PATH);
	CSystemUtils::ForceDirectories(szImagePath);
	TCHAR szwFileName[MAX_PATH] = {0};
	CStringConversion::StringToWideChar(szImagesFileName, szwFileName, MAX_PATH);
	ofstream ofs(szwFileName, std::ios::binary);

	sprintf(sql, "SELECT %s,%s,%s,%s,%s,%s,%s,%s FROM %s", 
		IMAGES_COL_ID, IMAGES_COL_IMAGEID,
		IMAGES_COL_TYPE, IMAGES_COL_DESCRIPTION,
		IMAGES_COL_SIZE, IMAGES_COL_ILFMT,
		IMAGES_COL_TRANSPCOLOR, IMAGES_COL_SUBIMGCOUNT,
		IMAGE_TABLE_NAME );
	bRet = m_DBOP.Open( sql, &result, row, col );
	if( !bRet )
		return FALSE;

	std::map<UINT, SKIN_IMAGE_PARAM> ImageList;
	//read images row by row
	SKIN_IMAGE_PARAM Info = {0};
	for( int iRow = 1; iRow <= row; ++iRow ) //GRAPHICTYPE ImageID
	{
		Info.nImageType = atoi( result[ iRow*col+2 ] );//validate!
		Info.nSrcLen = atoi(result[iRow * col + 4]);
		Info.nSubCount = atoi(result[iRow * col + 7]);
		Info.nVert = atoi(result[iRow * col + 5]);
		Info.nTransColor = atoi(result[iRow * col + 6]);
		UINT iid = atoi( result[iRow*col+1] );//validate!
		ImageList[iid] = Info;
	}//end for

	m_DBOP.Free_Result( result );
	char szXmlTemp[1024] = {0};
	//read blob image data
	std::map<UINT, SKIN_IMAGE_PARAM>::iterator it;
	for(it = ImageList.begin(); it != ImageList.end(); it ++ )
	{
		memset( sql, 0, sizeof(sql) );
		sprintf( sql, "SELECT %s FROM %s WHERE %s=%d",
			IMAGES_COL_DATA, IMAGE_TABLE_NAME,
			IMAGES_COL_IMAGEID, it->first );
		char * pImageData = NULL;
		DWORD dwDataLen = 0;
		bRet = m_DBOP.GetBlob(sql, &pImageData, dwDataLen, 0);
		printf(".");
		memset(szXmlTemp, 0, 1024);
		if (it->second.nVert != 0)
			sprintf(szXmlTemp, "<Image name=\"%d\" id=\"%d\"  subcount=\"%d\" layout=\"vertical\" transcolor=\"#%0.8X\" path=\".\\Image\\%s.%s\"/>\n",
			it->first, it->first, it->second.nSubCount,  it->second.nTransColor, SKIN_IMAGE_LIST[it->first].szFileFlag, IMAGE_FILE_EXT[it->second.nImageType]);
		else
			sprintf(szXmlTemp, "<Image name=\"%d\" id=\"%d\"  subcount=\"%d\" layout=\"horizontal\" transcolor=\"#%0.8X\" path=\".\\Image\\%s.%s\"/>\n",
			it->first, it->first, it->second.nSubCount,  it->second.nTransColor, SKIN_IMAGE_LIST[it->first].szFileFlag, IMAGE_FILE_EXT[it->second.nImageType]);
		ofs.write(szXmlTemp, strlen(szXmlTemp));
		if( bRet && (pImageData != NULL) && dwDataLen > 0 )
		{
			//解压缩
			if( it->second.nImageType == GRAPHIC_TYPE_BMP )
			{
				UINT32 dwDestLen = it->second.nSrcLen;
				BYTE *pDest = new BYTE[dwDestLen + 1];
				sprintf(szFileName, "%s%s.%s", szImagePath, SKIN_IMAGE_LIST[it->first].szFileFlag, IMAGE_FILE_EXT[it->second.nImageType]);
				if (zlib_uncompress( pDest, &dwDestLen, (BYTE *)pImageData, dwDataLen))
				{
					pDest[dwDestLen] = 0;					
					StreamToFile(szFileName, (const char *)pDest, dwDestLen);
				} else
				{
					sprintf(szFileName, "%s%s_X.%s", szImagePath, SKIN_IMAGE_LIST[it->first].szFileFlag, IMAGE_FILE_EXT[it->second.nImageType]);
					StreamToFile(szFileName, pImageData, dwDataLen);
				}
				delete [] pDest;
			}
			else
			{
				sprintf(szFileName, "%s%s.%s", szImagePath, SKIN_IMAGE_LIST[it->first].szFileFlag, IMAGE_FILE_EXT[it->second.nImageType]);
				StreamToFile(szFileName, pImageData, dwDataLen);
			}
		}
		//free blob data
		m_DBOP.Free_Blob( &pImageData );
	}
	ofs.close();
	return TRUE;
}