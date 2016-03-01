#include <windows.h>
#include "resource.h"
#include "AppDialog.h"

PVOID pResizeState = NULL;

// Constructor
CAppDialog::CAppDialog(HINSTANCE hInst)
{
    m_hInst = hInst;
    m_hWnd = NULL;
    LoadString(hInst, IDS_APP_NAME, m_szAppName, STR_MAX);
}

BOOL CAppDialog::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return FALSE;
}

BOOL CAppDialog::OnInitDialog(WPARAM wParam, LPARAM lParam)
{
	return TRUE;
}

BOOL CAppDialog::OnDrawItem(WPARAM wParam, LPDRAWITEMSTRUCT lParam)
{
	return TRUE;
}

BOOL CAppDialog::OnCommand(WPARAM wId)
{
	return TRUE;
}

BOOL CAppDialog::OnNotify(WPARAM wId, LPNMHDR nmhdr)
{
	return TRUE;
}

void CAppDialog::MsgBox(UINT type, UINT id, ...)
{
  	va_list arglist;

    TCHAR szFormat[80];
    ::LoadString(m_hInst, id, szFormat, sizeof(szFormat));

    TCHAR szBuffer[256];
    va_start(arglist, id );
    wvsprintf(szBuffer, szFormat, arglist);
    va_end(arglist);
    MessageBox(m_hWnd, szBuffer, m_szAppName, type);
}

void CAppDialog::SleepYield(DWORD msec)
{
    MSG msg;
    ::Sleep(msec);
    if (PeekMessage(&msg, m_hWnd, 0L, 0L, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void CAppDialog::OnClose()
{
    ::EndDialog(m_hWnd, IDOK);
}

BOOL CALLBACK AppDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CAppDialog* pDlg = (CAppDialog*) GetWindowLong(hDlg, GWL_USERDATA);

    switch (uMsg)
    {
    case WM_INITDIALOG:
		SetWindowLong(hDlg, GWL_USERDATA, lParam);
		pDlg = (CAppDialog*) lParam;
        pDlg->m_hWnd = hDlg;
        return pDlg->OnInitDialog(wParam, lParam);
    case WM_CLOSE:
        pDlg->OnClose();
        DestroyWindow(hDlg);
        return TRUE;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_COMMAND:
        return pDlg->OnCommand(wParam);
    case WM_NOTIFY:
        return pDlg->OnNotify(wParam, (LPNMHDR) lParam);
    case WM_DRAWITEM:
        return pDlg->OnDrawItem(wParam, (LPDRAWITEMSTRUCT) lParam);
    return pDlg->DialogProc(uMsg, wParam, lParam);
    }
    return FALSE;
}

BOOL CAppDialog::CreateDialogBox(LPCTSTR lpszResource, HWND hParent)
{
    m_hWnd = CreateDialogParam(m_hInst, lpszResource, hParent, (DLGPROC)AppDialogProc, (LPARAM) this);
	return m_hWnd != NULL;
}

int CAppDialog::Run(void)
{
    int status;
	MSG msg;

    while ((status = GetMessage(&msg, 0, 0, 0)) != 0)
    {
        if (status == -1)
            return -1;
        if (!::IsDialogMessage(m_hWnd, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
	return msg.wParam;
}

BOOL CAppDialog::ShowWindow(int nCmdShow)
{
	return ::ShowWindow(m_hWnd, nCmdShow);
}
