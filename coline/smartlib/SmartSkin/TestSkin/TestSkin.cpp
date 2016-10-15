// TestSkin.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "TestSkin.h"
#include <objbase.h>
#include <stdio.h>
#include <string>
#include <SmartSkin/SmartSkin.h>
#include <Commonlib/systemutils.h>
#include <Commonlib/stringutils.h>

#include "../CutImage/CutImage.h"

#pragma warning(disable:4996)

typedef struct FontStyle
{
	int nFontSize;
	int nFontStyle;
	int cfColor;
	TCHAR szFaceName[32];
}CFontStyle;

//回调函数
BOOL CALLBACK TestEventWindow(HWND hWnd, LPCTSTR pstrEvent, LPCTSTR pstrWndName, LPCTSTR pstrControlName, POINT *ptMouse, 
					WPARAM wParam, LPARAM lParam, void *pOverlapped);

void FillRectFile(COLORREF clr, const char *szFileName)
{
	::SkinFillRectToFile(clr, szFileName);
}

class CTestWindow
{
public:
	CTestWindow(const char *szWinName, const char *szTitle, HWND hParent, DWORD dwStyle, DWORD dwExStyle, RECT *prc, 
		       BOOL bForceCreate)
	{
		m_hWnd = SkinCreateWindowByName(szWinName, NULL, hParent, dwStyle, dwExStyle, prc, 
			           bForceCreate, TestEventWindow, NULL, this);
	}
	~CTestWindow()
	{
		::SkinCloseWindow(m_hWnd);
	}

	void ShiftColor(int r, int g, int b)
	{
		::SkinBlendSkinStyle(r, g, b);
		//::SkinMixSkinBackGround("E:\\data\\CustomImage\\New Heart.jpg");
	}

	void ToDesktop()
	{
		HWND hDesktop = FindWindow(L"progman", NULL);
		if (hDesktop)
		{
			LONG lStyle = ::GetWindowLong(m_hWnd, GWL_STYLE);
			lStyle |= WS_CHILD; 
			::SetWindowLong(m_hWnd, GWL_STYLE, lStyle); 
			HWND hTmp = hDesktop;
			hTmp = ::GetWindow(hDesktop, GW_CHILD);
			
			//::MoveWindow(hTmp, 0, 0, 1280, 780, TRUE);
			
			::SetParent(m_hWnd, hTmp);
			hTmp = ::GetWindow(hTmp, GW_CHILD);
			//::SetWindowLong(m_hWnd, GWL_STYLE
		}
	}
	void Show()
	{
		::ShowWindow(m_hWnd, SW_SHOW);
	}
	void DockWindow()
	{
		::SkinSetDockDesktop(m_hWnd, TRUE, RGB(255, 255, 255), 127);
	}
	void SetMinSize()
	{
		::SkinSetWindowMinSize(m_hWnd, 300, 500);
	}
	void SetMaxSize()
	{
		::SkinSetWindowMaxSize(m_hWnd, 600, 600);
	}
	void  LoadTreeViewDefImage(const char *szFileName)
	{
		::SkinLoadTreeDefaultImage(m_hWnd, L"friendtree", szFileName);
	}

    void AddShortCut()
	{
		TCHAR szwTmp[MAX_PATH] = {0};
		::SkinAddAutoShortCut(m_hWnd, L"oftenusing", L"测试快捷方式",
			L"c:\\windows\\system32\\calc.exe", 0, L"测试快捷方式提示", szwTmp);
		::SkinAddAutoShortCut(m_hWnd, L"ccc", L"测试快捷方式",
			L"c:\\windows\\system32\\notepad.exe", 0, L"测试快捷方式提示", szwTmp);
	}
	void AddTab()
	{
		static const char WIDGET_TAB_UI_XML[] = "<Container xsi:type=\"ImageTabPage\" image=\"35\" text=\"bb\" border=\"false\" background=\"#FF00FF\"><Container xsi:type=\"VerticalLayout\" border=\"false\" background=\"#FF00FF\"><Container xsi:type=\"AutoShortCut\" ButtonPad=\"80 100\" name=\"ccc\" border=\"false\" background=\"#FF00FF\" scrollbar=\"vertical\"></Container></Container></Container>";
		static const char SIMPLE_XML[] = "<Container xsi:type=\"ImageTabPage\" image=\"35\" text=\"bb\" border=\"true\" background=\"#FF00FF\"><Container xsi:type=\"PaddingPanel\"/></Container>";
		::SkinAddChildControl(m_hWnd, L"widgettab", SIMPLE_XML, NULL, NULL, 0);
		::SkinUpdateControlUI(m_hWnd, L"widgettab");
	}
	void * AddTreeNode(void *pParent, TCHAR *szNodeName, CTreeNodeType ctType, TCHAR *szImage = NULL)
	{
		return ::SkinAddTreeChildNode(m_hWnd, L"colleaguetree",0,  pParent, szNodeName, ctType, NULL, L"Node Label", szImage, NULL);
	}
	void SetTabEnable()
	{
		::SkinSetControlAttr(m_hWnd, L"normalpage", L"enabled", L"false");
	}

