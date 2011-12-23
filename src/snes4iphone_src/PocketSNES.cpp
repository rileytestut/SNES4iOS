// PocketSNES.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "PocketSNES.h"

#include <commctrl.h>
#include <aygshell.h>
#include <sipapi.h>
#ifdef _WIN32_WCE_EMULATION
//#include "gapi.h"
#else
#include "gx.h"
#endif

#include "snes9x.h"
#include "pocketpc.h"
#include "VOIMAGE.H"
#include "Prof/profiler.h"

#define MAX_LOADSTRING 100

extern bool InitializeEmulation(int,int,int);
extern bool LoadROM();
extern bool StartEmulation(HWND, HWND);
extern bool StopEmulation(HWND, HWND);
extern bool PauseEmulation(HWND, HWND);
extern bool ResumeEmulation(HWND, HWND);

extern "C" void S9xReset();
extern "C" void S9xSetTitle(const char *);
extern "C" void S9xMainLoop(void);

extern bool g_bResumeAfterLoadState;
extern bool g_bResumeAfterSaveState;

extern TCHAR		g_szTitle[64];
extern GXKeyList	g_gxkl;

class Skin
{
public:
    TCHAR            *pszBitmap;
    TCHAR            *pszName;
    TCHAR            *pszAuthor;
    short             iVersion;
    short             iNumberOfColors;
    unsigned short    iColor[32];
    unsigned short    iMask[32];
    bool              bResource;
    bool              bLandscape;

    Skin()
        : pszBitmap(NULL), pszName(NULL), pszAuthor(NULL), iNumberOfColors(0), iVersion(0),
          bResource(false), bLandscape(false)
    {
    }

    ~Skin()
    {
        if (pszBitmap)
            delete [] pszBitmap;
        if (pszName)
            delete [] pszName;
        if (pszAuthor)
            delete [] pszAuthor;
    }

    void AddColor(DWORD _dwColor, DWORD _dwMask)
    {
        iColor[iNumberOfColors] = ((_dwColor & 0xf80000) >> 8)|((_dwColor & 0xfc00) >> 5)|((_dwColor & 0xf8) >> 3);
        iMask[iNumberOfColors]  = _dwMask;

        iNumberOfColors++;
    }

    void AddColor(TCHAR *_pszColor, TCHAR *_pszMask)
    {
        DWORD   dwColor = 0;
        TCHAR  *pszTemp = _pszColor;

        pszTemp += 2;

        dwColor = TextToInt(pszTemp);
        iColor[iNumberOfColors] = ((dwColor & 0xf80000) >> 8)|((dwColor & 0xfc00) >> 5)|((dwColor & 0xf8) >> 3);
        iMask[iNumberOfColors]  = DecodeMask(_pszMask);

        iNumberOfColors++;
    }

    void SetBitmap(TCHAR *_pszBitmap)
    {
        pszBitmap = new TCHAR [_tcslen(_pszBitmap) + 1];

        _tcscpy(pszBitmap, _pszBitmap);
    }
    
    void SetName(TCHAR *_pszName)
    {
        pszName = new TCHAR [_tcslen(_pszName) + 1];

        _tcscpy(pszName, _pszName);
    }

    void SetAuthor(TCHAR *_pszAuthor)
    {
        static TCHAR *szAuthor = L"Designed by: ";

        pszAuthor = new TCHAR [_tcslen(_pszAuthor) + 1 + _tcslen(szAuthor)];

        _tcscpy(pszAuthor, szAuthor);
        _tcscat(pszAuthor, _pszAuthor);
    }

    void LoadBitmap(HBITMAP &_hbmSkin, HDC &_hSkinDC, HINSTANCE &_hInstance, HWND &_hWnd)
    {
        if (_hbmSkin)
            DeleteObject(_hbmSkin);
        if (_hSkinDC)
            DeleteObject(_hSkinDC);

        HDC hDC = GetDC(_hWnd);

        if (bResource)
        {
            _hbmSkin = ::LoadBitmap(_hInstance, pszBitmap);
        }
        else
        {
            _hbmSkin = ::SHLoadDIBitmap(pszBitmap);
        }

        if (_hbmSkin)
        {
            _hSkinDC = CreateCompatibleDC(hDC);

	        SelectObject(_hSkinDC, _hbmSkin);
        }

        ReleaseDC(_hWnd, hDC);
    }

private:

    DWORD TextToInt(TCHAR *_pszIn)
    {
        DWORD dwReturn = 0;

        for (int i = 0; i < (int) _tcslen(_pszIn); i++)
        {
            if ((_pszIn[i] >= _T('0')) && (_pszIn[i] <= _T('9')))
            {
                dwReturn = (dwReturn * 16) + (_pszIn[i] - _T('0'));
            }
            else if ((_pszIn[i] >= _T('A')) && (_pszIn[i] <= _T('F')))
            {
                dwReturn = (dwReturn * 16) + (_pszIn[i] - _T('A') + 10);
            }
            else if ((_pszIn[i] >= _T('a')) && (_pszIn[i] <= _T('f')))
            {
                dwReturn = (dwReturn * 16) + (_pszIn[i] - _T('a') + 10);
            }
        }

        return dwReturn;
    }

    unsigned short DecodeMask(TCHAR *_pszIn)
    {
        unsigned short iReturn = 0;

        TCHAR *pszToken = _tcstok(_pszIn, L"+");

        while (pszToken != NULL)
        {
            if (_tcsicmp(pszToken, L"n") == 0)
            {
                iReturn |= SNES_UP_MASK;
            }
            else if (_tcsicmp(pszToken, L"s") == 0)
            {
                iReturn |= SNES_DOWN_MASK;
            }
            else if (_tcsicmp(pszToken, L"e") == 0)
            {
                iReturn |= SNES_RIGHT_MASK;
            }
            else if (_tcsicmp(pszToken, L"w") == 0)
            {
                iReturn |= SNES_LEFT_MASK;
            }
            else if (_tcsicmp(pszToken, L"ne") == 0)
            {
                iReturn |= SNES_UP_MASK|SNES_RIGHT_MASK;
            }
            else if (_tcsicmp(pszToken, L"se") == 0)
            {
                iReturn |= SNES_DOWN_MASK|SNES_RIGHT_MASK;
            }
            else if (_tcsicmp(pszToken, L"sw") == 0)
            {
                iReturn |= SNES_DOWN_MASK|SNES_LEFT_MASK;
            }
            else if (_tcsicmp(pszToken, L"nw") == 0)
            {
                iReturn |= SNES_UP_MASK|SNES_LEFT_MASK;
            }
            else if (_tcsicmp(pszToken, L"a") == 0)
            {
                iReturn |= SNES_A_MASK;
            }
            else if (_tcsicmp(pszToken, L"b") == 0)
            {
                iReturn |= SNES_B_MASK;
            }
            else if (_tcsicmp(pszToken, L"x") == 0)
            {
                iReturn |= SNES_X_MASK;
            }
            else if (_tcsicmp(pszToken, L"y") == 0)
            {
                iReturn |= SNES_Y_MASK;
            }
            else if (_tcsicmp(pszToken, L"l") == 0)
            {
                iReturn |= SNES_TL_MASK;
            }
            else if (_tcsicmp(pszToken, L"r") == 0)
            {
                iReturn |= SNES_TR_MASK;
            }
            else if (_tcsicmp(pszToken, L"start") == 0)
            {
                iReturn |= SNES_START_MASK;
            }
            else if (_tcsicmp(pszToken, L"select") == 0)
            {
                iReturn |= SNES_SELECT_MASK;
            }

            pszToken = _tcstok(NULL, L"+");
        }

        return iReturn;
    }
};

class Keymap
{
public:
    DWORD   dwKeyMask[256];

    Keymap()
    {
        memset(dwKeyMask, 0, sizeof(DWORD) * 256);
    }

    short GetKeyFromMask(DWORD dwMask)
    {
        for (short i = 0; i < 256; i++)
        {
            if (dwKeyMask[i] == dwMask)
                return i;

        }

        return 0;
    }

    LPTSTR GetKeyDisplayFromMask(DWORD dwMask)
    {
        static TCHAR szDisplay[10];

        if (dwMask == 0)
        {
            _tcscpy(szDisplay, L"...");
        }
        else
        {
            short   iKey = GetKeyFromMask(dwMask);

            if (iKey == 0)
            {
                _tcscpy(szDisplay, L"...");
            }
            else
            {
                wsprintf(szDisplay, L"0x%2x", iKey);
            }
        }

        return szDisplay;
    }

    void SetLandscape(Keymap &_kmPortrait)
    {
        for (short i = 0; i < 256; i++)
        {
            dwKeyMask[i] = _kmPortrait.dwKeyMask[i];

            short iUp    = _kmPortrait.GetKeyFromMask(SNES_UP_MASK);
            short iDown  = _kmPortrait.GetKeyFromMask(SNES_DOWN_MASK);
            short iLeft  = _kmPortrait.GetKeyFromMask(SNES_LEFT_MASK);
            short iRight = _kmPortrait.GetKeyFromMask(SNES_RIGHT_MASK);

            dwKeyMask[iUp]    = SNES_RIGHT_MASK;
            dwKeyMask[iRight] = SNES_DOWN_MASK;
            dwKeyMask[iDown]  = SNES_LEFT_MASK;
            dwKeyMask[iLeft]  = SNES_UP_MASK;
        }
    }

    void SetLandscapeRight(Keymap &_kmPortrait)
    {
        for (short i = 0; i < 256; i++)
        {
            dwKeyMask[i] = _kmPortrait.dwKeyMask[i];

            short iUp    = _kmPortrait.GetKeyFromMask(SNES_UP_MASK);
            short iDown  = _kmPortrait.GetKeyFromMask(SNES_DOWN_MASK);
            short iLeft  = _kmPortrait.GetKeyFromMask(SNES_LEFT_MASK);
            short iRight = _kmPortrait.GetKeyFromMask(SNES_RIGHT_MASK);

            dwKeyMask[iUp]    = SNES_LEFT_MASK;
            dwKeyMask[iRight] = SNES_UP_MASK;
            dwKeyMask[iDown]  = SNES_RIGHT_MASK;
            dwKeyMask[iLeft]  = SNES_DOWN_MASK;
        }
    }

    DWORD GetStaticIDFromMask(DWORD dwMask)
    {
        switch (dwMask)
        {
            case SNES_UP_MASK:
                return IDC_STATIC_UP;

            case SNES_DOWN_MASK:
                return IDC_STATIC_DOWN;

            case SNES_LEFT_MASK:
                return IDC_STATIC_LEFT;

            case SNES_RIGHT_MASK:
                return IDC_STATIC_RIGHT;

            case SNES_A_MASK:
                return IDC_STATIC_A;

            case SNES_B_MASK:
                return IDC_STATIC_B;

            case SNES_X_MASK:
                return IDC_STATIC_X;

            case SNES_Y_MASK:
                return IDC_STATIC_Y;

            case SNES_START_MASK:
                return IDC_STATIC_START;

            case SNES_SELECT_MASK:
                return IDC_STATIC_SELECT;

            case SNES_TL_MASK:
                return IDC_STATIC_L;

            case SNES_TR_MASK:
                return IDC_STATIC_R;
        }

        return 0;
    }
};

//------------------------------------------------------------------------------
// Global Variables
//------------------------------------------------------------------------------
bool				g_bLoop			= true;
HWND				g_hWnd;
HINSTANCE			g_hInstance;			// The current instance
HWND				hwndCB;					// The command bar handle
HFONT				g_hFontBold;
HFONT				g_hFontNormal;
HFONT				g_hFontPaused;
HFONT				g_hFontHyperlink;
CVOImage			g_pngLogo;
HDC                 g_hLogoDC;
static TCHAR		g_sRootKey[256];
Skin               *g_pSkins        = NULL;
int                 g_iSkinCount    = 0;
int                 g_iSelectedSkin = 0;
HBITMAP             g_hbmSkin       = NULL;
HDC                 g_hSkinDC       = NULL;
Keymap              g_kmDefault;
Keymap              g_kmCurrent;
Keymap              g_kmLandscape;
Keymap             *g_pkmInUse;
GXKeyList           g_gxKeyList;
bool                g_bSetButtonMode;
int                 g_iSetButton;
RECT                g_rtLink;
HICON               g_hiLeftArrow;
HICON               g_hiRightArrow;
uint32              g_iFrameSkip;

