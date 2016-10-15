#include "StdAfx.h"
#include "SkinMove.h"

#include <fstream>
#include <map>
#include <commonlib/graphicplus.h>
#include <commonlib/systemutils.h>
#include <commonlib/stringutils.h>
#include <crypto/crypto.h>

#pragma warning(disable:4996)
//�ͻ��˳������е�ͼƬid
typedef enum ImageID
{
	IMGID_FIRST_ = 0,
	IMGID_UPLEFT_LOGO = 1,//���н������Ͻ�logo
	IMGID_LOGON_HEADBGROUND,//��½����ͷ�񱳾� 
	IMGID_MINBTN,	//��С��
	IMGID_MAXBTN,  //���
	IMGID_CLOSEBTN,//�رհ�ť
	IMGID_LOGONBTN,//��½��ť
	IMGID_REGISTER,//ע��
	IMGID_CLEARLOGONUSER,//������½��
	IMGID_OPTIONBOX,//��ѡ���ѡ��ͼ��
	IMGID_DROPDOWNBOX_BUTTON,//�������������ť-����״̬
	IMGID_LOGONDLG_BACKGROUND,//���н���ı���ͼ-�ɱ��С
	IMGID_RESTOREBTN,//�ָ���ť
	IMGID_MAIN_ONLINESTATUS_BTN,//�������������״̬��ť
	IMGID_MAIN_HEADBGOUND,//������ͷ�񱳾�
	IMGID_MAIN_SYSMENU,//������ϵͳ�˵�
	IMGID_MAIN_SEARCHBTN,//�������ѯ��ť
	IMGID_MAIN_ADDFRIEND,//��������Ӻ��ѻ�ȺͼƬ
	IMGID_MAIN_TABIT,//������tabҳ-���ѵ���ϵ��
	IMGID_MAIN_TABHOME,//������tabҳ-��ҳ��ת��ť
	IMGID_MAIN_TABMANAGE,//������tabҳ-��������ϵͳ���á�Ȧ�ӵ�
	IMGID_MAIN_TABMSN,//������tabҳ-MSN��أ��������ò���
	IMGID_SCROLLUP,//���������ϰ�ť
	IMGID_SCROLLBTN,//��������ק��ť
	IMGID_SCROLLDOWN,//���������°�ť
	IMGID_TREE_EXPAND,//����ͼ-����չ����ʾͼ
	IMGID_TREE_REDUCE,//����ͼ-����δչ����ʾͼ
	IMGID_SLIDEBUTTON,//��ҳ��ť
	IMGID_MAIN_MIDTABBTN,//�����棬�м���ϵ�˵ȵ�tab��ť
	IMGID_COMPANY_HOMEURL,//��˾��ҳurl
	IMGID_CUSTOM_EXPERIENCE,//�ͻ�����ͼ��
	IMGID_MAIN_SEARCHBGROUND,//����������������
	IMGID_STATUS_ONLINE,//����״̬ͼ�꣺����
	IMGID_STATUS_HIDDEN,//����
	IMGID_STATUS_AWAY,//�뿪
	IMGID_STATUS_LOGOUT,//ע��
	IMGID_FILE_TRANSFER,//�����ļ�
	IMGID_CHAT_INVITATION,//��������
	IMGID_SENDING_MAIL,//�����ʼ�
	IMGID_CHAT_HISTORY,//�����¼
	IMGID_MESSAGE_SETTING,//��Ϣ����
	IMGID_GROUP_MANAGEMENT,//Ⱥ����
	IMGID_EXIT_CHAT,//�˳�����
	IMGID_DISMISS_CHAT,//��ɢ����
	IMGID_SHIELD_MESSAGE,//������Ϣ
	IMGID_MSGDISPLAY_BGROUND,//��Ϣ��ʾ���򱳾�
	IMGID_MSGINPUT_BGROUND,//��Ϣ�������򱳾�
	IMGID_SIGNATURE_BGROUND,//����ǩ������
	IMGID_SIGNATURE_UPARROW,//����ǩ�������Ϲ�����ͷ
	IMGID_SIGNATURE_DOWNARROW,//����ǩ�������¹�����ͷ
	IMGID_PERSONALINFO_COPHAT,//��Ϣ��ʾ����������Ϣ���ֵ�ñ��ͼ��
	IMGID_PERSONALINFO_HEAD,//��Ϣ��ʾ��������ͷ��Ĭ��ͼ��
	IMGID_CHAT_CUSTOMTEST_BGROUND,//���죺�ͻ����鱳��
	IMGID_CHAT_CUSTOMTEXT_CLOSE,//���죺�ͻ�����closeͼ
	IMGID_CHAT_TOOLBTN_FACE,//���죺����
	IMGID_CHAT_TOOLBTN_FONT,//���죺����
	IMGID_CHAT_TOOLBTN_CUTSCREEN,//���죺����
	IMGID_CHAT_TOOLBTN_QUICKANS,//���죺��ݻظ�
	IMGID_CHAT_TOOLBTN_CALC,//���죺������
	IMGID_CHAT_TOOLBTN_NOTEBOOK,//���죺���±�
	IMGID_CHAT_TOOLBTN_DOWNARROW,//���죺�Ҳ�������ͷ
	IMGID_CHAT_SENDINGBTN,//���죺���Ͱ�ť
	IMGID_CHAT_TABBGROUND,//���죺�Ҳ�tabҳ����
	IMGID_CHAT_TABBUTTON,//���죺�Ҳ�tabҳ��ť
	IMGID_CHAT_PERSONALINFO,//����:�Ҳ�tabҳ�ڵ绰��
	IMGID_GROUP_RIGHTBGROUND,//Ⱥ����,�Ҳ౳��
	IMGID_GROUP_MANAGER,//Ⱥ��ͼ��
	IMGID_TEMPCHAT_RIGHTBGROUND,//��ʱ�����Ҳ౳��
	IMGID_MAIN_MIDTABBKGND,//�������м�tabҳ����
	IMGID_MAIN_BOTTAB_LEFTBTN,//������ײ�tab���tab��ť
	IMGID_MAIN_BOTTAB_RIGHTBTN,//������ײ�tab�ұ�tab��ť
	IMGID_MAIN_BOTTAB_MIDBTN,//������ײ�tab�м�tab��ť
	IMGID_MAIN_BOTTAB_BKGND,//������ײ�tab����
	IMGID_HISTORY_EXPORT,//����
	IMGID_HISTORY_CLEARALL,//���
	IMGID_HISTORY_REFRESH,//����
	IMGID_HISTORY_DELETE,//ɾ��
	IMGID_HISTORY_SEARCH,//������ť
	IMGID_HISTORY_CLOCK,//���Ϸ�labelͼ��
	IMGID_HISTORY_CHATNODE,//��-������ڵ�
	IMGID_HISTORY_DISCUSSNODE,//��-������ڵ�
	IMGID_HISTORY_LEAFNODE,//��-Ҷ�ӽڵ�
	IMGID_HISTORY_BROADCAST,//��-ϵͳ��Ϣͼ��
	IMGID_HISTORY_TABPLANE,//tabƽ��ģʽ
	IMGID_HISTORY_TABTRADITION,//tab��ͳģʽ
	IMGID_HISTORY_GOTO,//ת����ť
	IMGID_HISTORY_FRISTPAGE,//��ҳ
	IMGID_HISTORY_PREPAGE,//��һҳ
	IMGID_HISTORY_NEXTPAGE,//��һҳ
	IMGID_HISTORY_LASTPAGE,//ĩҳ
	IMGID_MSGBOX_BKGND,//msgbox����
	IMGID_MSGBOX_SUCCESS,//�������ͼ�꣨�Ժţ�
	IMGID_MSGBOX_INFORMATION,//��ʾͼ�꣨̾�ţ�
	IMGID_MSGBOX_QUESTION,//����ͼ�꣨�ʺţ�
	IMGID_MSGBOX_ERROR,//����ͼ��(X��)
	IMGID_MSGBOX_OK,
	IMGID_MSGBOX_CANCEL,
	IMGID_SYSCFG_TABPERSONAL,//tab-��������
	IMGID_SYSCFG_TABNORMAL,//tab-��������
	IMGID_SYSCFG_TABAUTHEN,//tab-�����֤
	IMGID_SYSCFG_TABREPLY,//tab-�ظ�����
	IMGID_SYSCFG_TABSOUND,//tab-��������
	IMGID_SYSCFG_TABFILETF,//tab-�ļ�����
	IMGID_SYSCFG_TABHOTKEY,//tab-�ȼ�����
	IMGID_SYSCFG_TABPWD,//tab-��������
	IMGID_SYSCFG_HEADBKGND,//��������-ͷ�񱳾�
	IMGID_SYSCFG_BTNCHANGEHEAD,//��������-����ͷ��ť
	IMGID_SYSCFG_BTNAPPLY,//Ӧ�ð�ť
	IMGID_SYSCFG_BTNRADIO,//ϵͳ����-radiobutton
	IMGID_SYSCFG_BTNCHECK,//ϵͳ����-checkbutton
	IMGID_SYSCFG_BTNMODIFY,//ϵͳ����-�޸İ�ť
	IMGID_SYSCFG_BTNADD,//ϵͳ����-��Ӱ�ť
	IMGID_SYSCFG_BTNDELETE,//ϵͳ����-ɾ����ť
	IMGID_SYSCFG_BTNPLAYMUSIC,//ϵͳ����-��������
	IMGID_SYSCFG_BTNBROWSE,//ϵͳ����-���
	IMGID_SYSCFG_BTNDEIRECTORY,//ϵͳ����-����Ŀ¼
	IMGID_SYSCFG_BTNOK,//ѡ��ͷ��-ȷ��
	IMGID_SYSCFG_BTNCANCEL,//ѡ��ͷ��-ȡ��
	IMGID_SYSCFG_BTNBROWSEBIG,//��ʡ�Ժŵ������ť
	IMGID_SYSCFG_IMGSEPARATORLINE,//ϵͳ����-�ָ���
	IMGID_SCROLL_BTNLEFT,//������-����
	IMGID_SCROLL_BTNDRAG,//������-�м�
	IMGID_SCROLL_BTNRIGHT,//������-����
	IMGID_SEARCH_ADIMAGE,//����-���ͼƬ
	IMGID_SEARCH_TABFRIEND,//����-��������
	IMGID_SEARCH_TABGROUP,//����-����Ⱥ
	IMGID_SEARCH_BTNRADIO,//����-radio
	IMGID_SEARCH_BTNPRIOR,//����-��һ��
	IMGID_SEARCH_BTNNEXT,//����-��һ��
	IMGID_SEARCH_BTNADD,//����-���
	IMGID_SEARCH_BTNSEARCH,//����-����
	IMGID_SEARCH_BTNEXIT,//����-�˳�
	IMGID_SEARCH_LOGO,//����-�Ŵ�
	IMGID_SEARCH_LOOKUP,//����-�鿴����
	IMGID_SEARCH_ARROWLEFT,//����-��ҳ
	IMGID_SEARCH_ARROWRIGHT,//����-�ҷ�ҳ
	IMGID_ABOUTDLG_BKGND,//aboutdlg-����
	IMGID_BROADCAST_BTNXCLOSE,//�㲥-x�رհ�ť
	IMGID_BROADCAST_BTNSEND,//�㲥-���Ͱ�ť
	IMGID_BROADCAST_BTNCLOSE,//�㲥-�ر�
	IMGID_BROADCAST_BTNADD,//�㲥-��ӽ�����
	IMGID_BROADCAST_BTNNEXTPAGE,//�㲥-��һҳ
	IMGID_BROADCAST_BTNPRIORPAGE,//�㲥-ǰһҳ
	IMGID_BROADCAST_BTNTALK,//�㲥-�뷢���˽�̸��ť
	IMGID_GENERATECODES_TABBTNHTML,//������ҳ-tabhtml
	IMGID_GENERATECODES_TABBTNDIZ,//������ҳ-tabdiz
	IMGID_GENERATECODES_BTNPLAYSOUND,//������ҳ-��������
	IMGID_GENERATECODES_BTNCOPYCODE,//������ҳ-���ƴ���
	IMGID_SELUSERS_BTNDELETE,//ѡ�������û�-ɾ����ť
	IMGID_SELUSERS_BTNADD,//ѡ�������û�-��Ӱ�ť
	IMGID_BKGND_UNSIZEABLE,//���ڱ���-���ɱ��С
	IMGID_SCREENTRUNCATION_BKGND,//�������ڱ���
	IMGID_MAIN_HIDDENSUBIMG,//������-������ͼ��
	IMGID_MAIN_AWAYSUBIMG,//������-�뿪��ͼ��
	IMGID_CHAT_SEND_DROPDOWNBTN,//���죺���Ͱ�ť�Ե�������ť
	IMGID_MENU_CHECK,//�˵���ѡ��״̬
	IMGID_NETTEST_TEST,//������ԣ����԰�ť
	IMGID_UPDATE_SEARCHAVAIL,//����-�����ø���
	IMGID_UPDATE_FILEDETAILED,//����-�ļ���ϸ
	IMGID_UPDATE_DOWNLOADINSTALL,//����-���ذ�װ
	IMGID_UPDATE_INSTALL,//����-��װ
	IMGID_UPDATE_SUCCESS,//����-�ɹ�
	IMGID_UPDATE_FAILED,//����-ʧ��
	IMGID_UPDATE_FILE_SUCCESS,//����-�ļ�-�ɹ�
	IMGID_UPDATE_FILE_FAILED,//����-�ļ�-ʧ��
	IMGID_UPDATE_FILE_ARROW,//����-�ļ�-��ͷ
	IMGID_UPDATE_FILE_WARNING,//����-�ļ�-����
	IMGID_UPDATE_FILE_ERROR,//����-�ļ�-����
	IMGID_UPDATE_FILE_EXCLAMATION,//����-�ļ�-̾��
	IMGID_SELUSERS_GROUP,//ѡ���û�-��ڵ�
	IMGID_SELUSERS_UPLEVEL,//ѡ���û�-��һ��
	IMGID_HISTORY_IMPORT,//�����¼-����
	IMGID_IMPORT_IMPORT,//����Ի���-����
	IMGID_LAST_,
}IMAGEID;