	void SetStatusList()
	{
		::SkinSetDropdownItemString(m_hWnd, L"cb_logonstatus", -1, L"在线", NULL);
		::SkinSetDropdownItemString(m_hWnd, L"cb_logonstatus", -1, L"离开", NULL);
		::SkinSetDropdownItemString(m_hWnd, L"cb_logonstatus", -1, L"繁忙", NULL);
		::SkinSetDropdownItemString(m_hWnd, L"cb_logonstatus", -1, L"显示为脱机", NULL);
	}

    void SetLabelText()
	{
		::SkinSetControlTextByName(m_hWnd, L"createdate", L"2010-05-10");
		::SkinSetControlTextByName(m_hWnd, L"modidate", L"2010-05-11");
	}
	void AddMsg(const char *szTip, const char *szTipName, const char *szNumber)
	{
		static char MSG_LIST_XML[] = "<Container xsi:type=\"HorizontalLayout\" height=\"20\"><Control xsi:type=\"PaddingPanel\" width=\"10\" /><Control xsi:type=\"TextPanel\" name=\"%s\"  text=\"%s\" border=\"false\" textColor=\"#0000FF\" horizonalign=\"left\" vertalign=\"vcenter\" singleline=\"true\" enablelinks=\"true\"/>\
                                      <Control xsi:type=\"PaddingPanel\" /> <Control xsi:type=\"TextPanel\" width=\"30\"   text=\"%s\" border=\"false\" textColor=\"#0000FF\" horizonalign=\"left\" vertalign=\"vcenter\" singleline=\"true\" enablelinks=\"true\"/> <Control xsi:type=\"PaddingPanel\" width=\"10\"/></Container>";
	    int nSize = ::strlen(MSG_LIST_XML) + 512;
		char *szTmp = new char[nSize + 1];
		memset(szTmp, 0, nSize + 1);
		char szUTF8Tip[MAX_PATH] = {0};
		char szUTF8Number[MAX_PATH] = {0};
		CStringConversion::StringToUTF8(szTip, szUTF8Tip, MAX_PATH - 1);
		CStringConversion::StringToUTF8(szNumber, szUTF8Number, MAX_PATH - 1);
		sprintf(szTmp, MSG_LIST_XML, szTipName, szUTF8Tip, szUTF8Number);
		::SkinAddChildControl(m_hWnd, L"msglist", szTmp, NULL, NULL, 99999);
		delete []szTmp;
		::SkinUpdateControlUI(m_hWnd, L"msglist");
	}
	void SetChatHeader(const TCHAR *szFileName)
	{
		::SkinSetControlAttr(m_hWnd, L"PeerHeader", L"floatimagefilename", szFileName);
	}
    void Expand()
	{
		::SkinExpandTree(m_hWnd, L"colleaguetree", NULL, TRUE, TRUE);
	}
	BOOL Event(LPCTSTR pstrEvent, LPCTSTR pstrControlName, POINT *ptMouse, 
					WPARAM wParam, LPARAM lParam)
	{
		if (::lstrcmp(pstrEvent, L"click") == 0)
		{
			if (::lstrcmp(pstrControlName, L"test") == 0)
			{
				RECT rc = {0};
				::SkinGetControlRect(m_hWnd, L"resultlist", &rc);
			} else if (::lstrcmp(pstrControlName, L"cancel") == 0)
			{
				SetChatHeader(L"C:\\Users\\lanya\\AppData\\Local\\GoCom\\wuxiaozhong\\UserHead\\wenxue@GoCom.bmp");
				ShiftColor(127, 63, 243);
				//::SkinSetControlAttr(m_hWnd, L"bg", L"bkgndcolor", L"#FF0000");
				//SetChatHeader(L"E:\\log\\a.png");
			}
		}
		wprintf(L"Event:%s   ControlName:%s \n", pstrEvent, pstrControlName); 
		return FALSE;
	}
private:
	HWND m_hWnd;
};