//------------------------------------------------------------------------------
// Function definitions
//------------------------------------------------------------------------------
bool                LoadOptions();
void                SaveOptions();
bool                LoadSkins();
void                SaveSkins();
bool                CheckKeyPad(int, int, bool);
void                ShowOptions();
LRESULT CALLBACK	SystemDlgProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	DisplayDlgProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	SoundDlgProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	SkinsDlgProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	KeysDlgProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	CreditsDlgProc(HWND, UINT, WPARAM, LPARAM);

ATOM				MyRegisterClass	(HINSTANCE, LPTSTR);
BOOL				InitInstance	(HINSTANCE, int);
LRESULT CALLBACK	WndProc			(HWND, UINT, WPARAM, LPARAM);
HWND				CreateRpCommandBar(HWND);


//------------------------------------------------------------------------------
// Joypad definitions
//      Eight directions
//      Eight buttons - start, select, A, B, X, Y, L, R
//------------------------------------------------------------------------------
#define KP_JOYPAD_N         0
#define KP_JOYPAD_E         1
#define KP_JOYPAD_S         2
#define KP_JOYPAD_W         3
#define KP_JOYPAD_NE        4
#define KP_JOYPAD_SE        5
#define KP_JOYPAD_SW        6
#define KP_JOYPAD_NW        7
#define KP_BUTTON_START     8
#define KP_BUTTON_SELECT    9
#define KP_BUTTON_A         10
#define KP_BUTTON_B         11
#define KP_BUTTON_X         12
#define KP_BUTTON_Y         13
#define KP_BUTTON_L         14
#define KP_BUTTON_R         15



int WINAPI WinMain(	HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPTSTR    lpCmdLine,
					int       nCmdShow)
{
	MSG msg;
	HACCEL hAccelTable;

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	if (!InitializeEmulation(Settings.Transparency, Settings.APUEnabled, Settings.SixteenBitSound))
	{
        MessageBox(NULL, L"PocketSNES needs more memory", NULL, MB_OK);
		return FALSE;
	}

	S9xSetTitle("");
    SetKeypad();

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_POCKETSNES);

	// Main message loop:
	while (g_bLoop) 
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		if (g_emMode == emRunning)
		{
#ifdef THREADCPU
			Sleep(0);
#else
			S9xMainLoop();
#endif
		}
	}

    SaveOptions();
#ifdef __PROFILER
	__profiler_dump();
#endif
	return msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    It is important to call this function so that the application 
//    will get 'well formed' small icons associated with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance, LPTSTR szWindowClass)
{
	WNDCLASS	wc;

    wc.style			= CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc		= (WNDPROC) WndProc;
    wc.cbClsExtra		= 0;
    wc.cbWndExtra		= 0;
    wc.hInstance		= hInstance;
    wc.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_POCKETSNES));
    wc.hCursor			= 0;
    wc.hbrBackground	= (HBRUSH) GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName		= 0;
    wc.lpszClassName	= szWindowClass;

	return RegisterClass(&wc);
}

//
//  FUNCTION: InitInstance(HANDLE, int)
//
//  PURPOSE: Saves instance handle and creates main window
//
//  COMMENTS:
//
//    In this function, we save the instance handle in a global variable and
//    create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND	hWnd = NULL;
	TCHAR	szTitle[MAX_LOADSTRING];			// The title bar text
	TCHAR	szWindowClass[MAX_LOADSTRING];		// The window class name

	g_hInstance = hInstance;		// Store instance handle in our global variable

	// Initialize global strings
	LoadString(hInstance, IDC_POCKETSNES, szWindowClass, MAX_LOADSTRING);
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);

	//If it is already running, then focus on the window
	hWnd = FindWindow(szWindowClass, szTitle);	
	if (hWnd) 
	{
		SetForegroundWindow ((HWND) (((DWORD)hWnd) | 0x01));    
		return FALSE;
	} 

	MyRegisterClass(hInstance, szWindowClass);
	
	RECT	rect;
	GetClientRect(hWnd, &rect);
	
	hWnd = CreateWindow(szWindowClass, szTitle, WS_VISIBLE,
		                CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                        NULL, NULL, hInstance, NULL);
	if (!hWnd)
	{	
        MessageBox(NULL, L"PocketSNES failed to initialize", NULL, MB_OK);
		return FALSE;
	}

	//When the main window is created using CW_USEDEFAULT the height of the menubar (if one
	// is created is not taken into account). So we resize the window after creating it
	// if a menubar is present
	{
		RECT rc;
		GetWindowRect(hWnd, &rc);
		rc.bottom -= MENU_HEIGHT;
		if (hwndCB)
			MoveWindow(hWnd, rc.left, rc.top, rc.right, rc.bottom, FALSE);
	}

	LOGFONT	lfFont;
	TCHAR   szFontName[32] = L"Tahoma";
	HDC		hDC = GetDC(hWnd);

	memset(&lfFont, 0, sizeof(LOGFONT));
	wcscpy(lfFont.lfFaceName, szFontName);
	lfFont.lfWeight = FW_BOLD;
	lfFont.lfHeight = (-12 * GetDeviceCaps(hDC, LOGPIXELSY)) / 72;
 	g_hFontBold = CreateFontIndirect(&lfFont);

	lfFont.lfWeight = FW_NORMAL;
	lfFont.lfHeight = (-10 * GetDeviceCaps(hDC, LOGPIXELSY)) / 72;
	g_hFontNormal = CreateFontIndirect(&lfFont);

	lfFont.lfUnderline = TRUE;
	lfFont.lfHeight = (-10 * GetDeviceCaps(hDC, LOGPIXELSY)) / 72;
	g_hFontHyperlink = CreateFontIndirect(&lfFont);

	lfFont.lfUnderline = FALSE;
	lfFont.lfWeight = FW_BOLD;
	g_hFontPaused = CreateFontIndirect(&lfFont);
	
    g_hLogoDC = CreateCompatibleDC(hDC);

	g_pngLogo.SetBitmap(g_hLogoDC, IDB_POCKETSNES, TEXT("IMAGE"), GetModuleHandle(NULL));
	SelectObject(g_hLogoDC, (HBITMAP)g_pngLogo);

    _tcscpy(g_sRootKey, _T("SOFTWARE\\Apps\\PocketSNES"));
    LoadOptions();

    g_hiLeftArrow  = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LEFT_ARROW));
    g_hiRightArrow = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_RIGHT_ARROW));

    LoadSkins();
    g_pSkins[g_iSelectedSkin].LoadBitmap(g_hbmSkin, g_hSkinDC, g_hInstance, g_hWnd);

	ReleaseDC(hWnd, hDC);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	g_hWnd = hWnd;

	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	TCHAR szHello[MAX_LOADSTRING];

	switch (message) 
	{
		case WM_COMMAND:
			{
				int wmId	= LOWORD(wParam); 
				int	wmEvent = HIWORD(wParam);

					// Parse the menu selections:
				switch (wmId)
				{	
					case IDM_TOOLS_LOAD:
						//CSNES
                        //StopEmulation(hWnd, hwndCB);
						if (LoadROM())  //hack to keep from crashing if no rom
						{
							StopEmulation(hWnd, hwndCB);
							LoadOptions();
							StartEmulation(hWnd, hwndCB);
						}
						break;

					case IDM_OPTIONS_RESET:
						if(g_emMode != emStopped)
						{
							StopEmulation(hWnd, hwndCB);
							S9xReset();
							StartEmulation(hWnd, hwndCB);
						}
						break;

					case IDM_OPTIONS_DISPLAY:
						DialogBox(g_hInstance, (LPCTSTR)IDD_DISPLAY, hWnd, (DLGPROC)DisplayDlgProc);
					    SaveOptions();
                        break;

					case IDM_OPTIONS_SOUND:
						DialogBox(g_hInstance, (LPCTSTR)IDD_SOUND, hWnd, (DLGPROC)SoundDlgProc);
					    SaveOptions();
                        break;

					case IDM_OPTIONS_SYSTEM:
						DialogBox(g_hInstance, (LPCTSTR)IDD_SYSTEM, hWnd, (DLGPROC)SystemDlgProc);
					    SaveOptions();
                        break;

                    case IDM_OPTIONS_KEYS:
						DialogBox(g_hInstance, (LPCTSTR)IDD_KEYS, hWnd, (DLGPROC)KeysDlgProc);
                        break;

                    case IDM_TOOLS_SKINS:
						DialogBox(g_hInstance, (LPCTSTR)IDD_SKINS, hWnd, (DLGPROC)SkinsDlgProc);
                        break;

                    case IDM_TOOLS_CREDITS:
						DialogBox(g_hInstance, (LPCTSTR)IDD_CREDITS, hWnd, (DLGPROC)CreditsDlgProc);
                        break;

                    case IDM_TOOLS_EXIT:
						StopEmulation(hWnd, hwndCB);
                        SendMessage(hWnd, WM_ACTIVATE, MAKEWPARAM(WA_INACTIVE, 0), (LPARAM)hWnd);
						SendMessage(hWnd, WM_CLOSE, 0, 0);                        
						g_bLoop = false;
						break;

                    case IDM_TOOLS_LOADSTATE:
                        LoadState();
						//StartEmulation(hWnd, hwndCB);
                        break;
					case IDM_TOOLS_LOAD1:
                        LoadSlot(0);
                        break;
					case IDM_TOOLS_LOAD2:
                        LoadSlot(1);
                        break;
					case IDM_TOOLS_LOAD3:
                        LoadSlot(2);
                        break;
					case IDM_TOOLS_LOAD4:
                        LoadSlot(3);
                        break;
					case IDM_TOOLS_LOAD5:
                        LoadSlot(4);
                        break;
					case IDM_TOOLS_LOAD6:
                        LoadSlot(5);
                        break;
					case IDM_TOOLS_LOAD7:
                        LoadSlot(6);
                        break;
					case IDM_TOOLS_LOAD8:
                        LoadSlot(7);
                        break;
					case IDM_TOOLS_LOAD9:
                        LoadSlot(8);
                        break;

                    case IDM_TOOLS_SAVESTATE:
                        SaveState();
                        break;
					case IDM_TOOLS_SAVE1:
                        SaveSlot(0);
                        break;
					case IDM_TOOLS_SAVE2:
                        SaveSlot(1);
                        break;
					case IDM_TOOLS_SAVE3:
                        SaveSlot(2);
                        break;
					case IDM_TOOLS_SAVE4:
                        SaveSlot(3);
                        break;
					case IDM_TOOLS_SAVE5:
                        SaveSlot(4);
                        break;
					case IDM_TOOLS_SAVE6:
                        SaveSlot(5);
                        break;
					case IDM_TOOLS_SAVE7:
                        SaveSlot(6);
                        break;
					case IDM_TOOLS_SAVE8:
                        SaveSlot(7);
                        break;
					case IDM_TOOLS_SAVE9:
                        SaveSlot(8);
                        break;

					case IDOK:
						SendMessage(hWnd, WM_ACTIVATE, MAKEWPARAM(WA_INACTIVE, 0), (LPARAM)hWnd);
						SendMessage(hWnd, WM_CLOSE, 0, 0);
						break;

					default:
					   return DefWindowProc(hWnd, message, wParam, lParam);
				}
			}
			break;

		case WM_CREATE:
			hwndCB = CreateRpCommandBar(hWnd);
			break;

        case WM_KILLFOCUS:
			GXSuspend();
			break;

		case WM_SETFOCUS:
			GXResume();
			break;

		case WM_PAINT:
            if (g_emMode == emStopped)
			{
				RECT		rtTop, rtBottom;				
				PAINTSTRUCT ps;
				HDC			hDC = BeginPaint(hWnd, &ps);

				GetClientRect(hWnd, &rtTop);
				rtBottom        = rtTop;
                g_rtLink        = rtTop;
				rtBottom.top    = ((rtBottom.bottom - rtBottom.top) / 2) + rtBottom.top + 36;
                rtBottom.bottom = rtBottom.top + 48;
                g_rtLink.top    = rtBottom.bottom;
				rtTop.bottom    = rtBottom.top;
                rtTop.top       = rtTop.bottom - 36;

				BitBlt(hDC, 0, 8, 240, 113, g_hLogoDC, 0, 0, SRCCOPY);

				LoadString(g_hInstance, IDS_HELLO, szHello, MAX_LOADSTRING);
				SelectObject(hDC, g_hFontBold);
				SetTextColor(hDC, COLORREF(0x00ff0000));
				DrawText(hDC, szHello, _tcslen(szHello), &rtTop, DT_CENTER);
				
				LoadString(g_hInstance, IDS_HELLO2, szHello, MAX_LOADSTRING);
				SelectObject(hDC, g_hFontNormal);
				SetTextColor(hDC, COLORREF(0x00000000));
				DrawText(hDC, szHello, _tcslen(szHello), &rtBottom, DT_CENTER);

				LoadString(g_hInstance, IDS_LINK, szHello, MAX_LOADSTRING);
				SelectObject(hDC, g_hFontHyperlink);
				SetTextColor(hDC, COLORREF(0x00ff0000));
				DrawText(hDC, szHello, _tcslen(szHello), &g_rtLink, DT_CENTER);
				EndPaint(hWnd, &ps);
			}
			else if (g_emMode == emPaused)
			{
				PAINTSTRUCT ps;
				HDC			hDC = BeginPaint(hWnd, &ps);
				RECT		rt  = { 0, 224, 240, 320 - 52 };

				BitBlt(hDC, 0, 0, 240, 224, g_hPausedDC, 0, 0, SRCCOPY);

				LoadString(g_hInstance, IDS_PAUSED, szHello, MAX_LOADSTRING);
				SelectObject(hDC, g_hFontPaused);
				SetTextColor(hDC, COLORREF(0x000000ff));
				DrawText(hDC, szHello, _tcslen(szHello), &rt, DT_CENTER|DT_SINGLELINE|DT_VCENTER);

				EndPaint(hWnd, &ps);
			}
			break;


		case WM_LBUTTONDOWN:
            {
				POINT       ptClick;

				ptClick.x = LOWORD(lParam);
				ptClick.y = HIWORD(lParam);

			    if (g_emMode == emRunning)
			    {
				    static RECT rtScreen = { 0, 0, 240, 180 };

                    if (!CheckKeyPad(ptClick.x, ptClick.y, true))
                    {
				        if (PtInRect(&rtScreen, ptClick))
				        {
					        PauseEmulation(hWnd, hwndCB);
				        }
                    }
			    }
			    else if (g_emMode == emPaused)
			    {
				    static RECT rtScreen = { 0, 26, 240, 320 - 26 };

				    if (PtInRect(&rtScreen, ptClick))
				    {
					    ResumeEmulation(hWnd, hwndCB);
				    }
			    }
                else if (PtInRect(&g_rtLink, ptClick))
                {
                    SHELLEXECUTEINFO  seInfo;
                    TCHAR            *szCommand = _T("\\Windows\\iexplore.exe");
                    TCHAR            *szURL     = _T("http://www.pocketsnes.com");

                    memset(&seInfo, 0, sizeof(SHELLEXECUTEINFO));
                    seInfo.cbSize       = sizeof(SHELLEXECUTEINFO);
                    seInfo.lpFile       = szCommand;
                    seInfo.lpParameters = szURL;
                    seInfo.nShow        = SW_SHOW;

                    ShellExecuteEx(&seInfo);
                }
            }
			break;

        case WM_LBUTTONUP:
            if (g_emMode == emRunning)
            {
                CheckKeyPad(LOWORD(lParam), HIWORD(lParam), false);
            }
            break;

        case WM_KEYDOWN:
            g_iJoypadState |= g_pkmInUse->dwKeyMask[(short) wParam];
			break;

        case WM_KEYUP:
            g_iJoypadState &= ~(g_pkmInUse->dwKeyMask[(short) wParam]);
			break;

        case WM_DESTROY:
            StopEmulation(hWnd, hwndCB);
			CommandBar_Destroy(hwndCB);
			PostQuitMessage(0);
			g_bLoop = false;
			break;

		case WM_SETTINGCHANGE:
            {
                SHACTIVATEINFO  shaInfo;

			    SHHandleWMSettingChange(hWnd, wParam, lParam, &shaInfo);
            }
     		break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }

   return 0;
}

