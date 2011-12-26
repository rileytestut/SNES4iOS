
#include "menu.h"

char romDir[MAX_PATH+1];
char snesRomDir[MAX_PATH+1];

DIRDATA dir;

unsigned short cpuSpeedLookup[40]={ 
    10,20, 30, 40, 50,
    60,70, 80, 90,100,
    110,120,130,144,150,
    160,170,180,190,200,
    210,220,230,240,250,
    260,270,280,290,300,
    310,320,330,340,350,
    360,370,380,390,400};
extern unsigned char *vrambuffer;
extern volatile int timer;
static int menutileXscroll=0;
static int menutileYscroll=0;
static int headerDone[4]; // variable that records if header graphics have been rendered or not
int quickSavePresent=0;

struct ROM_LIST_RECORD romList[MAX_ROMS];
struct SNES_MENU_OPTIONS snesMenuOptions;

static int romCount;
int currentRomIndex=2;
char currentRomFilename[MAX_PATH+1]="";
int currFB=0;
int prevFB=0;
int currentEmuMode=EMU_MODE_SNES;

char currentWorkingDir[MAX_PATH+1];
char snesOptionsDir[MAX_PATH+1];
char snesSramDir[MAX_PATH+1];
char snesSaveStateDir[MAX_PATH+1];
float soundRates[5]={8250.0,11025.0,16500.0,22050.0,44100.0};
char menutext[256][50];

struct SAVE_STATE saveState[10];  // holds the filenames for the savestate and "inuse" flags
char saveStateName[MAX_PATH+MAX_PATH+2];       // holds the last filename to be scanned for save states

unsigned char gammaConv[32*29]={   0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
    0, 2, 3, 5, 6, 7, 8, 9, 10, 12, 13, 14, 15, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 24, 25, 26, 27, 28, 29, 29, 30, 31,
    0, 3, 5, 7, 8, 9, 10, 11, 13, 14, 15, 16, 16, 17, 18, 19, 20, 21, 22, 22, 23, 24, 25, 25, 26, 27, 28, 28, 29, 30, 30, 31,
    0, 4, 6, 8, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 20, 21, 22, 23, 23, 24, 25, 25, 26, 27, 27, 28, 29, 29, 30, 30, 31,
    0, 6, 8, 10, 11, 12, 14, 15, 16, 17, 18, 18, 19, 20, 21, 22, 22, 23, 24, 24, 25, 26, 26, 27, 27, 28, 28, 29, 29, 30, 30, 31,
    0, 7, 9, 11, 12, 14, 15, 16, 17, 18, 19, 20, 20, 21, 22, 22, 23, 24, 24, 25, 26, 26, 27, 27, 28, 28, 29, 29, 30, 30, 31, 31,
    0, 8, 10, 12, 14, 15, 16, 17, 18, 19, 20, 20, 21, 22, 23, 23, 24, 24, 25, 25, 26, 27, 27, 28, 28, 28, 29, 29, 30, 30, 31, 31,
    0, 9, 11, 13, 15, 16, 17, 18, 19, 20, 21, 21, 22, 23, 23, 24, 24, 25, 25, 26, 26, 27, 27, 28, 28, 29, 29, 29, 30, 30, 31, 31,
    0, 10, 12, 14, 16, 17, 18, 19, 20, 21, 21, 22, 23, 23, 24, 24, 25, 25, 26, 26, 27, 27, 28, 28, 28, 29, 29, 30, 30, 30, 31, 31,
    0, 11, 13, 15, 17, 18, 19, 20, 20, 21, 22, 23, 23, 24, 24, 25, 25, 26, 26, 27, 27, 27, 28, 28, 29, 29, 29, 30, 30, 30, 31, 31,
    0, 12, 14, 16, 17, 18, 19, 20, 21, 22, 22, 23, 24, 24, 25, 25, 26, 26, 27, 27, 27, 28, 28, 28, 29, 29, 29, 30, 30, 30, 31, 31,
    0, 12, 15, 17, 18, 19, 20, 21, 22, 22, 23, 24, 24, 25, 25, 26, 26, 26, 27, 27, 28, 28, 28, 29, 29, 29, 30, 30, 30, 30, 31, 31,
    0, 13, 16, 17, 19, 20, 21, 21, 22, 23, 23, 24, 24, 25, 25, 26, 26, 27, 27, 27, 28, 28, 28, 29, 29, 29, 30, 30, 30, 30, 31, 31,
    0, 14, 16, 18, 19, 20, 21, 22, 23, 23, 24, 24, 25, 25, 26, 26, 27, 27, 27, 28, 28, 28, 29, 29, 29, 29, 30, 30, 30, 31, 31, 31,
    0, 14, 17, 18, 20, 21, 22, 22, 23, 24, 24, 25, 25, 26, 26, 26, 27, 27, 27, 28, 28, 28, 29, 29, 29, 30, 30, 30, 30, 31, 31, 31,
    0, 15, 17, 19, 20, 21, 22, 23, 23, 24, 24, 25, 25, 26, 26, 27, 27, 27, 28, 28, 28, 29, 29, 29, 29, 30, 30, 30, 30, 31, 31, 31,
    0, 16, 18, 19, 21, 22, 22, 23, 24, 24, 25, 25, 26, 26, 26, 27, 27, 27, 28, 28, 28, 29, 29, 29, 29, 30, 30, 30, 30, 31, 31, 31,
    0, 16, 18, 20, 21, 22, 23, 23, 24, 24, 25, 25, 26, 26, 27, 27, 27, 28, 28, 28, 29, 29, 29, 29, 30, 30, 30, 30, 30, 31, 31, 31,
    0, 17, 19, 20, 21, 22, 23, 24, 24, 25, 25, 26, 26, 26, 27, 27, 27, 28, 28, 28, 29, 29, 29, 29, 30, 30, 30, 30, 30, 31, 31, 31,
    0, 17, 19, 21, 22, 23, 23, 24, 24, 25, 25, 26, 26, 27, 27, 27, 28, 28, 28, 28, 29, 29, 29, 29, 30, 30, 30, 30, 30, 31, 31, 31,
    0, 17, 20, 21, 22, 23, 24, 24, 25, 25, 26, 26, 26, 27, 27, 27, 28, 28, 28, 29, 29, 29, 29, 29, 30, 30, 30, 30, 30, 31, 31, 31,
    0, 18, 20, 21, 22, 23, 24, 24, 25, 25, 26, 26, 27, 27, 27, 28, 28, 28, 28, 29, 29, 29, 29, 30, 30, 30, 30, 30, 30, 31, 31, 31,
    0, 18, 20, 22, 23, 23, 24, 25, 25, 26, 26, 26, 27, 27, 27, 28, 28, 28, 29, 29, 29, 29, 29, 30, 30, 30, 30, 30, 31, 31, 31, 31,
    0, 19, 21, 22, 23, 24, 24, 25, 25, 26, 26, 27, 27, 27, 28, 28, 28, 28, 29, 29, 29, 29, 29, 30, 30, 30, 30, 30, 31, 31, 31, 31,
    0, 19, 21, 22, 23, 24, 25, 25, 26, 26, 26, 27, 27, 27, 28, 28, 28, 28, 29, 29, 29, 29, 30, 30, 30, 30, 30, 30, 31, 31, 31, 31,
    0, 19, 21, 22, 23, 24, 25, 25, 26, 26, 27, 27, 27, 27, 28, 28, 28, 29, 29, 29, 29, 29, 30, 30, 30, 30, 30, 30, 31, 31, 31, 31,
    0, 20, 22, 23, 24, 24, 25, 25, 26, 26, 27, 27, 27, 28, 28, 28, 28, 29, 29, 29, 29, 29, 30, 30, 30, 30, 30, 30, 31, 31, 31, 31,
    0, 20, 22, 23, 24, 24, 25, 26, 26, 26, 27, 27, 27, 28, 28, 28, 28, 29, 29, 29, 29, 29, 30, 30, 30, 30, 30, 30, 31, 31, 31, 31,
    0, 20, 22, 23, 24, 25, 25, 26, 26, 27, 27, 27, 28, 28, 28, 28, 29, 29, 29, 29, 29, 30, 30, 30, 30, 30, 30, 30, 31, 31, 31, 31};