//回调函数
BOOL CALLBACK TestEventWindow(HWND hWnd, LPCTSTR pstrEvent, LPCTSTR pstrWndName, LPCTSTR pstrControlName, POINT *ptMouse, 
					WPARAM wParam, LPARAM lParam, void *pOverlapped)
{
	if (pOverlapped)
	{
		CTestWindow *pThis = (CTestWindow *)pOverlapped;
		return pThis->Event(pstrEvent, pstrControlName, ptMouse, wParam, lParam);
	}
	return FALSE;
}

void InitImSkin()
{
	RECT rc = {0, 0, 340, 600};
	::SkinCreateFromFile("F:\\lanya\\workarea\\SmartLib\\CloudMessage\\CloudPeer\\Skin\\default.xml");
	CTestWindow Win("LogonWindow", "", NULL, WS_POPUP | WS_SYSMENU | WS_SIZEBOX | WS_MAXIMIZEBOX | WS_MINIMIZEBOX, 
		    WS_EX_TOOLWINDOW, &rc, FALSE);
	Win.Show();

	Win.Expand();
	Win.SetLabelText();
	Win.SetStatusList();
	Win.SetMinSize();
	::SkinApplicationRun();
	::SkinReInitApplicationRun();
	CTestWindow Win2("MainWindow", "", NULL, WS_POPUP | WS_SYSMENU | WS_SIZEBOX | WS_MAXIMIZEBOX | WS_MINIMIZEBOX, 
		    WS_EX_TOOLWINDOW, &rc, FALSE);
	Win2.Show();
	//Win2.LoadTreeViewDefImage("F:\\skinpath\\09.bmp");
	void *pRoot = Win2.AddTreeNode(NULL, L"同事组", TREENODE_TYPE_GROUP);
	void *pChild = Win2.AddTreeNode(pRoot, L"同事1", TREENODE_TYPE_LEAF);
	Win2.AddTreeNode(pRoot, L"同事2", TREENODE_TYPE_LEAF, L"F:\\skinpath\\08.bmp");
	pRoot = Win2.AddTreeNode(NULL, L"同学组", TREENODE_TYPE_GROUP);
	Win2.AddTreeNode(pRoot, L"同学1", TREENODE_TYPE_LEAF);
	Win2.AddTreeNode(pRoot, L"同学2", TREENODE_TYPE_LEAF, L"F:\\skinpath\\07.bmp");
    Win2.Expand();
	Win2.SetMaxSize();
	::SkinApplicationRun();
	::SkinDestroyResource();
}

void InitGoComSkin()
{
	RECT rc = {100, 100, 450, 700};
	::SkinCreateFromFile("F:\\lanya\\workarea\\SmartLib\\CloudMessage\\CloudPeer\\Skin\\default.xml");
	CTestWindow Win("MainWindow", "", NULL, WS_POPUP | WS_SYSMENU | WS_SIZEBOX | WS_MAXIMIZEBOX | WS_MINIMIZEBOX, 
		    WS_EX_TOOLWINDOW, &rc, FALSE);
	Win.Show();
	void *pRoot = Win.AddTreeNode(NULL, L"同事组", TREENODE_TYPE_GROUP);
	void *pChild = Win.AddTreeNode(pRoot, L"同事1", TREENODE_TYPE_LEAF);
	Win.AddTreeNode(pRoot, L"同事2", TREENODE_TYPE_LEAF, L"F:\\skinpath\\08.bmp");
	pRoot = Win.AddTreeNode(NULL, L"同学组", TREENODE_TYPE_GROUP);
	Win.AddTreeNode(pRoot, L"同学1", TREENODE_TYPE_LEAF);
	Win.AddTreeNode(pRoot, L"同学2", TREENODE_TYPE_LEAF, L"F:\\skinpath\\07.bmp");
    Win.Expand();
	::SkinApplicationRun();
	::SkinDestroyResource();
}

