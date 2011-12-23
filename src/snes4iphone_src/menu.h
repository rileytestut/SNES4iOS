#ifndef _MENU_H_
#define _MENU_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dirent.h>
#include <errno.h>

#ifdef __GIZ__
#include <sys/wcetypes.h>
#include <sys/wcefile.h>
#include <KGSDK.h>
#include <Framework.h>
#include <Framework2D.h>
#include "giz_kgsdk.h"

#define DIR_SEPERATOR	"\\"
#define SYSTEM_DIR		"\\SD Card\\DrPocketSnes"
#endif

#ifdef __GP2X__
#include "gp2x_sdk.h"

#define DIR_SEPERATOR	"/"
#define SYSTEM_DIR		"/mnt/sd/DrPocketSnes"
#endif

#ifdef __IPHONE__
#include "iphone_sdk.h"

#import <Foundation/Foundation.h>
	
#define DIR_SEPERATOR	"/"
#define SYSTEM_DIR		[([NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) count] > 0) ? [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) objectAtIndex:0] : nil UTF8String]
#endif


#define SNES_OPTIONS_DIR		"options"
#define SNES_SRAM_DIR			"sram"
#define SNES_SAVESTATE_DIR		"saves"

#define ROM_LIST_FILENAME			"romlist.bin"
#define SRAM_FILE_EXT				"srm"
#define SAVESTATE_EXT				"sv"
#define MENU_OPTIONS_FILENAME		"menu"
#define MENU_OPTIONS_EXT			"opt"
#define DEFAULT_ROM_DIR_FILENAME	"romdir"
#define DEFAULT_ROM_DIR_EXT			"opt"


//define emulation modes
#define EMU_MODE_NONE	0
#define EMU_MODE_SNES 	1

#define SAVESTATE_MODE_SAVE				0
#define SAVESTATE_MODE_LOAD				1
#define SAVESTATE_MODE_DELETE			2

#define SNES_OPTIONS_VER 			1
#define DRSNES_VERSION				"snes4iphone v0.3.5"

#define ROM_SIZE 		0x500000 //ssf2(40mbits)
#define RGB(r,g,b) 		((r) << 11 | (g) << 6 | (b) << 0 )
#define MAX_ROMS		3000
#define MAX_CPU			39
#define MAX_PATH    			255

#define MENU_CPU_SPEED 			100
#define MENU_FAST_CPU_SPEED		200

#define FILE_TYPE_FILE								0
#define FILE_TYPE_DIRECTORY							1

#define MAIN_MENU_RETURN							0
#define MAIN_MENU_ROM_SELECT						1
#define MAIN_MENU_MANAGE_SAVE_STATE					2
#define MAIN_MENU_SAVE_SRAM							3
#define MAIN_MENU_SNES_OPTIONS						4
#define MAIN_MENU_RESET_GAME						5
#define MAIN_MENU_EXIT_APP							6
#define MAIN_MENU_COUNT								7

#define LOAD_ROM_MENU_SNES							0
#define LOAD_ROM_MENU_RETURN						1
#define LOAD_ROM_MENU_COUNT							2

#define SNES_MENU_SOUND								0
#define SNES_MENU_SOUND_RATE              			1
#define SNES_MENU_SOUND_VOL				       		2
#define SNES_MENU_CPUSPEED				       		3
#define SNES_MENU_FRAMESKIP               			4
#define SNES_MENU_ACTION_BUTTONS					5
#define SNES_MENU_FPS                     			6
#define SNES_MENU_GAMMA                   			7
#define SNES_MENU_TRANSPARENCY            			8
#define SNES_MENU_RENDER_MODE    					9
#define SNES_MENU_RAM_SETTINGS    					10
#define SNES_MENU_MMU_HACK		   					11
#define SNES_MENU_AUTO_SAVE_SRAM	    			12
#define SNES_MENU_LOAD_GLOBAL						13
#define SNES_MENU_SAVE_GLOBAL						14
#define SNES_MENU_DELETE_GLOBAL						15
#define SNES_MENU_LOAD_CURRENT						16
#define SNES_MENU_SAVE_CURRENT						17
#define SNES_MENU_DELETE_CURRENT					18
#define SNES_MENU_SET_ROMDIR						19
#define SNES_MENU_RETURN							20
#define SNES_MENU_COUNT								21