void UpdateMenuGraphicsGamma(void)
{
    unsigned int currPix=0;
    unsigned short pixel=0;
    unsigned char R,G,B;
    for(currPix=0;currPix<15360;currPix++)
    {
        // md  0000 bbb0 ggg0 rrr0
        // gp  rrrr rggg ggbb bbbi
        pixel=menuHeaderOrig[currPix];
        R=(pixel>>11)&0x1F; // 0000 0RRR - 3 bits Red
        G=(pixel>>6)&0x1F; 
        B=(pixel>>1)&0x1F;
        
        // Do gamma correction  
        R=gammaConv[R+(0<<5)];
        G=gammaConv[G+(0<<5)];
        B=gammaConv[B+(0<<5)];
        
        pixel=RGB(R,G,B);
        menuHeader[currPix]=pixel;
    }
    for(currPix=0;currPix<5120;currPix++)
    {
        // md  0000 bbb0 ggg0 rrr0
        // gp  rrrr rggg ggbb bbbi
        pixel=highLightBarOrig[currPix];
        R=(pixel>>11)&0x1F; // 0000 0RRR - 3 bits Red
        G=(pixel>>6)&0x1F; 
        B=(pixel>>1)&0x1F;
        
        // Do gamma correction  
        R=gammaConv[R+(0<<5)];
        G=gammaConv[G+(0<<5)];
        B=gammaConv[B+(0<<5)];
        
        pixel=RGB(R,G,B);
        highLightBar[currPix]=pixel;
        
    }
    
    for(currPix=0;currPix<(MENU_TILE_WIDTH*MENU_TILE_HEIGHT);currPix++)
    {
        // md  0000 bbb0 ggg0 rrr0
        // gp  rrrr rggg ggbb bbbi
        pixel=menuTileOrig[currPix];
        R=(pixel>>11)&0x1F; // 0000 0RRR - 3 bits Red
        G=(pixel>>6)&0x1F; 
        B=(pixel>>1)&0x1F;
        
        // Do gamma correction  
        R=gammaConv[R+(0<<5)];
        G=gammaConv[G+(0<<5)];
        B=gammaConv[B+(0<<5)];
        
        pixel=RGB(R,G,B);
        menuTile[currPix]=pixel;
        
    }
}

void SnesDefaultMenuOptions(void)
{
	// no options file loaded, so set to defaults
	snesMenuOptions.menuVer=SNES_OPTIONS_VER;
	snesMenuOptions.frameSkip=0;
	snesMenuOptions.soundOn = 1; 
	snesMenuOptions.volume=100; 
	memset(snesMenuOptions.padConfig,0xFF,sizeof(snesMenuOptions.padConfig));
	snesMenuOptions.showFps=1;
	snesMenuOptions.gamma=0;
	snesMenuOptions.soundRate=2;
	snesMenuOptions.cpuSpeed=19;
}

int LoadMenuOptions(char *path, char *filename, char *ext, char *optionsmem, int maxsize, int showMessage)
{
	char fullFilename[MAX_PATH+MAX_PATH+1];
	char _filename[MAX_PATH+1];
	char _ext[MAX_PATH+1];
	FILE *stream;
	int size=0;
	char text[50];
	
	sprintf(text,"Loading...");
	//Sometimes the on screen messages are not required
	if (showMessage)
	{
		PrintBar(prevFB,240-16);
		gp_drawString(40,228,strlen(text),text,(unsigned short)RGB(0,0,0),(unsigned short*)vrambuffer);
	}
	
    SplitFilename(filename, _filename, _ext);
	sprintf(fullFilename,"%s%s%s.%s",path,DIR_SEPERATOR,_filename,ext);
	stream=fopen(fullFilename,"rb");
	if(stream)
	{
		// File exists do try to load it
		fseek(stream,0,SEEK_END);
		size=ftell(stream);
		if (size>maxsize) size=maxsize;
		fseek(stream,0,SEEK_SET);
		fread(optionsmem, 1, size, stream);
		fclose(stream);
		return(0);
	}
	else
	{
		return(1);
	}
}

int SaveMenuOptions(char *path, char *filename, char *ext, char *optionsmem, int maxsize, int showMessage)
{
	char fullFilename[MAX_PATH+MAX_PATH+1];
	char _filename[MAX_PATH+1];
	char _ext[MAX_PATH+1];
	FILE *stream;
	char text[50];
	
	sprintf(text,"Saving...");
	//Sometimes the on screen messages are not required
	if (showMessage)
	{
		PrintBar(prevFB,240-16);
		gp_drawString(40,228,strlen(text),text,(unsigned short)RGB(0,0,0),(unsigned short*)vrambuffer);
	}
	
	SplitFilename(filename, _filename, _ext);
	sprintf(fullFilename,"%s%s%s.%s",path,DIR_SEPERATOR,_filename,ext);
	stream=fopen(fullFilename,"wb");
	if(stream)
	{
		fwrite(optionsmem, 1, maxsize, stream);
		fclose(stream);
		sync();
		return(0);
	}
	else
	{
		return(1);
	}
}

int DeleteMenuOptions(char *path, char *filename, char *ext, int showMessage)
{
	char fullFilename[MAX_PATH+MAX_PATH+1];
	char _filename[MAX_PATH+1];
	char _ext[MAX_PATH+1];
	char text[50];
	
	sprintf(text,"Deleting...");
	//Sometimes the on screen messages are not required
	if (showMessage)
	{
		PrintBar(prevFB,240-16);
		gp_drawString(40,228,strlen(text),text,(unsigned short)RGB(0,0,0),(unsigned short*)vrambuffer);
	}
	
	SplitFilename(filename, _filename, _ext);
	sprintf(fullFilename,"%s%s%s.%s",path,DIR_SEPERATOR,_filename,ext);
	remove(fullFilename);
	sync();
	return(0);
}

#ifdef __GIZ__
void sync(void)
{
}
#endif

static void WaitForButtonsUp(void)
{
	int i=0,j=0,z=0;
	
	for(i=0;i<100;i++)
	{
		while(1)
		{     
			InputUpdate(0);
			z=0;
			for (j=0;j<32;j++)
			{
				if (Inp.held[j]) z=1;
			}
			if (z==0) break;
		}
	}
}

void MenuPause()
{
	int i=0,j=0,z=0;
	// wait for keys to be released
	for(i=0;i<100;i++)  // deal with keybounce by checking a few times
	{
		while(1)
		{     
			InputUpdate(0);
			z=0;
			for (j=0;j<32;j++)
			{
				if (Inp.held[j]) z=1;
			}
			if (z==0) break;
		}
	}
	
	for(i=0;i<100;i++)  // deal with keybounce by checking a few times
	{
		while(1)
		{     
			InputUpdate(0);
			z=0;
			for (j=0;j<32;j++)
			{
				if (Inp.held[j]) z=1;
			}
			if (z==1) break;
		}
	}
}
#if defined (__GP2X__)	
void MenuFlip()
{
	prevFB=currFB;
	gp_setFramebuffer(currFB,1);
    currFB++;
	currFB&=3;
}
#endif
#if defined (__GIZ__)	
void MenuFlip()
{
	prevFB=currFB=0;
	gp_setFramebuffer(currFB,0);
}
#endif
#if defined (__IPHONE__)	
void MenuFlip()
{
	prevFB=currFB=0;
	gp_setFramebuffer(currFB,0);
}
#endif

void SplitFilename(char *wholeFilename, char *filename, char *ext)
{
	int len=strlen(wholeFilename);
	int i=0,y=-1;
    
	ext[0]=0;
	filename[0]=0;
	//Check given string is not null
	if (len<=0)
	{
		return;
	}
	y=-1;
	for(i=len-2;i>0;i--)
	{
		if (wholeFilename[i]=='.')
		{
			y=i;
			break;
		}
	}
	
	if (y>=0)
	{
		memcpy(filename,wholeFilename,y);
		filename[y]=0; // change "." to zero to end string
		memcpy(ext,wholeFilename+y+1,len-(y+1));
		//ext[len-(y+1)+1]=0;
		ext[len-(y+1)]=0;
	}
	else
	{
		strcpy(filename,wholeFilename);
	}
}


int MessageBox(char *message1,char *message2,char *message3,int mode)
{
    int select=0;
    int subaction=-1;
    int len=0;
    while(subaction==-1)
    {
        InputUpdate(0);
        if (Inp.repeat[INP_BUTTON_UP]) 
        {
            select^=1; // Up
        }
        if (Inp.repeat[INP_BUTTON_DOWN]) 
        {
            select^=1; // Down
        }
        if ((Inp.held[INP_BUTTON_MENU_SELECT]==1) || (Inp.held[INP_BUTTON_MENU_CANCEL]==1))
        {
            subaction=select;
        }
        PrintTile(currFB);
        PrintTitle(currFB);
        len=strlen(message1);
        if(len>39)len=39;
        gp_drawString(8,50,len,message1,(unsigned short)RGB(31,31,31),(unsigned short*)vrambuffer);
        len=strlen(message2);
        if(len>39)len=39;
        gp_drawString(8,60,len,message2,(unsigned short)RGB(31,31,31),(unsigned short*)vrambuffer);
        len=strlen(message3);
        if(len>39)len=39;
        gp_drawString(8,70,len,message3,(unsigned short)RGB(31,31,31),(unsigned short*)vrambuffer);
        switch(mode)
        {
            case 0: // yes no input
                if(select==0)
                {
                    PrintBar(currFB, 120-4);
                    gp_drawString(8,120,3,"YES",(unsigned short)RGB(0,0,0),(unsigned short*)vrambuffer);
                    gp_drawString(8,140,2,"NO",(unsigned short)RGB(31,31,31),(unsigned short*)vrambuffer);
                }
                else
                {
                    PrintBar(currFB, 140-4);
                    gp_drawString(8,120,3,"YES",(unsigned short)RGB(31,31,31),(unsigned short*)vrambuffer);
                    gp_drawString(8,140,2,"NO",(unsigned short)RGB(0,0,0),(unsigned short*)vrambuffer);
                    
                }
                break;
        }
        MenuFlip();
    }
    return(subaction);
}