//ͼƬ�б��ʽ
typedef enum ImageListFmt
{
	ILF_FIRST_ = 0,
	ILF_VERTICAL = 1,
	ILF_HORIZONTAL,
	ILF_LAST_,
	ILF_INVALID_
}IMAGELISTFMT;

//ͼƬ�б�͸ɫ
typedef enum
{
	TRANSCOLOR__FIRST = 0,
	TRANSCOLOR_PURPLE,//�Ϻ�
	TRANSCOLOR_BLUE,//��ɫ
	TRANSCOLOR_GREY,
	TRANSCOLOR__LAST,
	TRANSCOLOR__INVALID,
}TRASPARENT_COLOR;

//
#define   XML_FILENAME   "config.xml"
#define   IMAGE_PATH     "images"

//Ƥ�����ݿ��еı�����
const char SKIN_TABLE_NAME[] = "skin";	  //Ƥ�����ñ�
const char IMAGE_TABLE_NAME[] = "images";   //Ƥ�����õ���ͼƬ�б�
//skin���������
const char SKIN_COL_ID[] = "ID";
const char SKIN_COL_NAME[] = "name";
const char SKIN_COL_FORMAT[] = "format";
const char SKIN_COL_DATA[] = "data";
const char SKIN_COL_DATALEN[] = "datalen";//ѹ��֮ǰ�Ĵ�С
//images���������
const char IMAGES_COL_ID[] = "ID";
const char IMAGES_COL_IMAGEID[] = "imageid";
const char IMAGES_COL_TYPE[] = "imagetype";
const char IMAGES_COL_DESCRIPTION[] = "imagedescription";
const char IMAGES_COL_SIZE[] = "imagesize";
const char IMAGES_COL_DATA[] = "imagedata";
const char IMAGES_COL_ILFMT[] = "imagelistfmt";
const char IMAGES_COL_TRANSPCOLOR[] = "transpcolor";
const char IMAGES_COL_SUBIMGCOUNT[] = "subimgcount";
//�������sql���
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

//ͼƬ��չ��
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
//ͼƬ��Ϣ
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
			//���ص�ʱ������Ѿ���Ƥ�����ݣ�ɾ��֮
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
				if(( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) == 0 ) //��Ŀ¼
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
							//�������ݿ�
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
						//��ʼ������
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
			//��ѹ��
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