HWND CreateRpCommandBar(HWND hwnd)
{
	SHMENUBARINFO mbi;

	memset(&mbi, 0, sizeof(SHMENUBARINFO));
	mbi.cbSize     = sizeof(SHMENUBARINFO);
	mbi.hwndParent = hwnd;
	mbi.nToolBarId = IDM_MENU;
	mbi.hInstRes   = g_hInstance;
	mbi.nBmpId     = 0;
	mbi.cBmpImages = 0;

	if (!SHCreateMenuBar(&mbi)) 
		return NULL;

	return mbi.hwndMB;
}

// Message handler for the options dialog
LRESULT CALLBACK OptionsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
            {
                SendMessage(GetDlgItem(hDlg, IDC_TRANSPARENCY), BM_SETCHECK, (WPARAM) (Settings.Transparency == TRUE)?BST_CHECKED:BST_UNCHECKED, 0);
                SendMessage(GetDlgItem(hDlg, IDC_LANDSCAPE), BM_SETCHECK, (WPARAM) (g_bLandscape)?BST_CHECKED:BST_UNCHECKED, 0);
                SendMessage(GetDlgItem(hDlg, IDC_LEFT), BM_SETCHECK, (WPARAM) (g_bLandLeft)?BST_CHECKED:BST_UNCHECKED, 0);
                SendMessage(GetDlgItem(hDlg, IDC_COMPAT), BM_SETCHECK, (WPARAM) (g_bCompat)?BST_CHECKED:BST_UNCHECKED, 0);
                SendMessage(GetDlgItem(hDlg, IDC_AUTO), BM_SETCHECK, (WPARAM) (g_bAutoSkip)?BST_CHECKED:BST_UNCHECKED, 0);
                SendMessage(GetDlgItem(hDlg, IDC_SMOOTHSTRETCH), BM_SETCHECK, (WPARAM) (g_bSmoothStretch)?BST_CHECKED:BST_UNCHECKED, 0);
                SetDlgItemInt(hDlg, IDC_FRAMESKIP, g_iFrameSkip, false);
                SetDlgItemInt(hDlg, IDC_CYCLES, g_iCycles, false);
				SendMessage(GetDlgItem(hDlg, IDC_FRAMESKIP), UDM_SETRANGE32, (WPARAM) 1, (LPARAM) 20);
                SendMessage(GetDlgItem(hDlg, IDC_CYCLES), UDM_SETRANGE32, (WPARAM) 1, (LPARAM) 20);
                SendMessage(GetDlgItem(hDlg, IDC_SOUND), BM_SETCHECK, (WPARAM) (Settings.APUEnabled == TRUE)?BST_CHECKED:BST_UNCHECKED, 0);
                SendMessage(GetDlgItem(hDlg, IDC_SIXTEENBIT), BM_SETCHECK, (WPARAM) (Settings.SixteenBitSound == TRUE)?BST_CHECKED:BST_UNCHECKED, 0);
                SendMessage(GetDlgItem(hDlg, IDC_EIGHTBIT), BM_SETCHECK, (WPARAM) (Settings.SixteenBitSound == FALSE)?BST_CHECKED:BST_UNCHECKED, 0);
                SendMessage(GetDlgItem(hDlg, IDC_SOUNDQUALITY), TBM_SETRANGE, (WPARAM)(BOOL) TRUE, (LPARAM) MAKELONG(1,3));
                SendMessage(GetDlgItem(hDlg, IDC_SOUNDQUALITY), TBM_SETPOS, (WPARAM)(BOOL) TRUE, (LPARAM) Settings.SoundPlaybackRate);

                SHINITDLGINFO shidi;

				// Create a Done button and size it.
				shidi.dwMask  = SHIDIM_FLAGS;
				shidi.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIZEDLGFULLSCREEN;
				shidi.hDlg    = hDlg;
				
				//initialzes the dialog based on the dwFlags parameter
				SHInitDialog(&shidi);
            }
            return TRUE; 

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK)
            {
                Settings.Transparency      = (BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_TRANSPARENCY), BM_GETCHECK, 0, 0))?TRUE:FALSE;
                g_bLandscape               = (BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_LANDSCAPE), BM_GETCHECK, 0, 0))?true:false;
                g_bLandLeft                = (BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_LEFT), BM_GETCHECK, 0, 0))?true:false;
                g_bCompat                  = (BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_COMPAT), BM_GETCHECK, 0, 0))?true:false;
                g_bAutoSkip                = (BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_AUTO), BM_GETCHECK, 0, 0))?true:false;
                g_bSmoothStretch           = (BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_SMOOTHSTRETCH), BM_GETCHECK, 0, 0))?true:false;
                g_iFrameSkip               = GetDlgItemInt(hDlg, IDC_FRAMESKIP, NULL, false);
                g_iCycles				   = GetDlgItemInt(hDlg, IDC_CYCLES, NULL, false);
				Settings.CyclesPercentage  = g_iCycles;
				Settings.APUEnabled        = (BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_SOUND), BM_GETCHECK, 0, 0))?true:false;
                Settings.SixteenBitSound   = (BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_SIXTEENBIT), BM_GETCHECK, 0, 0))?TRUE:FALSE;
                Settings.SoundPlaybackRate = SendMessage(GetDlgItem(hDlg, IDC_SOUNDQUALITY), TBM_GETPOS, 0, 0);

                EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}
			break;

        case WM_NOTIFY:
            if ((int) wParam == IDC_SPIN_FRAMESKIP)
            {
                LPNMUPDOWN lpnmud = (LPNMUPDOWN) lParam;

                if ((lpnmud->iPos + lpnmud->iDelta) > 20)
                    return TRUE;

                return FALSE;
            }
            break;

        case WM_CTLCOLORSTATIC:
			{
				HDC		hDC  = (HDC) wParam;
				HWND	hctl = (HWND) lParam;
				LOGFONT lf;

				if (GetDlgCtrlID(hctl) != IDC_STATIC_TITLE)
					return DefWindowProc(hDlg, message, wParam, lParam);

				lf.lfWeight			= FW_BOLD;
				lf.lfEscapement		= 0;
				lf.lfOrientation	= 0;
				lf.lfHeight			= 13;
				lf.lfWidth			= 0;
				lf.lfCharSet		= DEFAULT_CHARSET;
				lf.lfClipPrecision	= CLIP_DEFAULT_PRECIS;
				lf.lfItalic			= FALSE;
				lf.lfUnderline		= FALSE;
				lf.lfStrikeOut      = FALSE;
				lf.lfQuality        = DEFAULT_QUALITY;
				lf.lfOutPrecision   = OUT_DEFAULT_PRECIS;
				lf.lfPitchAndFamily = FF_SWISS | DEFAULT_PITCH;

				_tcscpy(lf.lfFaceName, _T("Tahoma"));

				SelectObject(hDC, CreateFontIndirect(&lf));
				SetTextColor(hDC, RGB(0,0,153));

				return (BOOL) GetSysColorBrush(COLOR_STATIC);
			}

        case WM_PAINT:
			{
				HDC         hDC;
				PAINTSTRUCT ps;

				hDC = BeginPaint(hDlg, &ps); // begin painting for window
				
				SelectObject(hDC, GetStockObject(BLACK_BRUSH));
				Rectangle(hDC, 0, 24, 240, 25);

				EndPaint(hDlg, &ps);
			}
			return TRUE;

        default:
			return DefWindowProc(hDlg, message, wParam, lParam);
	}

    return 0;
}

