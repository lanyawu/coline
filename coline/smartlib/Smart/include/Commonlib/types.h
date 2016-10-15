#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <malloc.h>
#include <crtdbg.h>
#include <commctrl.h>
#include <tchar.h>
#include <assert.h>
#include <olectl.h>

#ifdef COMMONLIB_DLL
  #ifdef COMMONLIB_EXPORTS
     #define COMMONLIB_API __declspec(dllexport)
  #else
     #define COMMONLIB_API __declspec(dllimport)
  #endif
#else
  #define COMMONLIB_API
#endif

//������ʽ
#define FONT_STYLE_BOLB              1  //����
#define FONT_STYLE_ITALIC            2  //б��
#define FONT_STYLE_UNDERLINE         4  //�»���

#define  MAX_TEXT_FACENAME_SIZE      32

