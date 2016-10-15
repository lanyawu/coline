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

//字体样式
#define FONT_STYLE_BOLB              1  //粗体
#define FONT_STYLE_ITALIC            2  //斜体
#define FONT_STYLE_UNDERLINE         4  //下划线

#define  MAX_TEXT_FACENAME_SIZE      32