// Message handler for the options dialog
LRESULT CALLBACK DisplayDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
            {
				if(g_bLandscape)
				{
					if(g_bSmoothStretch)
					{
						if(g_bLandLeft)
						{
							SendMessage(GetDlgItem(hDlg, IDC_LANDLEFTSTRETCH), BM_SETCHECK, (WPARAM) (BST_CHECKED), 0);
						}
						else
						{
							SendMessage(GetDlgItem(hDlg, IDC_LANDRIGHTSTRETCH), BM_SETCHECK, (WPARAM) (BST_CHECKED), 0);
						}
					}
					else
					{
						if(g_bLandLeft)
						{
							SendMessage(GetDlgItem(hDlg, IDC_LANDLEFT), BM_SETCHECK, (WPARAM) (BST_CHECKED), 0);
						}
						else
						{
							SendMessage(GetDlgItem(hDlg, IDC_LANDRIGHT), BM_SETCHECK, (WPARAM) (BST_CHECKED), 0);
						}
					}
				}
				else
				{
					SendMessage(GetDlgItem(hDlg, IDC_PORTRAIT), BM_SETCHECK, (WPARAM) (BST_CHECKED), 0);
				}
				

				SendMessage(GetDlgItem(hDlg, IDC_DISPLAYFRAMERATE), BM_SETCHECK, (WPARAM) (Settings.DisplayFrameRate == TRUE)?BST_CHECKED:BST_UNCHECKED, 0);


                SendMessage(GetDlgItem(hDlg, IDC_TRANSPARENCY), BM_SETCHECK, (WPARAM) (Settings.Transparency == TRUE)?BST_CHECKED:BST_UNCHECKED, 0);
                SendMessage(GetDlgItem(hDlg, IDC_AUTO), BM_SETCHECK, (WPARAM) (g_bAutoSkip)?BST_CHECKED:BST_UNCHECKED, 0);
                SetDlgItemInt(hDlg, IDC_FRAMESKIP, g_iFrameSkip, false);
                SetDlgItemInt(hDlg, IDC_CYCLES, g_iCycles, false);
				SendMessage(GetDlgItem(hDlg, IDC_FRAMESKIP_SLIDER), UDM_SETRANGE32, (WPARAM) 0, (LPARAM) 10);
                SendMessage(GetDlgItem(hDlg, IDC_CYCLES), UDM_SETRANGE32, (WPARAM) 1, (LPARAM) 20);
                SHINITDLGINFO shidi;

				// Create a Done button and size it.
				shidi.dwMask  = SHIDIM_FLAGS;
				shidi.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIZEDLGFULLSCREEN;
				shidi.hDlg    = hDlg;
				
				//initialzes the dialog based on the dwFlags parameter
				SHInitDialog(&shidi);
            }
            return TRUE; 

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK)
            {
                Settings.Transparency      = (BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_TRANSPARENCY), BM_GETCHECK, 0, 0))?TRUE:FALSE;
                g_bCompat                  = (BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_COMPAT), BM_GETCHECK, 0, 0))?true:false;
                g_bAutoSkip                = (BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_AUTO), BM_GETCHECK, 0, 0))?true:false;
                //g_bSmoothStretch           = (BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_SMOOTHSTRETCH), BM_GETCHECK, 0, 0))?true:false;
                g_iFrameSkip               = GetDlgItemInt(hDlg, IDC_FRAMESKIP, NULL, false);
                //g_iCycles				   = GetDlgItemInt(hDlg, IDC_CYCLES, NULL, false);
				
				Settings.DisplayFrameRate  = (BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_DISPLAYFRAMERATE), BM_GETCHECK, 0, 0))?TRUE:FALSE;



				//Process DisplaySelection
				//if((BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_PORTRAIT), BM_GETCHECK, 0, 0))?TRUE:FALSE;))
				if(SendMessage(GetDlgItem(hDlg, IDC_PORTRAIT), BM_GETCHECK, 0, 0))
				{
					g_bLandscape = false;
					g_bLandLeft = false;
					g_bSmoothStretch = false;
				}

				if(SendMessage(GetDlgItem(hDlg, IDC_LANDLEFT), BM_GETCHECK, 0, 0))
				{
					g_bLandscape = true;
					g_bLandLeft = true;
					g_bSmoothStretch = false;
				}

				if(SendMessage(GetDlgItem(hDlg, IDC_LANDRIGHT), BM_GETCHECK, 0, 0))
				{
					g_bLandscape = true;
					g_bLandLeft = false;
					g_bSmoothStretch = false;
				}

				if(SendMessage(GetDlgItem(hDlg, IDC_LANDLEFTSTRETCH), BM_GETCHECK, 0, 0))
				{
					g_bLandscape = true;
					g_bLandLeft = true;
					g_bSmoothStretch = true;
				}

				if(SendMessage(GetDlgItem(hDlg, IDC_LANDRIGHTSTRETCH), BM_GETCHECK, 0, 0))
				{
					g_bLandscape = true;
					g_bLandLeft = false;
					g_bSmoothStretch = true;
				}


                EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}
			break;

        case WM_NOTIFY:
            if ((int) wParam == IDC_SPIN_FRAMESKIP)
            {
                LPNMUPDOWN lpnmud = (LPNMUPDOWN) lParam;

                if ((lpnmud->iPos + lpnmud->iDelta) > 20)
                    return TRUE;

                return FALSE;
            }
            break;

        case WM_CTLCOLORSTATIC:
			{
				HDC		hDC  = (HDC) wParam;
				HWND	hctl = (HWND) lParam;
				LOGFONT lf;

				if (GetDlgCtrlID(hctl) != IDC_STATIC_TITLE)
					return DefWindowProc(hDlg, message, wParam, lParam);

				lf.lfWeight			= FW_BOLD;
				lf.lfEscapement		= 0;
				lf.lfOrientation	= 0;
				lf.lfHeight			= 13;
				lf.lfWidth			= 0;
				lf.lfCharSet		= DEFAULT_CHARSET;
				lf.lfClipPrecision	= CLIP_DEFAULT_PRECIS;
				lf.lfItalic			= FALSE;
				lf.lfUnderline		= FALSE;
				lf.lfStrikeOut      = FALSE;
				lf.lfQuality        = DEFAULT_QUALITY;
				lf.lfOutPrecision   = OUT_DEFAULT_PRECIS;
				lf.lfPitchAndFamily = FF_SWISS | DEFAULT_PITCH;

				_tcscpy(lf.lfFaceName, _T("Tahoma"));

				SelectObject(hDC, CreateFontIndirect(&lf));
				SetTextColor(hDC, RGB(0,0,0));

				return (BOOL) GetSysColorBrush(COLOR_STATIC);
			}

        case WM_PAINT:
			{
				HDC         hDC;
				PAINTSTRUCT ps;

				hDC = BeginPaint(hDlg, &ps); // begin painting for window
				
				SelectObject(hDC, GetStockObject(LTGRAY_BRUSH));
				SelectObject(hDC, CreatePen(PS_NULL,2,RGB(50,50,50)));

				Rectangle(hDC, 5, 20, 235, 21);
				
				EndPaint(hDlg, &ps);
			}
			return TRUE;

        default:
			return DefWindowProc(hDlg, message, wParam, lParam);
	}

    return 0;
}

// Message handler for the options dialog
LRESULT CALLBACK SoundDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
            {
                SendMessage(GetDlgItem(hDlg, IDC_SOUND), BM_SETCHECK, (WPARAM) (Settings.APUEnabled == TRUE)?BST_CHECKED:BST_UNCHECKED, 0);
                SendMessage(GetDlgItem(hDlg, IDC_SIXTEENBIT), BM_SETCHECK, (WPARAM) (Settings.SixteenBitSound == TRUE)?BST_CHECKED:BST_UNCHECKED, 0);
                SendMessage(GetDlgItem(hDlg, IDC_EIGHTBIT), BM_SETCHECK, (WPARAM) (Settings.SixteenBitSound == FALSE)?BST_CHECKED:BST_UNCHECKED, 0);
                SendMessage(GetDlgItem(hDlg, IDC_SOUNDQUALITY), TBM_SETRANGE, (WPARAM)(BOOL) TRUE, (LPARAM) MAKELONG(1,7));
                SendMessage(GetDlgItem(hDlg, IDC_SOUNDQUALITY), TBM_SETPOS, (WPARAM)(BOOL) TRUE, (LPARAM) Settings.SoundPlaybackRate);

				//CSNES
				SendMessage(GetDlgItem(hDlg, IDC_ECHO), BM_SETCHECK, (WPARAM) (Settings.DisableSoundEcho == FALSE)?BST_CHECKED:BST_UNCHECKED, 0);
				SendMessage(GetDlgItem(hDlg, IDC_STEREO), BM_SETCHECK, (WPARAM) (Settings.Stereo == TRUE)?BST_CHECKED:BST_UNCHECKED, 0);
				SendMessage(GetDlgItem(hDlg, IDC_REVERSESTEREO), BM_SETCHECK, (WPARAM) (Settings.ReverseStereo == TRUE)?BST_CHECKED:BST_UNCHECKED, 0);
				SendMessage(GetDlgItem(hDlg, IDC_SYNCSOUND), BM_SETCHECK, (WPARAM) (Settings.SoundSync == TRUE)?BST_CHECKED:BST_UNCHECKED, 0);
				SendMessage(GetDlgItem(hDlg, IDC_INTERPOLATESOUND), BM_SETCHECK, (WPARAM) (Settings.InterpolatedSound == TRUE)?BST_CHECKED:BST_UNCHECKED, 0);
				SendMessage(GetDlgItem(hDlg, IDC_ENVELOPEHEIGHT), BM_SETCHECK, (WPARAM) (Settings.SoundEnvelopeHeightReading == TRUE)?BST_CHECKED:BST_UNCHECKED, 0);
				SendMessage(GetDlgItem(hDlg, IDC_ALTDECODE), BM_SETCHECK, (WPARAM) (Settings.AltSampleDecode == TRUE)?BST_CHECKED:BST_UNCHECKED, 0);
				SendMessage(GetDlgItem(hDlg, IDC_FIXFREQUENCY), BM_SETCHECK, (WPARAM) (Settings.FixFrequency == TRUE)?BST_CHECKED:BST_UNCHECKED, 0);


                SHINITDLGINFO shidi;

				// Create a Done button and size it.
				shidi.dwMask  = SHIDIM_FLAGS;
				shidi.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIZEDLGFULLSCREEN;
				shidi.hDlg    = hDlg;
				
				//initialzes the dialog based on the dwFlags parameter
				SHInitDialog(&shidi);
            }
            return TRUE; 

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK)
            {
				Settings.APUEnabled = (BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_SOUND), BM_GETCHECK, 0, 0))?true:false;
				Settings.SixteenBitSound	= SendMessage(GetDlgItem(hDlg, IDC_SIXTEENBIT), BM_GETCHECK, 0, 0);
                Settings.SoundPlaybackRate = SendMessage(GetDlgItem(hDlg, IDC_SOUNDQUALITY), TBM_GETPOS, 0, 0);
				Settings.Stereo				= SendMessage(GetDlgItem(hDlg, IDC_STEREO)			, BM_GETCHECK, 0, 0);
				Settings.ReverseStereo		= SendMessage(GetDlgItem(hDlg, IDC_REVERSESTEREO)	, BM_GETCHECK, 0, 0);
				Settings.DisableSoundEcho	= !(SendMessage(GetDlgItem(hDlg, IDC_ECHO)			, BM_GETCHECK, 0, 0));
				Settings.InterpolatedSound  = SendMessage(GetDlgItem(hDlg, IDC_INTERPOLATESOUND), BM_GETCHECK, 0, 0);
				Settings.SoundSync			= SendMessage(GetDlgItem(hDlg, IDC_SYNCSOUND)		, BM_GETCHECK, 0, 0);
				Settings.FixFrequency		= SendMessage(GetDlgItem(hDlg, IDC_FIXFREQUENCY)		, BM_GETCHECK, 0, 0);
				Settings.SoundEnvelopeHeightReading		= SendMessage(GetDlgItem(hDlg, IDC_ENVELOPEHEIGHT)		, BM_GETCHECK, 0, 0);
				Settings.AltSampleDecode	= SendMessage(GetDlgItem(hDlg, IDC_ALTDECODE)		, BM_GETCHECK, 0, 0);

				//CSNES - turning on/off sound involves more than APUEnabled
				/*
				Settings.NextAPUEnabled			= Settings.APUEnabled;
				Settings.DisableSampleCaching	= !(Settings.APUEnabled);
				Settings.DisableMasterVolume	= !(Settings.APUEnabled);
				Settings.ThreadSound			= FALSE;
				Settings.Mute					= !(Settings.APUEnabled);
				*/
                EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}
			break;

        case WM_NOTIFY:
            if ((int) wParam == IDC_SPIN_FRAMESKIP)
            {
                LPNMUPDOWN lpnmud = (LPNMUPDOWN) lParam;

                if ((lpnmud->iPos + lpnmud->iDelta) > 20)
                    return TRUE;

                return FALSE;
            }
            break;

        case WM_CTLCOLORSTATIC:
			{
				HDC		hDC  = (HDC) wParam;
				HWND	hctl = (HWND) lParam;
				LOGFONT lf;

				if (GetDlgCtrlID(hctl) != IDC_STATIC_TITLE)
					return DefWindowProc(hDlg, message, wParam, lParam);

				lf.lfWeight			= FW_BOLD;
				lf.lfEscapement		= 0;
				lf.lfOrientation	= 0;
				lf.lfHeight			= 13;
				lf.lfWidth			= 0;
				lf.lfCharSet		= DEFAULT_CHARSET;
				lf.lfClipPrecision	= CLIP_DEFAULT_PRECIS;
				lf.lfItalic			= FALSE;
				lf.lfUnderline		= FALSE;
				lf.lfStrikeOut      = FALSE;
				lf.lfQuality        = DEFAULT_QUALITY;
				lf.lfOutPrecision   = OUT_DEFAULT_PRECIS;
				lf.lfPitchAndFamily = FF_SWISS | DEFAULT_PITCH;

				_tcscpy(lf.lfFaceName, _T("Tahoma"));

				SelectObject(hDC, CreateFontIndirect(&lf));
				SetTextColor(hDC, RGB(0,0,0));

				return (BOOL) GetSysColorBrush(COLOR_STATIC);
			}

        case WM_PAINT:
			{
				HDC         hDC;
				PAINTSTRUCT ps;

				hDC = BeginPaint(hDlg, &ps); // begin painting for window
				
				SelectObject(hDC, GetStockObject(LTGRAY_BRUSH));
				SelectObject(hDC, CreatePen(PS_NULL,2,RGB(50,50,50)));

				Rectangle(hDC, 5, 20, 235, 21);
				
				EndPaint(hDlg, &ps);
			}
			return TRUE;

        default:
			return DefWindowProc(hDlg, message, wParam, lParam);
	}

    return 0;
}

