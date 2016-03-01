#ifndef __APP_DIALOG_H__
#define __APP_DIALOG_H__

#define STR_MAX		80

class CAppDialog
{
protected:
    HINSTANCE m_hInst;
public:
    CAppDialog(HINSTANCE hInst);

    HWND m_hWnd;
    TCHAR m_szAppName[STR_MAX];

    BOOL DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual BOOL OnInitDialog(WPARAM wParam, LPARAM lParam);
    virtual BOOL OnDrawItem(WPARAM wParam, LPDRAWITEMSTRUCT lParam);
    virtual BOOL OnCommand(WPARAM wId);
    virtual BOOL OnNotify(WPARAM wId, LPNMHDR nmhdr);

    BOOL CreateDialogBox(LPCTSTR pszResource, HWND hParent);

    void MsgBox(UINT type, UINT id, ...);
    void OnClose();
    void SleepYield(DWORD msec);
	BOOL ShowWindow(int nCmdShow);
	int Run();

protected:
    HWND m_hListView;
    HWND m_hCombo;
	HWND m_hProgress;

	void ShowProgress(UINT done, UINT total);
    BOOL SendBytes(LPVOID pBuffer, UINT count);
	void DoSend(void);
	BOOL DoLoopback(void);
    void FillCombo(LPCTSTR pszFilename);
    void ClearLog(void);
    void GetCellRect(int col, LPCRECT pItemRect, LPRECT pCellRect);
    BOOL DrawListView(LPDRAWITEMSTRUCT lpDIS);
};

#endif // __APP_DIALOG_H__
