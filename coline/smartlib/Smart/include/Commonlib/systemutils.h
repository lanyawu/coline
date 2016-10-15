#ifndef __SYSTEMUTILS_H__
#define __SYSTEMUTILS_H__

#include <CommonLib/Types.h>
#include <nb30.h>
#include <vector>
#include <string>

#ifndef VER_PLATFORM_WIN32s
#define VER_PLATFORM_WIN32s             0
#endif
#ifndef VER_PLATFORM_WIN32_WINDOWS
#define VER_PLATFORM_WIN32_WINDOWS      1
#endif
#ifndef VER_PLATFORM_WIN32_NT
#define VER_PLATFORM_WIN32_NT           2
#endif
#ifndef VER_PLATFORM_WIN32_CE
#define VER_PLATFORM_WIN32_CE           3
#endif

//操作系统版本定义
#define OS_VERSION_OLD      1
#define OS_VERSION_WINXP    2
#define OS_VERSION_WIN2000  3
#define OS_VERSION_WIN2003  4
#define OS_VERSION_WINXP64  5
#define OS_VERSION_VISTA    6
#define OS_VERSION_WIN7     7

#define DEFAULT_FLAHS_WINDOW_COUNT      10  //闪动窗口次数
typedef std::vector<std::string> CStringList_;

#pragma pack(push)
#pragma pack(1)
typedef struct tagLastInputInfo
{
	UINT  cbSize;
	DWORD dwActTime;
}LASTINPUTINFO_, *LPLASTINPUTINFO;

#pragma pack(pop)

typedef void (WINAPI *PROCSWITCHTOTHISWINDOW)(HWND,BOOL);
typedef void (WINAPI *PROCGETLASTINPUTINFO)(LPLASTINPUTINFO);
typedef UCHAR (WINAPI *PROCNETBIOS)(PNCB);
typedef BOOL (WINAPI *LPALPHABLEND)(HDC, int, int, int, int,HDC, int, int, int, int, BLENDFUNCTION);

class IImageInterface
{
public: 
	virtual ~IImageInterface() {};
	//
	virtual BOOL LoadFromFile(const char *FileName,  BOOL bGray) = 0; //从文件中载入
	virtual BOOL LoadFromBuff(const char *lpBuff, DWORD dwSize, BOOL bGray) = 0; //从数据流中载入
	virtual BOOL LoadFromGraphic(IImageInterface *pSrc) = 0; //从源图中载入
	virtual BOOL LoadFromIcon(HICON hIcon) = 0; //从Icon 中载入
	virtual BOOL SetGray() = 0; //灰化图片
	virtual void SetImageMask(int nMask) =0;
	virtual BOOL FillColorToImage(BYTE r, BYTE g, BYTE b) = 0; //填充颜色
	//将图片画到指定dc
	virtual void DrawToDc(HDC dc,  int x, int y, int nWidth, int nHeight) = 0;
	virtual void DrawToDc(HDC dc, const RECT& rc ) = 0;
	virtual BOOL IsEmpty() const = 0;
	virtual int  GetWidth() const = 0;
	virtual int  GetHeight() const = 0;
	virtual BOOL GetAlphaChannel() = 0;
	virtual int  GetMask() = 0;
	virtual HBITMAP GetBitmap() = 0;
	virtual	BOOL DrawPlus(HDC hDc, const RECT& rc, const RECT& rcPaint, 
        const RECT& rcBmpPart, const RECT& rcCorners, LPALPHABLEND lpAlphaBlend, BYTE uFade = 255, 
        bool hole = false, bool xtiled = false, bool ytiled = false) =0;
};