// Message handler for the System dialog
LRESULT CALLBACK SystemDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
            {
                //SendMessage(GetDlgItem(hDlg, IDC_TRANSPARENCY), BM_SETCHECK, (WPARAM) (Settings.Transparency == TRUE)?BST_CHECKED:BST_UNCHECKED, 0);
                //SendMessage(GetDlgItem(hDlg, IDC_LANDSCAPE), BM_SETCHECK, (WPARAM) (g_bLandscape)?BST_CHECKED:BST_UNCHECKED, 0);
                //SendMessage(GetDlgItem(hDlg, IDC_LEFT), BM_SETCHECK, (WPARAM) (g_bLandLeft)?BST_CHECKED:BST_UNCHECKED, 0);
                SendMessage(GetDlgItem(hDlg, IDC_COMPAT), BM_SETCHECK, (WPARAM) (g_bCompat)?BST_CHECKED:BST_UNCHECKED, 0);
                SendMessage(GetDlgItem(hDlg, IDC_AUTO), BM_SETCHECK, (WPARAM) (g_bAutoSkip)?BST_CHECKED:BST_UNCHECKED, 0);
                //SendMessage(GetDlgItem(hDlg, IDC_SMOOTHSTRETCH), BM_SETCHECK, (WPARAM) (g_bSmoothStretch)?BST_CHECKED:BST_UNCHECKED, 0);
                SetDlgItemInt(hDlg, IDC_FRAMESKIP, g_iFrameSkip, false);
                SetDlgItemInt(hDlg, IDC_CYCLES, g_iCycles, false);
				SendMessage(GetDlgItem(hDlg, IDC_FRAMESKIP), UDM_SETRANGE32, (WPARAM) 1, (LPARAM) 20);
                SendMessage(GetDlgItem(hDlg, IDC_CYCLES), UDM_SETRANGE32, (WPARAM) 1, (LPARAM) 20);
                SendMessage(GetDlgItem(hDlg, IDC_SOUND), BM_SETCHECK, (WPARAM) (Settings.APUEnabled == TRUE)?BST_CHECKED:BST_UNCHECKED, 0);
                SendMessage(GetDlgItem(hDlg, IDC_SIXTEENBIT), BM_SETCHECK, (WPARAM) (Settings.SixteenBitSound == TRUE)?BST_CHECKED:BST_UNCHECKED, 0);
                SendMessage(GetDlgItem(hDlg, IDC_EIGHTBIT), BM_SETCHECK, (WPARAM) (Settings.SixteenBitSound == FALSE)?BST_CHECKED:BST_UNCHECKED, 0);
                SendMessage(GetDlgItem(hDlg, IDC_SOUNDQUALITY), TBM_SETRANGE, (WPARAM)(BOOL) TRUE, (LPARAM) MAKELONG(1,3));
                SendMessage(GetDlgItem(hDlg, IDC_SOUNDQUALITY), TBM_SETPOS, (WPARAM)(BOOL) TRUE, (LPARAM) Settings.SoundPlaybackRate);

				//CSNES
				SendMessage(GetDlgItem(hDlg, IDC_RESUMEAFTERLOADSTATE), BM_SETCHECK, (WPARAM) (g_bResumeAfterLoadState)?BST_CHECKED:BST_UNCHECKED, 0);
                SendMessage(GetDlgItem(hDlg, IDC_RESUMEAFTERSAVESTATE), BM_SETCHECK, (WPARAM) (g_bResumeAfterSaveState)?BST_CHECKED:BST_UNCHECKED, 0);
				SendMessage(GetDlgItem(hDlg, IDC_DISPLAYFRAMERATE), BM_SETCHECK, (WPARAM) (Settings.DisplayFrameRate)?BST_CHECKED:BST_UNCHECKED, 0);

                SHINITDLGINFO shidi;

				// Create a Done button and size it.
				shidi.dwMask  = SHIDIM_FLAGS;
				shidi.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIZEDLGFULLSCREEN;
				shidi.hDlg    = hDlg;
				
				//initialzes the dialog based on the dwFlags parameter
				SHInitDialog(&shidi);
            }
            return TRUE; 

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK)
            {
				g_iCycles				   = GetDlgItemInt(hDlg, IDC_CYCLES, NULL, false);
				Settings.CyclesPercentage  = g_iCycles;
				
				//CSNES
				g_bResumeAfterLoadState = (BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_RESUMEAFTERLOADSTATE), BM_GETCHECK, 0, 0))?true:false;
				g_bResumeAfterSaveState = (BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_RESUMEAFTERSAVESTATE), BM_GETCHECK, 0, 0))?true:false;
				Settings.DisplayFrameRate = (BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_DISPLAYFRAMERATE), BM_GETCHECK, 0, 0))?true:false;

                EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}
			break;

        case WM_NOTIFY:
            if ((int) wParam == IDC_SPIN_FRAMESKIP)
            {
                LPNMUPDOWN lpnmud = (LPNMUPDOWN) lParam;

                if ((lpnmud->iPos + lpnmud->iDelta) > 20)
                    return TRUE;

                return FALSE;
            }
            break;

        case WM_CTLCOLORSTATIC:
			{
				HDC		hDC  = (HDC) wParam;
				HWND	hctl = (HWND) lParam;
				LOGFONT lf;

				if (GetDlgCtrlID(hctl) != IDC_STATIC_TITLE)
					return DefWindowProc(hDlg, message, wParam, lParam);

				lf.lfWeight			= FW_BOLD;
				lf.lfEscapement		= 0;
				lf.lfOrientation	= 0;
				lf.lfHeight			= 13;
				lf.lfWidth			= 0;
				lf.lfCharSet		= DEFAULT_CHARSET;
				lf.lfClipPrecision	= CLIP_DEFAULT_PRECIS;
				lf.lfItalic			= FALSE;
				lf.lfUnderline		= FALSE;
				lf.lfStrikeOut      = FALSE;
				lf.lfQuality        = DEFAULT_QUALITY;
				lf.lfOutPrecision   = OUT_DEFAULT_PRECIS;
				lf.lfPitchAndFamily = FF_SWISS | DEFAULT_PITCH;

				_tcscpy(lf.lfFaceName, _T("Tahoma"));

				SelectObject(hDC, CreateFontIndirect(&lf));
				SetTextColor(hDC, RGB(0,0,0));

				return (BOOL) GetSysColorBrush(COLOR_STATIC);
			}

        case WM_PAINT:
			{
				HDC         hDC;
				PAINTSTRUCT ps;

				hDC = BeginPaint(hDlg, &ps); // begin painting for window
				
				SelectObject(hDC, GetStockObject(LTGRAY_BRUSH));
				SelectObject(hDC, CreatePen(PS_NULL,2,RGB(50,50,50)));

				Rectangle(hDC, 5, 20, 235, 21);
				
				EndPaint(hDlg, &ps);
			}
			return TRUE;

        default:
			return DefWindowProc(hDlg, message, wParam, lParam);
	}

    return 0;
}



// Message handler for the options dialog
LRESULT CALLBACK SkinsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
            {
            	SHINITDLGINFO shidi;

				// Create a Done button and size it.
				shidi.dwMask  = SHIDIM_FLAGS;
				shidi.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIZEDLGFULLSCREEN;
				shidi.hDlg    = hDlg;
				
				//initialzes the dialog based on the dwFlags parameter
				SHInitDialog(&shidi);
            }
            return TRUE; 

		case WM_COMMAND:
            {
                static RECT rtPaint = { 0, 28, 240, 240 };

                switch (LOWORD(wParam))
                {
                    case IDOK:
	    			    EndDialog(hDlg, LOWORD(wParam));
		    		    return TRUE;

                    case IDC_BUTTON_PREVIOUS:
                        g_iSelectedSkin--;
                        if (g_iSelectedSkin < 0)
                            g_iSelectedSkin = g_iSkinCount - 1;
                        g_pSkins[g_iSelectedSkin].LoadBitmap(g_hbmSkin, g_hSkinDC, g_hInstance, g_hWnd);
                        InvalidateRect(hDlg, &rtPaint, TRUE);
                        break;

                    case IDC_BUTTON_NEXT:
                        g_iSelectedSkin = (g_iSelectedSkin + 1) % g_iSkinCount;
                        g_pSkins[g_iSelectedSkin].LoadBitmap(g_hbmSkin, g_hSkinDC, g_hInstance, g_hWnd);
                        InvalidateRect(hDlg, &rtPaint, TRUE);
                        break;
			    }
            }
			break;

        case WM_CTLCOLORSTATIC:
			{
				HDC		hDC  = (HDC) wParam;
				HWND	hctl = (HWND) lParam;
				LOGFONT lf;

				if (GetDlgCtrlID(hctl) != IDC_STATIC_TITLE)
					return DefWindowProc(hDlg, message, wParam, lParam);

				lf.lfWeight			= FW_BOLD;
				lf.lfEscapement		= 0;
				lf.lfOrientation	= 0;
				lf.lfHeight			= 13;
				lf.lfWidth			= 0;
				lf.lfCharSet		= DEFAULT_CHARSET;
				lf.lfClipPrecision	= CLIP_DEFAULT_PRECIS;
				lf.lfItalic			= FALSE;
				lf.lfUnderline		= FALSE;
				lf.lfStrikeOut      = FALSE;
				lf.lfQuality        = DEFAULT_QUALITY;
				lf.lfOutPrecision   = OUT_DEFAULT_PRECIS;
				lf.lfPitchAndFamily = FF_SWISS | DEFAULT_PITCH;

				_tcscpy(lf.lfFaceName, _T("Tahoma"));

				SelectObject(hDC, CreateFontIndirect(&lf));
				SetTextColor(hDC, RGB(0,0,153));

				return (BOOL) GetSysColorBrush(COLOR_STATIC);
			}

        case WM_PAINT:
			{
				HDC         hDC;
				PAINTSTRUCT ps;
                RECT        rtName   = { 0, 28, 240, 48 };
                RECT        rtAuthor = { 0, 160, 240, 180 };

				hDC = BeginPaint(hDlg, &ps); // begin painting for window
				
				SelectObject(hDC, GetStockObject(BLACK_BRUSH));
				Rectangle(hDC, 0, 24, 240, 25);

				Rectangle(hDC, 0, 51, 240, 52);
                if (g_pSkins[g_iSelectedSkin].bLandscape)
                {
                    BitBlt(hDC, 0, 52, 240, 64, g_hSkinDC, 0, 0, SRCCOPY);
				    Rectangle(hDC, 0, 52 + 96, 240, 52 + 64 + 1);
                }
                else
                {
                    BitBlt(hDC, 0, 52, 240, 96, g_hSkinDC, 0, 0, SRCCOPY);
				    Rectangle(hDC, 0, 52 + 96, 240, 52 + 96 + 1);
                }

                SelectObject(hDC, g_hFontPaused);
                SetTextColor(hDC, COLORREF(0x00000000));
                DrawText(hDC, g_pSkins[g_iSelectedSkin].pszName, _tcslen(g_pSkins[g_iSelectedSkin].pszName), &rtName, DT_CENTER|DT_SINGLELINE|DT_VCENTER);
                SelectObject(hDC, g_hFontNormal);
                SetTextColor(hDC, COLORREF(0x0000000));
                DrawText(hDC, g_pSkins[g_iSelectedSkin].pszAuthor, _tcslen(g_pSkins[g_iSelectedSkin].pszAuthor), &rtAuthor, DT_CENTER|DT_SINGLELINE|DT_VCENTER);

				EndPaint(hDlg, &ps);
			}
			return TRUE;

        case WM_DRAWITEM:
            {
                DRAWITEMSTRUCT *psDrawItem = (LPDRAWITEMSTRUCT) lParam;

                if (psDrawItem->CtlID == IDC_BUTTON_PREVIOUS)
                    DrawIcon(psDrawItem->hDC, 0, 0, g_hiLeftArrow);
                else
                    DrawIcon(psDrawItem->hDC, 0, 0, g_hiRightArrow);
            }
            return TRUE;

        default:
			return DefWindowProc(hDlg, message, wParam, lParam);
	}

    return 0;
}