static
int deleterom(int romindex)
{
	char text[MAX_PATH+1];
	char fullfilename[MAX_PATH+MAX_PATH+1];
	int x;
	FILE *stream=NULL;
	
    PrintTile(currFB);
    PrintTitle(currFB);
    MenuFlip();
	
    sprintf(text,"Deleting Rom..");
    gp_drawString(8,50,strlen(text),text,(unsigned short)RGB(31,31,31),(unsigned short*)vrambuffer);
	
    sprintf(text,"%s",romList[romindex].filename);
	x=strlen(text);
	if(x>40) x=40;
	gp_drawString(0,60,x,text,(unsigned short)RGB(31,31,31),(unsigned short*)vrambuffer);
	
	sprintf(fullfilename,"%s%s%s",romDir,DIR_SEPERATOR,romList[romindex].filename);
    remove(fullfilename);
	sync();
	
    sprintf(text,"Updating Rom List..");
    gp_drawString(8,70,strlen(text),text,(unsigned short)RGB(31,31,31),(unsigned short*)vrambuffer);
    for(x=romindex;x<romCount;x++)
    {
		strcpy(romList[x].filename, romList[x+1].filename);
		romList[x].type = romList[x+1].type;
    }
    romCount--;
    currentRomIndex--;
	
	return(1);
}

static int tileCounter=0;
void PrintTile(int flip)
{
	short x=0,x2=0;
	short y=0,y2=0;
	unsigned short *framebuffer1 = (unsigned short*)vrambuffer+(48*320);
	unsigned short *graphics1 = NULL;
    
	x2=menutileXscroll;
	y2=(menutileYscroll*MENU_TILE_WIDTH);
	graphics1 = menuTile+y2;
	for (y=0; y<(240-48); y++)
	{
		for (x=0; x<320; x++)
		{
			*framebuffer1++ = graphics1[x2];
			x2++;
			x2&=(MENU_TILE_WIDTH-1);
		}
		y2+=MENU_TILE_WIDTH;
		y2&=((MENU_TILE_HEIGHT*MENU_TILE_WIDTH)-1);
		graphics1=menuTile+y2;
	}
    
	tileCounter++;
	if (tileCounter > 5)
	{
		tileCounter=0;
		menutileXscroll++;
		if(menutileXscroll>=MENU_TILE_WIDTH) menutileXscroll=0;
        
		menutileYscroll++;
		if(menutileYscroll>=MENU_TILE_HEIGHT) menutileYscroll=0;
	}  
	return; 
}

void PrintTitle(int flip)
{
	unsigned short *framebuffer = (unsigned short*)vrambuffer;
	unsigned short *graphics = (unsigned short*)menuHeader;
	unsigned int x,y;
	char text[256];
	//If header already drawn for this layer exit
	if (headerDone[flip]) return;
    
	for (y=0; y<48; y++)
	{
		for (x=0; x<320; x++)
		{
			*framebuffer++ = *graphics++;
		}
	}
    
	sprintf(text,"%s",DRSNES_VERSION);
	gp_drawString(175,15,strlen(text),text,(unsigned short)RGB(0,0,31),(unsigned short*)vrambuffer);
	headerDone[currFB] = 1;
}

void PrintBar(int flip, unsigned int givenY)
{
    unsigned int *framebuffer1 = NULL;
    unsigned int *graphics1 = (unsigned int *)highLightBar;
    unsigned int x,y;
    
	framebuffer1 = (unsigned int*)(unsigned short*)vrambuffer+(givenY*160);
	for (y=0; y<16; y++)
	{
		for (x=0; x<160; x++)
		{
			*framebuffer1++ = *graphics1++;
		}
	}
}

static int StringCompare(char *string1, char *string2)
{
	int i=0;
	char c1=0,c2=0;
	while(1)
	{
		c1=string1[i];
		c2=string2[i];
		// check for string end
		
		if ((c1 == 0) && (c2 == 0)) return 0;
		if (c1 == 0) return 1;
		if (c2 == 0) return -1;
		
		if ((c1 >= 0x61)&&(c1<=0x7A)) c1-=0x20;
		if ((c2 >= 0x61)&&(c2<=0x7A)) c2-=0x20;
		if (c1>c2)
			return 1;
		else if (c1<c2)
			return -1;
		i++;
	}
    
}

#ifdef __GIZ__
static BOOL CharToWChar(wchar_t *wc, char *c)
{
	int len=strlen(c);
	int x=0;
	for (x=0;x<len;x++)
	{
		wc[x] = btowc(c[x]);
	}
	wc[len]=0;
	return TRUE;
}
#endif