void InitChatFrameSkin()
{
	RECT rc = {100, 100, 800, 600};
	::SkinCreateFromFile("F:\\lanya\\workarea\\SmartLib\\CloudMessage\\CloudPeer\\Skin\\chatframeall.xml");
	CTestWindow Win("ChatWindow", "聊天窗口", NULL, WS_POPUP | WS_SYSMENU | WS_SIZEBOX | WS_MAXIMIZEBOX | WS_MINIMIZEBOX, 
		    0, &rc, FALSE);
	Win.Show(); 
	::SkinApplicationRun();
	::SkinDestroyResource();
}

void AddPluginXml(const char *szFileName)
{
	FILE *fp = fopen(szFileName, "r+b");
	if (fp)
	{
		fseek(fp, 0, SEEK_END);
		int l = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		char *szXmlStr = new char[l + 1];
		fread(szXmlStr, l, 1, fp);
		szXmlStr[l] = '\0';
		::SkinAddPluginXML(szXmlStr);
		delete []szXmlStr;
		fclose(fp);
	}
}

void InitSkin()
{
	char szAppFileName[MAX_PATH] = {0};
	char szAppPath[MAX_PATH] = {0};
	char szFileName[MAX_PATH] = {0};
	CSystemUtils::GetApplicationFileName(szAppFileName, MAX_PATH - 1);
	CSystemUtils::ExtractFilePath(szAppFileName, szAppPath, MAX_PATH - 1);
	strcpy(szFileName, szAppPath);
	strcat(szFileName, "default.xml");
	::SkinCreateFromFile(szFileName);
	strcpy(szFileName, szAppPath);
	strcat(szFileName, "loginframe.xml");
	AddPluginXml(szFileName);
	strcpy(szFileName, szAppPath);
	strcat(szFileName, "mainframe.xml");
	AddPluginXml(szFileName);
	strcpy(szFileName, szAppPath);
	strcat(szFileName, "CompanyOrg.xml");
	AddPluginXml(szFileName); 
	strcpy(szFileName, szAppPath);
	strcat(szFileName, "chatframe.xml");
	AddPluginXml(szFileName); 
    std::string strSkinXml = szAppPath;
	strSkinXml += "*.xml";
	TCHAR szTmp[MAX_PATH] = {0};
	TCHAR szwPath[MAX_PATH] = {0};
	CStringConversion::StringToWideChar(szAppPath, szwPath, MAX_PATH - 1);
	CStringConversion::StringToWideChar(strSkinXml.c_str(), szTmp, MAX_PATH - 1);
	WIN32_FIND_DATA fd = {0};
	HANDLE h = ::FindFirstFile(szTmp, &fd);
	if (h != NULL)
	{
		do
		{
			if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
			{
				if ((::lstrcmpi(fd.cFileName, L"chatframe.xml") == 0)
					|| (::lstrcmpi(fd.cFileName, L"CompanyOrg.xml") == 0)
					|| (::lstrcmpi(fd.cFileName, L"mainframe.xml") == 0)
					|| (::lstrcmpi(fd.cFileName, L"loginframe.xml") == 0)
					|| (::lstrcmpi(fd.cFileName, L"default.xml") == 0))
					continue;

				CStdString_ strFileName = szwPath;
				strFileName += fd.cFileName;
				char szName[MAX_PATH] = {0};
				CStringConversion::WideCharToString(strFileName.GetData(), szName, MAX_PATH - 1);
				AddPluginXml(szName);
			}
		} while (::FindNextFile(h, &fd)); 
		::FindClose(h);
	}
	CStdString_ strIniFile = szwPath;
	strIniFile += L"testskin.ini";
	TCHAR szwTemp[MAX_PATH] = {0};
	::GetPrivateProfileString(L"Skin", L"window", L"mainwindow", szwTemp, MAX_PATH - 1, strIniFile.GetData());
	
	char szWindowName[MAX_PATH - 1] = {0};
	RECT rc = {100, 100, 800, 600};
	CStringConversion::WideCharToString(szwTemp, szWindowName, MAX_PATH - 1);
	memset(szwTemp, 0, sizeof(TCHAR) * MAX_PATH);
	::GetPrivateProfileString(L"Skin", L"pos", L"100 100 600 700", szwTemp, MAX_PATH - 1, strIniFile.GetData());
	char szTemp[MAX_PATH] = {0};
	CStringConversion::WideCharToString(szwTemp, szTemp, MAX_PATH - 1);
	CSystemUtils::StringToRect(&rc, szTemp);
	CTestWindow Win(szWindowName, "窗口", NULL, WS_POPUP | WS_SYSMENU | WS_SIZEBOX | WS_MAXIMIZEBOX | WS_MINIMIZEBOX, 
		    WS_EX_TOOLWINDOW, &rc, FALSE);
	Win.AddShortCut();
	Win.AddTab();
	Win.AddMsg("收到渔夫的新消息", "adf", "3条");
	Win.AddMsg("收到渔夫2的新消息", "adf132134","4条");
	Win.AddMsg("收到渔夫3的新消息", "adf12341","5条");
	Win.AddMsg("收到渔夫4的新消息", "adf234","6条");
	Win.AddMsg("收到渔夫5的新消息", "adf23","7条");
	Win.AddMsg("收到渔夫6的新消息","adf2", "8条");
	Win.AddMsg("收到渔夫7的新消息", "adfw","9条");
	Win.AddMsg("收到渔夫8的新消息", "adfe", "10条");
	Win.SetTabEnable();
	void *pRoot = Win.AddTreeNode(NULL, L"同事组", TREENODE_TYPE_GROUP);
	void *pChild = Win.AddTreeNode(pRoot, L"同事1", TREENODE_TYPE_LEAF);
	Win.AddTreeNode(pRoot, L"同事2", TREENODE_TYPE_LEAF, L"F:\\skinpath\\08.bmp");
	Win.AddTreeNode(pRoot, L"同事2", TREENODE_TYPE_LEAF, L"F:\\skinpath\\08.bmp");
	Win.AddTreeNode(pRoot, L"同事2", TREENODE_TYPE_LEAF, L"F:\\skinpath\\08.bmp");
	Win.AddTreeNode(pRoot, L"同事2", TREENODE_TYPE_LEAF, L"F:\\skinpath\\08.bmp");
	Win.AddTreeNode(pRoot, L"同事2", TREENODE_TYPE_LEAF, L"F:\\skinpath\\08.bmp");
	Win.AddTreeNode(pRoot, L"同事2", TREENODE_TYPE_LEAF, L"F:\\skinpath\\08.bmp");
	pRoot = Win.AddTreeNode(NULL, L"同学组", TREENODE_TYPE_GROUP);
	Win.AddTreeNode(pRoot, L"同学1", TREENODE_TYPE_LEAF);
	Win.AddTreeNode(pRoot, L"同学2", TREENODE_TYPE_LEAF, L"F:\\skinpath\\07.bmp");
	Win.AddTreeNode(pRoot, L"同学2", TREENODE_TYPE_LEAF, L"F:\\skinpath\\07.bmp");
	Win.AddTreeNode(pRoot, L"同学2", TREENODE_TYPE_LEAF, L"F:\\skinpath\\07.bmp");
	Win.AddTreeNode(pRoot, L"同学2", TREENODE_TYPE_LEAF, L"F:\\skinpath\\07.bmp");
	Win.AddTreeNode(pRoot, L"同学2", TREENODE_TYPE_LEAF, L"F:\\skinpath\\07.bmp");
	Win.AddTreeNode(pRoot, L"同学2", TREENODE_TYPE_LEAF, L"F:\\skinpath\\07.bmp");
	Win.Expand();
	Win.Show(); 
	//Win.ToDesktop();
	Win.DockWindow();
	//Win.ShiftColor(50, 0, -50);
	::SkinApplicationRun();
	::SkinDestroyResource();
}

