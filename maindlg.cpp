#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <tchar.h>
#include <winsock.h>
#include "AppDialog.h"
#include "resource.h"
#include "Trace.h"

#include "ftd2xx.h"

#define ASYNC_BITBANG   0x01
#define CBUS_BITBANG    0x20

#define ProgressBar_SetRange(hwnd,lo,hi)  SendMessage(hwnd, PBM_SETRANGE, 0, MAKELONG(lo,hi))
#define ProgressBar_SetPos(hwnd,pos)      SendMessage(hwnd, PBM_SETPOS, pos, 0L)

#define BUF_SIZE        16384

#define DEFAULT_COUNT   100
#define MAX_COUNT       1024

#define LOG_SENT_FLAG   0x80000000
#define LOG_COUNT_MASK  0x7FFFFFFF

TCHAR szProfile[MAX_PATH];
TCHAR szFilename[MAX_PATH];

DWORD g_start;
UINT g_logindex = 0;

#define STR_MAX             80
#define DEFAULT_TIMEOUT     1000

FT_HANDLE W32_OpenDevice(LPTSTR pszFile, DWORD dwBaud)
{
    FT_HANDLE hDevice;
    FT_STATUS status;
    ULONG bufsize = 2048;

    status = FT_OpenEx(pszFile, FT_OPEN_BY_SERIAL_NUMBER, &hDevice);
    if (status != FT_OK)
        return INVALID_HANDLE_VALUE;

    bufsize = 65536;

    // Set 8 data bits, 1 stop bit and no parity
    status = FT_SetBaudRate(hDevice, dwBaud);
    if (status != FT_OK)
        return INVALID_HANDLE_VALUE;

    status = FT_SetDataCharacteristics
        (hDevice, FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_NONE);
    if (status != FT_OK)
        return INVALID_HANDLE_VALUE;

    status = FT_SetLatencyTimer(hDevice, 2);
    if (status != FT_OK)
        return INVALID_HANDLE_VALUE;

    status = FT_SetUSBParameters(hDevice, bufsize, bufsize);
    if (status != FT_OK)
        return INVALID_HANDLE_VALUE;

    status = FT_SetTimeouts(hDevice, DEFAULT_TIMEOUT, DEFAULT_TIMEOUT);
    if (status != FT_OK)
        return INVALID_HANDLE_VALUE;

    return hDevice;
}

BOOL W32_CloseDevice(FT_HANDLE handle)
{
    if (handle != INVALID_HANDLE_VALUE)
    {
        // Set CBUS back to all inputs
        FT_STATUS status = FT_SetBitMode(handle, 0x00, CBUS_BITBANG);
    }
    return FT_Close(handle) == FT_OK;
}

int QueryCOMPorts(HWND hCombo)
{
    FT_STATUS status;
    DWORD dwNumDevs;
    
    status = FT_ListDevices(&dwNumDevs, NULL, FT_LIST_NUMBER_ONLY);
    if (status != FT_OK)
        return -1;

    ComboBox_ResetContent(hCombo);
    for (DWORD index = 0; index < dwNumDevs; ++index)
    {
        char szDevice[64];

        status = FT_ListDevices((PVOID) index, szDevice, FT_LIST_BY_INDEX|FT_OPEN_BY_SERIAL_NUMBER);
        if (status < FT_OK)
            return -2;

        ComboBox_AddString(hCombo, szDevice);
    }
    return dwNumDevs;
}

class CMainDlg : public CAppDialog
{
protected:
    HWND m_hComboPort;
    FT_HANDLE m_hDevice;
    BOOL m_bLog;
	DWORD m_dwThreadId;
	UINT m_uRepeat;
	size_t m_uHeaderBytes;
	size_t m_uTrailerBytes;
	size_t m_uLengthMSB;
	size_t m_uLengthLSB;
public:
    CMainDlg(HINSTANCE hInst);
    ~CMainDlg();


    virtual BOOL OnInitDialog(WPARAM wParam, LPARAM lParam);
//    virtual BOOL OnDrawItem(WPARAM wParam, LPDRAWITEMSTRUCT lParam);
//    virtual BOOL OnNotify(WPARAM wId, LPNMHDR nmhdr);
    virtual BOOL OnCommand(WPARAM wId);

    void UpdateUI();

    void UpdateBitBang(void);
    void UpdateCBUS(void);
    void FillCombo(LPCTSTR pszFilename);
    void GetCellRect(int col, LPCRECT pItemRect, LPRECT pCellRect);
};

// Constructor
CMainDlg::CMainDlg(HINSTANCE hInst) : CAppDialog(hInst)
{
    m_hComboPort = NULL;
    m_hDevice = INVALID_HANDLE_VALUE;
}

// Destructor
CMainDlg::~CMainDlg()
{
    if (W32_CloseDevice(m_hDevice))
    {
        m_hDevice = INVALID_HANDLE_VALUE;
    }
}