#define SETBUTTON(a,b) \
    { \
        DWORD dwCurrentMask = g_kmCurrent.dwKeyMask[vkKey]; \
        if (dwCurrentMask != 0) \
            SendMessage(GetDlgItem(GetParent(hWnd), g_kmCurrent.GetStaticIDFromMask(dwCurrentMask)), WM_SETTEXT, 0, (LPARAM)(LPCTSTR) g_kmCurrent.GetKeyDisplayFromMask(0)); \
        g_kmCurrent.dwKeyMask[g_kmCurrent.GetKeyFromMask(a)] = 0; \
        g_kmCurrent.dwKeyMask[vkKey] = a; \
        SendMessage(GetDlgItem(GetParent(hWnd), b), WM_SETTEXT, 0, (LPARAM)(LPCTSTR) g_kmCurrent.GetKeyDisplayFromMask(a)); \
    }

DWORD   g_pOriginalWndProc = NULL;

LRESULT CALLBACK ButtonProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_GETDLGCODE:
            return DLGC_WANTARROWS|DLGC_WANTALLKEYS;

        case WM_KEYDOWN:
            if (g_bSetButtonMode)
            {
                short vkKey = (short) wParam;

                if (vkKey == VK_LWIN)
                    return TRUE;

                switch (g_iSetButton)
                {
                    case IDC_BUTTON_UP:
                        SETBUTTON(SNES_UP_MASK, IDC_STATIC_UP);
                        break;

                    case IDC_BUTTON_DOWN:
                        SETBUTTON(SNES_DOWN_MASK, IDC_STATIC_DOWN);
                        break;

                    case IDC_BUTTON_LEFT:
                        SETBUTTON(SNES_LEFT_MASK, IDC_STATIC_LEFT);
                        break;

                    case IDC_BUTTON_RIGHT:
                        SETBUTTON(SNES_RIGHT_MASK, IDC_STATIC_RIGHT);
                        break;

                    case IDC_BUTTON_A:
                        SETBUTTON(SNES_A_MASK, IDC_STATIC_A);
                        break;

                    case IDC_BUTTON_B:
                        SETBUTTON(SNES_B_MASK, IDC_STATIC_B);
                        break;

                    case IDC_BUTTON_X:
                        SETBUTTON(SNES_X_MASK, IDC_STATIC_X);
                        break;

                    case IDC_BUTTON_Y:
                        SETBUTTON(SNES_Y_MASK, IDC_STATIC_Y);
                        break;

                    case IDC_BUTTON_START:
                        SETBUTTON(SNES_START_MASK, IDC_STATIC_START);
                        break;

                    case IDC_BUTTON_SELECT:
                        SETBUTTON(SNES_SELECT_MASK, IDC_STATIC_SELECT);
                        break;

                    case IDC_BUTTON_L:
                        SETBUTTON(SNES_TL_MASK, IDC_STATIC_L);
                        break;

                    case IDC_BUTTON_R:
                        SETBUTTON(SNES_TR_MASK, IDC_STATIC_R);
                        break;
                }

                g_bSetButtonMode = false;

                GXCloseInput();

                return TRUE;
            }
            break;

        default:
            break;
    }

    return CallWindowProc((WNDPROC) g_pOriginalWndProc, hWnd, message, wParam, lParam);
}

// Message handler for the keys dialog
LRESULT CALLBACK KeysDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
            {
                SendMessage(GetDlgItem(hDlg, IDC_STATIC_UP),     WM_SETTEXT, 0, (LPARAM)(LPCTSTR) g_kmCurrent.GetKeyDisplayFromMask(SNES_UP_MASK));
                SendMessage(GetDlgItem(hDlg, IDC_STATIC_DOWN),   WM_SETTEXT, 0, (LPARAM)(LPCTSTR) g_kmCurrent.GetKeyDisplayFromMask(SNES_DOWN_MASK));
                SendMessage(GetDlgItem(hDlg, IDC_STATIC_LEFT),   WM_SETTEXT, 0, (LPARAM)(LPCTSTR) g_kmCurrent.GetKeyDisplayFromMask(SNES_LEFT_MASK));
                SendMessage(GetDlgItem(hDlg, IDC_STATIC_RIGHT),  WM_SETTEXT, 0, (LPARAM)(LPCTSTR) g_kmCurrent.GetKeyDisplayFromMask(SNES_RIGHT_MASK));
                SendMessage(GetDlgItem(hDlg, IDC_STATIC_B),      WM_SETTEXT, 0, (LPARAM)(LPCTSTR) g_kmCurrent.GetKeyDisplayFromMask(SNES_B_MASK));
                SendMessage(GetDlgItem(hDlg, IDC_STATIC_A),      WM_SETTEXT, 0, (LPARAM)(LPCTSTR) g_kmCurrent.GetKeyDisplayFromMask(SNES_A_MASK));
                SendMessage(GetDlgItem(hDlg, IDC_STATIC_Y),      WM_SETTEXT, 0, (LPARAM)(LPCTSTR) g_kmCurrent.GetKeyDisplayFromMask(SNES_Y_MASK));
                SendMessage(GetDlgItem(hDlg, IDC_STATIC_X),      WM_SETTEXT, 0, (LPARAM)(LPCTSTR) g_kmCurrent.GetKeyDisplayFromMask(SNES_X_MASK));
                SendMessage(GetDlgItem(hDlg, IDC_STATIC_START),  WM_SETTEXT, 0, (LPARAM)(LPCTSTR) g_kmCurrent.GetKeyDisplayFromMask(SNES_START_MASK));
                SendMessage(GetDlgItem(hDlg, IDC_STATIC_SELECT), WM_SETTEXT, 0, (LPARAM)(LPCTSTR) g_kmCurrent.GetKeyDisplayFromMask(SNES_SELECT_MASK));
                SendMessage(GetDlgItem(hDlg, IDC_STATIC_L),      WM_SETTEXT, 0, (LPARAM)(LPCTSTR) g_kmCurrent.GetKeyDisplayFromMask(SNES_TL_MASK));
                SendMessage(GetDlgItem(hDlg, IDC_STATIC_R),      WM_SETTEXT, 0, (LPARAM)(LPCTSTR) g_kmCurrent.GetKeyDisplayFromMask(SNES_TR_MASK));

                g_bSetButtonMode = false;

                g_pOriginalWndProc = SetWindowLong(GetDlgItem(hDlg, IDC_BUTTON_UP),     GWL_WNDPROC, (DWORD) ButtonProc);
                g_pOriginalWndProc = SetWindowLong(GetDlgItem(hDlg, IDC_BUTTON_DOWN),   GWL_WNDPROC, (DWORD) ButtonProc);
                g_pOriginalWndProc = SetWindowLong(GetDlgItem(hDlg, IDC_BUTTON_LEFT),   GWL_WNDPROC, (DWORD) ButtonProc);
                g_pOriginalWndProc = SetWindowLong(GetDlgItem(hDlg, IDC_BUTTON_RIGHT),  GWL_WNDPROC, (DWORD) ButtonProc);
                g_pOriginalWndProc = SetWindowLong(GetDlgItem(hDlg, IDC_BUTTON_A),      GWL_WNDPROC, (DWORD) ButtonProc);
                g_pOriginalWndProc = SetWindowLong(GetDlgItem(hDlg, IDC_BUTTON_B),      GWL_WNDPROC, (DWORD) ButtonProc);
                g_pOriginalWndProc = SetWindowLong(GetDlgItem(hDlg, IDC_BUTTON_X),      GWL_WNDPROC, (DWORD) ButtonProc);
                g_pOriginalWndProc = SetWindowLong(GetDlgItem(hDlg, IDC_BUTTON_Y),      GWL_WNDPROC, (DWORD) ButtonProc);
                g_pOriginalWndProc = SetWindowLong(GetDlgItem(hDlg, IDC_BUTTON_START),  GWL_WNDPROC, (DWORD) ButtonProc);
                g_pOriginalWndProc = SetWindowLong(GetDlgItem(hDlg, IDC_BUTTON_SELECT), GWL_WNDPROC, (DWORD) ButtonProc);
                g_pOriginalWndProc = SetWindowLong(GetDlgItem(hDlg, IDC_BUTTON_L),      GWL_WNDPROC, (DWORD) ButtonProc);
                g_pOriginalWndProc = SetWindowLong(GetDlgItem(hDlg, IDC_BUTTON_R),      GWL_WNDPROC, (DWORD) ButtonProc);

            	SHINITDLGINFO shidi;

				// Create a Done button and size it.
				shidi.dwMask  = SHIDIM_FLAGS;
				shidi.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIZEDLGFULLSCREEN;
				shidi.hDlg    = hDlg;
				
				//initialzes the dialog based on the dwFlags parameter
				SHInitDialog(&shidi);
            }
            return TRUE; 

		case WM_COMMAND:
            {
                switch (LOWORD(wParam))
                {
                    case IDOK:
	    			    EndDialog(hDlg, LOWORD(wParam));
		    		    return TRUE;

                    case IDC_BUTTON_UP:
                    case IDC_BUTTON_DOWN:
                    case IDC_BUTTON_LEFT:
                    case IDC_BUTTON_RIGHT:
                    case IDC_BUTTON_A:
                    case IDC_BUTTON_B:
                    case IDC_BUTTON_X:
                    case IDC_BUTTON_Y:
                    case IDC_BUTTON_START:
                    case IDC_BUTTON_SELECT:
                    case IDC_BUTTON_L:
                    case IDC_BUTTON_R:
                        if (!g_bSetButtonMode)
                        {
                            if (GXOpenInput() != 0)
                            {
                                g_bSetButtonMode = true;
                                g_iSetButton     = LOWORD(wParam);
                            }
                        }
                        else
                        {
                            // g_bSetButtonMode = false;
                        }
                        return TRUE;

                    default:
                        break;
			    }
            }
			break;

        case WM_CTLCOLORSTATIC:
			{
				HDC		hDC  = (HDC) wParam;
				HWND	hctl = (HWND) lParam;
				LOGFONT lf;

				if ((GetDlgCtrlID(hctl) != IDC_STATIC_TITLE) &&
                    (GetDlgCtrlID(hctl) != IDC_STATIC_INFO))
					return DefWindowProc(hDlg, message, wParam, lParam);

				lf.lfWeight			= FW_BOLD;
				lf.lfEscapement		= 0;
				lf.lfOrientation	= 0;
				lf.lfHeight			= 13;
				lf.lfWidth			= 0;
				lf.lfCharSet		= DEFAULT_CHARSET;
				lf.lfClipPrecision	= CLIP_DEFAULT_PRECIS;
				lf.lfItalic			= FALSE;
				lf.lfUnderline		= FALSE;
				lf.lfStrikeOut      = FALSE;
				lf.lfQuality        = DEFAULT_QUALITY;
				lf.lfOutPrecision   = OUT_DEFAULT_PRECIS;
				lf.lfPitchAndFamily = FF_SWISS | DEFAULT_PITCH;

				_tcscpy(lf.lfFaceName, _T("Tahoma"));

				SelectObject(hDC, CreateFontIndirect(&lf));
                if (GetDlgCtrlID(hctl) == IDC_STATIC_TITLE)
				    SetTextColor(hDC, RGB(0,0,153));
                else
				    SetTextColor(hDC, RGB(0,0,0));

				return (BOOL) GetSysColorBrush(COLOR_STATIC);
			}

        case WM_PAINT:
			{
				HDC         hDC;
				PAINTSTRUCT ps;

				hDC = BeginPaint(hDlg, &ps); // begin painting for window
				
				SelectObject(hDC, GetStockObject(BLACK_BRUSH));
				Rectangle(hDC, 0, 24, 240, 25);

				EndPaint(hDlg, &ps);
			}
			return TRUE;

        default:
			return DefWindowProc(hDlg, message, wParam, lParam);
	}

    return 0;
}

