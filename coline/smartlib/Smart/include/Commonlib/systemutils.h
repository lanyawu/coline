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

//����ϵͳ�汾����
#define OS_VERSION_OLD      1
#define OS_VERSION_WINXP    2
#define OS_VERSION_WIN2000  3
#define OS_VERSION_WIN2003  4
#define OS_VERSION_WINXP64  5
#define OS_VERSION_VISTA    6
#define OS_VERSION_WIN7     7

#define DEFAULT_FLAHS_WINDOW_COUNT      10  //�������ڴ���
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
	virtual BOOL LoadFromFile(const char *FileName,  BOOL bGray) = 0; //���ļ�������
	virtual BOOL LoadFromBuff(const char *lpBuff, DWORD dwSize, BOOL bGray) = 0; //��������������
	virtual BOOL LoadFromGraphic(IImageInterface *pSrc) = 0; //��Դͼ������
	virtual BOOL LoadFromIcon(HICON hIcon) = 0; //��Icon ������
	virtual BOOL SetGray() = 0; //�һ�ͼƬ
	virtual void SetImageMask(int nMask) =0;
	virtual BOOL FillColorToImage(BYTE r, BYTE g, BYTE b) = 0; //�����ɫ
	//��ͼƬ����ָ��dc
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

//��ϵͳ��غ���
class COMMONLIB_API CSystemUtils
{
public:
	//��ȡ�ãУյģɣĺ�
	static DWORD     GetCpuId();
	//��ȡ��������IP
	static DWORD     GetLocalIP();
	//����תIP
	static BOOL      GetIpByHostName(const char *szHostName, std::string &strIp);
	//��ȡ����mac
	static BOOL      GetNetCardMac(std::string &strMac);
	//��ȡ����Ψһ��ʶ
	static DWORD     GetNetCardFlag();
	//��ȡ��������״̬
	static DWORD     GetConnectState();
	//��ȡ����ϵͳ�汾
	static DWORD     GetOSVersion();
	//��ȡ�ļ���С
	static __int64   GetFileSize(const char *szFileName);
	//
	static HWND FindDesktopWindow();
	//
	static BOOL StartShellProcessor(const char *szAppName, const char *szParams, const char *szWorkPath, BOOL bWait);
	//��ȡ�ҵ��ĵ�·��
	static char *    GetMyDocumentPath(char *szMyDocPath, int nMaxSize);
	//��ȡ�û�Ӧ�ó�������·�� AppPath
	static char *    GetLocalAppPath(char *szLocalAppPath, int nMaxSize);
	//��ȡϵͳӦ�ó���Ŀ¼
	static char *    GetSystemAppPath(char *szLocalAppPath, int nMaxSize);
	//��ȡ��ʱ�ļ�Ŀ¼
	static char *    GetSystemTempPath(char *szTmpPath, int nMaxSize);
	//��ȡһ����ʱ�ļ�����
	static char *    GetSystemTempFileName(char *szFileName, int nMaxSize);
	//��ȡ��װ����·��
	static char *    GetProgramFilesPath(char *szPath, int nMaxSize);
	//��ȡϵͳ·��
	static char *    GetSystemDirectory(char *szPath, int nMaxSize);
	//����ļ�·���Ƿ����
	static BOOL      DirectoryIsExists(const char *szPath);
	//����ļ��Ƿ����
	static BOOL      FileIsExists(const char *szFileName);
	//�����ļ�Ŀ¼
	static BOOL      ForceDirectories(const char *szPath);
	//����һ���ļ�Ŀ¼
	static BOOL      CreateDir(const char *szPath);
	//���ݷָ���ȡ����
	static BOOL      GetStringBySep(const char *szString, char *szDest, char c, int nIdx);
	//�����ļ� bForce��ʾ�����ļ�ʱ��д
	static BOOL      CopyFilePlus(const char *szSrcFileName, const char *szDestFileName, BOOL bForce);
	//�ƶ�һ���ļ�
	static BOOL      MoveFilePlus(const char *szSrcFileName, const char *szDestFileName, BOOL bForce);
	//ɾ��һ���ļ�
	static BOOL      DeleteFilePlus(const char *szFileName);
	//ɾ��һ��Ŀ¼
	static BOOL		 DeleteDirectory(const char* szPath);
	//��ȡ��Ļ��С
	static BOOL      GetScreenRect(RECT *lprc);
	//��ȡӦ�ó����ļ���
	static char *    GetApplicationFileName(char *szFileName, int nMaxSize);
	//
	static char *    GetModuleAppFileName(HMODULE hModule, char *szFileName, int nMaxSize);
	//�����ļ������·��
	static char *    ExtractFilePath(const char *szFileName, char *szPath, int nMaxPath);
	//��ȡ�ļ����ƣ�������·��
	static char *    ExtractFileName(const char *szFileName, char *szDest, int nMaxSize);
	//��ȡ�ļ���չ��
	static char *    ExtractFileExtName(const char *szFileName, char *szExt, int nMaxSize);
	//����ļ�·���Ƿ�����ֽ��"/"
	static char *    IncludePathDelimiter(const char *szPath, char *szDest, int nMaxPath);
	//ȥ���ļ�·���ֽ��"/"
	static char *    DeletePathDelimiter(const char *szPath, char *szDest, int nMaxPath);
	//��ȡע�����ֵ
	static BOOL      ReadRegisterItems(HKEY nKey, const char *szPath, std::vector<std::string> &Items);
	//��ȡע����ֵ
	static char *    ReadRegisterKey(HKEY nKey, const char *szPath, const char *szItem, char *szResult, int nMaxSize);
	//д��ע����ֵ
	static BOOL      WriteRegisterKey(HKEY nKey, const char *szPath, const char *szItem, const char *szKey, DWORD dwRegType = REG_SZ);
	//����һ���Ӽ�
	static BOOL      CreateChildKey(HKEY nKey, const char *szPath, const char *szChildKey);
	//ɾ��ע���ĳ����ֵ
	static BOOL      DeleteRegisterKey(HKEY nKey, const char *szPath, const char *szItem);
	//ע�������WebЭ��
	static BOOL      RegisterWebProtocol(const char *szProtoName, const char *szAppName, const int iIconIdx);
	//��ȡwindows �汾 szWinver ���� nWinVer �汾�� szMajorBuilder builder��
	static BOOL      GetWindowsVersion(char *szWinVer, int &nWinVer, char *szMajorBuilder);
    //��һ���ļ�ѡ��Ի���
	static BOOL      OpenFileDialog(HINSTANCE hInstance, HWND hOwner, char *szTitle, char *szFilter, 
		                char *szSelFile, CStringList_ &FileList, BOOL bIsMultiSelect, BOOL IsSaveDlg = FALSE);
	//ѡȡһ��Ŀ¼
	static BOOL      SelectFolder(HWND hOwner, char *szSelPath);
	//��һ������ѡ���
	static BOOL      OpenFontDialog(HINSTANCE hInstance, HWND hOwner, int &nFontSize, int &nFontStyle, 
		               int &nFontColor, char *szFontName);
	//����ɫѡ���
	static BOOL      OpenColorDialog(HINSTANCE hInstance, HWND hOwner, COLORREF &cr);
	//ʱ��ת�� ��time_t ת���ַ��� 2009-01-01 01:00:00
	static char *    DateTimeToStr(DWORD dwTime, char *szDateTime, BOOL bSep = TRUE);
	//ʱ��ת�� ��time_t ת���ַ��� 01:00:00 ��ת����
	static char *    TimeToStr(DWORD dwTime, char *szTime);
	//����ת�� ��time_t ת���ַ��� 2010-01-01 ��תʱ��
	static char *    DateToStr(DWORD dwDate, char *szDate);
	//���������ַ��������ڲ� ��Ϊ��λ ֻȡ��� ��:��
	static DWORD  MinusTimeString(const char *strTime1, const char *strTime2);
	//�ϲ���һ��flag
	static DWORD     MakeCustomFlag(DWORD dwFileId, BYTE byteLink);
	//�ֽ�һ��flag
    static void      ParserCustomFlag(DWORD dwFlag, DWORD &dwFileId, BYTE &byteLink);
	//��һ�����ӵ�ַ
	static void      OpenURL(const char *szURL);
	//��������
	static void      FlashWindow(HWND hWnd, BOOL bFlash = TRUE);
	//�л����ڵ�ǰ��
	static void      BringToFront(HWND hWnd);
	//�Ƿ���ToolBar����ʾ
	static BOOL      Show2ToolBar(HWND hWnd, BOOL bShow);
	//���������ļ�
	static void      PlaySoundFile(const char *szFileName, BOOL bLoop);
	//��ͨ�ַ���תurl��
	static char *    StringToUrlChars(const char *szSrc, char *szUrlChars, int nMaxLen);
	//��ȡ�û��������ʱ�� ����Ϊ��λ
	static DWORD     GetUserLastActiveTime();
	//ʮ������ת����
	static int       HexToInt(const char *szStr);
	//��ȡ�����в�������
	static int       GetParamCount();
	//��ȡ�����в���
	static std::string GetParamStr(int idx);
	//ɱ����Ļ��������
	static BOOL KillScreenSaver();
	//���ü��а���ı�����
	static BOOL SetClipboardText(HWND hOwner, const char *szText);
	//ִ��һ������ϵͳ�ⲿ����
	static BOOL ExecuteCommand(const char *szCommand, const char *szParam);
	//����ͼ��䰵
	static BOOL AreaGray(HDC hDC, const RECT *prc);
	//����ͼ��
	static BOOL DrawImage(HDC hDC, HBITMAP hBitmap, const RECT& rc, const RECT& rcPaint, 
        const RECT& rcBmpPart, const RECT& rcCorners, LPALPHABLEND lpAlphaBlend, BOOL bAlphaChannel, BYTE uFade, 
        bool hole, bool xtiled, bool ytiled);
	//��ȡGUID�ַ���
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
	static PROCSWITCHTOTHISWINDOW m_pSwitchToThisWindow; //�л����ں���
	static PROCGETLASTINPUTINFO m_pGetLastInputInfo; //��ȡ�������ʱ��
};

static char a2A_INTERVAL = 'a' - 'A';

//תСд
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