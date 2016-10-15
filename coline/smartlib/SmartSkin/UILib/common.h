#pragma once

#include <windows.h>
#include <tchar.h>
#include <commctrl.h>
#include <crtdbg.h>

#include <UILib/UIBase.h>
using namespace UILib;

#include <UILib/UIManager.h>
#include <UILib/UIBlue.h>

#define lengthof(x) (sizeof(x)/sizeof(*x))
#define MAX max
#define MIN min
#define CLAMP(x,a,b) (MIN(b,MAX(a,x)))