#define SAVESTATE_MENU_LOAD							0
#define SAVESTATE_MENU_SAVE							1
#define SAVESTATE_MENU_DELETE						2
#define SAVESTATE_MENU_RETURN						3
#define SAVESTATE_MENU_COUNT						4

#define SRAM_MENU_LOAD								0
#define SRAM_MENU_SAVE								1
#define SRAM_MENU_DELETE							2
#define SRAM_MENU_RETURN							3
#define SRAM_MENU_COUNT								4

#define EVENT_EXIT_APP								1
#define EVENT_LOAD_SNES_ROM							2
#define EVENT_RUN_SNES_ROM							3
#define EVENT_RESET_SNES_ROM						4

#define RENDER_MODE_UNSCALED						0
#define RENDER_MODE_SCALED							1

#define MENU_TILE_WIDTH      64
#define MENU_TILE_HEIGHT     64

#define GP32_GCC

#ifdef __GIZ__
#define INP_BUTTON_MENU_SELECT			INP_BUTTON_PLAY
#define INP_BUTTON_MENU_CANCEL			INP_BUTTON_STOP
#define INP_BUTTON_MENU_ENTER			INP_BUTTON_BRIGHT
#define INP_BUTTON_MENU_DELETE			INP_BUTTON_REWIND
#define INP_BUTTON_MENU_QUICKSAVE1		INP_BUTTON_R
#define INP_BUTTON_MENU_QUICKSAVE2		INP_BUTTON_BRIGHT
#define INP_BUTTON_MENU_QUICKLOAD1		INP_BUTTON_L
#define INP_BUTTON_MENU_QUICKLOAD2		INP_BUTTON_BRIGHT

//Menu Text
#define MENU_TEXT_LOAD_SAVESTATE 		"Press Play to load"
#define MENU_TEXT_OVERWRITE_SAVESTATE	"Press Play to overwrite"
#define MENU_TEXT_DELETE_SAVESTATE 		"Press Play to delete"
#define MENU_TEXT_PREVIEW_SAVESTATE 	"Press R to preview"
#endif

#ifdef __GP2X__
#define INP_BUTTON_MENU_SELECT			INP_BUTTON_B
#define INP_BUTTON_MENU_CANCEL			INP_BUTTON_X
#define INP_BUTTON_MENU_ENTER			INP_BUTTON_SELECT
#define INP_BUTTON_MENU_DELETE			INP_BUTTON_SELECT
#define INP_BUTTON_MENU_QUICKSAVE1		INP_BUTTON_R
#define INP_BUTTON_MENU_QUICKSAVE2		INP_BUTTON_SELECT
#define INP_BUTTON_MENU_QUICKLOAD1		INP_BUTTON_L
#define INP_BUTTON_MENU_QUICKLOAD2		INP_BUTTON_SELECT


//Menu Text
#define MENU_TEXT_LOAD_SAVESTATE 		"Press B to load"
#define MENU_TEXT_OVERWRITE_SAVESTATE	"Press B to overwrite"
#define MENU_TEXT_DELETE_SAVESTATE 		"Press B to delete"
#define MENU_TEXT_PREVIEW_SAVESTATE 	"Press Y to preview"
#endif

#ifdef __IPHONE__
#define INP_BUTTON_MENU_SELECT			INP_BUTTON_HARDLEFT
#define INP_BUTTON_MENU_CANCEL			INP_BUTTON_HARDDOWN
#define INP_BUTTON_MENU_ENTER			INP_BUTTON_SELECT
#define INP_BUTTON_MENU_DELETE			INP_BUTTON_SELECT
#define INP_BUTTON_MENU_QUICKSAVE1		INP_BUTTON_R
#define INP_BUTTON_MENU_QUICKSAVE2		INP_BUTTON_SELECT
#define INP_BUTTON_MENU_QUICKLOAD1		INP_BUTTON_L
#define INP_BUTTON_MENU_QUICKLOAD2		INP_BUTTON_SELECT


//Menu Text
#define MENU_TEXT_LOAD_SAVESTATE 		"Press B to load"
#define MENU_TEXT_OVERWRITE_SAVESTATE	"Press B to overwrite"
#define MENU_TEXT_DELETE_SAVESTATE 		"Press B to delete"
#define MENU_TEXT_PREVIEW_SAVESTATE 	"Press Y to preview"
#endif

