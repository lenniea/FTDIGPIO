#include <windows.h>
#include "Trace.h"
#include <tchar.h>

void DebugTrace(const char* szFormat, ...)
{
    TCHAR szText[1024];
   	va_list arglist;

    va_start(arglist, szFormat );
    wvsprintf(szText, szFormat, arglist);
    OutputDebugString(szText);
    va_end(arglist);
}