int FileScan()
{
	int i=0,j=0;
	char text[256];
	DIR *d;
	struct dirent  *de;
	char dirCheck[MAX_PATH+1];
	int dirCount=0;
	char _filename[MAX_PATH+1];
	char _ext[MAX_PATH+1];
    
#ifdef __GIZ__	
	wchar_t  wc[MAX_PATH+1];
	HANDLE hTest;
    WIN32_FIND_DATAW fileInfo;
#endif
	
	//gp_setCpuspeed(MENU_FAST_CPU_SPEED);
    
	PrintTile(currFB);
	PrintTitle(currFB);
	gp_drawString(8,120,25,"Getting Directory Info...",(unsigned short)RGB(31,31,31),(unsigned short*)vrambuffer);
	MenuFlip();
	
	for (i=0;i<MAX_ROMS;i++)
	{
		romList[i].filename[0] = 0;
	}
    
	//Get rom directory details
	romCount=0;
	
	// Now sort the directory details
	sprintf(romList[0].filename,"Back To Main Menu");
	sprintf(romList[1].filename,"..");
	romList[2].filename[0] = 0;
	romCount=3;
    
	d = opendir(romDir);
    
	if (d)
	{
		while ((de = readdir(d)))
		{
			if (de->d_name[0] != '.')
			{
#if defined(__GP2X__) || defined(__IPHONE__)
				if (de->d_type == 4) // Directory
				{
#endif
#ifdef __GIZ__
                    // Because windows GNU library does not return the file type
                    // property I will have to try and open each file as a directory instead
                    sprintf(dirCheck,"%s%s%s",romDir,DIR_SEPERATOR,de->d_name);
                    CharToWChar(wc, dirCheck);
                    hTest=FindFirstFileW(wc, &fileInfo);
                    if (fileInfo.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
                    {
#endif				
                        for (i=3;i<=(romCount+1);i++)
                        {
                            if (romList[i].filename[0] == 0) // string is empty so shove new value in
                            {
                                strcpy(romList[i].filename,de->d_name);
                                romList[i].type=FILE_TYPE_DIRECTORY;//de->d_type;
                                break;
                            }
                            else
                            {
                                if ((StringCompare(romList[i].filename,de->d_name) > 0) ||
                                    (romList[i].type != FILE_TYPE_DIRECTORY))
                                {
                                    // new entry is lower than current string so move all entries up one and insert
                                    // new value in
                                    for (j=romCount;j>=i;j--)
                                    {
                                        strcpy(romList[j+1].filename,romList[j].filename);
                                        romList[j+1].type=romList[j].type;
                                    }
                                    strcpy(romList[i].filename,de->d_name);
                                    romList[i].type=FILE_TYPE_DIRECTORY;//de->d_type;
                                    break;
                                }
                            }
                        }
                        dirCount++;
                        romCount++;
                    }
                    else // File
                    {
                        // only interested in Zip and SMC files
                        SplitFilename(de->d_name,_filename,_ext);
                        if ((StringCompare(_ext,"zip") == 0) ||
                            (StringCompare(_ext,"smc") == 0) ||
                            (StringCompare(_ext,"sfc") == 0))
                        {
                            for (i=3+dirCount;i<=(romCount+1);i++)
                            {
                                if (romList[i].filename[0] == 0) // string is empty so shove new value in
                                {
                                    strcpy(romList[i].filename,de->d_name);
                                    romList[i].type=FILE_TYPE_FILE;//de->d_type;
                                    break;
                                }
                                else
                                {
                                    if (StringCompare(romList[i].filename,de->d_name) > 0)
                                    {
                                        // new entry is lower than current string so move all entries up one and insert
                                        // new value in
                                        for (j=romCount;j>=i;j--)
                                        {
                                            strcpy(romList[j+1].filename,romList[j].filename);
                                            romList[j+1].type=romList[j].type;
                                        }
                                        strcpy(romList[i].filename,de->d_name);
                                        romList[i].type=FILE_TYPE_FILE;//de->d_type;
                                        break;
                                    }
                                }
                            }
                            romCount++;
                        }
                    }
                    
                    
                    if (romCount > MAX_ROMS)
                    {
                        PrintTile(currFB);
                        PrintTitle(currFB);
                        sprintf(text,"Max rom limit exceeded! %d max",MAX_ROMS);
                        gp_drawString(8,120,strlen(text),text,(unsigned short)RGB(31,31,31),(unsigned short*)vrambuffer);
                        sprintf(text,"Please reduce number of roms");
                        gp_drawString(8,130,strlen(text),text,(unsigned short)RGB(31,31,31),(unsigned short*)vrambuffer);
                        MenuFlip();
                        MenuPause();
                        return MAX_ROMS;
                    }
                }
            }
            closedir(d);
        }
        //gp_setCpuspeed(MENU_CPU_SPEED);
        return romCount;
    }
    
    int FileSelect(int mode)
    {
        char text[256];
        int romname_length;
        int action=0;
        int smooth=0;
        unsigned short color=0;
        int i=0;
        int focus=2;
        int menuExit=0;
        int scanstart=0,scanend=0;
        char directorySeparator[2] = DIR_SEPERATOR; // used for char comparison;
        
        FileScan();
        if(focus<2) focus=2; 	// default menu to non menu item
        // just to stop directory scan being started 
        smooth=focus<<8;
        
        while (menuExit==0)
        {
            InputUpdate(0);
            
            // Change which rom is focused on:
            if (Inp.repeat[INP_BUTTON_UP])
            {
                focus--; // Up
            }
            if (Inp.repeat[INP_BUTTON_DOWN])
            {
                focus++; // Down
            }
            
            if (Inp.held[INP_BUTTON_MENU_CANCEL]==1 ) {action=0; menuExit=1;}
            
            if (Inp.repeat[INP_BUTTON_LEFT] || Inp.repeat[INP_BUTTON_RIGHT]   )
            {
                if (Inp.repeat[INP_BUTTON_LEFT]) 
                {
                    focus-=12;
                    smooth=(focus<<8)-1;
                }      
                else if (Inp.repeat[INP_BUTTON_RIGHT])
                {
                    focus+=12;
                    smooth=(focus<<8)-1;
                }
                
                if (focus>romCount-1) 
                {
                    focus=romCount-1;
                    smooth=(focus<<8)-1;
                }
                else if (focus<0)
                {
                    focus=0;
                    smooth=(focus<<8)-1;
                }
            }
            
            if (focus>romCount-1) 
            {
                focus=0;
                smooth=(focus<<8)-1;
            }
            else if (focus<0)
            {
                focus=romCount-1;
                smooth=(focus<<8)-1;
            }
            
            if (Inp.held[INP_BUTTON_MENU_SELECT]==1)
            {
                if(focus==0)
                {
                    action=0;
                    menuExit=1;
                }
                else if(focus==1)
                {
                    // up a directory
                    //Remove a directory from RomPath and rescan
                    for(i=strlen(romDir)-1;i>0;i--) // don't want to change first char in screen
                    {
                        if(romDir[i] == directorySeparator[0])
                        {
                            romDir[i] = 0;
                            break;
                        }
                    }
                    FileScan();
                    focus=2; // default menu to non menu item
                    // just to stop directory scan being started 
                    smooth=focus<<8;
                }	
                else if(focus==2)
                {
                    // nothing blank entry
                }		
                else
                {
                    // normal file or dir selected
                    if (romList[focus].type == FILE_TYPE_DIRECTORY)
                    {
                        sprintf(romDir,"%s%s%s",romDir,DIR_SEPERATOR,romList[focus].filename);
                        FileScan();
                        focus=2; // default menu to non menu item
                        // just to stop directory scan being started 
                        smooth=focus<<8;
                    }
                    else
                    {
                        // user has selected a rom, so load it
                        currentRomIndex=focus;
                        quickSavePresent=0;  // reset any quick saves
                        action=1;
                        menuExit=1;
                    }
                }
            }
            
            if (Inp.held[INP_BUTTON_MENU_DELETE]==1)
            {
                if(focus>2)
                {
                    //delete current rom
                    if (romList[focus].type != FILE_TYPE_DIRECTORY)
                    {
                        sprintf(text,"%s",romList[focus].filename);
                        
                        if(MessageBox("Are you sure you want to delete",text,"",0)==0)
                        {
                            deleterom(focus);
                        }
                    }
                }
            }
            
            // Draw screen:
            PrintTile(currFB);
            PrintTitle(currFB);
            if(mode==0) gp_drawString(6,35,10,"Select Rom",(unsigned short)RGB(31,0,0),(unsigned short*)vrambuffer); 
            if(mode==1) gp_drawString(6,35,10,"Delete Rom",(unsigned short)RGB(31,0,0),(unsigned short*)vrambuffer); 
            
            smooth=smooth*7+(focus<<8); smooth>>=3;
            
            scanstart=focus-15;
            if (scanstart<0) scanstart=0;
            scanend = focus+15;
            if (scanend>romCount) scanend=romCount;
            
            for (i=scanstart;i<scanend;i++)
            {
                int x=0,y=0;
                
                y=(i<<4)-(smooth>>4);
                x=8;
                y+=112;
                if (y<=48 || y>=232) continue;
                
                if (i==focus)
                {
                    color=(unsigned short)RGB(0,0,0);
                    PrintBar(currFB,y-4);
                }
                else
                {
                    color=(unsigned short)RGB(31,31,31);
                }
                
                // Draw Directory icon if current entry is a directory
                if(romList[i].type == FILE_TYPE_DIRECTORY)
                {
                    gp_drawString(x-8,y,1,"+",color,(unsigned short*)vrambuffer); 
                }
                
                romname_length=strlen(romList[i].filename);
                if(romname_length>39) romname_length=39;
                gp_drawString(x,y,romname_length,romList[i].filename,color,(unsigned short*)vrambuffer); 
            }
            
            MenuFlip();
        }
        
        return action;
    }
    
    static void ScanSaveStates(char *romname)
    {
        FILE *stream;
        int i=0;
        char savename[MAX_PATH+1];
        char filename[MAX_PATH+1];
        char ext[MAX_PATH+1];
        
        if(!strcmp(romname,saveStateName)) return; // is current save state rom so exit
        
        SplitFilename(romname,filename,ext);
        
        sprintf(savename,"%s.%s",filename,SAVESTATE_EXT);
        
        for(i=0;i<10;i++)
        {
            /*
             need to build a save state filename
             all saves are held in current working directory (snesSaveStateDir)
             save filename has following format
             shortname(minus file ext) + SV + saveno ( 0 to 9 )
             */
            sprintf(saveState[i].filename,"%s%d",savename,i);
            sprintf(saveState[i].fullFilename,"%s%s%s",snesSaveStateDir,DIR_SEPERATOR,saveState[i].filename);
            stream=(FILE*)fopen(saveState[i].fullFilename,"rb");
            if(stream)
            {
                // we have a savestate
                saveState[i].inUse = 1;
                fclose(stream);	
            }
            else
            {
                // no save state
                saveState[i].inUse = 0;
            }
        }
        strcpy(saveStateName,romname);  // save the last scanned romname
    }
    
    void LoadStateFile(char *filename)
    {
        S9xUnfreezeGame(filename);
    }
    
    void SaveStateFile(char *filename)
    {
        S9xFreezeGame(filename);
        sync();
    }
    
    static int SaveStateSelect(int mode)
    {
        char text[128];
        int action=11;
        int saveno=0;
        
        if(currentRomIndex<=2)
        {
            // no rom loaded
            // display error message and exit
            return(0);
        }
        
        memset(&headerDone,0,sizeof(headerDone));
        ScanSaveStates(currentRomFilename);
        
        while (action!=0&&action!=100)
        {
            InputUpdate(0);
            if(Inp.held[INP_BUTTON_UP]==1) {saveno--; action=1;}
            if(Inp.held[INP_BUTTON_DOWN]==1) {saveno++; action=1;}
            if(saveno<-1) saveno=9;
            if(saveno>9) saveno=-1;
            
            if(Inp.held[INP_BUTTON_MENU_CANCEL]==1) action=0; // exit
            else if((Inp.held[INP_BUTTON_MENU_SELECT]==1)&&(saveno==-1)) action=0; // exit
            else if((Inp.held[INP_BUTTON_MENU_SELECT]==1)&&(mode==0)&&((action==2)||(action==5))) action=6;  // pre-save mode
            else if((Inp.held[INP_BUTTON_MENU_SELECT]==1)&&(mode==1)&&(action==5)) action=8;  // pre-load mode
            else if((Inp.held[INP_BUTTON_MENU_SELECT]==1)&&(mode==2)&&(action==5))
            {
                if(MessageBox("Are you sure you want to delete","this save?","",0)==0) action=13;  //delete slot with no preview
            }
            //else if((Inp.held[INP_BUTTON_R]==1)&&(action==12)) action=3;  // preview slot mode
            else if((Inp.held[INP_BUTTON_MENU_SELECT]==1)&&(mode==1)&&(action==12)) action=8;  //load slot with no preview
            else if((Inp.held[INP_BUTTON_MENU_SELECT]==1)&&(mode==0)&&(action==12)) action=6;  //save slot with no preview
            else if((Inp.held[INP_BUTTON_MENU_SELECT]==1)&&(mode==2)&&(action==12))
            {
                if(MessageBox("Are you sure you want to delete","this save?","",0)==0) action=13;  //delete slot with no preview
            }
            
            PrintTile(currFB);
            PrintTitle(currFB);
            if(mode==SAVESTATE_MODE_SAVE) gp_drawString(6,35,10,"Save State",(unsigned short)RGB(31,0,0),(unsigned short*)vrambuffer); 
            if(mode==SAVESTATE_MODE_LOAD) gp_drawString(6,35,10,"Load State",(unsigned short)RGB(31,0,0),(unsigned short*)vrambuffer); 
            if(mode==SAVESTATE_MODE_DELETE) gp_drawString(6,35,12,"Delete State",(unsigned short)RGB(31,0,0),(unsigned short*)vrambuffer); 
            sprintf(text,"Press UP and DOWN to change save slot");
            gp_drawString(12,230,strlen(text),text,(unsigned short)RGB(31,15,5),(unsigned short*)vrambuffer);
            
            if(saveno==-1) 
            {
                if(action!=10&&action!=0) 
                {
                    action=10;
                }
            }
            else
            {
                PrintBar(currFB,60-4);
                sprintf(text,"SLOT %d",saveno);
                gp_drawString(136,60,strlen(text),text,(unsigned short)RGB(31,31,31),(unsigned short*)vrambuffer);
            }
            
            switch(action)
            {
                case 1:
                    //gp_drawString(112,145,14,"Checking....",(unsigned short)RGB(31,31,31),(unsigned short*)vrambuffer);
                    break;
                case 2:
                    gp_drawString(144,145,4,"FREE",(unsigned short)RGB(31,31,31),(unsigned short*)vrambuffer);
                    break;
                case 3:
                    gp_drawString(104,145,14,"Previewing....",(unsigned short)RGB(31,31,31),(unsigned short*)vrambuffer);
                    break;
                case 4:
                    gp_drawString(88,145,18,"Previewing....fail",(unsigned short)RGB(31,31,31),(unsigned short*)vrambuffer);
                    break;
                case 5: 
                    gp_drawString(112,145,17, "Not gonna happen!",(unsigned short)RGB(31,31,31),(unsigned short*)vrambuffer);
                    if(mode==1) gp_drawString((320-(strlen(MENU_TEXT_LOAD_SAVESTATE)<<3))>>1,210,strlen(MENU_TEXT_LOAD_SAVESTATE), MENU_TEXT_LOAD_SAVESTATE,(unsigned short)RGB(31,31,31),(unsigned short*)vrambuffer);
                    else if(mode==0) gp_drawString((320-(strlen(MENU_TEXT_OVERWRITE_SAVESTATE)<<3))>>1,210,strlen(MENU_TEXT_OVERWRITE_SAVESTATE), MENU_TEXT_OVERWRITE_SAVESTATE,(unsigned short)RGB(31,31,31),(unsigned short*)vrambuffer);
                    else if(mode==2) gp_drawString((320-(strlen(MENU_TEXT_DELETE_SAVESTATE)<<3))>>1,210,strlen(MENU_TEXT_DELETE_SAVESTATE), MENU_TEXT_DELETE_SAVESTATE,(unsigned short)RGB(31,31,31),(unsigned short*)vrambuffer);
                    break;
                case 6:
                    gp_drawString(124,145,9,"Saving...",(unsigned short)RGB(31,31,31),(unsigned short*)vrambuffer);
                    break;
                case 7:
                    gp_drawString(124,145,14,"Saving...Fail!",(unsigned short)RGB(31,31,31),(unsigned short*)vrambuffer);
                    break;
                case 8:
                    gp_drawString(116,145,11,"loading....",(unsigned short)RGB(31,31,31),(unsigned short*)vrambuffer);
                    break;
				case 9:
                    gp_drawString(116,145,15,"loading....Fail",(unsigned short)RGB(31,31,31),(unsigned short*)vrambuffer);
                    break;
                case 10:	
                    PrintBar(currFB,145-4);
                    gp_drawString(104,145,14,"Return To Menu",(unsigned short)RGB(31,31,31),(unsigned short*)vrambuffer);
                    break;
                case 12:
                    gp_drawString(124,145,9,"Slot used",(unsigned short)RGB(31,31,31),(unsigned short*)vrambuffer);
                    //gp_drawString((320-(strlen(MENU_TEXT_PREVIEW_SAVESTATE)<<3))>>1,165,strlen(MENU_TEXT_PREVIEW_SAVESTATE),MENU_TEXT_PREVIEW_SAVESTATE,(unsigned short)RGB(31,31,31),(unsigned short*)vrambuffer);
                    if(mode==1) gp_drawString((320-(strlen(MENU_TEXT_LOAD_SAVESTATE)<<3))>>1,175,strlen(MENU_TEXT_LOAD_SAVESTATE), MENU_TEXT_LOAD_SAVESTATE,(unsigned short)RGB(31,31,31),(unsigned short*)vrambuffer);
                    else if(mode==0) gp_drawString((320-(strlen(MENU_TEXT_OVERWRITE_SAVESTATE)<<3))>>1,175,strlen(MENU_TEXT_OVERWRITE_SAVESTATE), MENU_TEXT_OVERWRITE_SAVESTATE,(unsigned short)RGB(31,31,31),(unsigned short*)vrambuffer);
                    else if(mode==2) gp_drawString((320-(strlen(MENU_TEXT_DELETE_SAVESTATE)<<3))>>1,175,strlen(MENU_TEXT_DELETE_SAVESTATE), MENU_TEXT_DELETE_SAVESTATE,(unsigned short)RGB(31,31,31),(unsigned short*)vrambuffer);
                    break;
                case 13:
                    gp_drawString(116,145,11,"Deleting....",(unsigned short)RGB(31,31,31),(unsigned short*)vrambuffer);
                    break;
            }
            
            MenuFlip();
            
            switch(action)
            {
                case 1:
                    if(saveState[saveno].inUse) 
                    {
                        action=12;
                    }
                    else 
                    {
                        action=2;
                    }
                    break;
                case 3:
                    LoadStateFile(saveState[saveno].fullFilename);
                    action=5;
                    break;
                case 6:
                    SaveStateFile(saveState[saveno].fullFilename);
                    saveState[saveno].inUse=1;
                    action=1;
                    break;
                case 7:
                    action=1;
                    break;
                case 8:
                    LoadStateFile(saveState[saveno].fullFilename);
                    action=100;  // loaded ok so exit
                    break;
                case 9:
                    action=1;
                    break;
                case 11:
                    action=1;
                    break;
                case 13:
                    remove(saveState[saveno].fullFilename);
                    sync();
                    saveState[saveno].inUse = 0;
                    action=1;
                    break;
            }
        }
        memset(&headerDone,0,sizeof(headerDone));
        return(action);
    }
    
    static
    void RenderMenu(char *menuName, int menuCount, int menuSmooth, int menufocus)
    {
        
        int i=0;
        char text[50];
        unsigned short color=0;
        PrintTile(currFB);
        PrintTitle(currFB);
        
        gp_drawString(6,35,strlen(menuName),menuName,(unsigned short)RGB(31,0,0),(unsigned short*)vrambuffer); 
        
        // RRRRRGGGGGBBBBBI  gp32 color format
        for (i=0;i<menuCount;i++)
        {
            int x=0,y=0;
            
            y=(i<<4)-(menuSmooth>>4);
            x=8;
            y+=112;
            
            if (y<=48 || y>=232) continue;
            
            if (i==menufocus)
            {
                color=(unsigned short)RGB(0,0,0);
                PrintBar(currFB,y-4);
            }
            else
            {
                color=(unsigned short)RGB(31,31,31);
            }
            
            sprintf(text,"%s",menutext[i]);
            gp_drawString(x,y,strlen(text),text,color,(unsigned short*)vrambuffer);
        }
        
    }
    
    static
    int LoadRomMenu(void)
    {
        int menuExit=0,menuCount=LOAD_ROM_MENU_COUNT,menufocus=0,menuSmooth=0;
        int action=0;
        int subaction=0;
        
        memset(&headerDone,0,sizeof(headerDone));
        strcpy(romDir,snesRomDir);
        subaction=FileSelect(0);
        memset(&headerDone,0,sizeof(headerDone));
        if(subaction)
        {
            action=EVENT_LOAD_SNES_ROM;
            menuExit=1;
        }
        
        return action;
    }
    
    static
    int SaveStateMenu(void)
    {
        int menuExit=0,menuCount=SAVESTATE_MENU_COUNT,menufocus=0,menuSmooth=0;
        int action=0;
        int subaction=0;
        
        memset(&headerDone,0,sizeof(headerDone));
        
        //Update
        sprintf(menutext[SAVESTATE_MENU_LOAD],"Load State");
        sprintf(menutext[SAVESTATE_MENU_SAVE],"Save State");
        sprintf(menutext[SAVESTATE_MENU_DELETE],"Delete State");
        sprintf(menutext[SAVESTATE_MENU_RETURN],"Back");
        
        while (!menuExit)
        {
            InputUpdate(0);
            
            // Change which rom is focused on:
            if (Inp.repeat[INP_BUTTON_UP]) menufocus--; // Up
            if (Inp.repeat[INP_BUTTON_DOWN]) menufocus++; // Down
            
            if (Inp.held[INP_BUTTON_MENU_CANCEL]==1 ) menuExit=1;
            
            if (menufocus>menuCount-1)
            {
                menufocus=0;
                menuSmooth=(menufocus<<8)-1;
            }   
            else if (menufocus<0) 
            {
                menufocus=menuCount-1;
                menuSmooth=(menufocus<<8)-1;
            }
            
            if (Inp.held[INP_BUTTON_MENU_SELECT]==1)
            {
                switch(menufocus)
                {
                    case SAVESTATE_MENU_LOAD:
                        subaction=SaveStateSelect(SAVESTATE_MODE_LOAD);
                        if(subaction==100)
                        {
                            menuExit=1;
                            action=100;
                        }
                        break;
                    case SAVESTATE_MENU_SAVE:
                        SaveStateSelect(SAVESTATE_MODE_SAVE);
                        break;
                    case SAVESTATE_MENU_DELETE:
                        SaveStateSelect(SAVESTATE_MODE_DELETE);
                        break;
                    case SAVESTATE_MENU_RETURN:
                        menuExit=1;
                        break;
                }	
            }
            // Draw screen:
            menuSmooth=menuSmooth*7+(menufocus<<8); menuSmooth>>=3;
            RenderMenu("Save States", menuCount,menuSmooth,menufocus);
            MenuFlip();
            
        }
        
        return action;
    }
    
    static
    int SramMenu(void)
    {
        int menuExit=0,menuCount=SRAM_MENU_COUNT,menufocus=0,menuSmooth=0;
        int action=0;
        int subaction=0;
        char *srammem=NULL;
        
        
        memset(&headerDone,0,sizeof(headerDone));
        
        //Update
        sprintf(menutext[SRAM_MENU_LOAD],"Load SRAM");
        sprintf(menutext[SRAM_MENU_SAVE],"Save SRAM");
        sprintf(menutext[SRAM_MENU_DELETE],"Delete SRAM");
        sprintf(menutext[SRAM_MENU_RETURN],"Back");
        
        while (!menuExit)
        {
            InputUpdate(0);
            
            // Change which rom is focused on:
            if (Inp.repeat[INP_BUTTON_UP]) menufocus--; // Up
            if (Inp.repeat[INP_BUTTON_DOWN]) menufocus++; // Down
            
            if (Inp.held[INP_BUTTON_MENU_CANCEL]==1 ) menuExit=1;
            
            if (menufocus>menuCount-1)
            {
                menufocus=0;
                menuSmooth=(menufocus<<8)-1;
            }   
            else if (menufocus<0) 
            {
                menufocus=menuCount-1;
                menuSmooth=(menufocus<<8)-1;
            }
            
            if (Inp.held[INP_BUTTON_MENU_SELECT]==1)
            {
                switch(menufocus)
                {
                    case SRAM_MENU_LOAD:
                        //LoadSram(snesSramDir,currentRomFilename,SRAM_FILE_EXT,(char*)&sram);
                        break;
                    case SRAM_MENU_SAVE:
                        //SaveSram(snesSramDir,currentRomFilename,SRAM_FILE_EXT,(char*)&sram);
                        break;
                    case SRAM_MENU_DELETE:
                        //DeleteSram(snesSramDir,currentRomFilename,SRAM_FILE_EXT);
                        break;
                    case SRAM_MENU_RETURN:
                        menuExit=1;
                        break;
                }	
            }
            // Draw screen:
            menuSmooth=menuSmooth*7+(menufocus<<8); menuSmooth>>=3;
            RenderMenu("SRAM", menuCount,menuSmooth,menufocus);
            MenuFlip();
            
        }
        
        return action;
    }
    
    static 
    void SNESOptionsUpdateText(int menu_index)
    {
        switch(menu_index)
        {
            case SNES_MENU_SOUND:
                switch(snesMenuOptions.soundOn)
            {
                case 0:
                    sprintf(menutext[SNES_MENU_SOUND],"Sound: OFF");
                    break;
                case 1:
                    sprintf(menutext[SNES_MENU_SOUND],"Sound: ON");
                    break;
            }
                break;
                
            case SNES_MENU_SOUND_RATE:
                sprintf(menutext[SNES_MENU_SOUND_RATE],"Sound Rate: %d",(unsigned int)soundRates[snesMenuOptions.soundRate]);
                break;
                
            case SNES_MENU_CPUSPEED:
                sprintf(menutext[SNES_MENU_CPUSPEED],"Cpu Speed: %d",(unsigned int)cpuSpeedLookup[snesMenuOptions.cpuSpeed]);
                break;
                
            case SNES_MENU_SOUND_VOL:
                sprintf(menutext[SNES_MENU_SOUND_VOL],"Volume: %d",snesMenuOptions.volume);
                break;
                
            case SNES_MENU_FRAMESKIP:
                switch(snesMenuOptions.frameSkip)
			{
				case 0:
					sprintf(menutext[SNES_MENU_FRAMESKIP],"Frameskip: AUTO");
					break;
				default:
					sprintf(menutext[SNES_MENU_FRAMESKIP],"Frameskip: %d",snesMenuOptions.frameSkip-1);
					break;
			}
                break;
                
            case SNES_MENU_FPS:
                switch(snesMenuOptions.showFps)
			{
				case 0:
					sprintf(menutext[SNES_MENU_FPS],"Show FPS: OFF");
					break;
				case 1:
					sprintf(menutext[SNES_MENU_FPS],"Show FPS: ON");
					break;
			}
                break;
                
            case SNES_MENU_GAMMA:
                sprintf(menutext[SNES_MENU_GAMMA],"Brightness: %d",snesMenuOptions.gamma+100);
                break;
                
            case SNES_MENU_TRANSPARENCY:
                switch(snesMenuOptions.transparency)
			{
				case 0:
					sprintf(menutext[SNES_MENU_TRANSPARENCY],"Transparencies: OFF");
					break;
				case 1:
					sprintf(menutext[SNES_MENU_TRANSPARENCY],"Transparencies: ON");
					break;
			}
                break;
                
            case SNES_MENU_LOAD_GLOBAL:
                sprintf(menutext[SNES_MENU_LOAD_GLOBAL],"Load Global Settings");
                break;
                
            case SNES_MENU_SAVE_GLOBAL:
                sprintf(menutext[SNES_MENU_SAVE_GLOBAL],"Save Global Settings");
                break;
                
            case SNES_MENU_DELETE_GLOBAL:
                sprintf(menutext[SNES_MENU_DELETE_GLOBAL],"Delete Global Settings");
                break;
                
            case SNES_MENU_LOAD_CURRENT:
                sprintf(menutext[SNES_MENU_LOAD_CURRENT],"Load Settings For Current Game");
                break;
                
            case SNES_MENU_SAVE_CURRENT:
                sprintf(menutext[SNES_MENU_SAVE_CURRENT],"Save Settings For Current Game");
                break;
                
            case SNES_MENU_DELETE_CURRENT:
                sprintf(menutext[SNES_MENU_DELETE_CURRENT],"Delete Settings For Current Game");
                break;
                
            case SNES_MENU_SET_ROMDIR:
                sprintf(menutext[SNES_MENU_SET_ROMDIR],"Save Current Rom Directory");
                break;
                
            case SNES_MENU_RETURN:
                sprintf(menutext[SNES_MENU_RETURN],"Back");
                break;
                
            case SNES_MENU_RENDER_MODE:
                switch(snesMenuOptions.renderMode)
			{
				case RENDER_MODE_UNSCALED:
					sprintf(menutext[SNES_MENU_RENDER_MODE],"Render Mode: Unscaled");
					break;
				case RENDER_MODE_SCALED:
					sprintf(menutext[SNES_MENU_RENDER_MODE],"Render Mode: Scaled");
					break;
				default:
					sprintf(menutext[SNES_MENU_RENDER_MODE],"Render Mode: Unscaled");
					break;
			}
                break;
                
            case SNES_MENU_ACTION_BUTTONS:
                switch(snesMenuOptions.actionButtons)
			{
				case 0:
					sprintf(menutext[SNES_MENU_ACTION_BUTTONS],"Action Buttons: Normal");
					break;
				case 1:
					sprintf(menutext[SNES_MENU_ACTION_BUTTONS],"Action Buttons: Swapped");
					break;
			}
                break;
            case SNES_MENU_AUTO_SAVE_SRAM:
                switch(snesMenuOptions.autoSram)
			{
				case 0:
					sprintf(menutext[SNES_MENU_AUTO_SAVE_SRAM],"Saving SRAM: Manual");
					break;
				case 1:
					sprintf(menutext[SNES_MENU_AUTO_SAVE_SRAM],"Saving SRAM: Automatic");
					break;
			}
                break;
            case SNES_MENU_RAM_SETTINGS:
                switch(snesMenuOptions.ramSettings)
			{
				case 0:
					sprintf(menutext[SNES_MENU_RAM_SETTINGS],"RAM timing (Restart Required): NORMAL");
					break;
				case 1:
					sprintf(menutext[SNES_MENU_RAM_SETTINGS],"RAM timing (Restart Required): CRAIG");
					break;
			}
                break;
            case SNES_MENU_MMU_HACK:
                switch(snesMenuOptions.mmuHack)
			{
				case 0:
					sprintf(menutext[SNES_MENU_MMU_HACK],"MMU Hack (Restart Required): OFF");
					break;
				case 1:
					sprintf(menutext[SNES_MENU_MMU_HACK],"MMU Hack (Restart Required): ON");
					break;
			}
                break;
        }
    }
    
    static
    void SNESOptionsUpdateText_All()
    {
        SNESOptionsUpdateText(SNES_MENU_SOUND);
        SNESOptionsUpdateText(SNES_MENU_SOUND_RATE);
        SNESOptionsUpdateText(SNES_MENU_CPUSPEED);
        SNESOptionsUpdateText(SNES_MENU_SOUND_VOL);
        SNESOptionsUpdateText(SNES_MENU_FRAMESKIP);
        SNESOptionsUpdateText(SNES_MENU_FPS);
        SNESOptionsUpdateText(SNES_MENU_GAMMA);
        SNESOptionsUpdateText(SNES_MENU_TRANSPARENCY);
        SNESOptionsUpdateText(SNES_MENU_LOAD_GLOBAL);
        SNESOptionsUpdateText(SNES_MENU_SAVE_GLOBAL);
        SNESOptionsUpdateText(SNES_MENU_DELETE_GLOBAL);
        SNESOptionsUpdateText(SNES_MENU_LOAD_CURRENT);
        SNESOptionsUpdateText(SNES_MENU_SAVE_CURRENT);
        SNESOptionsUpdateText(SNES_MENU_DELETE_CURRENT);
        SNESOptionsUpdateText(SNES_MENU_SET_ROMDIR);
        SNESOptionsUpdateText(SNES_MENU_RETURN);
        SNESOptionsUpdateText(SNES_MENU_ACTION_BUTTONS);
        SNESOptionsUpdateText(SNES_MENU_RENDER_MODE);
        SNESOptionsUpdateText(SNES_MENU_AUTO_SAVE_SRAM);
        SNESOptionsUpdateText(SNES_MENU_RAM_SETTINGS);
        SNESOptionsUpdateText(SNES_MENU_MMU_HACK);
    }
    
    static
    int SNESOptionsMenu(void)
    {
        int menuExit=0,menuCount=SNES_MENU_COUNT,menufocus=0,menuSmooth=0;
        int action=0;
        int subaction=0;
        
        memset(&headerDone,0,sizeof(headerDone));
        
        //Update all items
        SNESOptionsUpdateText_All();
        
        while (!menuExit)
        {
            InputUpdate(0);
            
            // Change which rom is focused on:
            if (Inp.repeat[INP_BUTTON_UP]) menufocus--; // Up
            if (Inp.repeat[INP_BUTTON_DOWN]) menufocus++; // Down
            
            if (Inp.held[INP_BUTTON_MENU_CANCEL]==1 ) menuExit=1;
            
            if (menufocus>menuCount-1)
            {
                menufocus=0;
                menuSmooth=(menufocus<<8)-1;
            }   
            else if (menufocus<0) 
            {
                menufocus=menuCount-1;
                menuSmooth=(menufocus<<8)-1;
            }
            
            if (Inp.held[INP_BUTTON_LEFT]==1||
                Inp.held[INP_BUTTON_RIGHT]==1||
                Inp.repeat[INP_BUTTON_LEFT]||
                Inp.repeat[INP_BUTTON_RIGHT])
            {
                switch(menufocus)
                {
                    case SNES_MENU_SOUND:
                        snesMenuOptions.soundOn^=1;
                        SNESOptionsUpdateText(SNES_MENU_SOUND);
                        break;
                    case SNES_MENU_SOUND_RATE:
                        if (Inp.held[INP_BUTTON_RIGHT]==1||Inp.repeat[INP_BUTTON_RIGHT])
                        {
                            snesMenuOptions.soundRate++;
                            if(snesMenuOptions.soundRate>4) snesMenuOptions.soundRate=0;
                        }
                        else
                        {
                            snesMenuOptions.soundRate--;
                            if(snesMenuOptions.soundRate>4) snesMenuOptions.soundRate=4;
                        }
                        SNESOptionsUpdateText(SNES_MENU_SOUND_RATE);
                        break;
                    case SNES_MENU_SOUND_VOL:
                        if (Inp.held[INP_BUTTON_RIGHT]==1||Inp.repeat[INP_BUTTON_RIGHT])
                        {
                            snesMenuOptions.volume+=1;
                            if(snesMenuOptions.volume>100) snesMenuOptions.volume=0;
                        }
                        else
                        {
                            snesMenuOptions.volume-=1;
                            if(snesMenuOptions.volume>100) snesMenuOptions.volume=100;
                        }
                        SNESOptionsUpdateText(SNES_MENU_SOUND_VOL);
                        break;
                    case SNES_MENU_CPUSPEED:
                        if (Inp.held[INP_BUTTON_RIGHT]==1||Inp.repeat[INP_BUTTON_RIGHT])
                        {
                            snesMenuOptions.cpuSpeed++;
                            if(snesMenuOptions.cpuSpeed>40) snesMenuOptions.cpuSpeed=0;
                        }
                        else
                        {
                            snesMenuOptions.cpuSpeed--;
                            if(snesMenuOptions.cpuSpeed>40) snesMenuOptions.cpuSpeed=0;
                        }
                        SNESOptionsUpdateText(SNES_MENU_CPUSPEED);
                        break;
                    case SNES_MENU_FRAMESKIP:
                        if (Inp.held[INP_BUTTON_RIGHT]==1||Inp.repeat[INP_BUTTON_RIGHT])
                        {
                            snesMenuOptions.frameSkip++;
                            if(snesMenuOptions.frameSkip>6) snesMenuOptions.frameSkip=0;
                        }
                        else
                        {
                            snesMenuOptions.frameSkip--;
                            if(snesMenuOptions.frameSkip>6) snesMenuOptions.frameSkip=6;
                        }
                        SNESOptionsUpdateText(SNES_MENU_FRAMESKIP);
                        break;
                    case SNES_MENU_FPS:
                        snesMenuOptions.showFps^=1;
                        SNESOptionsUpdateText(SNES_MENU_FPS);
                        break;
                    case SNES_MENU_GAMMA:
                        if (Inp.held[INP_BUTTON_RIGHT]==1||Inp.repeat[INP_BUTTON_RIGHT])
                        {
                            snesMenuOptions.gamma++;
                            if(snesMenuOptions.gamma>100) snesMenuOptions.gamma=100;
                        }
                        else
                        {
                            snesMenuOptions.gamma--;
                            if(snesMenuOptions.gamma<-100) snesMenuOptions.gamma=-100;
                        }
                        set_gamma(snesMenuOptions.gamma+100);
                        SNESOptionsUpdateText(SNES_MENU_GAMMA);
                        break;
                    case SNES_MENU_ACTION_BUTTONS:
                        snesMenuOptions.actionButtons^=1;
                        SNESOptionsUpdateText(SNES_MENU_ACTION_BUTTONS);
                        break;
                    case SNES_MENU_TRANSPARENCY:
                        snesMenuOptions.transparency^=1;
                        SNESOptionsUpdateText(SNES_MENU_TRANSPARENCY);
                        break;
                    case SNES_MENU_RENDER_MODE:
                        snesMenuOptions.renderMode^=1;
                        SNESOptionsUpdateText(SNES_MENU_RENDER_MODE);
                        break;
                    case SNES_MENU_AUTO_SAVE_SRAM:
                        snesMenuOptions.autoSram^=1;
                        SNESOptionsUpdateText(SNES_MENU_AUTO_SAVE_SRAM);
                        break;
                    case SNES_MENU_RAM_SETTINGS:
                        snesMenuOptions.ramSettings^=1;
                        SNESOptionsUpdateText(SNES_MENU_RAM_SETTINGS);
                        break;
                    case SNES_MENU_MMU_HACK:
                        snesMenuOptions.mmuHack^=1;
                        SNESOptionsUpdateText(SNES_MENU_MMU_HACK);
                        break;
                }
            }
            if (Inp.held[INP_BUTTON_MENU_SELECT]==1)
            {
                switch(menufocus)
                {
                    case SNES_MENU_LOAD_GLOBAL:
                        LoadMenuOptions(snesOptionsDir, MENU_OPTIONS_FILENAME, MENU_OPTIONS_EXT, (char*)&snesMenuOptions, sizeof(snesMenuOptions),1);
                        SNESOptionsUpdateText_All();
                        break;
                    case SNES_MENU_SAVE_GLOBAL:
                        SaveMenuOptions(snesOptionsDir, MENU_OPTIONS_FILENAME, MENU_OPTIONS_EXT, (char*)&snesMenuOptions, sizeof(snesMenuOptions),1);
                        break;
                    case SNES_MENU_DELETE_GLOBAL:
                        DeleteMenuOptions(snesOptionsDir,MENU_OPTIONS_FILENAME,MENU_OPTIONS_EXT,1);
                        break;
                    case SNES_MENU_LOAD_CURRENT:
                        if(currentRomFilename[0]!=0)
                        {
                            LoadMenuOptions(snesOptionsDir, currentRomFilename, MENU_OPTIONS_EXT, (char*)&snesMenuOptions, sizeof(snesMenuOptions),1);
                            SNESOptionsUpdateText_All();
                        }
                        break;
                    case SNES_MENU_SAVE_CURRENT:
                        if(currentRomFilename[0]!=0)
                        {
                            SaveMenuOptions(snesOptionsDir, currentRomFilename, MENU_OPTIONS_EXT, (char*)&snesMenuOptions, sizeof(snesMenuOptions),1);
                        }
                        break;
                    case SNES_MENU_DELETE_CURRENT:
                        if(currentRomFilename[0]!=0)
                        {
                            DeleteMenuOptions(snesOptionsDir, currentRomFilename, MENU_OPTIONS_EXT,1);
                        }
                        break;
                    case SNES_MENU_SET_ROMDIR:
                        SaveMenuOptions(snesOptionsDir, DEFAULT_ROM_DIR_FILENAME, DEFAULT_ROM_DIR_EXT, romDir, strlen(romDir),1);
                        strcpy(snesRomDir,romDir);
                        break;
                    case SNES_MENU_RETURN:
                        menuExit=1;
                        break;
                }	
            }
            // Draw screen:
            menuSmooth=menuSmooth*7+(menufocus<<8); menuSmooth>>=3;
            RenderMenu("SNES Options", menuCount,menuSmooth,menufocus);
            MenuFlip();
            
        }
        
        return action;
    }
    
    static
    void MainMenuUpdateText(void)
    {
        sprintf(menutext[MAIN_MENU_ROM_SELECT],"Select Rom");
        sprintf(menutext[MAIN_MENU_MANAGE_SAVE_STATE],"Manage Save States");
        sprintf(menutext[MAIN_MENU_SAVE_SRAM],"Save SRAM");
        sprintf(menutext[MAIN_MENU_SNES_OPTIONS],"SNES Options");
        sprintf(menutext[MAIN_MENU_RESET_GAME	],"Reset Game");
        sprintf(menutext[MAIN_MENU_EXIT_APP],"Exit Application");
        sprintf(menutext[MAIN_MENU_RETURN],"Return To Game");
    }
    
    
    int MainMenu(int prevaction)
    {
        int menuExit=0,menuCount=MAIN_MENU_COUNT,menufocus=0,menuSmooth=0;
        int action=prevaction;
        int subaction=0;
        
        gp_setCpuspeed(MENU_CPU_SPEED);
        
        gp_initGraphics(16,currFB,snesMenuOptions.mmuHack);
        gp_clearFramebuffer16((unsigned short*)vrambuffer,0x0);
        MenuFlip();
        gp2x_video_RGB_setscaling(320,240);
        
        memset(&headerDone,0,sizeof(headerDone));
        MainMenuUpdateText();
        
        while (!menuExit)
        {
            InputUpdate(0);
            
            // Change which rom is focused on:
            if (Inp.repeat[INP_BUTTON_UP]) menufocus--; // Up
            if (Inp.repeat[INP_BUTTON_DOWN]) menufocus++; // Down
            
            if (Inp.held[INP_BUTTON_MENU_CANCEL]==1 ) 
            {
                if(currentRomFilename[0]!=0)
                {
                    menuExit=1;
                }
            }
            
            if (menufocus>menuCount-1)
            {
                menufocus=0;
                menuSmooth=(menufocus<<8)-1;
            }   
            else if (menufocus<0) 
            {
                menufocus=menuCount-1;
                menuSmooth=(menufocus<<8)-1;
            }
            
            if (Inp.held[INP_BUTTON_MENU_SELECT]==1)
            {
                switch(menufocus)
                {
                    case MAIN_MENU_ROM_SELECT:
                        memset(&headerDone,0,sizeof(headerDone));
                        subaction=LoadRomMenu();
                        memset(&headerDone,0,sizeof(headerDone));
                        if(subaction)
                        {
                            action=subaction;
                            menuExit=1;
                        }
                        MainMenuUpdateText();
                        break;
                        
                    case MAIN_MENU_MANAGE_SAVE_STATE:
                        if(currentRomFilename[0]!=0)
                        {
                            memset(&headerDone,0,sizeof(headerDone));
                            subaction=SaveStateMenu();
                            if (subaction==100)
                            {
                                menuExit=1;
                            }
                            memset(&headerDone,0,sizeof(headerDone));
                        }
                        MainMenuUpdateText();
                        break;
                    case MAIN_MENU_SAVE_SRAM:
#if 0 // TEMP EDIT
                        if(currentRomFilename[0]!=0)
                        {
                            S9xSaveSRAM();
                        }
#endif
                        break;
                    case MAIN_MENU_SNES_OPTIONS:
                        
                        memset(&headerDone,0,sizeof(headerDone));
                        subaction=SNESOptionsMenu();
                        memset(&headerDone,0,sizeof(headerDone));
                        MainMenuUpdateText();
                        break;
                    case MAIN_MENU_RESET_GAME	:
                        if(currentRomFilename[0]!=0)
                        {
                            switch(currentEmuMode)
                            {
                                case EMU_MODE_SNES:
                                    action=EVENT_RESET_SNES_ROM;
                                    menuExit=1;
                                    break;
                            }
                        }
                        break;
                    case MAIN_MENU_RETURN:
                        if(currentRomFilename[0]!=0)
                        {
                            menuExit=1;
                        }
                        break;
                    case MAIN_MENU_EXIT_APP:
                        action=EVENT_EXIT_APP;
                        menuExit=1;
                        break;
                }	
            }
            // Draw screen:
            menuSmooth=menuSmooth*7+(menufocus<<8); menuSmooth>>=3;
            RenderMenu("Main Menu (visit www.zodttd.com)", menuCount,menuSmooth,menufocus);
            MenuFlip();
            
        }
        
        WaitForButtonsUp();
        
        return action;
    }
    
    
    