void InitToolTip()
{
	RECT rc = {100, 100, 450, 700};
	::SkinCreateFromFile("F:\\lanya\\workarea\\SmartLib\\CloudMessage\\CloudPeer\\Skin\\default.xml");
	::SkinShowHintWindow();
	::SkinApplicationRun();
	::SkinDestroyResource();
}

void AddWeatherPluginToMain()
{
	RECT rc = {100, 100, 450, 700};
	::SkinCreateFromFile("F:\\lanya\\workarea\\SmartLib\\CloudMessage\\CloudPeer\\Skin\\default.xml");
	FILE *fp = fopen("F:\\lanya\\workarea\\SmartLib\\CloudMessage\\CloudPeer\\Skin\\weather.xml", "r+b");
	if (fp)
	{
		fseek(fp, 0, SEEK_END);
		long l = ftell(fp);
		if (l > 0)
		{
			char *szBuf = new char[l + 1];
			memset(szBuf, 0, l + 1);
			fseek(fp, 0, SEEK_SET);
			fread(szBuf, 1, l, fp);
			::SkinAddPluginXML(szBuf);
		}
		fclose(fp);
	}
	CTestWindow Win("MainWindow", "", NULL, WS_POPUP | WS_SYSMENU | WS_SIZEBOX | WS_MAXIMIZEBOX | WS_MINIMIZEBOX, 
		    WS_EX_TOOLWINDOW, &rc, FALSE);
	Win.Show();
	void *pRoot = Win.AddTreeNode(NULL, L"同事组", TREENODE_TYPE_GROUP);
	void *pChild = Win.AddTreeNode(pRoot, L"同事1", TREENODE_TYPE_LEAF);
	Win.AddTreeNode(pRoot, L"同事2", TREENODE_TYPE_LEAF, L"F:\\skinpath\\08.bmp");
	pRoot = Win.AddTreeNode(NULL, L"同学组", TREENODE_TYPE_GROUP);
	Win.AddTreeNode(pRoot, L"同学1", TREENODE_TYPE_LEAF);
	Win.AddTreeNode(pRoot, L"同学2", TREENODE_TYPE_LEAF, L"F:\\skinpath\\07.bmp");
    Win.Expand();
	::SkinApplicationRun();
	::SkinDestroyResource();
}