#define CREDITS_LENGTH  4000

// Message handler for the credits dialog
LRESULT CALLBACK CreditsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
            {
                TCHAR   szCredits[CREDITS_LENGTH];
                int     iLength;

                LoadString(g_hInstance, IDS_CREDITS0, szCredits, CREDITS_LENGTH);
                iLength = _tcslen(szCredits);
                LoadString(g_hInstance, IDS_CREDITS1, &(szCredits[iLength]), CREDITS_LENGTH - iLength);
                iLength = _tcslen(szCredits);
                LoadString(g_hInstance, IDS_CREDITS2, &(szCredits[iLength]), CREDITS_LENGTH - iLength);

                SendMessage(GetDlgItem(hDlg, IDC_CREDITS), WM_SETTEXT, (WPARAM) 0, (LPARAM)(LPCTSTR) szCredits);
                SendMessage(GetDlgItem(hDlg, IDC_CREDITS), EM_SETSEL, (WPARAM)(INT) -1, (LPARAM) 0);

                SHINITDLGINFO shidi;

				// Create a Done button and size it.
				shidi.dwMask  = SHIDIM_FLAGS;
				shidi.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIZEDLGFULLSCREEN;
				shidi.hDlg    = hDlg;
				
				//initialzes the dialog based on the dwFlags parameter
				SHInitDialog(&shidi);

        		RECT  rc;
                HWND  hDlgItem = GetDlgItem(hDlg, IDC_CREDITS);

                rc.top    = 25;
                rc.left   = 0;
                rc.right  = 240;
                rc.bottom = 320 - 26 - 25;
		        MoveWindow(hDlgItem, rc.left, rc.top, rc.right, rc.bottom - rc.top, TRUE);
            }
            return TRUE; 

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK)
            {
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}
			break;

        case WM_CTLCOLORSTATIC:
			{
				HDC		hDC  = (HDC) wParam;
				HWND	hctl = (HWND) lParam;
				LOGFONT lf;

				if (GetDlgCtrlID(hctl) != IDC_STATIC_TITLE)
					return DefWindowProc(hDlg, message, wParam, lParam);

				lf.lfWeight			= FW_BOLD;
				lf.lfEscapement		= 0;
				lf.lfOrientation	= 0;
				lf.lfHeight			= 13;
				lf.lfWidth			= 0;
				lf.lfCharSet		= DEFAULT_CHARSET;
				lf.lfClipPrecision	= CLIP_DEFAULT_PRECIS;
				lf.lfItalic			= FALSE;
				lf.lfUnderline		= FALSE;
				lf.lfStrikeOut      = FALSE;
				lf.lfQuality        = DEFAULT_QUALITY;
				lf.lfOutPrecision   = OUT_DEFAULT_PRECIS;
				lf.lfPitchAndFamily = FF_SWISS | DEFAULT_PITCH;

				_tcscpy(lf.lfFaceName, _T("Tahoma"));

				SelectObject(hDC, CreateFontIndirect(&lf));
				SetTextColor(hDC, RGB(0,0,153));

				return (BOOL) GetSysColorBrush(COLOR_STATIC);
			}

        case WM_PAINT:
			{
				HDC         hDC;
				PAINTSTRUCT ps;

				hDC = BeginPaint(hDlg, &ps); // begin painting for window
				
				SelectObject(hDC, GetStockObject(BLACK_BRUSH));
				Rectangle(hDC, 0, 24, 240, 25);

				EndPaint(hDlg, &ps);
			}
			return TRUE;

        default:
			return DefWindowProc(hDlg, message, wParam, lParam);
	}

    return 0;
}

//      Eight directions
//      Eight buttons - start, select, A, B, X, Y, L, R

bool CheckKeyPad(int _iX, int _iY, bool _bDown)
{
    if (g_bLandscape || (g_pKeypad == NULL) || (_iY < 224))
        return false;

    DWORD dwColor = *(g_pKeypad + _iX + ((96 - (_iY - 224)) * 240));

    for (int i = 0; i < g_pSkins[g_iSelectedSkin].iNumberOfColors; i++)
    {
        if (dwColor == g_pSkins[g_iSelectedSkin].iColor[i])
        {
            if (_bDown)
                g_iJoypadState |= g_pSkins[g_iSelectedSkin].iMask[i];
            else
                g_iJoypadState &= ~(g_pSkins[g_iSelectedSkin].iMask[i]);

            return true;
        }
    }

    return false;
}


//------------------------------------------------------------------------------
// OpenRegistry
//      Opens and returns a key to the program's registry.
//------------------------------------------------------------------------------
HKEY OpenRegistry()
{
	HKEY  hRootKey;
    DWORD dwDisposition;

	if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE, g_sRootKey, 0, NULL, 0, 0, NULL, &hRootKey, &dwDisposition))
	{
		return hRootKey;
	}

	return NULL;
}

//------------------------------------------------------------------------------
// CloseRegistry
//      Closes the program's registry.
//------------------------------------------------------------------------------
void CloseRegistry(HKEY hRootKey)
{
	RegCloseKey(hRootKey);
}

//------------------------------------------------------------------------------
// RegQueryBool
//      Reads a boolean flag from the registry.
//------------------------------------------------------------------------------
bool RegQueryBool(HKEY hKey, LPCWSTR lpValueName, bool bDefault = true)
{
    DWORD dwValue;
    DWORD dwType;
	DWORD dwSize   = sizeof(dwValue);

	if (ERROR_SUCCESS == RegQueryValueEx(hKey, lpValueName, 0, &dwType, (unsigned char *) &dwValue, &dwSize))
	{
        return ((DWORD) 0 != dwValue);
    }

    return bDefault;
}

//------------------------------------------------------------------------------
// RegSetBool
//      Sets a boolean flag in the registry.
//------------------------------------------------------------------------------
void RegSetBool(HKEY hKey, LPCWSTR lpValueName, bool bValue)
{
	DWORD dwValue  = (DWORD) bValue;

	RegSetValueEx(hKey, lpValueName, 0, REG_DWORD, (unsigned char *) &dwValue, sizeof(dwValue));
}

//------------------------------------------------------------------------------
// RegQueryDword
//      Reads a dword value from the registry.
//------------------------------------------------------------------------------
DWORD RegQueryDword(HKEY hKey, LPCWSTR lpValueName, DWORD dwDefault)
{
    DWORD dwValue;
    DWORD dwType;
	DWORD dwSize   = sizeof(dwValue);

	if (ERROR_SUCCESS == RegQueryValueEx(hKey, lpValueName, 0, &dwType, (unsigned char *) &dwValue, &dwSize))
	{
        return dwValue;
    }

    return dwDefault;
}

//------------------------------------------------------------------------------
// RegSetDword
//      Sets a dword value in the registry.
//------------------------------------------------------------------------------
void RegSetDword(HKEY hKey, LPCWSTR lpValueName, DWORD dwValue)
{
	RegSetValueEx(hKey, lpValueName, 0, REG_DWORD, (unsigned char *) &dwValue, sizeof(dwValue));
}

//------------------------------------------------------------------------------
// LoadDefaultKeys
//      Loads default key mappings.
//------------------------------------------------------------------------------
void LoadDefaultKeys()
{
    g_gxKeyList = GXGetDefaultKeys(GX_NORMALKEYS);

    g_kmDefault.dwKeyMask[g_gxKeyList.vkUp]    = SNES_UP_MASK;
    g_kmDefault.dwKeyMask[g_gxKeyList.vkDown]  = SNES_DOWN_MASK;
    g_kmDefault.dwKeyMask[g_gxKeyList.vkLeft]  = SNES_LEFT_MASK;
    g_kmDefault.dwKeyMask[g_gxKeyList.vkRight] = SNES_RIGHT_MASK;
    g_kmDefault.dwKeyMask[g_gxKeyList.vkA]     = SNES_B_MASK;
    g_kmDefault.dwKeyMask[g_gxKeyList.vkB]     = SNES_A_MASK;
    g_kmDefault.dwKeyMask[g_gxKeyList.vkC]     = SNES_SELECT_MASK;
    g_kmDefault.dwKeyMask[g_gxKeyList.vkStart] = SNES_START_MASK;

    g_kmDefault.dwKeyMask[0xc1]                = SNES_X_MASK;
    g_kmDefault.dwKeyMask[0xc2]                = SNES_Y_MASK;
}

//------------------------------------------------------------------------------
// LoadOptions
//      Loads stored options from the registry.
//------------------------------------------------------------------------------
#define SETKEYMASK(a,b) \
    g_kmCurrent.dwKeyMask[RegQueryDword(hRootKey, a, g_kmDefault.GetKeyFromMask(b))] = b

bool LoadOptions()
{
	HKEY  hRootKey = OpenRegistry();
	bool  bReturn  = false;

    Settings.Transparency      = RegQueryBool (hRootKey, L"Transparency");
    g_bLandscape               = RegQueryBool (hRootKey, L"Landscape", false);
	g_bLandLeft				   = RegQueryBool (hRootKey, L"LandscapeLeft", true);
    g_bAutoSkip				   = RegQueryBool (hRootKey, L"AutoSkip", true);
	g_bCompat				   = RegQueryBool (hRootKey, L"Compat", false);
	g_bSmoothStretch           = RegQueryBool (hRootKey, L"Widescreen", false);
    g_iFrameSkip               = RegQueryDword(hRootKey, L"FrameSkip", 5);
    g_iCycles				   = RegQueryDword(hRootKey, L"Cycles", 100);
	Settings.APUEnabled        = RegQueryBool (hRootKey, L"SoundEmulation", false);
    Settings.SixteenBitSound   = RegQueryBool (hRootKey, L"SixteenBitSound", false);
    Settings.SoundPlaybackRate = RegQueryDword(hRootKey, L"SoundQuality", 1);
	//CSNES
	g_bResumeAfterLoadState	   = RegQueryBool (hRootKey, L"ResumeAfterLoadState",false);
	g_bResumeAfterSaveState	   = RegQueryBool (hRootKey, L"ResumeAfterSaveState",false);
	Settings.DisplayFrameRate  = RegQueryBool (hRootKey, L"DisplayFrameRate",false);
	Settings.DisableSoundEcho  = RegQueryBool (hRootKey, L"DisableSoundEcho",true);
	Settings.Stereo			   = RegQueryBool (hRootKey, L"Stereo",false);
	Settings.ReverseStereo		= RegQueryBool (hRootKey, L"ReverseStereo",false);
	Settings.SoundSync			= RegQueryBool (hRootKey, L"SoundSync",true);
	Settings.InterpolatedSound  = RegQueryBool (hRootKey, L"InterpolatedSound",false);
	Settings.FixFrequency  = RegQueryBool (hRootKey, L"FixFrequency",false);
	Settings.AltSampleDecode  = RegQueryBool (hRootKey, L"AltSampleDecode",false);
	Settings.SoundEnvelopeHeightReading  = RegQueryBool (hRootKey, L"SoundEnvelopeHeightReading",false);
	g_bUseGameFolders			= RegQueryBool (hRootKey, L"UseGameFolders",false);

    LoadDefaultKeys();

    SETKEYMASK(L"SNES up",     SNES_UP_MASK);
    SETKEYMASK(L"SNES down",   SNES_DOWN_MASK);
    SETKEYMASK(L"SNES left",   SNES_LEFT_MASK);
    SETKEYMASK(L"SNES right",  SNES_RIGHT_MASK);
    SETKEYMASK(L"SNES A",      SNES_A_MASK);
    SETKEYMASK(L"SNES B",      SNES_B_MASK);
    SETKEYMASK(L"SNES X",      SNES_X_MASK);
    SETKEYMASK(L"SNES Y",      SNES_Y_MASK);
    SETKEYMASK(L"SNES start",  SNES_START_MASK);
    SETKEYMASK(L"SNES select", SNES_SELECT_MASK);
    SETKEYMASK(L"SNES L",      SNES_TL_MASK);
    SETKEYMASK(L"SNES R",      SNES_TR_MASK);

    CloseRegistry(hRootKey);

	return bReturn;
}