typedef struct {
   char name[MAX_ROMS][MAX_PATH];    //  128 entrys,16 Bytes long
   int size[MAX_ROMS];
} DIRDATA;

//Graphics - moved to objects because they get updated with current gamma setting
extern unsigned short menuHeader[];
extern unsigned short menuHeaderOrig[];
extern unsigned short highLightBar[];
extern unsigned short highLightBarOrig[];
extern unsigned short menuTile[];
extern unsigned short menuTileOrig[];

extern unsigned char padConfig[];
extern float soundRates[];
extern char currentWorkingDir[];
extern char snesOptionsDir[];
extern char snesSramDir[];
extern char snesSaveStateDir[];
extern unsigned char gammaConv[];
extern char lastSaveName[];
extern short *soundBuffer;
extern unsigned char *RomData;
extern int currentEmuMode;
extern int lastStage;
extern int currFB;
extern int prevFB;
extern int saveStateSize;
extern int romLoaded;
extern int frames,taken; // Frames and 60hz ticks
extern char showFps;
extern char soundRate;
extern char soundOn;

void LoadStateFile(char *filename);
void SaveStateFile(char *filename);
void UpdateMenuGraphicsGamma(void);
int RoundDouble(double val);
void ClearScreen(unsigned int *buffer,unsigned int data);
void LoadSram(char *path,char *romname,char *ext,char *srammem);
void SaveSram(char *path,char *romname,char *ext,char *srammem);
void DeleteSram(char *path,char *romname,char *ext);
int SaveMenuOptions(char *path, char *filename, char *ext, char *optionsmem, int maxsize, int showMessage);
int LoadMenuOptions(char *path, char *filename, char *ext, char *optionsmem, int maxsize, int showMessage);
int DeleteMenuOptions(char *path, char *filename, char *ext, int showMessage);
void SnesDefaultMenuOptions(void);
#ifdef __GIZ__
void sync(void);
#endif
// menu.cpp
void MenuPause(void);
void MenuFlip(void);
void SplitFilename(char *wholeFilename, char *filename, char *ext);
int FileSelect(int mode);
int MainMenu(int prevAction);
void PrintTitle(int flip);
void PrintTile(int flip);
void PrintBar(int flip, unsigned int givenY);

int FileScan();
extern void loadStateFile(char *filename);
extern int quickSavePresent;
extern unsigned short cpuSpeedLookup[];
extern float gammaLookup[];

struct ROM_LIST_RECORD
{
	char filename[MAX_PATH+1];
	char type;
};

extern struct ROM_LIST_RECORD romList[];
extern int currentRomIndex;
extern char currentRomFilename[];
extern char romDir[];
extern char snesRomDir[];

struct SNES_MENU_OPTIONS
{
  unsigned char menuVer;
  unsigned char frameSkip;
  unsigned char soundOn;
  unsigned char cpuSpeed;
  unsigned char padConfig[32];
  unsigned char tripleBuffer;
  unsigned char forceRegion;
  unsigned char showFps;
  signed char gamma;
  unsigned char lcdver;
  unsigned char stereo;
  unsigned char soundRate;
  unsigned char autoSram;
  unsigned char renderMode;
  unsigned char volume;
  unsigned char actionButtons;
  unsigned char transparency;
  unsigned char ramSettings;
  unsigned char mmuHack;
  unsigned char spare13;
  unsigned char spare14;
  unsigned char spare15;
  unsigned char spare16;
  unsigned char spare17;
  unsigned char spare18;
  unsigned char spare19;
  unsigned char spare1A;
  unsigned char spare1B;
  unsigned char spare1C;
  unsigned char spare1D;
  unsigned char spare1E;
  unsigned char spare1F;
};

extern struct SNES_MENU_OPTIONS snesMenuOptions;

struct SAVE_STATE
{
  char filename[MAX_PATH+1];
  char fullFilename[MAX_PATH+1];
  unsigned int inUse;
};


extern struct SAVE_STATE saveState[];  // holds the filenames for the savestate and "inuse" flags
extern char saveStateName[];

// Input.cpp
struct INPUT
{
  unsigned int held[32];
  unsigned int repeat[32];
};
extern struct INPUT Inp;

int InputInit();
int InputUpdate(int EnableDiagnals);

#ifdef __cplusplus
}
#endif

#endif /* _MENU_H_ */