BOOL CMainDlg::OnInitDialog(WPARAM wParam, LPARAM lParam)
{
    // Set icons
    // Attach icon to main dialog
    HICON hIcon = LoadIcon(m_hInst, MAKEINTRESOURCE (IDD_MAIN));
    SendMessage(m_hWnd, WM_SETICON, TRUE, (LPARAM) hIcon);
    SendMessage (m_hWnd, WM_SETICON, FALSE, (LPARAM) hIcon);

    m_hComboPort = GetDlgItem(m_hWnd, IDC_PORT);

	GetModuleFileName(NULL, szProfile, MAX_PATH);
	int len = lstrlen(szProfile);
	if (szProfile[len - 4] == '.')
	{
		lstrcpy(szProfile + len - 3, "ini");
	}

    // Fill in USB device names
    if (QueryCOMPorts(m_hComboPort) > 0)
    {
        ComboBox_SetCurSel(m_hComboPort, 0);
    }

    UpdateUI();
    return TRUE;
}

void CMainDlg::UpdateUI()
{
    TCHAR szText[STR_MAX];
    UINT uId = (m_hDevice == INVALID_HANDLE_VALUE) ? IDS_OPEN : IDS_CLOSE;
    if (LoadString(m_hInst, uId, szText, STR_MAX))
    {
        SetDlgItemText(m_hWnd, IDC_OPEN_CLOSE, szText);
    }
}

void CMainDlg::UpdateBitBang(void)
{
    BYTE bits;
    bits = 0;
    if (IsDlgButtonChecked(m_hWnd, IDC_BIT_D0))
        bits |= 0x01;
    if (IsDlgButtonChecked(m_hWnd, IDC_BIT_D1))
        bits |= 0x02;
    if (IsDlgButtonChecked(m_hWnd, IDC_BIT_D2))
        bits |= 0x04;
    if (IsDlgButtonChecked(m_hWnd, IDC_BIT_D3))
        bits |= 0x08;
    if (IsDlgButtonChecked(m_hWnd, IDC_BIT_D4))
        bits |= 0x10;
    if (IsDlgButtonChecked(m_hWnd, IDC_BIT_D5))
        bits |= 0x20;
    if (IsDlgButtonChecked(m_hWnd, IDC_BIT_D6))
        bits |= 0x40;
    if (IsDlgButtonChecked(m_hWnd, IDC_BIT_D7))
        bits |= 0x80;
    TRACE1("UpdateBitBang(%02x)\n", bits);
    DWORD dwWritten = 0;
    // Set BITBANG Mode (all outputs)
    int status = FT_SetBitMode(m_hDevice, 0xFF, ASYNC_BITBANG);
    if (status == FT_OK)
    {
        FT_Write(m_hDevice, &bits, 1, &dwWritten);
    }
}

void CMainDlg::UpdateCBUS(void)
{
    BYTE bits;
    bits = 0;
    if (IsDlgButtonChecked(m_hWnd, IDC_BIT_C0))
        bits |= 0x01;
    if (IsDlgButtonChecked(m_hWnd, IDC_BIT_C1))
        bits |= 0x02;
    if (IsDlgButtonChecked(m_hWnd, IDC_BIT_C2))
        bits |= 0x04;
    if (IsDlgButtonChecked(m_hWnd, IDC_BIT_C3))
        bits |= 0x08;
    TRACE1("UpdateCBUS(%02x)\n", bits);
    DWORD dwWritten = 0;

    // Set CBUS (4 bit output)
    int status = FT_SetBitMode(m_hDevice, bits | 0xF0, CBUS_BITBANG);
    if (status == FT_OK)
    {
        FT_Write(m_hDevice, &bits, 1, &dwWritten);
    }
}

BOOL CMainDlg::OnCommand(WPARAM wId)
{
    WORD code = HIWORD(wId);
    WORD id = LOWORD(wId);
    TRACE2("OnCommand(code=%04x,id=%u\n", code, id);

    switch (id)
    {
    case IDC_BIT_D0:
    case IDC_BIT_D1:
    case IDC_BIT_D2:
    case IDC_BIT_D3:
    case IDC_BIT_D4:
    case IDC_BIT_D5:
    case IDC_BIT_D6:
    case IDC_BIT_D7:
        UpdateBitBang();
        break;
    case IDC_BIT_C0:
    case IDC_BIT_C1:
    case IDC_BIT_C2:
    case IDC_BIT_C3:
        UpdateCBUS();
        break;
    case IDC_OPEN_CLOSE:
        if (m_hDevice != INVALID_HANDLE_VALUE)
        {
            if (W32_CloseDevice(m_hDevice))
            {
                m_hDevice = INVALID_HANDLE_VALUE;
            }
        }
        else
        {
            TCHAR szPort[STR_MAX];
            if (GetDlgItemText(m_hWnd, IDC_PORT, szPort, STR_MAX))
            {
                m_hDevice = W32_OpenDevice(szPort, 9600);
                if (m_hDevice != INVALID_HANDLE_VALUE)
                {
                    UpdateCBUS();
                    UpdateBitBang();
                }
            }
        }
        UpdateUI();
        break;
    }
    return TRUE;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
    CMainDlg dlg(hInst);
 
    if (dlg.CreateDialogBox(MAKEINTRESOURCE(IDD_MAIN), /*hParent=*/NULL))
    {
        dlg.ShowWindow(nCmdShow);
        return dlg.Run();
    }
    return FALSE;
}