//与系统相关函数
class COMMONLIB_API CSystemUtils
{
public:
	//获取ＣＰＵ的ＩＤ号
	static DWORD     GetCpuId();
	//获取本机网卡IP
	static DWORD     GetLocalIP();
	//域名转IP
	static BOOL      GetIpByHostName(const char *szHostName, std::string &strIp);
	//获取网卡mac
	static BOOL      GetNetCardMac(std::string &strMac);
	//获取网卡唯一标识
	static DWORD     GetNetCardFlag();
	//获取本机连接状态
	static DWORD     GetConnectState();
	//获取操作系统版本
	static DWORD     GetOSVersion();
	//获取文件大小
	static __int64   GetFileSize(const char *szFileName);
	//
	static HWND FindDesktopWindow();
	//
	static BOOL StartShellProcessor(const char *szAppName, const char *szParams, const char *szWorkPath, BOOL bWait);
	//获取我的文档路径
	static char *    GetMyDocumentPath(char *szMyDocPath, int nMaxSize);
	//获取用户应用程序数据路径 AppPath
	static char *    GetLocalAppPath(char *szLocalAppPath, int nMaxSize);
	//获取系统应用程序目录
	static char *    GetSystemAppPath(char *szLocalAppPath, int nMaxSize);
	//获取临时文件目录
	static char *    GetSystemTempPath(char *szTmpPath, int nMaxSize);
	//获取一个临时文件名称
	static char *    GetSystemTempFileName(char *szFileName, int nMaxSize);
	//获取安装程序路径
	static char *    GetProgramFilesPath(char *szPath, int nMaxSize);
	//获取系统路径
	static char *    GetSystemDirectory(char *szPath, int nMaxSize);
	//检测文件路径是否存在
	static BOOL      DirectoryIsExists(const char *szPath);
	//检测文件是否存在
	static BOOL      FileIsExists(const char *szFileName);
	//创建文件目录
	static BOOL      ForceDirectories(const char *szPath);
	//创建一个文件目录
	static BOOL      CreateDir(const char *szPath);
	//根据分隔符取数据
	static BOOL      GetStringBySep(const char *szString, char *szDest, char c, int nIdx);
	//拷贝文件 bForce表示存在文件时改写
	static BOOL      CopyFilePlus(const char *szSrcFileName, const char *szDestFileName, BOOL bForce);
	//移动一个文件
	static BOOL      MoveFilePlus(const char *szSrcFileName, const char *szDestFileName, BOOL bForce);
	//删除一个文件
	static BOOL      DeleteFilePlus(const char *szFileName);
	//删除一个目录
	static BOOL		 DeleteDirectory(const char* szPath);
	//获取屏幕大小
	static BOOL      GetScreenRect(RECT *lprc);
	//获取应用程序文件名
	static char *    GetApplicationFileName(char *szFileName, int nMaxSize);
	//
	static char *    GetModuleAppFileName(HMODULE hModule, char *szFileName, int nMaxSize);
	//根据文件名获得路径
	static char *    ExtractFilePath(const char *szFileName, char *szPath, int nMaxPath);
	//获取文件名称，不包括路径
	static char *    ExtractFileName(const char *szFileName, char *szDest, int nMaxSize);
	//获取文件扩展名
	static char *    ExtractFileExtName(const char *szFileName, char *szExt, int nMaxSize);
	//检测文件路径是否包括分界符"/"
	static char *    IncludePathDelimiter(const char *szPath, char *szDest, int nMaxPath);
	//去除文件路径分界符"/"
	static char *    DeletePathDelimiter(const char *szPath, char *szDest, int nMaxPath);
	//读取注册表项值
	static BOOL      ReadRegisterItems(HKEY nKey, const char *szPath, std::vector<std::string> &Items);
	//读取注册表键值
	static char *    ReadRegisterKey(HKEY nKey, const char *szPath, const char *szItem, char *szResult, int nMaxSize);
	//写入注册表键值
	static BOOL      WriteRegisterKey(HKEY nKey, const char *szPath, const char *szItem, const char *szKey, DWORD dwRegType = REG_SZ);
	//创建一个子键
	static BOOL      CreateChildKey(HKEY nKey, const char *szPath, const char *szChildKey);
	//删除注册表某个键值
	static BOOL      DeleteRegisterKey(HKEY nKey, const char *szPath, const char *szItem);
	//注册关联的Web协议
	static BOOL      RegisterWebProtocol(const char *szProtoName, const char *szAppName, const int iIconIdx);
	//获取windows 版本 szWinver 名称 nWinVer 版本号 szMajorBuilder builder号
	static BOOL      GetWindowsVersion(char *szWinVer, int &nWinVer, char *szMajorBuilder);
    //打开一个文件选择对话框
	static BOOL      OpenFileDialog(HINSTANCE hInstance, HWND hOwner, char *szTitle, char *szFilter, 
		                char *szSelFile, CStringList_ &FileList, BOOL bIsMultiSelect, BOOL IsSaveDlg = FALSE);
	//选取一个目录
	static BOOL      SelectFolder(HWND hOwner, char *szSelPath);
	//打开一个字体选择框
	static BOOL      OpenFontDialog(HINSTANCE hInstance, HWND hOwner, int &nFontSize, int &nFontStyle, 
		               int &nFontColor, char *szFontName);
	//打开颜色选择框
	static BOOL      OpenColorDialog(HINSTANCE hInstance, HWND hOwner, COLORREF &cr);
	//时间转换 从time_t 转成字符串 2009-01-01 01:00:00
	static char *    DateTimeToStr(DWORD dwTime, char *szDateTime, BOOL bSep = TRUE);
	//时间转换 从time_t 转成字符串 01:00:00 不转日期
	static char *    TimeToStr(DWORD dwTime, char *szTime);
	//日期转换 从time_t 转成字符串 2010-01-01 不转时间
	static char *    DateToStr(DWORD dwDate, char *szDate);
	//计算两个字符串的日期差 秒为单位 只取最后 分:秒
	static DWORD  MinusTimeString(const char *strTime1, const char *strTime2);
	//合并成一个flag
	static DWORD     MakeCustomFlag(DWORD dwFileId, BYTE byteLink);
	//分解一个flag
    static void      ParserCustomFlag(DWORD dwFlag, DWORD &dwFileId, BYTE &byteLink);
	//打开一个链接地址
	static void      OpenURL(const char *szURL);
	//闪动窗口
	static void      FlashWindow(HWND hWnd, BOOL bFlash = TRUE);
	//切换窗口到前面
	static void      BringToFront(HWND hWnd);
	//是否在ToolBar上显示
	static BOOL      Show2ToolBar(HWND hWnd, BOOL bShow);
	//播放声音文件
	static void      PlaySoundFile(const char *szFileName, BOOL bLoop);
	//普通字符串转url串
	static char *    StringToUrlChars(const char *szSrc, char *szUrlChars, int nMaxLen);
	//获取用户电脑最后活动时间 毫秒为单位
	static DWORD     GetUserLastActiveTime();
	//十六进制转数据
	static int       HexToInt(const char *szStr);
	//获取命令行参数个数
	static int       GetParamCount();
	//获取命令行参数
	static std::string GetParamStr(int idx);
	//杀掉屏幕保护程序
	static BOOL KillScreenSaver();
	//设置剪切板的文本数据
	static BOOL SetClipboardText(HWND hOwner, const char *szText);
	//执行一个操作系统外部程序
	static BOOL ExecuteCommand(const char *szCommand, const char *szParam);
	//区域图像变暗
	static BOOL AreaGray(HDC hDC, const RECT *prc);
	//绘制图像
	static BOOL DrawImage(HDC hDC, HBITMAP hBitmap, const RECT& rc, const RECT& rcPaint, 
        const RECT& rcBmpPart, const RECT& rcCorners, LPALPHABLEND lpAlphaBlend, BOOL bAlphaChannel, BYTE uFade, 
        bool hole, bool xtiled, bool ytiled);
	//获取GUID字符串
	static BOOL GetGuidString(char *strGuid, int *nSize);
	//rect to string 0 0 100 100
	static BOOL RectToString(const RECT &rc, std::string &strRect);
	//string to rect
	static BOOL StringToRect(RECT *prc, const char *strRect);
	//
	static BOOL IsMobileNumber(const char *szNumber);
	//
	static BOOL GetScreenCenterRect(int nWidth, int nHeight, RECT &rc);
private:
	static DWORD m_ScreenPixels;
	static PROCSWITCHTOTHISWINDOW m_pSwitchToThisWindow; //切换窗口函数
	static PROCGETLASTINPUTINFO m_pGetLastInputInfo; //获取最后输入时间
};

static char a2A_INTERVAL = 'a' - 'A';

//转小写
inline const char *LowerCase(const char *szSrc, char *szDest, const size_t nSize)
{
	const char *p1 = szSrc;
	char *p2 = szDest;
	for (size_t i = 0; i < nSize; i ++)
	{
		if ((*p1 >= 'A') && (*p1 <= 'Z'))
		{
			*p2 = *p1 + a2A_INTERVAL;
		} else
			*p2 = *p1;
		p1 ++;
		p2 ++;
	}
	*p2 = '\0';
	return szDest;
}

#endif