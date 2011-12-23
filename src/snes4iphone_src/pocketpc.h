enum EmulationMode { emStopped, emRunning, emPaused };

extern	EmulationMode	g_emMode;
extern	HBITMAP			g_hbmPausedDIB;
extern	HDC				g_hPausedDC;
extern  HINSTANCE		g_hInstance;
extern  uint32          g_iJoypadState;
extern	uint32			g_iCycles;
extern  bool            g_bLandscape;
extern  bool			g_bLandLeft;
extern  bool			g_bCompat;
extern  bool			g_bAutoSkip;
extern  bool            g_bSmoothStretch;
extern  HBITMAP         g_hbmSkin;
extern  HDC             g_hSkinDC;
extern  unsigned short *g_pKeypad;
extern  uint32          g_iFrameSkip;
extern  uint32          g_iSoundQuality;
extern  bool			g_bUseGameFolders;

extern void SetKeypad();
extern bool LoadState();
extern bool SaveState();
extern void StopEmulation();
extern bool StartEmulation(HWND, HWND);
extern bool8_32 SaveSlot(int);
extern bool8_32 LoadSlot(int);
extern bool LoadOptions();
  