//------------------------------------------------------------------------------
// SaveOptions
//      Stores options into the registry.
//------------------------------------------------------------------------------
void SaveOptions()
{
	HKEY  hRootKey = OpenRegistry();

    RegSetBool (hRootKey, L"Transparency",    (Settings.Transparency == TRUE)?true:false);
	RegSetBool (hRootKey, L"Landscape",       g_bLandscape);
	RegSetBool (hRootKey, L"LandscapeLeft",   g_bLandLeft);
	RegSetBool (hRootKey, L"AutoSkip",		  g_bAutoSkip);
	RegSetBool (hRootKey, L"Compat",		  g_bCompat);
	
	RegSetBool (hRootKey, L"Widescreen",      g_bSmoothStretch);
    RegSetDword(hRootKey, L"FrameSkip",      (DWORD) g_iFrameSkip);
    RegSetDword(hRootKey, L"Cycles",		 (DWORD) g_iCycles);
	RegSetBool (hRootKey, L"SoundEmulation",  (Settings.APUEnabled == TRUE)?true:false);
    RegSetBool (hRootKey, L"SixteenBitSound", (Settings.SixteenBitSound == TRUE)?true:false);
    RegSetDword(hRootKey, L"SoundQuality",   (DWORD)Settings.SoundPlaybackRate);

    RegSetDword(hRootKey, L"SNES up",     g_kmCurrent.GetKeyFromMask(SNES_UP_MASK));
    RegSetDword(hRootKey, L"SNES down",   g_kmCurrent.GetKeyFromMask(SNES_DOWN_MASK));
    RegSetDword(hRootKey, L"SNES left",   g_kmCurrent.GetKeyFromMask(SNES_LEFT_MASK));
    RegSetDword(hRootKey, L"SNES right",  g_kmCurrent.GetKeyFromMask(SNES_RIGHT_MASK));
    RegSetDword(hRootKey, L"SNES A",      g_kmCurrent.GetKeyFromMask(SNES_A_MASK));
    RegSetDword(hRootKey, L"SNES B",      g_kmCurrent.GetKeyFromMask(SNES_B_MASK));
    RegSetDword(hRootKey, L"SNES X",      g_kmCurrent.GetKeyFromMask(SNES_X_MASK));
    RegSetDword(hRootKey, L"SNES Y",      g_kmCurrent.GetKeyFromMask(SNES_Y_MASK));
    RegSetDword(hRootKey, L"SNES start",  g_kmCurrent.GetKeyFromMask(SNES_START_MASK));
    RegSetDword(hRootKey, L"SNES select", g_kmCurrent.GetKeyFromMask(SNES_SELECT_MASK));
    RegSetDword(hRootKey, L"SNES L",      g_kmCurrent.GetKeyFromMask(SNES_TL_MASK));
    RegSetDword(hRootKey, L"SNES R",      g_kmCurrent.GetKeyFromMask(SNES_TR_MASK));

	//CSNES
	RegSetBool (hRootKey, L"DisplayFrameRate", (Settings.DisplayFrameRate == TRUE)?true:false);	
	RegSetBool (hRootKey, L"DisableSoundEcho", (Settings.DisableSoundEcho == TRUE)?true:false);
	RegSetBool (hRootKey, L"ResumeAfterLoadState", (g_bResumeAfterLoadState == TRUE)?true:false);
	RegSetBool (hRootKey, L"ResumeAfterSaveState", (g_bResumeAfterSaveState == TRUE)?true:false);
	RegSetBool (hRootKey, L"Stereo", (Settings.Stereo == TRUE)?true:false);	
	RegSetBool (hRootKey, L"ReverseStereo", (Settings.ReverseStereo == TRUE)?true:false);
	RegSetBool (hRootKey, L"SoundSync", (Settings.SoundSync	 == TRUE)?true:false);
	RegSetBool (hRootKey, L"InterpolatedSound", (Settings.InterpolatedSound == TRUE)?true:false);
	RegSetBool (hRootKey, L"DisableSoundEcho", (Settings.DisableSoundEcho == TRUE)?true:false);
	RegSetBool (hRootKey, L"SoundEnvelopeHeightReading", (Settings.SoundEnvelopeHeightReading == TRUE)?true:false);
	RegSetBool (hRootKey, L"AltSampleDecode", (Settings.AltSampleDecode == TRUE)?true:false);
	RegSetBool (hRootKey, L"FixFrequency", (Settings.FixFrequency == TRUE)?true:false);
	
	
	CloseRegistry(hRootKey);
}
//------------------------------------------------------------------------------
// InitializeDefaultSkins
//      Loads default skins.
//------------------------------------------------------------------------------
bool InitializeDefaultSkins()
{
    g_pSkins[0].SetAuthor(L"999");
    g_pSkins[0].SetName(L"Four Button [default]");
    g_pSkins[0].bResource = true;
    g_pSkins[0].pszBitmap = MAKEINTRESOURCE(IDB_4BUTTON);
    g_pSkins[0].AddColor(0x0000ff, SNES_UP_MASK);
    g_pSkins[0].AddColor(0xff0000, SNES_DOWN_MASK);
    g_pSkins[0].AddColor(0xffff00, SNES_LEFT_MASK);
    g_pSkins[0].AddColor(0x00ff00, SNES_RIGHT_MASK);
    g_pSkins[0].AddColor(0xff9900, SNES_B_MASK);
    g_pSkins[0].AddColor(0x593400, SNES_A_MASK);
    g_pSkins[0].AddColor(0xff00ff, SNES_Y_MASK);
    g_pSkins[0].AddColor(0x00ffff, SNES_X_MASK);
    g_pSkins[0].AddColor(0x996363, SNES_TR_MASK);
    g_pSkins[0].AddColor(0x990000, SNES_TL_MASK);
    g_pSkins[0].AddColor(0x475911, SNES_START_MASK);
    g_pSkins[0].AddColor(0x99bf26, SNES_SELECT_MASK);

    g_pSkins[1].SetAuthor(L"999");
    g_pSkins[1].SetName(L"Six Button [default]");
    g_pSkins[1].bResource = true;
    g_pSkins[1].pszBitmap = MAKEINTRESOURCE(IDB_6BUTTON);
    g_pSkins[1].AddColor(0x0000ff, SNES_UP_MASK);
    g_pSkins[1].AddColor(0xff0000, SNES_DOWN_MASK);
    g_pSkins[1].AddColor(0xffff00, SNES_LEFT_MASK);
    g_pSkins[1].AddColor(0x00ff00, SNES_RIGHT_MASK);
    g_pSkins[1].AddColor(0x00ffff, SNES_B_MASK);
    g_pSkins[1].AddColor(0x00b8bf, SNES_A_MASK);
    g_pSkins[1].AddColor(0xfd40ff, SNES_Y_MASK);
    g_pSkins[1].AddColor(0xe200e5, SNES_X_MASK);
    g_pSkins[1].AddColor(0x005559, SNES_TR_MASK);
    g_pSkins[1].AddColor(0x570059, SNES_TL_MASK);

    return true;
}

//------------------------------------------------------------------------------
// LoadSkins
//      Loads stored skins from the registry.
//------------------------------------------------------------------------------
bool LoadSkins()
{
	HKEY  hRootKey = OpenRegistry();
	bool  bReturn  = false;
    TCHAR szSkinDir[128];
    TCHAR szSkinFilename[128];
    TCHAR szSkinSearch[128];
    DWORD dwType;
	DWORD dwSize   = 128 * sizeof(TCHAR);

    g_iSkinCount = 2;
    g_pSkins     = new Skin [32 + 2];

    InitializeDefaultSkins();

    if (ERROR_SUCCESS == RegQueryValueEx(hRootKey, L"Skins Path", 0, &dwType, (unsigned char *) szSkinDir, &dwSize))
	{
        CreateDirectory(szSkinDir, NULL);

        _tcscpy(szSkinSearch, szSkinDir);
        _tcscat(szSkinSearch, L"\\*.txt");

        WIN32_FIND_DATA w32FindData;
        HANDLE          hSearch;
        FILE           *pFile;
        bool            bFinished = false;
        TCHAR           szBuffer[256];

        hSearch = FindFirstFile(szSkinSearch, &w32FindData);

        if (INVALID_HANDLE_VALUE == hSearch)
            return true;

        do
        {
            _tcscpy(szSkinFilename, szSkinDir);
            _tcscat(szSkinFilename, _T("\\"));
            _tcscat(szSkinFilename, w32FindData.cFileName);

            if ((pFile = _tfopen(szSkinFilename, _T("r"))) != NULL)
            {
                if (_fgetts(szBuffer, 256, pFile) != NULL)
                {
                    if (_tcsnicmp(szBuffer, _T("PocketSNES.Skin.1"), 17) == 0)
                    {
                        while (_fgetts(szBuffer, 256, pFile) != NULL)
                        {
                            if (_tcsncmp(szBuffer, _T("//"), 2) == 0)
                            {
                                continue;
                            }
                            else if (_tcsnicmp(szBuffer, _T("Bitmap="), 7) == 0)
                            {
                                TCHAR szTemp[128];

                                _tcscpy(szTemp, szSkinDir);
                                _tcscat(szTemp, _T("\\"));
                                _tcscat(szTemp, &(szBuffer[7]));
                                szTemp[_tcslen(szTemp) - 1] = _T('\0');

                                g_pSkins[g_iSkinCount].SetBitmap(szTemp);
                            }
                            else if (_tcsnicmp(szBuffer, _T("Name="), 5) == 0)
                            {
                                szBuffer[_tcslen(szBuffer) - 1] = _T('\0');
                                g_pSkins[g_iSkinCount].SetName(&(szBuffer[5]));
                            }
                            else if (_tcsnicmp(szBuffer, _T("Author="), 7) == 0)
                            {
                                szBuffer[_tcslen(szBuffer) - 1] = _T('\0');
                                g_pSkins[g_iSkinCount].SetAuthor(&(szBuffer[7]));
                            }
                            else if (_tcsnicmp(szBuffer, _T("Landscape="), 10) == 0)
                            {
                                if (szBuffer[10] == _T('1'))
                                    g_pSkins[g_iSkinCount].bLandscape = true;
                            }
                            else if (_tcsnicmp(szBuffer, _T("0x"), 2) == 0)
                            {
                                szBuffer[_tcslen(szBuffer) - 1] = _T('\0');
                                szBuffer[8] = _T('\0');
                                g_pSkins[g_iSkinCount].AddColor(szBuffer, &(szBuffer[9]));
                            }
                        }

                        g_iSkinCount++;
                    }                        
                }

                fclose(pFile);
            }

            if (!FindNextFile(hSearch, &w32FindData))
            {
                bFinished = true;
            }
        } while (!bFinished);

        FindClose(hSearch);
    }

    return true;
}


void SetKeypad()
{
    if (g_bLandscape)
    {
        if(g_bLandLeft)
			g_kmLandscape.SetLandscape(g_kmCurrent);
		else
			g_kmLandscape.SetLandscapeRight(g_kmCurrent);

        g_pkmInUse = &g_kmLandscape;
    }
    else
        g_pkmInUse = &g_kmCurrent;        
}