void InitUpdater()
{
	char szAppFileName[MAX_PATH] = {0};
	char szAppPath[MAX_PATH] = {0};
	char szFileName[MAX_PATH] = {0};
	CSystemUtils::GetApplicationFileName(szAppFileName, MAX_PATH - 1);
	CSystemUtils::ExtractFilePath(szAppFileName, szAppPath, MAX_PATH - 1);
	strcpy(szFileName, szAppPath);
	strcat(szFileName, "updater.xml");
	::SkinCreateFromFile(szFileName);
	RECT rc = {100, 100, 500, 350};
	CTestWindow Win("updatewindow", "自动升级 ", NULL, WS_POPUP | WS_SYSMENU | WS_SIZEBOX | WS_MAXIMIZEBOX | WS_MINIMIZEBOX, 
		    0, &rc, FALSE);
	Win.Show();
	::SkinApplicationRun();
	::SkinDestroyResource();
}

void InitK6Skin()
{
	char szAppFileName[MAX_PATH] = {0};
	char szAppPath[MAX_PATH] = {0};
	char szFileName[MAX_PATH] = {0};
	CSystemUtils::GetApplicationFileName(szAppFileName, MAX_PATH - 1);
	CSystemUtils::ExtractFilePath(szAppFileName, szAppPath, MAX_PATH - 1);
	strcpy(szFileName, szAppPath);
	strcat(szFileName, "G5.xml");
	::SkinCreateFromFile(szFileName);
	RECT rc = {100, 100, 473, 375};
	CTestWindow Win("G5Login", "窗口", NULL, WS_POPUP | WS_SYSMENU | WS_SIZEBOX | WS_MAXIMIZEBOX | WS_MINIMIZEBOX, 
		    0, &rc, FALSE);
	Win.Show();
	RECT rc2 = {200, 200, 980, 650};
    CTestWindow Win2("G5Window", "窗口", NULL, WS_POPUP | WS_SYSMENU | WS_SIZEBOX | WS_MAXIMIZEBOX | WS_MINIMIZEBOX, 
		    0, &rc2, FALSE);
	Win2.Show();
	RECT rc3 = {300, 200, 660, 760};
    CTestWindow Win3("G5EditWindow", "窗口", NULL, WS_POPUP | WS_SYSMENU | WS_SIZEBOX | WS_MAXIMIZEBOX | WS_MINIMIZEBOX, 
		    0, &rc3, FALSE);
	Win3.Show();
	//Win.ShiftColor(50, 0, -50);
	::SkinApplicationRun();
	::SkinDestroyResource();
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int nCmdShow)
{
	if (!::SkinCheckRunOption())
		::MessageBox(NULL, L"初始化环境出错", L"警告", MB_OK);
	//InitToolTip();
	//InitGoComSkin();
	//InitImSkin();
	//AddWeatherPluginToMain();
	//InitChatFrameSkin();
	InitSkin();
	//CSystemUtils::RegisterWebProtocol("gocom", "E:\\lanya\\workarea\\GoCom\\Bin\\Debug\\GoCom.exe", 1);
	//InitK6Skin();
	//InitUpdater();
	return 0;
}

#pragma warning(default:4996)
